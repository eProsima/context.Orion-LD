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
 * @file DDSHelperLogConsumer.cpp
 */

#include <string>
#include <fstream>
#include <string>

#include <cpp_utils/exception/InitializationException.hpp>

#include <ddshelper_participants/common/types/logging/DDSHelperLogEntry.hpp>
#include <ddshelper_participants/helper_participants/logging/DDSHelperLogConsumer.hpp>

namespace eprosima {
namespace ddshelper {
namespace participants {

DDSHelperLogConsumer::DDSHelperLogConsumer(
        const ddspipe::core::DdsPipeLogConfiguration* configuration)
    : utils::BaseLogConsumer(configuration)
{
}

void DDSHelperLogConsumer::Consume(
        const utils::Log::Entry& entry)
{
    if (!accept_entry_(entry))
    {
        return;
    }

    //CALLBACK FOR CB
}

} /* namespace participants */
} /* namespace ddshelper */
} /* namespace eprosima */
