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
#include <macros.h>
#include "systemsplugin1.h"
#include "system1.h"
#include "system2.h"
#include "system3.h"
#include "system4.h"
#include "system5.h"
#include "system6.h"
#include "system7.h"
#include "system8.h"
#include "system11.h"
#include "systemTemplate.h"
#include "bollingerbandsystem.h"
#include "exttradessystem.h"
#include "simpletradingsystem.h"
#include "mama.h"
#include "adaptivebbands.h"
#include "lindqlongshort.h"
#include "calendarsystem.h"

#include "IBCommission.h"
#include "sampleslippage.h"
#include "mfi.h"

class RunnablePluginImpl : public SimplePlugin<Runnable> {
 public:
  virtual void init() {
    insert<System1>();
    insert<System2>();
    insert<System3>();
    insert<System4>();
    insert<System5>();
    insert<System6>();
    insert<System7>();
    insert<System8>();
    insert<System11>();
    insert<BollingerBandSystem>();
    insert<ExtTradesSystem>();
    insert<SimpleTradingSystem>();
    insert<BollingerBandClassicSystem>();
    insert<MAMA_ttrcrep>();
    insert<AdaptiveBBands>();
    insert<LongShortKeltnerBands>();
    insert<MFISystem>();
    insert<CalendarSystem>();
  }

 public:
  RunnablePluginImpl()
      : SimplePlugin<Runnable>(Info(
            "cd365c65-91aa-11d9-b3a4-000c6e067d35", "Sample systems plugin",
            "This plugin illustrates some of the techniques employed in "
            "writing plugins, and more specifically system (or Runnable) "
            "plugins")) {}
};

///////////////////////////////////////////////////////////////////
// commission plugin
class SampleCommissionPlugin : public SimplePlugin<Commission> {
 public:
  virtual void init() { insert<IBCommission>(); }

  SampleCommissionPlugin()
      : SimplePlugin<Commission>(Info(
            "D47E020B-FFB2-46e5-97D5-11F23C3014A2", "Sample commission plugin",
            "Sample commission plugin - for the moment just an Interactive "
            "Brokers commission model")) {}
};

////////////////////////////////////////////////////////////////
// slippage plugin
class SampleSlippagePlugin : public SimplePlugin<Slippage> {
 public:
  virtual void init() { insert<SampleSlippage>(); }

  SampleSlippagePlugin()
      : SimplePlugin<Slippage>(Info("D5DD3FA7-52D2-45fc-9AAB-8F661CBF1FF1",
                                    "Sample slippage plugin",
                                    "Sample slippage plugin - for the moment "
                                    "just one very simple configuration")) {}
};

SampleCommissionPlugin cplugin;
SampleSlippagePlugin splugin;
RunnablePluginImpl rplugin;

PLUGIN_API tradery::SlippagePlugin* getSlippagePlugin() { return &splugin; }

PLUGIN_API tradery::CommissionPlugin* getCommissionPlugin() { return &cplugin; }

PLUGIN_API tradery::RunnablePlugin* getRunnablePlugin() { return &rplugin; }

//////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      rplugin.init();
      cplugin.init();
      splugin.init();
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
