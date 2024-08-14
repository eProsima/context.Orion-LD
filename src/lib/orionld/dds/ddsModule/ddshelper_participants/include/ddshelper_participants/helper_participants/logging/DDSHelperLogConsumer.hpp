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
 * @file DDSHelperLogConsumer.hpp
 */

#pragma once

#pragma once

#include <cpp_utils/Log.hpp>
#include <cpp_utils/logging/BaseLogConsumer.hpp>

#include <ddspipe_core/configuration/DdsPipeLogConfiguration.hpp>
#include <ddspipe_core/types/logging/LogEntry.hpp>
#include <ddspipe_core/types/logging/LogEntryPubSubTypes.hpp>
#include <ddspipe_core/types/logging/LogEntryTypeObjectSupport.hpp>

#include <ddspipe_core/configuration/DdsPipeLogConfiguration.hpp>

#include <ddshelper_participants/library/library_dll.h>

namespace eprosima {
namespace ddshelper {
namespace participants {

/**
 * DDS Helper Log Consumer.
 */
class DDSHelperLogConsumer : public utils::BaseLogConsumer
{
public:

    //! Create a new \c DDSHelperLogConsumer from a \c DdsPipeLogConfiguration .
    DDSHELPER_PARTICIPANTS_DllAPI
    DDSHelperLogConsumer(
            const ddspipe::core::DdsPipeLogConfiguration* configuration);

    DDSPIPE_CORE_DllAPI
    ~DDSHelperLogConsumer(){};

    /**
     * @brief Implements \c LogConsumer \c Consume method.
     *
     * The entry's kind must be higher or equal to the verbosity level \c verbosity_ .
     * The entry's content or category must match the \c filter_ regex.
     *
     * This method will publish the \c entry with DDS.
     *
     * @param entry entry to consume
     */
    DDSPIPE_CORE_DllAPI
    void Consume(
            const utils::Log::Entry& entry) override;

};

} /* namespace participants */
} /* namespace ddshelper */
} /* namespace eprosima */
