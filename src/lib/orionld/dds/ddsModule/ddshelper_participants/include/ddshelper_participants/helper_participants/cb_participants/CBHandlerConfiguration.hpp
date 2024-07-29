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
 * @file CBHandlerConfiguration.hpp
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace eprosima {
namespace ddshelper {
namespace participants {

/**
 * Structure encapsulating all of \c CBHandler configuration options.
 */
struct CBHandlerConfiguration
{
    CBHandlerConfiguration(
            const int& max_pending_samples,
            const unsigned int& event_window)
        : max_pending_samples(max_pending_samples)
        , event_window(event_window)
    {
    }

    //! Max number of messages to store in memory when schema not yet available
    int max_pending_samples;

    //! Keep in memory samples received in time frame
    unsigned int event_window;

};

} /* namespace participants */
} /* namespace ddshelper */
} /* namespace eprosima */
