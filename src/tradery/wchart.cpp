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
#include "wchart.h"

WebBarsChart* WebChartManager::createWebChart(const std::string& name,
                                              const std::string& symbol) {
  WebBarsChart* chart =
      new WebBarsChart(name, symbol, _chartRootPath, _reduced);
  // the default pane name is the symbol for the time being
  chart->createDefaultPane("Prices");

  return chart;
}

std::string getUniqueFileName() {
  static Mutex _mx;
  Lock lock(_mx);

  static unsigned int ix = 1;

  std::ostringstream os;

  os << ix++;

  return os.str();
}

WebChartManager::WebChartManager(const std::string& name,
                                 const std::string& symbolsToChartFile,
                                 const std::string& chartRootPath,
                                 const std::string& chartDescriptionFile,
                                 bool reduced)
    : _chartRootPath(chartRootPath),
      _chartsDescriptionFile(chartDescriptionFile),
      _reduced(reduced) {
  try {
    if (!symbolsToChartFile.empty() && !chartDescriptionFile.empty()) {
      // get the list of symbols
      StrListPtr symbols = getSymbols(symbolsToChartFile, true);
      assert(symbols.get());

      LOG(log_info, "---------symbols to chart: " << symbolsToChartFile);
      LOG(log_info, "symbols count: " << symbols->size());

      std::string list;
      for (StrList::const_iterator i = symbols->begin(); i != symbols->end();
           i++) {
        list += *i + " ";
        __super::addChart(*i, createWebChart(name, *i));
      }
      LOG(log_info, "symbols: " << list);

      LOG(log_info, "---- chart Description File: " << chartDescriptionFile);
      LOG(log_info, "---- chart root path: " << chartRootPath);
    }

  } catch (const FileSymbolsParserException&) {
    // could not open the symbols file
    throw ChartManagerException();
  }
}
