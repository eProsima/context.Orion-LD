#ifndef SRC_LIB_ORIONLD_DDS_NGSILDSUBSCRIBER_H_
#define SRC_LIB_ORIONLD_DDS_NGSILDSUBSCRIBER_H_

/*
*
* Copyright 2024 FIWARE Foundation e.V.
*
* This file is part of Orion-LD Context Broker.
*
* Orion-LD Context Broker is free software: you can redistribute it and/or
* modify it under the terms of the GNU Affero General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* Orion-LD Context Broker is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
* General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with Orion-LD Context Broker. If not, see http://www.gnu.org/licenses/.
*
* For those usages not covered by this license please contact with
* orionld at fiware dot org
*
*/

//
// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
//
#include <chrono>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

extern "C"
{
#include "ktrace/kTrace.h"                                  // trace messages - ktrace library
#include "kjson/kjBuilder.h"                                // kjObject, kjString, kjChildAdd, ...
#include "kjson/kjParse.h"                                  // kjParse
#include "kjson/kjClone.h"                                  // kjClone
}

#include "orionld/common/orionldState.h"                    // orionldState
#include "orionld/common/traceLevels.h"                     // Trace Levels
#include "orionld/dds/NgsildEntityPubSubTypes.h"            // DDS stuff ...
#include "orionld/dds/config.h"                             // DDS_RELIABLE, ...
#include "orionld/dds/kjTreeLog.h"                          // kjTreeLog2

using namespace eprosima::fastdds::dds;



// -----------------------------------------------------------------------------
//
//  ddsDumpArray - accumulating data from DDS notifications
//
extern KjNode* ddsDumpArray;



// -----------------------------------------------------------------------------
//
// NgsildSubscriber -
//
// FIXME: All the implementation needs to go to NgsildSubscriber.cpp
//
class NgsildSubscriber
{
 private:
  DomainParticipant*  participant_;
  Subscriber*         subscriber_;
  DataReader*         reader_;
  Topic*              topic_;
  TypeSupport         type_;

  class SubListener : public DataReaderListener
  {
  public:
  SubListener() : samples_(0)    { }
  ~SubListener() override        { }

  void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override
  {
    if (info.current_count_change == 1)
      KT_T(StDds, "Subscriber matched.");
    else if (info.current_count_change == -1)
      KT_T(StDds, "Subscriber unmatched.");
    else
      KT_T(StDds, "'%d' is not a valid value for SubscriptionMatchedStatus current count change", info.current_count_change);
  }

  void on_data_available(DataReader* reader) override
  {
    SampleInfo info;

    KT_T(StDds, "Notification arrived");

    if (reader->take_next_sample(&ngsildEntity_, &info) == ReturnCode_t::RETCODE_OK)
    {
      if (info.valid_data)
      {
        samples_++;

        //
        // This is "more or less" how it should work:
        //   KjNode* entityP = kjEntityFromDds(&ngsildEntity_);
        //   notificationReceived(entityP);
        // The callback 'notificationReceived' is set in some constructor or init() method
        //
        KT_T(StDds, "Entity Id: %s with type: %s RECEIVED.", ngsildEntity_.id().c_str(), ngsildEntity_.type().c_str());

        //
        // Accumulate notifications
        //
        KjNode* dump         = kjObject(NULL, "item");  // No name as it is part of an array
        KjNode* tenantP      = (ngsildEntity_.tenant()     != "")? kjString(NULL,   "tenant",     ngsildEntity_.tenant().c_str()) : NULL;
        KjNode* idP          = (ngsildEntity_.id()         != "")? kjString(NULL,   "id",         ngsildEntity_.id().c_str())     : NULL;
        KjNode* typeP        = (ngsildEntity_.type()       != "")? kjString(NULL,   "type",       ngsildEntity_.type().c_str())   : NULL;
        KjNode* scopeP       = (ngsildEntity_.scope()      != "")? kjString(NULL,   "scope",      ngsildEntity_.scope().c_str())  : NULL;
        KjNode* createdAtP   = (ngsildEntity_.createdAt()  != 0)?  kjInteger(NULL,  "createdAt",  ngsildEntity_.createdAt())      : NULL;
        KjNode* modifiedAtP  = (ngsildEntity_.modifiedAt() != 0)?  kjInteger(NULL,  "modifiedAt", ngsildEntity_.modifiedAt())     : NULL;
        char*   attributes   = (ngsildEntity_.attributes() != "")? (char*) ngsildEntity_.attributes().c_str() : NULL;

        if (tenantP     != NULL)  kjChildAdd(dump, tenantP);
        if (idP         != NULL)  kjChildAdd(dump, idP);
        if (typeP       != NULL)  kjChildAdd(dump, typeP);
        if (scopeP      != NULL)  kjChildAdd(dump, scopeP);
        if (createdAtP  != NULL)  kjChildAdd(dump, createdAtP);
        if (modifiedAtP != NULL)  kjChildAdd(dump, modifiedAtP);

        if (attributes != NULL)
        {
          KT_T(StDds, "Entity '%s' has attributes: '%s'", ngsildEntity_.id().c_str(), attributes);

          // Initializing orionldState, to call kjParse (not really necessary, it's overkill)
          orionldStateInit(NULL);
          KT_T(StDds, "Called orionldStateInit for thread 0x%x", gettid());

          // parse the string 'attributes' and add all attributes to 'dump'
          KT_T(StDds, "Calling kjParse with orionldState (tid: 0x%x)", gettid());
          KjNode* attrsNode = kjParse(orionldState.kjsonP, attributes);
          if (attrsNode != NULL)
            attrsNode = kjClone(NULL, attrsNode);
          KT_T(StDds, "After kjParse");

          kjTreeLog2(attrsNode, "attrsNode", StDds);
          kjTreeLog2(dump, "dump w/o attrs", StDds);
          // Concatenate the attributes to the "dump entity"
          dump->lastChild->next = attrsNode->value.firstChildP;
          dump->lastChild       = attrsNode->lastChild;
          kjTreeLog2(dump, "dump with attrs", StDds);
        }
        else
          KT_T(StDds, "Entity Id: %s has no attributes", ngsildEntity_.id().c_str());

        if (ddsDumpArray == NULL)
          ddsDumpArray = kjArray(NULL, "ddsDumpArray");

        kjChildAdd(ddsDumpArray, dump);
      }
    }
  }

  NgsildEntity     ngsildEntity_;
  std::atomic_int  samples_;
  } listener_;

 public:
  explicit NgsildSubscriber(const char* topicType)
    : participant_(nullptr)
    , subscriber_(nullptr)
    , reader_(nullptr)
    , topic_(nullptr)
    , type_(new NgsildEntityPubSubType(topicType))
  {
  }

  virtual ~NgsildSubscriber()
  {
    if (reader_ != nullptr)
    {
      subscriber_->delete_datareader(reader_);
    }
    if (topic_ != nullptr)
    {
      participant_->delete_topic(topic_);
    }
    if (subscriber_ != nullptr)
    {
      participant_->delete_subscriber(subscriber_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
  }

  bool init(const char* topicName)
  {
    DomainParticipantQos participantQos;
    participantQos.name("Participant_subscriber");
    participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

    if (participant_ == nullptr)
      return false;

    // Register the Type
    type_.register_type(participant_);

    // Create the subscriptions Topic
    const char* topicType = type_->getName();
    topic_ = participant_->create_topic(topicName, topicType, TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
      KT_V("Error creating topic (type: '%s') '%s'", topicType, topicName);
      return false;
    }

    // Create the Subscriber
    subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if (subscriber_ == nullptr)
      return false;

    // Create the DataReader
    KT_V("Creating reader");
#ifdef DDS_RELIABLE
    DataReaderQos  rqos = DATAREADER_QOS_DEFAULT;

    rqos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
    rqos.durability().kind  = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
//    rqos.history().kind     = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
//    rqos.history().depth    = 5;
    reader_                 = subscriber_->create_datareader(topic_, rqos, &listener_);
#else
    reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);
#endif

    if (reader_ == nullptr)
    {
      KT_E("Error creating DataReader");
      return false;
    }

    KT_V("Created reader");
    KT_V("Init done");

    return true;
  }

  void run(uint32_t samples)
  {
    KT_V("Awaiting notifications");
    while ((uint32_t) listener_.samples_ < samples)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
};

#endif  // SRC_LIB_ORIONLD_DDS_NGSILDSUBSCRIBER_H_
