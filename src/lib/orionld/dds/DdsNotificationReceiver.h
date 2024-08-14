#ifndef SRC_LIB_ORIONLD_DDS_DDSNOTIFICATIONRECEIVER_H_
#define SRC_LIB_ORIONLD_DDS_DDSNOTIFICATIONRECEIVER_H_

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
#include <string>

#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "fastdds/dds/domain/DomainParticipantFactory.hpp"
#include "fastdds/dds/subscriber/DataReader.hpp"
#include "fastdds/dds/subscriber/DataReaderListener.hpp"
#include "fastdds/dds/subscriber/qos/DataReaderQos.hpp"
#include "fastdds/dds/subscriber/SampleInfo.hpp"
#include "fastdds/dds/subscriber/Subscriber.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"

#include "orionld/dds/config.h"                             // DDS_RELIABLE, ...
#include "orionld/dds/NgsildEntity.hpp"                     // NgsildEntity
#include "orionld/dds/kjTreeLog.h"                          // kjTreeLog2



using namespace eprosima::fastdds::dds;



// -----------------------------------------------------------------------------
//
// DdsNotificationFunction - callback for reception of DDS samples
//
typedef void (*DdsNotificationFunction)(const char* entityType, const char* entityId, const char* topicName, KjNode* notificationP);



// -----------------------------------------------------------------------------
//
// DdsNotificationReceiver -
//
// FIXME: All the implementation to DdsNotificationReceiver.cpp
//
class DdsNotificationReceiver : public DataReaderListener
{
 public:
  DdsNotificationReceiver() : samples_(0), callback_(NULL)    { }
  explicit DdsNotificationReceiver(DdsNotificationFunction callback) : samples_(0), callback_(callback)    { }
  ~DdsNotificationReceiver() override { }

  std::atomic_int          samples_;
  std::string              topicName_;

  void                     on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override;
  void                     on_data_available(DataReader* reader) override;

  NgsildEntity             ngsildEntity_;
  DdsNotificationFunction  callback_;
};

#endif  // SRC_LIB_ORIONLD_DDS_DDSNOTIFICATIONRECEIVER_H_
