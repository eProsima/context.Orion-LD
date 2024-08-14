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
 * @file CBHandler.hpp
 */

#pragma once

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <stdexcept>
#include <thread>

#include <ddspipe_core/efficiency/payload/PayloadPool.hpp>
#include <ddspipe_core/types/data/RtpsPayloadData.hpp>
#include <ddspipe_core/types/dds/Payload.hpp>
#include <ddspipe_core/types/topic/dds/DdsTopic.hpp>

#include <ddspipe_participants/participant/dynamic_types/ISchemaHandler.hpp>

#include <ddshelper_participants/helper_participants/cb_participants/CBHandlerConfiguration.hpp>
#include <ddshelper_participants/helper_participants/cb_participants/CBMessage.hpp>
#include <ddshelper_participants/helper_participants/cb_participants/CBWriter.hpp>
#include <ddshelper_participants/library/library_dll.h>

namespace std {
template<>
struct hash<eprosima::fastdds::dds::xtypes::TypeIdentifier>
{
    std::size_t operator ()(
            const eprosima::fastdds::dds::xtypes::TypeIdentifier& k) const
    {
        // The collection only has direct hash TypeIdentifiers so the EquivalenceHash can be used.
        return (static_cast<size_t>(k.equivalence_hash()[0]) << 16) |
               (static_cast<size_t>(k.equivalence_hash()[1]) << 8) |
               (static_cast<size_t>(k.equivalence_hash()[2]));
    }
};

} // std

namespace eprosima {
namespace ddshelper {
namespace participants {

/**
 * Class that manages the interaction between DDS Pipe \c (SchemaParticipant) and CB.
 * Payloads are efficiently passed from DDS Pipe to CB without copying data (only references).
 *
 * @implements ISchemaHandler
 */
class CBHandler : public ddspipe::participants::ISchemaHandler
{
public:

    /**
     * CBHandler constructor by required values.
     *
     * Creates CBHandler instance with given configuration, payload pool.
     * Opens temporal CB where data is to be written.
     *
     * @throw InitializationException if creation fails (fail to open CB).
     *
     * @param config:       Structure encapsulating all configuration options.
     * @param payload_pool: Owner of every payload contained in received messages.
     */
    DDSHELPER_PARTICIPANTS_DllAPI
    CBHandler(
            const CBHandlerConfiguration& config,
            const std::shared_ptr<ddspipe::core::PayloadPool>& payload_pool);

    /**
     * @brief Destructor
     */
    DDSHELPER_PARTICIPANTS_DllAPI
    ~CBHandler();

    /**
     * @brief Create and store in \c schemas_ an OMG IDL (.idl format) schema.
     * Any samples following this schema that were received before the schema itself are moved to the memory buffer.
     *
     * @param [in] dyn_type DynamicType containing the type information required to generate the schema.
     */
    DDSHELPER_PARTICIPANTS_DllAPI
    void add_schema(
            const fastdds::dds::DynamicType::_ref_type& dyn_type,
            const std::string& type_name,
            const fastdds::dds::xtypes::TypeIdentifier& type_id) override;

    /**
     * @brief Add a data sample, associated to the given \c topic.
     *
     * The sample is added to buffer without schema.
     *
     * @param [in] topic DDS topic associated to this sample.
     * @param [in] data payload data to be added.
     */
    DDSHELPER_PARTICIPANTS_DllAPI
    void add_data(
            const ddspipe::core::types::DdsTopic& topic,
            ddspipe::core::types::RtpsPayloadData& data) override;

protected:

    /**
     * @brief Write to CB.
     *
     * @param [in] msg CBMessage to be added
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    void write_sample(
            const CBMessage& msg,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);
    /**
     * @brief Add to pending samples collection.
     *
     * @param [in] msg CBMessage to be added
     * @param [in] topic Topic of message to be added
     */
    void add_pending_sample(
            CBMessage& msg,
            const ddspipe::core::types::DdsTopic& topic);

    /**
     * @brief Add any pending samples associated to \c schema_name
     *
     * Samples in \c pending_samples_ structure for this schema are written.
     *
     * @param [in] schema_name Name of the schema for which pending samples using it are added.
     * @param [in] dyn_type DynamicType containing the type information required.
     */
    void write_pending_samples(
            const std::string& schema_name,
            const fastdds::dds::DynamicType::_ref_type& dyn_type);

    /**
     * @brief Remove buffered samples older than [now - event_window]
     *
     */
    void remove_outdated_samples();

    //! Handler configuration
    CBHandlerConfiguration configuration_;

    //! Payload pool
    std::shared_ptr<ddspipe::core::PayloadPool> payload_pool_;

    //! CB writer
    CBWriter cb_writer_;

    //! Schemas map
    std::unordered_map<fastdds::dds::xtypes::TypeIdentifier, fastdds::dds::DynamicType::_ref_type> schemas_;

    //! Structure where messages with unknown type are kept
    std::map<std::string, std::list<std::pair<ddspipe::core::types::DdsTopic, CBMessage>>> pending_samples_;
    
    //! Unique sequence number assigned to received messages. It is incremented with every sample added
    unsigned int unique_sequence_number_{0};

    //! Variable used to stop the periodic cleanup thread
    std::atomic<bool> periodic_cleanup_running_;

    //! Thread used to remove outdated samples
    std::thread periodic_cleanup_thread_;

    //! Mutex synchronizing access to object's data structures
    std::mutex mtx_;
};

} /* namespace participants */
} /* namespace ddshelper */
} /* namespace eprosima */
