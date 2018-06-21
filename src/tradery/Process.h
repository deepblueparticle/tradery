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

#include "Configuration.h"
#include "SessionContext.h"

enum SessionResult {
  normal,
  cancel,
  timeout,
  failed,
};

class ProcessResult {
 private:
  SessionResult _status;
  DWORD _exitCode;

 public:
  ProcessResult(SessionResult status, DWORD exitCode)
      : _status(status), _exitCode(exitCode) {}

  DWORD exitCode() const { return _exitCode; }
  SessionResult status() const { return _status; }
};

class SessionController {
 public:
  virtual ~SessionController() {}
  virtual void terminate() const = 0;
  virtual bool isActive() const = 0;
};

class ProcessSessionController : public SessionController {
 private:
  const HANDLE _hProcess = 0;
  const HANDLE _hThread = 0;

 public:
  ProcessSessionController(HANDLE hProcess, HANDLE hThread)
      : _hProcess(hProcess), _hThread(hThread) {}

  virtual ~ProcessSessionController() {
    LOG(log_debug, "Closing handles");
    if (_hProcess != 0) CloseHandle(_hProcess);
    if (_hThread != 0) CloseHandle(_hThread);
  }

  virtual void terminate() const { TerminateProcess(_hProcess, 0); }

  virtual bool isActive() const {
    return WaitForSingleObject(_hProcess, 0) != WAIT_OBJECT_0;
  }

  DWORD getExitCode() const {
    DWORD exitCode;
    GetExitCodeProcess(_hProcess, &exitCode);
    LOG(log_debug, "Process exit code: " << exitCode);
    return exitCode;
  }
};

SessionResult timeoutHandler(SessionContextPtr context, bool& _cancel,
                             const SessionController& sessionController);

const ProcessResult process(SessionContextPtr _cmdLine, bool& _cancel,
                            const std::string& processFileName,
                            const std::string& cmdLine,
                            const std::string* startingDirectory,
                            const Environment& env) throw(RunProcessException);
