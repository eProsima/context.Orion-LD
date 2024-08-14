// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <filesystem>
#include <math.h>

#include <cpp_utils/exception/InitializationException.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/types/dynamic_types/types.hpp>

#include "DDSHelper.hpp"

namespace eprosima {
namespace ddshelper {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::rtps;
using namespace eprosima::ddshelper::participants;
using namespace eprosima::utils;

DDSHelper::DDSHelper(
        const yaml::HelperConfiguration& configuration,
        std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler)
    : configuration_(configuration)
    , event_handler_(event_handler)
{
    // Load the Helper's internal topics from a configuration object.
    load_internal_topics_(configuration_);

    // Create Discovery Database
    discovery_database_ = std::make_shared<DiscoveryDatabase>();

    // Create Payload Pool
    payload_pool_ = std::make_shared<FastPayloadPool>();

    // Create Thread Pool
    thread_pool_ = std::make_shared<SlotThreadPool>(configuration_.n_threads);

    // Create CB Handler configuration
    participants::CBHandlerConfiguration handler_config(
        configuration_.max_pending_samples,
        configuration_.event_window);

    // Create DynTypes Participant
    dyn_participant_ = std::make_shared<DynTypesParticipant>(
        configuration_.simple_configuration,
        payload_pool_,
        discovery_database_);
    dyn_participant_->init();

    // Create CB Handler
    cb_handler_ = std::make_shared<participants::CBHandler>(
        handler_config,
        payload_pool_);

    // Create Helper Participant
    helper_participant_ = std::make_shared<SchemaParticipant>(
        configuration_.helper_configuration,
        payload_pool_,
        discovery_database_,
        cb_handler_);

    // Create Participant Database
    participants_database_ = std::make_shared<ParticipantsDatabase>();

    // Populate Participant Database
    participants_database_->add_participant(
        dyn_participant_->id(),
        dyn_participant_);

    participants_database_->add_participant(
        helper_participant_->id(),
        helper_participant_);

    // Create DDS Pipe
    pipe_ = std::make_unique<DdsPipe>(
        configuration_.ddspipe_configuration,
        discovery_database_,
        payload_pool_,
        participants_database_,
        thread_pool_);
}

utils::ReturnCode DDSHelper::reload_configuration(
        yaml::HelperConfiguration& new_configuration)
{
    // Load the Helper's internal topics from a configuration object.
    load_internal_topics_(new_configuration);

    // Update the Helper's configuration
    configuration_ = new_configuration;

    return pipe_->reload_configuration(new_configuration.ddspipe_configuration);
}

void DDSHelper::load_internal_topics_(
        yaml::HelperConfiguration& configuration)
{
    // Create an internal topic to transmit the dynamic types
    configuration.ddspipe_configuration.builtin_topics.insert(
        utils::Heritable<DdsTopic>::make_heritable(type_object_topic()));

    if (!configuration.ddspipe_configuration.allowlist.empty())
    {
        // The allowlist is not empty. Add the internal topic.
        WildcardDdsFilterTopic internal_topic;
        internal_topic.topic_name.set_value(TYPE_OBJECT_TOPIC_NAME);

        configuration.ddspipe_configuration.allowlist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(internal_topic));
    }
}

} /* namespace ddshelper */
} /* namespace eprosima */
