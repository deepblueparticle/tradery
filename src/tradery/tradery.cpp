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
#include "tradery.h"
#include "Configuration.h"
#include "wdoc.h"
#include "runsystem.h"
#include "service.h"
#include "strings.h"
#include "ThriftServer.h"

int startSimulator(const Configuration& cmdLine);
int runSimulator(const Configuration& cmdLine);
int startService();

PluginTree globalPluginTree;
ConfigurationPtr config;

const PluginTree& getGlobalPluginTree() { return globalPluginTree; }

ConfigurationPtr getConfig() { return config; }

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define _TEST

using namespace std;

std::auto_ptr<Log> _log;

// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[]) {
  //  std::ostream& ofs( std::cout );
  int nRetCode = success;
  unsigned int count = 0;

  Log::setLogToConsole();

  LOG(log_debug, "start");
  // initialize MFC and print and error on failure
  if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
    // TODO: change error code to suit your needs
    LOG(log_debug, _T("Fatal Error: MFC initialization failed" ));
    nRetCode = mfc_init_error;
  } else {
    try {
#define COMMON_CONFIG "--configfile \"tradery.conf\" --configfile "
#if defined _DEBUG
#define CONFIG COMMON_CONFIG "\"tradery_debug.conf\""
#else
#define CONFIG COMMON_CONFIG "\"tradery_release.conf\""
#endif
      config = boost::make_shared<Configuration>(CONFIG, false);
      tradery::init(config->cacheSize());
    } catch (ConfigurationException& e) {
      LOG(log_error, "ConfigurationException: " << e.what());
      return config_error;
    }

    globalPluginTree.explore(config->getPluginPath(), config->getPluginExt(),
                             false, 0);

    try {
      Configuration cmdLine;

      LOG(log_debug, "after cmd line processing");

      if (cmdLine.uniqueId()) {
        LOG(log_debug, "unique Id: " << UniqueId().toString());
        nRetCode = success;
      } else if (cmdLine.installService()) {
        LOG(log_debug, "installing service");
        CmdInstallService(_T( "-Z" ));
        nRetCode = success;
      } else if (cmdLine.removeService()) {
        LOG(log_debug, "removing service");
        CmdRemoveService();
        nRetCode = success;
      } else if (cmdLine.debugService()) {
        LOG(log_debug, "debugging service");
        bDebug = true;
        CmdDebugService(argc, argv);
        nRetCode = success;
      } else if (cmdLine.startService()) {
        nRetCode = startService();
      }
      // TODO  fix running tradery from the command line
      /*

                     else if ( cmdLine.runSimulator() )
           {
                       nRetCode = runSimulator(cmdLine);
           }
           else
           {
                       nRetCode = startSimulator(cmdLine);
               }
               */
    } catch (const ConfigurationException& e) {
      LOG(log_debug, _T( "Exiting with ConfigurationException: " ) << e.what());
      nRetCode = cmd_line_error;
    } catch (exception& e) {
      LOG(log_debug, _T( "Exiting with exception: " ) << e.what());
      nRetCode = unknown_error;
    } catch (...) {
      LOG(log_debug, "Unknown error");
      nRetCode = unknown_error;
    }

    tradery::uninit();
  }

  return nRetCode;
}

int startSimulator(const Configuration& cmdLine) {
  int nRetCode = success;
  LOG(log_debug, "in simulator mode");
  char inbuf[8000];
  DWORD bytesRead;

  char buf[8000];

  std::string cl(cmdLine.getCmdLineString());
  strcpy(buf, cl.c_str());

  LOG(log_debug, "calling named pipe");

  if (!CallNamedPipe(lpszPipeName.c_str(), buf, cl.length() + 1, inbuf,
                     sizeof(inbuf), &bytesRead, NMPWAIT_WAIT_FOREVER)) {
    // todo: handle errors
    //          oss << "exiting with system_run_error" << std::endl;
    LOG(log_error, "calling named pipe failed" << GetLastError());
    nRetCode = system_run_error;
  } else {
    LOG(log_error, "calling named pipe succeeded");
    nRetCode = success;
  }

  return nRetCode;
}

int runSimulator(SessionContextPtr context) {
  int nRetCode = success;

  LOG(log_debug, "tradery.cpp in simulator");
  LOG(log_debug, "config: " << context->getConfig()->getCmdLineString());
  try {
    LOG(log_debug, "before run system");

    RunSystem runSystem(context);
    runSystem.run();
    LOG(log_debug, "after run system");

    nRetCode = success;
  } catch (const RunSystemException& e) {
    LOG(log_debug, "run system exception: " << e.errorCode());
    nRetCode = e.errorCode();
  }

  return nRetCode;
}

int startService() {
  LOG(log_debug, "starting service");

  SERVICE_TABLE_ENTRY dispatchTable[] = {
      {TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)service_main},
      {NULL, NULL}};
  if (!StartServiceCtrlDispatcher(dispatchTable)) {
    AddToMessageLog(TEXT("StartServiceCtrlDispatcher failed."));
  }
  return success;
}
