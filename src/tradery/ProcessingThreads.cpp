/*
Copyright (C) 2018 Adrian Michel
http://www.amichel.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#include <log.h>
#include "ProcessingThreads.h"
#include "SessionContext.h"

ProcessingThreads ProcessingThreads::processingThreads;

void ProcessingThreads::run(SessionContextPtr context) {
  processingThreads.clean();
  // if the first char of the request is a *, than this is a file request from
  // a running simulation
  // otherwise is a command line to start a simulation
  //    std::cout << std::endl << "Before run" << std::endl;

  LOG1(log_debug, context->getSessionConfig()->getSessionId(),
       "get answer to request after cmdline");
  //  std::cout << cmdLine.getCmdLineString() << std::endl;
  // this is the session name
  // todo: handle uniqueid exception
  // the runnable name is the session id
  UniqueId runnableName(context->getSessionConfig()->getSessionId().c_str());

  if (context->getConfig()->asyncRun()) {
    LOG1(log_debug, context->getSessionConfig()->getSessionId(),
         "get answer to request before async run");

    processingThreads.addAndRunAsync(runnableName, context);
    LOG1(log_debug, context->getSessionConfig()->getSessionId(),
         "get answer to request after async run");
  } else {
    LOG1(log_debug, context->getSessionConfig()->getSessionId(),
         "get answer to request before processing thread");
    ProcessingThread pt(context);
    LOG1(log_debug, context->getSessionConfig()->getSessionId(),
         "get answer to request after processing thread");

    pt.startSync();
    LOG1(log_debug, context->getSessionConfig()->getSessionId(), "After run");
  }
}

void ProcessingThreads::addAndRunAsync(
    const UniqueId& id,
    SessionContextPtr context) throw(ProcessingThreadsException) {
  Lock lock(_mutex);

  ManagedPtr<ProcessingThread> pt;

  for (IdToProcessingThreadMap::iterator i = _map.find(id); i != _map.end();
       i = _map.find(id)) {
    // if the thread is still running, then cancel
    if (i->second->isRunning()) i->second->cancel();

    // clean the inactive processing threads, including this one
    clean();
  }

  pt = ManagedPtr<ProcessingThread>(new ProcessingThread(context));
  _map.insert(IdToProcessingThreadMap::value_type(id, pt));

  LOG1(log_debug, context->getSessionConfig()->getSessionId(),
       "*******async run**********");
  pt->start();
}

void ProcessingThreads::clean() {
  Lock lock(_mutex);

  for (IdToProcessingThreadMap::iterator i = _map.begin(); i != _map.end();) {
    if (!i->second->isRunning())
      i = _map.erase(i);
    else
      i++;
  }
}
