// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file CBHandler.cpp
 */

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <vector>

#include <yaml-cpp/yaml.h>

#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/exception/InconsistencyException.hpp>
#include <cpp_utils/time/time_utils.hpp>
#include <cpp_utils/utils.hpp>

#include <ddshelper_participants/constants.hpp>

#include <ddshelper_participants/helper_participants/cb_participants/CBHandler.hpp>

namespace eprosima {
namespace ddshelper {
namespace participants {

using namespace eprosima::ddspipe::core::types;

CBHandler::CBHandler(
        const CBHandlerConfiguration& config,
        const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool)
    : configuration_(config)
    , payload_pool_(payload_pool)
{
    logInfo(DDSHELPER_CB_HANDLER, "Creating CB handler instance.");
        
    periodic_cleanup_running_ = true;
    periodic_cleanup_thread_ = std::thread(&CBHandler::remove_outdated_samples, this);
}

CBHandler::~CBHandler()
{
    logInfo(DDSHELPER_CB_HANDLER, "Destroying CB handler.");

    periodic_cleanup_running_ = false;
    if (periodic_cleanup_thread_.joinable())
    {
        periodic_cleanup_thread_.join();
    }
}

void CBHandler::add_schema(
        const fastdds::dds::DynamicType::_ref_type& dyn_type,
        const std::string& type_name,
        const fastdds::dds::xtypes::TypeIdentifier& type_id)
{
    std::lock_guard<std::mutex> lock(mtx_);

    // NOTE: Process schemas to avoid losing them (only sent/received once in discovery)

    assert(nullptr != dyn_type);

    // Check if it exists already
    auto it = schemas_.find(type_id);
    if (it == schemas_.end())
    {
        return;
    }

    // Add to schemas map
    logInfo(DDSHELPER_CB_HANDLER, "Adding schema with name " << dyn_type->get_name().to_string() << " :\n" << data << ".");

    schemas_[type_id] = dyn_type;

    // Check if there are any pending samples for this new schema. If so, add them.
    if (pending_samples_.find(type_name) != pending_samples_.end())
    {
        write_pending_samples(type_name, dyn_type);
    }
}

void CBHandler::add_data(
        const DdsTopic& topic,
        RtpsPayloadData& data)
{
    std::unique_lock<std::mutex> lock(mtx_);

    logInfo(DDSHELPER_CB_HANDLER, "Adding data in topic: " << topic << ".");

    CBMessage msg;
    msg.sequence_number = unique_sequence_number_++;
    msg.publish_time = data.source_timestamp;
    if (data.payload.length > 0)
    {
        msg.topic = topic;
        msg.instanceHandle = data.instanceHandle;
        msg.source_guid = data.source_guid;

        if (data.payload_owner != nullptr)
        {
            payload_pool_->get_payload(
                data.payload,
                msg.payload);

            msg.payload_owner = payload_pool_.get();
        }
        else
        {
            throw utils::InconsistencyException(STR_ENTRY << "Payload owner not found in data received.");
        }
    }
    else
    {
        throw utils::InconsistencyException(STR_ENTRY << "Received sample with no payload.");
    }

    fastdds::dds::DynamicType::_ref_type dyn_type;
    auto it = schemas_.find(topic.type_ids.type_identifier1());
    if (it != schemas_.end())
    {
        dyn_type = it->second;

        // Schema available -> write
        write_sample(msg, dyn_type);
    }
    else
    {
        if (configuration_.max_pending_samples == 0)
        {
            // No pending samples -> Discard message
            return;
        }
        else
        {
            logInfo(DDSHELPER_CB_HANDLER, "Schema for topic " << topic << " not yet available.ss");

            // Schema not available -> add to pending
            add_pending_sample(msg, topic);
        }
    }
}

void CBHandler::write_sample(
        const CBMessage& msg,
        const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
        cb_writer_.write_data(msg, dyn_type);
}

void CBHandler::add_pending_sample(
        CBMessage& msg,
        const DdsTopic& topic)
{
    assert(configuration_.max_pending_samples != 0);

    logInfo(DDSHELPER_CB_HANDLER, "Adding pending samples for type: " << schema_name << ".");

    if (configuration_.max_pending_samples > 0 &&
            pending_samples_[topic.type_name].size() == static_cast<unsigned int>(configuration_.max_pending_samples))
    {
        pending_samples_[topic.type_name].pop_front();
    }

    pending_samples_[topic.type_name].push_back({topic, msg});
}

void CBHandler::write_pending_samples(
        const std::string& schema_name,
        const fastdds::dds::DynamicType::_ref_type& dyn_type)
{
    if (pending_samples_.find(schema_name) != pending_samples_.end())
    {
        logInfo(DDSHELPER_CB_HANDLER, "Writing pending samples for type: " << schema_name << ".");
    
        while (!pending_samples_[schema_name].empty())
        {
            // Move samples from pending list to buffer, or write them directly to CB
            auto& sample = pending_samples_[schema_name].front();
            write_sample(sample.second, dyn_type);

            pending_samples_[schema_name].pop_front();
        }
        pending_samples_.erase(schema_name);
    }
}

void CBHandler::remove_outdated_samples()
{
    while (periodic_cleanup_running_)
    {
        std::this_thread::sleep_for(std::chrono::seconds(configuration_.event_window));

        logInfo(DDSHELPER_CB_HANDLER, "Removing outdated samples.");

        eprosima::ddspipe::core::types::DataTime now;
        eprosima::ddspipe::core::types::DataTime::now(now);

        eprosima::ddspipe::core::types::DataTime event_window;
        event_window.from_duration_t(eprosima::fastdds::Duration_t(configuration_.event_window));

        eprosima::ddspipe::core::types::DataTime threshold = now - event_window;

        std::lock_guard<std::mutex> lock(mtx_);
        for (auto& pending_sample : pending_samples_)
        {
            pending_sample.second.remove_if([&](auto& sample)
                    {
                        return sample.second.publish_time < threshold;
                    });
        }
    }
}

} /* namespace participants */
} /* namespace ddshelper */
} /* namespace eprosima */
