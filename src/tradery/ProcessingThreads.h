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

#pragma once

#include "ProcessingThread.h"

class ProcessingThreadsException {};

class ProcessingThreads {
 private:
  typedef std::map<UniqueId, ManagedPtr<ProcessingThread> >
      IdToProcessingThreadMap;

  IdToProcessingThreadMap _map;
  mutable Mutex _mutex;

  static ProcessingThreads processingThreads;

 public:
  ProcessingThreads() {}

  // will remove non-running elements in the map
  void clean();

  /*  void cancel( const UniqueId& id )
  {
  Lock lock( _mutex );
  IdToProcessingThreadMap::iterator i = _map.find( id );

  if( i != _map.end() && i->second->isRunning() )
  i->second->cancel();
  }
  */
  static void run(SessionContextPtr config);
  void addAndRunAsync(const UniqueId& id, SessionContextPtr context) throw(
      ProcessingThreadsException);
};
