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
# Parse Participants Test #
###################

set(TEST_NAME DdsHelperParticipantsTest)

set(TEST_SOURCES
        ${PROJECT_SOURCE_DIR}/test/DdsHelperParticipantsTest.cpp
    )

set(TEST_LIST
    # AAAAA    
    ddshelper_participants_cb_handler_creation
    ddshelper_participants_add_new_schemas
    ddshelper_participants_add_same_type_id_schema
    ddshelper_participants_add_same_type_name_schema
    ddshelper_participants_add_data_with_schema
    ddshelper_participants_add_data_without_schema_no_pending
    ddshelper_participants_add_data_without_schema_with_max_pending
    ddshelper_participants_add_data_without_schema_over_max_pending
    ddshelper_participants_add_schema_with_pending_samples
    ddshelper_participants_write_schema_first_time
    ddshelper_participants_write_schema_repeated
)

set(TEST_EXTRA_LIBRARIES
        fastdds
        ddspipe_core
        ddspipe_participants
        ddshelper_participants
    )

add_unittest_executable(
    "${TEST_NAME}"
    "${TEST_SOURCES}"
    "${TEST_LIST}"
    "${TEST_EXTRA_LIBRARIES}")
