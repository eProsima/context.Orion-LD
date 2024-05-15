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
* Author: Ken Zangelin, David Campo, Luis Arturo Frigolet
*/
#include <unistd.h>                                         // sleep
#include <strings.h>                                        // bzero
#include <stdlib.h>                                         // exit, malloc, calloc, free
#include <stdarg.h>                                         // va_start, ...

#include <string>                                           // std::string

#include "fastdds/dds/log/FileConsumer.hpp"                 // DDS logging

extern "C"
{
#include "ktrace/kTrace.h"                                  // trace messages - ktrace library
#include "ktrace/ktGlobals.h"                               // globals for KT library
#include "kargs/kargs.h"                                    // argument parsing - kargs library
#include "kalloc/kaInit.h"                                  // kaInit
#include "kjson/KjNode.h"                                   // KjNode
#include "kjson/kjBuilder.h"                                // kjObject, ...
}

#include "types/Verb.h"                                     // HTTP Verbs
#include "common/traceLevels.h"                             // Trace levels for ktrace
#include "ftClient/mhdInit.h"                               // mhdInit



using namespace eprosima::fastdds::dds;  // FIXME: remove this - use "absolute paths"



// -----------------------------------------------------------------------------
//
// FTCLIENT_VERSION -
//
#define FTCLIENT_VERSION "0.0.1"



// -----------------------------------------------------------------------------
//
// CLI Param variables
//
char*          traceLevels;
char*          logDir        = NULL;
char*          logLevel;
KBool          logToScreen;
KBool          fixme;
unsigned short ldPort;
char*          httpsKey;
char*          httpsCertificate;
unsigned int   mhdPoolSize;
unsigned int   mhdMemoryLimit;
unsigned int   mhdTimeout;
unsigned int   mhdMaxConnections;
bool           distributed;
long long      inReqPayloadMaxSize = 64 * 1024;



// -----------------------------------------------------------------------------
//
// kargs - vector of CLI parameters
//
KArg kargs[] =
{
  //
  // Potential builtins
  //
  { "--trace",            "-t",    KaString,  &traceLevels,       KaOpt, 0,         KA_NL,    KA_NL,    "trace levels (csv of levels/ranges)"               },
  { "--logDir",           "-ld",   KaString,  &logDir,            KaOpt, _i "/tmp", KA_NL,    KA_NL,    "log file directory"                                },
  { "--logLevel",         "-ll",   KaString,  &logLevel,          KaOpt, 0,         KA_NL,    KA_NL,    "log level (ERR|WARN|INFO|INFO|VERBOSE|TRACE|DEBUG" },
  { "--logToScreen",      "-ls",   KaBool,    &logToScreen,       KaOpt, KFALSE,    KA_NL,    KA_NL,    "log to screen"                                     },
  { "--fixme",            "-fix",  KaBool,    &fixme,             KaOpt, KFALSE,    KA_NL,    KA_NL,    "FIXME messages"                                    },

  //
  // Broker options
  //
  { "--port",             "-p",    KaUShort,  &ldPort,            KaOpt, _i 7701,   _i 1027,  _i 65535, "TCP port for incoming requests"                    },
  { "--httpsKey",         "-k",    KaString,  &httpsKey,          KaOpt, NULL,      KA_NL,    KA_NL,    "https key file"                                    },
  { "--httpsCertificate", "-c",    KaString,  &httpsCertificate,  KaOpt, NULL,      KA_NL,    KA_NL,    "https certificate file"                            },

  { "--mhdPoolSize",      "-mps",  KaUInt,    &mhdPoolSize,       KaOpt, _i 8,      _i 0,     _i 1024,  "MHD request thread pool size"                      },
  { "--mhdMemoryLimit",   "-mlim", KaUInt,    &mhdMemoryLimit,    KaOpt, _i 64,     _i 0,     _i 1024,  "MHD memory limit (in kb)"                          },
  { "--mhdTimeout",       "-mtmo", KaUInt,    &mhdTimeout,        KaOpt, _i 2000,   _i 0,     KA_NL,    "MHD connection timeout (in milliseconds)"          },
  { "--mhdConnections",   "-mcon", KaUInt,    &mhdMaxConnections, KaOpt, _i 512,    _i 1,     KA_NL,    "Max number of MHD connections"                     },

  { "--distOps",          "-dops", KaBool,    &distributed,       KaOpt, KFALSE,    KA_NL,    KA_NL,    "support for distributed operations"                },

  KARGS_END
};








// -----------------------------------------------------------------------------
//
// klibLogBuffer -
//
char klibLogBuffer[4 * 1024];



// -----------------------------------------------------------------------------
//
// klibLogFunction -
//
static void klibLogFunction
(
  int          severity,              // 1: Error, 2: Warning, 3: Info, 4: Verbose, 5: Trace
  int          level,                 // Trace level || Error code || Info Code
  const char*  fileName,
  int          lineNo,
  const char*  functionName,
  const char*  format,
  ...
)
{
  va_list  args;

  /* "Parse" the variable arguments */
  va_start(args, format);

  /* Print message to variable */
  vsnprintf(klibLogBuffer, sizeof(klibLogBuffer), format, args);
  va_end(args);

  // LM_K(("Got a lib log message, severity: %d: %s", severity, libLogBuffer));

  if (severity == 1)
    ktOut(fileName, lineNo, functionName, 'E', 0, "klib: %s", klibLogBuffer);
  else if (severity == 2)
    ktOut(fileName, lineNo, functionName, 'W', 0, "klib: %s", klibLogBuffer);
  else if (severity == 3)
    ktOut(fileName, lineNo, functionName, 'I', 0, "klib: %s", klibLogBuffer);
  else if (severity == 4)
    ktOut(fileName, lineNo, functionName, 'V', 0, "klib: %s", klibLogBuffer);
  else if (severity == 5)
    ktOut(fileName, lineNo, functionName, 'T', level + 1000, "klib: %s", klibLogBuffer);
}



// -----------------------------------------------------------------------------
//
// main -
//
int main(int argC, char* argV[])
{
  KArgsStatus ks;
  const char* progName = "ftClient";

  ks = kargsInit(progName, kargs, "FTCLIENT");
  if (ks != KargsOk)
  {
    fprintf(stderr, "error reading CLI parameters\n");
    exit(1);
  }

  ks = kargsParse(argC, argV);
  if (ks != KargsOk)
  {
    kargsUsage();
    exit(1);
  }

  int kt = ktInit(progName, logDir, logToScreen, logLevel, traceLevels, kaBuiltinVerbose, kaBuiltinDebug, fixme);

  if (kt != 0)
  {
    fprintf(stderr, "Error initializing logging library\n");
    exit(1);
  }

  kaInit(klibLogFunction);

  //
  // Traces for eProsima FastDDS libraries
  //
  // EPROS:
  //   For now, all traces from the eProsima libraries are kept in a separate logfile.
  //   This needs to change. It's not acceptable that a library demands to have its own log file.
  //   Orion-LD uses perhaps 25 different libraries. Should we have 25 different logfiles?
  //   No, we should not.
  //
  //   We need a new type for "Log Consumer", one that accepts a callback function pointer for the application
  //   that uses the eProsima libraries to do whatever needs to be done with the log data.
  //   Proposed definition of the callback function:
  //
  //     void ddsLog(const char* file, const char* function, int lineNo, int logLevel, int traceLevel, const char* logMsg);
  //
  std::string ddsLogFile = std::string(logDir) + "/ftClient_dds.log";
  std::unique_ptr<FileConsumer> append_file_consumer(new FileConsumer(ddsLogFile, true));
  Log::RegisterConsumer(std::move(append_file_consumer));


  //
  // Perhaps the most important feature of ftClient is the ability to report on received notifications.
  // For this purpose, the dumpArray contains all payloads received as notifications.
  //
  // NOTE: not only notifications, also forwarded requests, or just about anything received out of the defined API it supports for
  //       configuration.
  //
  KT_D("%s version:                   %s", progName, FTCLIENT_VERSION);

  mhdInit(ldPort);

  while (1)
  {
    sleep(1);
  }
}
