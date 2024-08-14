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
 * @file yaml_configuration_tags.hpp
 */

#pragma once

#include <set>
#include <string>

namespace eprosima {
namespace ddshelper {
namespace yaml {

constexpr const char* HELPER_DDS_TAG("dds");
constexpr const char* HELPER_HELPER_TAG("ddshelper");
constexpr const char* HELPER_EVENT_WINDOW_TAG("event-window");
constexpr const char* HELPER_ONLY_WITH_TYPE_TAG("only-with-type");
constexpr const char* HELPER_SPECS_MAX_PENDING_SAMPLES_TAG("max-pending-samples");

} /* namespace yaml */
} /* namespace ddshelper */
} /* namespace eprosima */
