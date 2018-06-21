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

// disallowed keywords

#include "runtimeproj.h"

// disabled should be right before defines
#include "disabled.h"
#include "defines.h"
#include <windows.h>

class RunnablePluginImpl : public SimplePlugin<Runnable> {
 public:
  virtual void init() { PLUGIN_INIT_METHOD }

 public:
  RunnablePluginImpl()
      : SimplePlugin<Runnable>(Info("Sample systems plugin",
                                    "This plugin illustrates some of the "
                                    "techniques employed in writing plugins, "
                                    "and more specifically system (or "
                                    "Runnable) plugins")) {}
};

RunnablePluginImpl rplugin;

PLUGIN_API tradery::RunnablePlugin* getRunnablePlugin() { return &rplugin; }

//////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      rplugin.init();
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      //		uninitRunnables();
      break;
  }
  return TRUE;
}
