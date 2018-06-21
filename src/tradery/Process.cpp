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
#include "Process.h"
#include "SessionContext.h"

const ProcessResult process(SessionContextPtr context, bool& _cancel,
                            const std::string& processFileName,
                            const std::string& cmdLine,
                            const std::string* startingDirectory,
                            const Environment& env) throw(RunProcessException) {
  try {
    LOG(log_debug, "\tprocess file name: " << processFileName);
    LOG(log_debug, "\tcmd line: " << cmdLine);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Run the autoupdate program.
    if (CreateProcess(
            s2ws(processFileName).c_str(),
            const_cast<LPTSTR>(s2ws(cmdLine).c_str()),  // Command line.
            NULL,  // Process handle not inheritable.
            NULL,  // Thread handle not inheritable.
            TRUE,  // Set handle inheritance to FALSE.
            0,     // No creation flags.
            env,   // Use parent's environment block.
            startingDirectory != 0 ? s2ws(*startingDirectory).c_str()
                                   : 0,  // Use parent's starting directory.
            &si,                         // Pointer to STARTUPINFO structure.
            &pi)  // Pointer to PROCESS_INFORMATION structure.
    ) {
      //      std::cout << _T( "In process - hProcess: " ) << pi.hProcess <<
      //      std::endl << std::endl;
      LOG(log_info,
          "Process \"" << cmdLine << "\" created, hProcess: " << pi.hProcess);
      ProcessSessionController psc(pi.hProcess, pi.hThread);
      SessionResult status = timeoutHandler(context, _cancel, psc);
      return ProcessResult(status, psc.getExitCode());
    } else {
      LOG(log_info, "ProcessCreate failed, GetLastError: " << GetLastError())
      throw RunProcessException(processFileName, cmdLine, GetLastError());
    }
  } catch (...) {
    LOG(log_info, "unknown exception, GetLastError: " << GetLastError());
    throw RunProcessException(processFileName, cmdLine, GetLastError());
  }
}
