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

#include "builderrorsparser.h"
#include "Configuration.h"
#include "Process.h"
#include "SessionContext.h"

#pragma once

class ProcessingThread : public Thread {
 private:
  SessionContextPtr _context;
  bool _cancel;

  class InstanceCounter {
    friend ProcessingThread;

   private:
    static Mutex _m;
    static unsigned int _instances;

   public:
    InstanceCounter() {
      Lock lock(_m);
      ++_instances;
    }

    ~InstanceCounter() {
      Lock lock(_m);
      assert(_instances > 0);
      --_instances;
    }

    static unsigned int getInstances() {
      Lock lock(_m);
      return _instances;
    }
  };

 public:
  ProcessingThread(SessionContextPtr context);

  void cancel() {
    _cancel = true;

    while (isRunning()) Sleep(50);
  }

  void zipFiles();

  void run(ThreadContext* context = 0);

  bool SetPrivilege(HANDLE hToken, char const* privilege, bool enable);

  const ProcessResult processAsUser(
      const std::string& userName, const std::string& domain,
      const std::string& password, const std::string& processFileName,
      const std::string& cmdLine, const std::string* startingDirectory,
      const Environment& env) throw(RunProcessException);
};
