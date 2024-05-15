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
* Author: David Campo, Ken Zangelin
*/
extern "C"
{
#include "ktrace/kTrace.h"                                  // trace messages - ktrace library
#include "kjson/KjNode.h"                                   // KjNode
}

#include "orionld/dds/NgsildEntityPubSubTypes.h"
#include "orionld/dds/NgsildEntity.h"
#include "orionld/dds/NgsildPublisher.h"
#include "orionld/dds/config.h"                             // DDS_RELIABLE, ...



// -----------------------------------------------------------------------------
//
// namespacea ... (to be removed!)
//
using namespace eprosima::fastdds::dds;



// -----------------------------------------------------------------------------
//
// ddsPublish -
//
void ddsPublish(const char* topicType, const char* topicName, KjNode* entityP)
{
  NgsildPublisher* publisherP = new NgsildPublisher(topicType);

  KT_V("Initializing publisher for topicType '%s', topicName '%s'", topicType, topicName);
  if (publisherP->init(topicName))
  {
    //
    // FIXME: we can't do new+publish+delete for each and every publication!
    // There might easily be 10,000 publications per second.
    //

#ifdef DDS_SLEEP
    usleep(5000);
#endif

    KT_V("Publishing on topicType '%s', topicName '%s'", topicType, topicName);
    if (publisherP->publish(entityP))
      KT_V("Published on topicType '%s', topicName '%s'", topicType, topicName);
    else
      KT_V("Error publishing on topicType '%s', topicName '%s'", topicType, topicName);

#ifdef DDS_SLEEP
    usleep(5000);
#endif
  }

  KT_V("Deleting publisher");
  delete publisherP;
}
