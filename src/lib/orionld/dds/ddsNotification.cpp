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
#include "orionld/serviceRoutines/orionldPutAttribute.h"    // orionldPutAttribute
#include "orionld/dds/kjTreeLog.h"                          // kjTreeLog2
#include "orionld/dds/ddsNotification.h"                    // Own interface



// -----------------------------------------------------------------------------
//
// ddsNotification -
//
void ddsNotification(const char* entityType, const char* entityId, const char* attrName, KjNode* notificationP)
{
  KT_V("Got a notification from DDS");
  kjTreeLog2(notificationP, "notification", StDds);

  //
  // We receive entire NGSI-LD Attributes
  //
  KjNode* aValueNodeP = kjLookup(notificationP, "attributeValue");

  if (entityId == NULL)
  {
    KT_W("Entity without id from DDS");
    return;
  }

  // orionldState.payloadIdNode   = idNodeP;
  // orionldState.payloadTypeNode = typeNodeP;

  orionldState.wildcard[0]         = (char*) entityId;
  orionldState.wildcard[1]         = (char*) attrName;  // The topic is the attribute long name

  orionldState.requestTree         = aValueNodeP;
  orionldState.tenantP             = &tenant0;  // FIXME ... Use tenants?
  orionldState.uriParams.type      = (char*) entityType;
  orionldState.in.pathAttrExpanded = (char*) attrName;
  orionldState.ddsSample           = true;

  KT_T(StDds, "Calling orionldPutAttribute");

  //
  // If the entity does not exist, it needs to be created
  // Except of course, if it is registered and exists elsewhere
  //
  orionldPutAttribute();
}
