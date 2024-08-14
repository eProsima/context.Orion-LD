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

#pragma once

#include <memory>
#include <set>

#include <cpp_utils/event/MultipleEventHandler.hpp>
#include <cpp_utils/ReturnCode.hpp>
#include <cpp_utils/thread_pool/pool/SlotThreadPool.hpp>

#include <ddspipe_core/core/DdsPipe.hpp>
#include <ddspipe_core/dynamic/AllowedTopicList.hpp>
#include <ddspipe_core/dynamic/DiscoveryDatabase.hpp>
#include <ddspipe_core/dynamic/ParticipantsDatabase.hpp>
#include <ddspipe_core/efficiency/payload/FastPayloadPool.hpp>
#include <ddspipe_core/types/topic/dds/DistributedTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/DynTypesParticipant.hpp>
#include <ddspipe_participants/participant/dynamic_types/SchemaParticipant.hpp>

#include <CBHandler.hpp>
#include <CBHandlerConfiguration.hpp>

#include <HelperConfiguration.hpp>

namespace eprosima {
namespace ddshelper {

/**
 * Wrapper class that encapsulates all dependencies required to launch DDS Helper.
 */
class DDSHelper
{
public:

    /**
     * DDSHelper constructor by required values and event handler reference.
     *
     * Creates DDSHelper instance with given configuration.
     *
     * @param configuration: Structure encapsulating all helper configuration options.
     * @param event_handler: Reference to event handler used for thread synchronization in main application.
     */
    DDSHelper(
            const yaml::HelperConfiguration& configuration,
            std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler);

    /**
     * Reconfigure the Helper with the new configuration.
     *
     * @param new_configuration: The configuration to replace the previous configuration with.
     *
     * @return \c RETCODE_OK if allowed topics list has been updated correctly
     * @return \c RETCODE_NO_DATA if new allowed topics list is the same as the previous one
     */
    utils::ReturnCode reload_configuration(
            yaml::HelperConfiguration& new_configuration);

protected:

    /**
     * Load the Helper's internal topics into a configuration object.
     *
     * @param configuration: The configuration to load the internal topics into.
     */
    void load_internal_topics_(
            yaml::HelperConfiguration& configuration);

    //! Configuration of the DDS Helper
    yaml::HelperConfiguration configuration_;

    //! Payload Pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! Thread Pool
    std::shared_ptr<utils::SlotThreadPool> thread_pool_;

    //! Discovery Database
    std::shared_ptr<ddspipe::core::DiscoveryDatabase> discovery_database_;

    //! Participants Database
    std::shared_ptr<ddspipe::core::ParticipantsDatabase> participants_database_;

    //! CB Handler
    std::shared_ptr<eprosima::ddshelper::participants::CBHandler> cb_handler_;

    //! Dynamic Types Participant
    std::shared_ptr<eprosima::ddspipe::participants::DynTypesParticipant> dyn_participant_;

    //! Schema Participant
    std::shared_ptr<eprosima::ddspipe::participants::SchemaParticipant> helper_participant_;

    //! DDS Pipe
    std::unique_ptr<ddspipe::core::DdsPipe> pipe_;

    //! Reference to event handler used for thread synchronization in main application
    std::shared_ptr<eprosima::utils::event::MultipleEventHandler> event_handler_;
};

} /* namespace ddshelper */
} /* namespace eprosima */
