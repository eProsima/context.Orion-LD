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
 * @file YamlReaderConfiguration.cpp
 *
 */

#include <cpp_utils/Log.hpp>
#include <cpp_utils/utils.hpp>

#include <ddspipe_core/configuration/DdsPipeLogConfiguration.hpp>
#include <ddspipe_core/types/dynamic_types/types.hpp>
#include <ddspipe_core/types/topic/filter/ManualTopic.hpp>
#include <ddspipe_core/types/topic/filter/WildcardDdsFilterTopic.hpp>
#include <ddspipe_participants/types/address/Address.hpp>

#include <ddspipe_yaml/yaml_configuration_tags.hpp>
#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlManager.hpp>

#include <ddshelper_yaml/yaml_configuration_tags.hpp>
#include <ddshelper_yaml/YamlReaderConfiguration.hpp>

namespace eprosima {
namespace ddshelper {
namespace yaml {

using namespace eprosima::ddspipe::core;
using namespace eprosima::ddspipe::core::types;
using namespace eprosima::ddspipe::participants;
using namespace eprosima::ddspipe::participants::types;
using namespace eprosima::ddspipe::yaml;

HelperConfiguration::HelperConfiguration(
        const Yaml& yml)
{
    load_ddshelper_configuration_(yml);
}

HelperConfiguration::HelperConfiguration(
        const std::string& file_path)
{
    load_ddshelper_configuration_from_file_(file_path);
}

bool HelperConfiguration::is_valid(
        utils::Formatter& error_msg) const noexcept
{
    return true;
}

void HelperConfiguration::load_ddshelper_configuration_(
        const Yaml& yml)
{
    try
    {
        YamlReaderVersion version = LATEST;

        ////////////////////////////////////////
        // Create participants configurations //
        ////////////////////////////////////////

        /////
        // Create Simple Participant Configuration
        simple_configuration = std::make_shared<SimpleParticipantConfiguration>();
        simple_configuration->id = "SimpleHelperParticipant";
        simple_configuration->app_id = "DDS_HELPER";
        simple_configuration->app_metadata = "";
        simple_configuration->is_repeater = false;

        /////
        // Create Helper Participant Configuration
        helper_configuration = std::make_shared<ParticipantConfiguration>();
        helper_configuration->id = "HelperHelperParticipant";
        helper_configuration->app_id = "DDS_HELPER";
        // TODO: fill metadata field once its content has been defined.
        helper_configuration->app_metadata = "";
        helper_configuration->is_repeater = false;

        /////
        // Get optional Helper configuration options
        if (YamlReader::is_tag_present(yml, HELPER_HELPER_TAG))
        {
            auto helper_yml = YamlReader::get_value_in_tag(yml, HELPER_HELPER_TAG);
            load_helper_configuration_(helper_yml, version);
        }

        /////
        // Get optional specs configuration
        // WARNING: Parse builtin topics (dds tag) AFTER specs, as some topic-specific default values are set there
        if (YamlReader::is_tag_present(yml, SPECS_TAG))
        {
            auto specs_yml = YamlReader::get_value_in_tag(yml, SPECS_TAG);
            load_specs_configuration_(specs_yml, version);
        }

        /////
        // Get optional DDS configuration options
        if (YamlReader::is_tag_present(yml, HELPER_DDS_TAG))
        {
            auto dds_yml = YamlReader::get_value_in_tag(yml, HELPER_DDS_TAG);
            load_dds_configuration_(dds_yml, version);
        }

        // Block ROS 2 services (RPC) topics
        // RATIONALE:
        // At the time of this writing, services in ROS 2 behave in the following manner: a ROS 2 service
        // client awaits to discover a server, and it is then when a request is sent to this (and only this) server,
        // from which a response is expected.
        // Hence, if these topics are not blocked, the client would wrongly believe DDS-Helper is a server, thus
        // sending a request for which a response will not be received.
        WildcardDdsFilterTopic rpc_request_topic, rpc_response_topic;
        rpc_request_topic.topic_name.set_value("rq/*");
        rpc_response_topic.topic_name.set_value("rr/*");

        ddspipe_configuration.blocklist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(rpc_request_topic));

        ddspipe_configuration.blocklist.insert(
            utils::Heritable<WildcardDdsFilterTopic>::make_heritable(rpc_response_topic));

        ddspipe_configuration.init_enabled = true;

        // Only trigger the DdsPipe's callbacks when discovering or removing writers
        ddspipe_configuration.discovery_trigger = DiscoveryTrigger::WRITER;
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Helper configuration from yaml:\n " << e.what());
    }

}

void HelperConfiguration::load_helper_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get optional only_with_type
    if (YamlReader::is_tag_present(yml, HELPER_ONLY_WITH_TYPE_TAG))
    {
        only_with_type = YamlReader::get<bool>(yml, HELPER_ONLY_WITH_TYPE_TAG, version);
    }

    // Get event window
    if (YamlReader::is_tag_present(yml, HELPER_EVENT_WINDOW_TAG))
    {
        event_window = YamlReader::get_positive_int(yml, HELPER_EVENT_WINDOW_TAG);
    }
}

void HelperConfiguration::load_specs_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get number of threads
    if (YamlReader::is_tag_present(yml, NUMBER_THREADS_TAG))
    {
        n_threads = YamlReader::get_positive_int(yml, NUMBER_THREADS_TAG);
    }

    // Get max pending samples
    if (YamlReader::is_tag_present(yml, HELPER_SPECS_MAX_PENDING_SAMPLES_TAG))
    {
        max_pending_samples = YamlReader::get<int>(yml, HELPER_SPECS_MAX_PENDING_SAMPLES_TAG, version);
        if (max_pending_samples < -1)
        {
            throw eprosima::utils::ConfigurationException(
                      utils::Formatter() << "Error reading value under tag <" << HELPER_SPECS_MAX_PENDING_SAMPLES_TAG <<
                          "> : value cannot be lower than -1.");
        }
    }

    /////
    // Get optional Log Configuration
    if (YamlReader::is_tag_present(yml, LOG_CONFIGURATION_TAG))
    {
        ddspipe_configuration.log_configuration = YamlReader::get<DdsPipeLogConfiguration>(yml, LOG_CONFIGURATION_TAG,
                        version);
    }
}

void HelperConfiguration::load_dds_configuration_(
        const Yaml& yml,
        const YamlReaderVersion& version)
{
    // Get optional DDS domain
    if (YamlReader::is_tag_present(yml, DOMAIN_ID_TAG))
    {
        simple_configuration->domain = YamlReader::get<DomainId>(yml, DOMAIN_ID_TAG, version);
    }

    /////
    // Get optional whitelist interfaces
    if (YamlReader::is_tag_present(yml, WHITELIST_INTERFACES_TAG))
    {
        simple_configuration->whitelist = YamlReader::get_set<IpType>(yml, WHITELIST_INTERFACES_TAG,
                        version);
    }

    // Optional get Transport protocol
    if (YamlReader::is_tag_present(yml, TRANSPORT_DESCRIPTORS_TRANSPORT_TAG))
    {
        simple_configuration->transport = YamlReader::get<TransportDescriptors>(yml,
                        TRANSPORT_DESCRIPTORS_TRANSPORT_TAG,
                        version);
    }
    else
    {
        simple_configuration->transport = TransportDescriptors::builtin;
    }

    // Optional get ignore participant flags
    if (YamlReader::is_tag_present(yml, IGNORE_PARTICIPANT_FLAGS_TAG))
    {
        simple_configuration->ignore_participant_flags = YamlReader::get<IgnoreParticipantFlags>(yml,
                        IGNORE_PARTICIPANT_FLAGS_TAG,
                        version);
    }
    else
    {
        simple_configuration->ignore_participant_flags = IgnoreParticipantFlags::no_filter;
    }

    /////
    // Get optional allowlist
    if (YamlReader::is_tag_present(yml, ALLOWLIST_TAG))
    {
        ddspipe_configuration.allowlist = YamlReader::get_set<utils::Heritable<IFilterTopic>>(yml, ALLOWLIST_TAG,
                        version);
    }

    /////
    // Get optional blocklist
    if (YamlReader::is_tag_present(yml, BLOCKLIST_TAG))
    {
        ddspipe_configuration.blocklist = YamlReader::get_set<utils::Heritable<IFilterTopic>>(yml, BLOCKLIST_TAG,
                        version);
    }

    /////
    // Get optional topics
    if (YamlReader::is_tag_present(yml, TOPICS_TAG))
    {
        const auto& manual_topics = YamlReader::get_list<ManualTopic>(yml, TOPICS_TAG, version);
        ddspipe_configuration.manual_topics =
                std::vector<ManualTopic>(manual_topics.begin(), manual_topics.end());
    }

    /////
    // Get optional builtin topics
    if (YamlReader::is_tag_present(yml, BUILTIN_TAG))
    {
        // WARNING: Parse builtin topics AFTER specs and helper, as some topic-specific default values are set there
        ddspipe_configuration.builtin_topics = YamlReader::get_set<utils::Heritable<DistributedTopic>>(yml, BUILTIN_TAG,
                        version);
    }
}

void HelperConfiguration::load_ddshelper_configuration_from_file_(
        const std::string& file_path)
{
    Yaml yml;

    // Load file
    try
    {
        if (!file_path.empty())
        {
            yml = YamlManager::load_file(file_path);
        }
    }
    catch (const std::exception& e)
    {
        throw eprosima::utils::ConfigurationException(
                  utils::Formatter() << "Error loading DDS Helper configuration from file: <" << file_path <<
                      "> :\n " << e.what());
    }

    HelperConfiguration::load_ddshelper_configuration_(yml);
}

} /* namespace yaml */
} /* namespace ddshelper */
} /* namespace eprosima */
