# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###################
# Parse Yaml Test #
###################

set(TEST_NAME DdsHelperYamlTest)

set(TEST_SOURCES
        ${PROJECT_SOURCE_DIR}/test/DdsHelperYamlTest.cpp
    )

set(TEST_LIST
        get_ddshelper_correct_configuration_yaml
        get_ddshelper_incorrect_event_window_configuration_yaml
        get_ddshelper_incorrect_only_with_type_configuration_yaml
        get_ddshelper_incorrect_n_threads_configuration_yaml
        get_ddshelper_incorrect_pending_samples_configuration_yaml
        get_ddshelper_default_values_configuration_yaml
        get_ddshelper_no_pending_samples_configuration_yaml
        get_ddshelper_no_limit_pending_samples_configuration_yaml
        get_ddshelper_correct_path_configuration_yaml
        get_ddshelper_incorrect_path_configuration_yaml
    )

set(TEST_EXTRA_LIBRARIES
        yaml-cpp
        ddspipe_core
        ddspipe_yaml
        ddshelper_yaml
    )

add_unittest_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_EXTRA_LIBRARIES}")
