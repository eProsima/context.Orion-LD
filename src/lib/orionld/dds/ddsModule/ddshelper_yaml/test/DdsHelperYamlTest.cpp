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

#include <cpp_utils/testing/gtest_aux.hpp>
#include <gtest/gtest.h>

#include <ddspipe_yaml/YamlReader.hpp>

#include <HelperConfiguration.hpp>

using namespace eprosima;
using namespace eprosima::ddshelper::yaml;

TEST(DdsHelperYamlTest, get_ddshelper_correct_configuration_yaml)
{
    const char* yml_str =
            R"(
            ddshelper:
              only-with-type: false
              event-window: 30

            specs:
              threads: 12
              max-pending-samples: 20
              logging:
                verbosity: info
                filter:
                  error: "DDSHELPER_ERROR"
                  warning: "DDSHELPER_WARNING"
                  info: "DDSHELPER_INFO"
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    HelperConfiguration configuration(yml);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.only_with_type, false);
    ASSERT_EQ(configuration.event_window, 30);
    ASSERT_EQ(configuration.n_threads, 12);
    ASSERT_EQ(configuration.max_pending_samples, 20);

    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.verbosity.get_value(), utils::VerbosityKind::Info);
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Error].get_value(),
            "DDSHELPER_ERROR");
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Warning].get_value(),
            "DDSHELPER_WARNING");
    ASSERT_EQ(configuration.ddspipe_configuration.log_configuration.filter[utils::VerbosityKind::Info].get_value(),
            "DDSHELPER_INFO");
}

TEST(DdsHelperYamlTest, get_ddshelper_incorrect_event_window_configuration_yaml)
{
    const char* yml_str =
            R"(
            ddshelper:
              only-with-type: false
              event-window: error
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EXPECT_THROW({HelperConfiguration configuration(yml);}, std::exception);

    yml_str =
            R"(
            ddshelper:
              only-with-type: false
              event-window: -1
        )";

    yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EXPECT_THROW({HelperConfiguration configuration(yml);}, std::exception);
}

TEST(DdsHelperYamlTest, get_ddshelper_incorrect_only_with_type_configuration_yaml)
{
    const char* yml_str =
            R"(
            ddshelper:
              only-with-type: 1
              event-window: 30
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EXPECT_THROW({HelperConfiguration configuration(yml);}, std::exception);

    yml_str =
            R"(
            ddshelper:
              only-with-type: error
              event-window: 30
        )";

    yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EXPECT_THROW({HelperConfiguration configuration(yml);}, std::exception);
}

TEST(DdsHelperYamlTest, get_ddshelper_incorrect_n_threads_configuration_yaml)
{
    const char* yml_str =
            R"(
            specs:
              threads: error
        )";

    Yaml yml = YAML::Load(yml_str);

    EXPECT_THROW({HelperConfiguration configuration(yml);}, std::exception);

    yml_str =
            R"(
            specs:
              threads: 0
        )";

    yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EXPECT_THROW({HelperConfiguration configuration(yml);}, std::exception);

    yml_str =
            R"(
            specs:
              threads: -1
        )";

    yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EXPECT_THROW({HelperConfiguration configuration(yml);}, std::exception);
}

TEST(DdsHelperYamlTest, get_ddshelper_incorrect_pending_samples_configuration_yaml)
{
    const char* yml_str =
            R"(
            specs:
              max-pending-samples: -2
        )";

    Yaml yml = YAML::Load(yml_str);

    EXPECT_THROW({HelperConfiguration configuration(yml);}, std::exception);

    yml_str =
            R"(
            specs:
              max-pending-samples: error
        )";

    yml = YAML::Load(yml_str);

    // Load configuration from YAML
    EXPECT_THROW({HelperConfiguration configuration(yml);}, std::exception);
}

TEST(DdsHelperYamlTest, get_ddshelper_default_values_configuration_yaml)
{
    const char* yml_str =
            R"(
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    HelperConfiguration configuration(yml);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.only_with_type, DEFAULT_ONLY_WITH_TYPE);
    ASSERT_EQ(configuration.event_window, DEFAULT_EVENT_WINDOW);
    ASSERT_EQ(configuration.n_threads, DEFAULT_N_THREADS);
    ASSERT_EQ(configuration.max_pending_samples, DEFAULT_MAX_PENDING_SAMPLES);
}

TEST(DdsHelperYamlTest, get_ddshelper_no_pending_samples_configuration_yaml)
{
    const char* yml_str =
            R"(
            specs:
              max-pending-samples: 0
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    HelperConfiguration configuration(yml);

    utils::Formatter error_msg;

    ASSERT_EQ(configuration.max_pending_samples, 0);
}

TEST(DdsHelperYamlTest, get_ddshelper_no_limit_pending_samples_configuration_yaml)
{
    const char* yml_str =
            R"(
            specs:
              max-pending-samples: -1
        )";

    Yaml yml = YAML::Load(yml_str);

    // Load configuration from YAML
    HelperConfiguration configuration(yml);

    utils::Formatter error_msg;

    ASSERT_EQ(configuration.max_pending_samples, -1);
}

TEST(DdsHelperYamlTest, get_ddshelper_correct_path_configuration_yaml)
{
    // Load configuration from YAML
    HelperConfiguration configuration(DEFAULT_CONFIGURATION_FILE_NAME);

    utils::Formatter error_msg;

    ASSERT_TRUE(configuration.is_valid(error_msg));
    ASSERT_EQ(configuration.only_with_type, false);
    ASSERT_EQ(configuration.event_window, 30);
    ASSERT_EQ(configuration.n_threads, 12);
    ASSERT_EQ(configuration.max_pending_samples, 20);

    ASSERT_TRUE(configuration.ddspipe_configuration.log_configuration.is_valid(error_msg));
}

TEST(DdsHelperYamlTest, get_ddshelper_incorrect_path_configuration_yaml)
{
    const char* path_str = "incorrect/path/file";

    EXPECT_THROW({HelperConfiguration configuration(path_str);}, eprosima::utils::ConfigurationException);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
