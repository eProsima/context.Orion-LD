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
#include "kjson/KjNode.h"                                   // KjNode
#include "kjson/kjLookup.h"                                 // kjLookup
#include "kjson/kjBuilder.h"                                // kjChildRemove, ...
}

#include "orionld/common/orionldState.h"                    // orionldState, kjTreeLog
#include "orionld/common/traceLevels.h"                     // KT_T trace levels
#include "orionld/common/tenantList.h"                      // tenant0
#include "orionld/serviceRoutines/orionldPutEntity.h"       // orionldPutEntity
#include "orionld/dds/kjTreeLog.h"                          // kjTreeLog2
#include "orionld/dds/ddsNotification.h"                    // Own interface



// -----------------------------------------------------------------------------
//
// ddsNotification -
//
void ddsNotification(KjNode* notificationP)
{
  KT_V("Got a notification from DDS");
  kjTreeLog2(notificationP, "notification", StDds);

  //
  // Assuming the received entity is complete, we perform a PUT on the entity
  //
  KjNode* idNodeP   = kjLookup(notificationP, "id");
  KjNode* typeNodeP = kjLookup(notificationP, "type");
  char*   id        = (idNodeP != NULL)? idNodeP->value.s : NULL;
  char*   type      = (typeNodeP != NULL)? typeNodeP->value.s : NULL;

  if ((id == NULL) || (type == NULL))
  {
    KT_W("Entity without id|type from DDS");
    return;
  }

  if ((idNodeP->type != KjString) || (typeNodeP->type != KjString))
  {
    KT_W("Entity with id|type that is not a string from DDS");
    return;
  }

  kjChildRemove(notificationP, idNodeP);
  // kjChildRemove(notificationP, typeNodeP);

  orionldState.payloadIdNode   = idNodeP;
  // orionldState.payloadTypeNode = typeNodeP;

  orionldState.requestTree = notificationP;
  orionldState.wildcard[0] = id;
  orionldState.tenantP     = &tenant0;
  orionldState.upsert      = true;

  KT_T(StDds, "Calling orionldPutEntity");

  orionldPutEntity();
}



