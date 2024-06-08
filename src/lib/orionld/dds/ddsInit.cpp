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
* Author: Ken Zangelin
*/
extern "C"
{
#include "ktrace/kTrace.h"                                  // trace messages - ktrace library
#include "kbase/kStringSplit.h"                             // kStringSplit
}

#include "orionld/common/orionldState.h"                    // orionldState, kjTreeLog
#include "orionld/dds/ddsSubscribe.h"                       // ddsSubscribe
#include "orionld/dds/ddsNotification.h"                    // ddsNotification
#include "orionld/dds/ddsInit.h"                            // Own interface



// -----------------------------------------------------------------------------
//
// ddsInit - initialization function for DDS
//
// PARAMETERS
// * ddsTopicType
// * ddsSubsTopics
// * mode - the DDS mode the broker is working in
//
int ddsInit(const char* ddsTopicType, char* ddsSubsTopics)
{
  if (ddsSubsTopics[0] == 0)
    return 0;

  KT_V("topics: %s", ddsSubsTopics);
  char* topicV[100];
  int   topics = kStringSplit(ddsSubsTopics, ',', topicV, 3);

  KT_V("no of topics: %d", topics);
  for (int ix = 0; ix < topics; ix++)
  {
    KT_V("Subscribing to DDS topic %s::%s", ddsTopicType, topicV[ix]);
    ddsSubscribe(ddsTopicType, topicV[ix], ddsNotification);
  }

  return 0;
}
