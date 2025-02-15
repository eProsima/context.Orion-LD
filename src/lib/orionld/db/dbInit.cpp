/*
*
* Copyright 2019 FIWARE Foundation e.V.
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
#include "orionld/common/orionldState.h"                                   // mongocOnly
#include "orionld/db/dbConfiguration.h"                                    // This is where the function pointers for the DB functions are
#include "orionld/mongoCppLegacy/mongoCppLegacyInit.h"                     // mongoCppLegacyInit
#include "orionld/db/dbInit.h"                                             // Own interface



// ----------------------------------------------------------------------------
//
// dbInit -
//
// PARAMETERS
//   dbHost - the host and port where the mongodb server runs. E.g. "localhost:27017"
//   dbName - the name of the database. Default is 'orion'
//
void dbInit(const char* dbHost, const char* dbName)
{
  if (mongocOnly == false)
    mongoCppLegacyInit(dbHost, dbName);
}
