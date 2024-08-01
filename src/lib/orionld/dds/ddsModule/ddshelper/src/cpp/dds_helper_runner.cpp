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
 * @file initDDSHelper.cpp
 *
 */

#include <fstream>
#include <string>

#include "fastdds/dds/log/FileConsumer.hpp"

#include "dds_helper_runner.hpp"

using namespace eprosima::ddspipe;

namespace eprosima {
namespace ddshelper {

std::unique_ptr<eprosima::utils::event::FileWatcherHandler> create_filewatcher(
        const std::unique_ptr<DDSHelper>& helper,
        const std::string& file_path)
{
    // Callback will reload configuration and pass it to DdsPipe
    // WARNING: it is needed to pass file_path, as FileWatcher only retrieves file_name
    std::function<void(std::string)> filewatcher_callback =
            [&helper, &file_path]
                (std::string file_name)
            {
                logInfo(DDSHELPER_EXECUTION,
                    "FileWatcher notified changes in file " << file_name << ". Reloading configuration");

                try
                {
                    eprosima::ddshelper::yaml::HelperConfiguration new_configuration(file_path);
                    helper->reload_configuration(new_configuration);
                }
                catch (const std::exception& e)
                {
                    logWarning(DDSHELPER_EXECUTION,
                            "Error reloading configuration file " << file_name << " with error: " <<
                            e.what());
                }
            };

    // Creating FileWatcher event handler
    return std::make_unique<eprosima::utils::event::FileWatcherHandler>(filewatcher_callback, file_path);
}

std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> create_periodic_handler(
        const std::unique_ptr<DDSHelper>& helper,
        const std::string& file_path,
        const eprosima::utils::Duration_ms& reload_time)
{
    // Callback will reload configuration and pass it to DdsPipe
    std::function<void()> periodic_callback =
            [&helper, &file_path]
                ()
            {
                logInfo(DDSHELPER_EXECUTION, "Periodic Timer raised. Reloading configuration from file " << file_path << ".");

                try
                {
                    eprosima::ddshelper::yaml::HelperConfiguration new_configuration(file_path);
                    helper->reload_configuration(new_configuration);
                }
                catch (const std::exception& e)
                {
                    logWarning(DDSHELPER_EXECUTION,
                            "Error reloading configuration file " << file_path << " with error: " <<
                            e.what());
                }
            };

    // Creating periodic handler
    return std::make_unique<eprosima::utils::event::PeriodicEventHandler>(periodic_callback, reload_time);
}

int init_dds_helper()
{
    logInfo(DDSHELPER_EXECUTION, "Starting DDS Helper execution.");

    // Encapsulating execution in block to erase all memory correctly before closing process
    try
    {
        // Load configuration from YAML
        eprosima::ddshelper::yaml::HelperConfiguration configuration(DEFAULT_CONFIGURATION_FILE_NAME);
        
        // Verify that the configuration is correct
        eprosima::utils::Formatter error_msg;
        if (!configuration.is_valid(error_msg))
        {
            throw eprosima::utils::ConfigurationException(
                      eprosima::utils::Formatter() << "Invalid configuration: " << error_msg);
        }

        // Logging
        {
            const auto log_configuration = configuration.ddspipe_configuration.log_configuration;

            eprosima::utils::Log::ClearConsumers();
            eprosima::utils::Log::SetVerbosity(log_configuration.verbosity);

            // DDS Helper Log Consumer
            eprosima::utils::Log::RegisterConsumer(
                std::make_unique<eprosima::ddshelper::participants::DDSHelperLogConsumer>(&log_configuration));


            // Std Log Consumer
            if (log_configuration.stdout_enable)
            {
                eprosima::utils::Log::RegisterConsumer(
                    std::make_unique<eprosima::utils::StdLogConsumer>(&log_configuration));
            }
        }

        // DDS Helper Initialization

        // Create a multiple event handler that handles all events that make the helper stop
        auto close_handler = std::make_shared<eprosima::utils::event::MultipleEventHandler>();

        // Start recording right away
        auto helper = std::make_unique<DDSHelper>(configuration, close_handler);

        logInfo(DDSHELPER_EXECUTION, "DDS Helper running.");

        // // Create File Watcher Handler
        // std::unique_ptr<eprosima::utils::event::FileWatcherHandler> file_watcher_handler;
        // file_watcher_handler = create_filewatcher(helper, DEFAULT_CONFIGURATION_FILE_NAME);

        // // Create Periodic Handler
        // std::unique_ptr<eprosima::utils::event::PeriodicEventHandler> periodic_handler;
        // periodic_handler = create_periodic_handler(helper, DEFAULT_CONFIGURATION_FILE_NAME, 10);

        // Wait until signal arrives
        close_handler->wait_for_event();

        logInfo(DDSHELPER_EXECUTION, "Stopping DDS Helper.");

        logInfo(DDSHELPER_EXECUTION, "DDS Helper stopped correctly.");
    }
    catch (const eprosima::utils::ConfigurationException& e)
    {
        logError(DDSHELPER_ERROR,
                "Error Loading DDS Helper Configuration from file " << DEFAULT_CONFIGURATION_FILE_NAME <<
                ". Error message:\n " << e.what());
        return -1;
    }
    catch (const eprosima::utils::InitializationException& e)
    {
        logError(DDSHELPER_ERROR,
                "Error Initializing DDS Helper. Error message:\n " << e.what());
        return -1;
    }

    logInfo(DDSHELPER_EXECUTION, "Finishing DDS Helper execution correctly.");

    // Force print every log before closing
    eprosima::utils::Log::Flush();

    // Delete the consumers before closing
    eprosima::utils::Log::ClearConsumers();

    return 0;
}

} /* namespace ddshelper */
} /* namespace eprosima */
