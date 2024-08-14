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
 * @file CBWriter.hpp
 */

#pragma once

#include <mutex>

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>

#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddshelper_participants/library/library_dll.h>
#include <CBMessage.hpp>

namespace eprosima {
namespace ddshelper {
namespace participants {

class CBWriter
{
public:

    CBWriter() = default;
    ~CBWriter() = default;

    /**
     * @brief Writes data.
     *
     * @param [in] msg Pointer to the data to be written.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    virtual void write_data(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

protected:

    /**
     * @brief Writes the type information used in this topic the first time it is received.
     *
     * @param [in] msg Pointer to the data.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    virtual void write_schema(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    /**
     * @brief Returns the dyn_data of a dyn_type.
     *
     * @param [in] msg Pointer to the data.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    fastdds::dds::DynamicData::_ref_type get_dynamic_data_(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type) noexcept;

    //! Schemas map
    std::unordered_map<std::string, fastdds::dds::xtypes::TypeIdentifier> stored_schemas_;

    // The mutex to protect the calls to write
    std::mutex mutex_;
};

} /* namespace participants */
} /* namespace ddshelper */
} /* namespace eprosima */
