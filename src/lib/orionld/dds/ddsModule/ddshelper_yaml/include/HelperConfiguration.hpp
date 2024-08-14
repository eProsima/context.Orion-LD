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

/**
 * @file HelperConfiguration.hpp
 */

#pragma once

#include <cpp_utils/Formatter.hpp>
#include <cpp_utils/memory/Heritable.hpp>

#include <ddspipe_core/configuration/DdsPipeConfiguration.hpp>
#include <ddspipe_core/configuration/MonitorConfiguration.hpp>
#include <ddspipe_core/types/dds/TopicQoS.hpp>
#include <ddspipe_core/types/topic/dds/DistributedTopic.hpp>
#include <ddspipe_core/types/topic/filter/IFilterTopic.hpp>

#include <ddspipe_participants/configuration/ParticipantConfiguration.hpp>
#include <ddspipe_participants/configuration/SimpleParticipantConfiguration.hpp>

#include <ddspipe_yaml/Yaml.hpp>
#include <ddspipe_yaml/YamlReader.hpp>

#include <ddshelper_yaml/library/library_dll.h>

namespace eprosima {
namespace ddshelper {
namespace yaml {

constexpr const char* DEFAULT_CONFIGURATION_FILE_NAME("/home/adrian/DDS_HELPER_CONFIGURATION.yaml");

constexpr const int DEFAULT_N_THREADS = 8;
constexpr const bool DEFAULT_ONLY_WITH_TYPE = false;
constexpr const int DEFAULT_EVENT_WINDOW = 20;
constexpr const int DEFAULT_MAX_PENDING_SAMPLES = 500;// -1 <-> no limit || 0 <-> no pending samples

/**
 * @brief Class that encapsulates specific methods to get a full DDS Helper Configuration from a yaml node.
 *
 * TODO: Add version configuration so it could load different versions
 */
class DDSHELPER_YAML_DllAPI HelperConfiguration
{
public:

    HelperConfiguration(
            const Yaml& yml);

    HelperConfiguration(
            const std::string& file_path);

    virtual bool is_valid(
            utils::Formatter& error_msg) const noexcept;

    // DDS Pipe Configuration
    ddspipe::core::DdsPipeConfiguration ddspipe_configuration;

    // Participants configurations
    std::shared_ptr<ddspipe::participants::SimpleParticipantConfiguration> simple_configuration;
    std::shared_ptr<ddspipe::participants::ParticipantConfiguration> helper_configuration;

    unsigned int n_threads = DEFAULT_N_THREADS;

    bool only_with_type = DEFAULT_ONLY_WITH_TYPE;
    unsigned int event_window = DEFAULT_EVENT_WINDOW;
    int max_pending_samples = DEFAULT_MAX_PENDING_SAMPLES;
    ddspipe::core::types::TopicQoS topic_qos{};

protected:

    void load_ddshelper_configuration_(
            const Yaml& yml);

    void load_helper_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_specs_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_dds_configuration_(
            const Yaml& yml,
            const ddspipe::yaml::YamlReaderVersion& version);

    void load_ddshelper_configuration_from_file_(
            const std::string& file_path);
};

} /* namespace yaml */
} /* namespace ddshelper */
} /* namespace eprosima */
