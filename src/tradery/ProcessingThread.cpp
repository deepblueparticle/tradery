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

#include "RunnablePluginBuilder.h"
#include "ProcessingThread.h"
#include "runsystem.h"

#define PROCESSING_THREAD "ProcessingThread"

void ProcessingThread::zipFiles() {
  CZipArchive zip;
  CZipMemFile mf;
  // create archive in memory
  zip.Open(mf, CZipArchive::zipCreate);

  CFileFind finder;
  // build a string with wildcards
  std::ostringstream o;

  o << _context->getSessionConfig()->getSessionPath() << "\\*.csv";

  LOG(log_info, "in zip, wild card: " << o.str());

  // start working for files
  BOOL bWorking = finder.FindFile(s2ws(o.str()).c_str());

  while (bWorking) {
    bWorking = finder.FindNextFile();

    // skip . and .. files; otherwise, we'd
    // recur infinitely!

    if (!finder.IsDots() && !finder.IsDirectory()) {
      CString str = finder.GetFilePath();
      LOG(log_info, "found file: " << str);
      zip.AddNewFile(str, -1, false);
    }
  }

  finder.Close();

  // ...
  // add some files to archive here e.g. by calling CZipArchive::AddNewFile
  // ...
  zip.Close();
  // write the archive to disk
  CZipFile f;
  if (f.Open(s2ws(_context->getConfig()->zipFile()).c_str(),
             CZipFile::modeWrite | CZipFile::modeCreate, false)) {
    int iLen = mf.GetLength();
    BYTE* b = mf.Detach();
    f.Write(b, iLen);
    f.Close();
    // we must free the detached memory
    free(b);
  }
}

void ProcessingThread::run(ThreadContext* threadContext) {
  LOG1(log_info, _context->getSessionConfig()->getSessionId(),
       "in ProcessingThread run");
  std::wstring signalFile;
  try {
    // first build

    SessionResult sessionResult = SessionResult::normal;

    // used to generate the end of session file (will give info on the type of
    // session end to the web part)
    bool enableProcess;

    RunnablePluginBuilder builder(_context, _cancel);
    LOG1(log_info, _context->getSessionConfig()->getSessionId(),
         "In run, after builder - "
             << (builder.success() ? "success" : "failure"));
    enableProcess = builder.success();

    if (enableProcess) {
      TCHAR fileName[MAX_PATH + 1];

      GetModuleFileName(0, fileName, MAX_PATH);

      std::string newCmdLine;
      // set the threads
      newCmdLine << "--threads "
                 << (InstanceCounter::getInstances() <
                             _context->getConfig()->getCPUCount()
                         ? _context->getConfig()->getCPUCount() -
                               InstanceCounter::getInstances()
                         : 1)
                 << " ";
      // set the simulation mode
      LOG(log_debug, "setting simulation mode");
      newCmdLine +=
          std::string("-v ") + _context->getConfig()->getCmdLineString();
      LOG(log_debug,
          "new command line: " << _context->getConfig()->getCmdLineString());
      // a very crude thread control algorithm. If the number of instances is 0,
      // the threads == number of cpus, decrement for each new instance until we
      // get to 1 thread for any additional task
      //
      // todo: this should be done by using a thread pool, and dynamically
      // allocating tasks to threads as they come
      ;

      // then run

      LOG(log_info, "Before running process");
      // in debug mode will runProcess to make sure we can see the whole process
      // (if not, a privilege error will privent runProcessAsUser from running)

      InstanceCounter ic;

      {
        // temporarily (or maybe permanently, with some changes)
        // running the system directly instead of spawning a new process
        // this is for easier testing, but may also make things faster by not
        // wasting time and resources with a new process
        // to make it in process, will have to handle the thread the same
        // way I handle the process - check for timeout, cancel etc

        try {
          RunSystem runSystem(_context);
          runSystem.run();
        } catch (const RunSystemException& e) {
          LOG(log_debug,
              "run system exception: " << e.message()
                                       << ", error code: " << e.errorCode());
        }
        return;
      }

      const ProcessResult pr(
          _context->getConfig()->enableRunAsUser()
              ? processAsUser(_context->getConfig()->userName(), ".",
                              _context->getConfig()->password(), ws2s(fileName),
                              newCmdLine, 0,
                              *_context->getConfig()->getEnvironment())
              : process(_context, _cancel, ws2s(fileName), newCmdLine, 0,
                        *_context->getConfig()->getEnvironment()));
      LOG(log_info, "After running process");

      sessionResult = pr.status();

      switch (pr.status()) {
        case SessionResult::normal:
          if (_context->getConfig()->hasZipFile()) zipFiles();
          break;
        case SessionResult::cancel: {
          LOG(log_info, "Session "
                            << _context->getSessionConfig()->getSessionId()
                            << " canceling - creating error file "
                            << _context->getSessionConfig()->getErrorsFile());
          std::fstream ofs(_context->getConfig()->errorsFile().c_str(),
                           ios_base::out | ios_base::app);
          if (ofs.is_open()) {
            ofs << "Session canceled by user";
            LOG(log_info, "Session canceled by user");

          } else {
            LOG(log_info, "Session "
                              << _context->getSessionConfig()->getSessionId()
                              << " canceling - could not create error file "
                              << _context->getSessionConfig()->getErrorsFile()
                              << " GetLastError: " << GetLastError());
          }
          break;
        }
        case SessionResult::timeout: {
          std::fstream ofs(
              _context->getSessionConfig()->getErrorsFile().c_str(),
              ios_base::out | ios_base::app);
          if (ofs.is_open()) {
            ofs << "Session timedout - client no longer waiting for result (no "
                   "heartbeat)";
            LOG(log_info,
                "Session timedout - client no longer waiting for result (no "
                "heartbeat)");
          } else {
            LOG(log_info, "Can't open error file for appending");
            // todo: handle error - can't open errors file for appending
          }
        } break;
        default:
          LOG(log_error, "Unknown process status: " << pr.status());
          assert(false);
          break;
      }
      // then zip

    } else
      sessionResult = SessionResult::failed;

    if (_context->getSessionConfig()->hasEndRunFile()) {
      LOG(log_info, "****Writing end file: "
                        << _context->getSessionConfig()->getEndRunFile());
      // todo - handle file error
      std::ofstream ofs(_context->getSessionConfig()->getEndRunFile().c_str());

      LOG(log_info, "done!");
      ofs << sessionResult << std::endl;
    }

  } catch (const RunProcessException& e) {
    LOG(log_error, "RunProcessException: " << e.message());
  } catch (ConfigurationException& e) {
    LOG(log_error, "ConfigurationException: " << e.what());
  }
}

const ProcessResult ProcessingThread::processAsUser(
    const std::string& userName, const std::string& domain,
    const std::string& password, const std::string& processFileName,
    const std::string& cmdLine, const std::string* startingDirectory,
    const Environment& env) throw(RunProcessException) {
  LOG(log_info,
      "[" << userName << "] \"" << processFileName << " - " << cmdLine << "\"");
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  LPCSTR startingDir = startingDirectory != 0 ? startingDirectory->c_str() : 0;

  HANDLE token;

  /*
  // boosting privileges, not needed when called from a service
  HANDLE hCurrent = GetCurrentProcess();
  HANDLE currentToken;
  OpenProcessToken( hCurrent, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
  &currentToken );

  SetPrivilege( currentToken, SE_ASSIGNPRIMARYTOKEN_NAME, true );
  SetPrivilege( currentToken, SE_INCREASE_QUOTA_NAME, true );
  */

  if (!LogonUser(s2ws(userName).c_str(),
                 (domain.empty() ? 0 : s2ws(domain).c_str()),
                 s2ws(password).c_str(), LOGON32_LOGON_NETWORK,
                 LOGON32_PROVIDER_DEFAULT, &token)) {
    LOG(log_error, "LogonUser failed with error: " << GetLastError);
    throw RunProcessException("LogonUser failed with error", cmdLine,
                              GetLastError());
  }

  HANDLE newToken;
  if (!ImpersonateLoggedOnUser(token)) {
    LOG(log_error,
        "ImpersonateLoggedOnUser failed with error: " << GetLastError);
    throw RunProcessException(
        "ImpersonateLoggedOnUser failed with error: ", cmdLine, GetLastError());
  }
  /*
  LPVOID envBlock;
  bool b = CreateEnvironmentBlock( &envBlock, token, true ) != 0;
  TCHAR profileDir[ 2000 ];
  DWORD size( 1990 );
  GetUserProfileDirectory( token, profileDir, &size );
  PROFILEINFO profileInfo;
  LoadUserProfile( token, &profileInfo );
  */
  if (!DuplicateTokenEx(token, MAXIMUM_ALLOWED, 0, SecurityImpersonation,
                        TokenPrimary, &newToken)) {
    LOG(log_error, "DuplicateTokenEx failed with error: " << GetLastError());
    throw RunProcessException("DuplicateTokenEx failed with error: ", cmdLine,
                              GetLastError());
  }

  if (CreateProcessAsUser(
          newToken, s2ws(processFileName).c_str(),
          const_cast<LPTSTR>(s2ws(cmdLine).c_str()),  // Command line.
          NULL,                       // Process handle not inheritable.
          NULL,                       // Thread handle not inheritable.
          TRUE,                       // Set handle inheritance to FALSE.
          0,                          // No creation flags.
          env,                        // Use parent's environment block.
          s2ws(startingDir).c_str(),  // Use parent's starting directory.
          &si,                        // Pointer to STARTUPINFO structure.
          &pi)  // Pointer to PROCESS_INFORMATION structure.
  ) {
    LOG(log_info, "completed, before timeoutHandler");
    ProcessSessionController psc(pi.hProcess, pi.hThread);
    return ProcessResult(timeoutHandler(_context, _cancel, psc),
                         psc.getExitCode());
  } else {
    LOG(log_error, "CreateProcessAsUser failed with error: " << GetLastError());
    throw RunProcessException(processFileName, cmdLine, GetLastError());
  }
}

SessionResult timeoutHandler(SessionContextPtr context, bool& _cancel,
                             const SessionController& sessionController) {
  SessionResult status = SessionResult::normal;

  Timer heartBeatTimer;
  Timer reverseHeartBeatTimer;

  for (bool run = true; run;) {
    if (!sessionController.isActive()) {
      // logging
      LOG(log_info, PROCESSING_THREAD
                        << "Session "
                        << context->getSessionConfig()->getSessionId()
                        << " process has finished processing, exiting")
      break;
    }

    if (fileExists(context->getSessionConfig()->getHeartBeatFile())) {
      LOG(log_debug, "process - hearbeat event");
      DeleteFile(s2ws(context->getSessionConfig()->getHeartBeatFile()).c_str());
      heartBeatTimer.restart();
    } else if (heartBeatTimer.elapsed() >
               context->getConfig()->heartBeatTimeout()) {
      LOG(log_info, "heart beat elapsed: " << heartBeatTimer.elapsed());

      LOG(log_info, "Session " << context->getSessionConfig()->getSessionId()
                               << " has not received heart beat signal within "
                               << context->getConfig()->heartBeatTimeout()
                               << " seconds, terminating session");
      // no heartbeat within the heartbeat timout, cancel run
      sessionController.terminate();
      status = SessionResult::timeout;
      break;
    }

    bool fe = fileExists(context->getSessionConfig()->getCancelFile());
    if (fe || _cancel) {
      // received a cancel signal
      LOG(log_info, PROCESSING_THREAD
                        << "Session "
                        << context->getSessionConfig()->getSessionId()
                        << " received cancel signal through "
                        << (fe ? "cancel file" : "cancel method call"));

      sessionController.terminate();
      status = SessionResult::cancel;
      break;
    }

    if (reverseHeartBeatTimer.elapsed() >
            context->getConfig()->reverseHeartBeatPeriod() &&
        !fileExists(context->getSessionConfig()->getReverseHeartBeatFile())) {
      LOG(log_debug, "reverse heartbeat");
      ofstream rhb(
          context->getSessionConfig()->getReverseHeartBeatFile().c_str());
      rhb << "reverse heart beat";
      reverseHeartBeatTimer.restart();
    }
    Sleep(50);
  }

  LOG(log_debug, "returning ProcessResult");
  return status;
}

bool ProcessingThread::SetPrivilege(HANDLE hToken, char const* privilege,
                                    bool enable) {
  TOKEN_PRIVILEGES tp;
  LUID luid;

  if (!LookupPrivilegeValue(NULL,
                            s2ws(privilege).c_str(),  // privilege to lookup
                            &luid  // receives LUID of privilege
                            )) {
    printf("LookupPrivilegeValue error: %u\n", GetLastError());
    return false;
  }

  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  if (enable)
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  else
    tp.Privileges[0].Attributes = 0;

  // Enable the privilege or disable all privileges.
  if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
                             (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
    printf("AdjustTokenPrivileges error: %u\n", GetLastError());
    return false;
  }

  return true;
}

ProcessingThread::ProcessingThread(SessionContextPtr context) try
    : _context(context),
      Thread("Processing thread"),
      _cancel(false) {
  { LOG(log_debug, "in Processingthread constructor"); }
} catch (const ConfigurationException& e) {
  LOG(log_error, "ConfigurationException: " << e.what());
} catch (const std::exception& e) {
  LOG(log_error, "std::exception: " << e.what());
} catch (...) {
  LOG(log_info, "unknown exception");
  //	std::cout << context->getCmdLineString() << std::endl;
}
