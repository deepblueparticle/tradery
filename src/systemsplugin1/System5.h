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

/**
 * IMPORTANT DISCLAIMER
 *
 * These sample trading systems are presented for illustrative purpose only and
 * are not intended for use in real trading.
 *
 */

#pragma once

/**
 * Shows a basic use for data and series synchronization code.
 * It also shows how to get data for a symbol different than the default symbol
 */
class System5 : public BarSystem<System5> {
 public:
  System5(const std::vector<std::string>* params = 0)
      : BarSystem<System5>(Info(
            "6A01AF7B-C49C-4334-8C89-DFB99637963D", "System 5 - multi-symbol",
            "Illustrates how to create a system that uses more than one "
            "symbol, and how to synchronize data from multiple symbols")) {}

  virtual bool init(const std::string& defSymbol) {
    // reset the symbols iterator, so it will point to the first symbol
    resetSymbols();
    // get the first symbol
    // STRING firstSymbol = GET_NEXT_SYMBOL;
    String firstSymbol("yhoo");

    // only process if the current symbol is not the first symbol
    if (defSymbol != firstSymbol) {
      // get data for the first symbol
      Bars firstSymbolData = bars(firstSymbol);

      if (firstSymbolData) {
        // if data available
        // create a data synchronizer for the first symbol. The refernce is the
        // current symbol
        firstSymbolData.synchronize(DEF_BARS);
        // create a series synchronizer for the sma 5 of the closing prices of
        // the first symbol
        Series ss = firstSymbolData.closeSeries().SMA(5);
        //        SeriesSynchronizer ss( ds, CLOSE_SERIES( firstSymbolData
        //        ).SMA( 5 ) );

        // add the current close series to the synchronized first symbol close
        // series
        Series sum = CLOSE_SERIES + ss;

        // print to the output window some of the values of the two close prices
        // and the sum
        for (size_t n = 0; n < 20; n++)
          PRINT_LINE(time(n).to_simple_string()
                     << ", " << close(n) << ", " << ss[n] << ", " << sum[n]);
      }
    }
    return true;
  }

  virtual void run() {}

  void onBar(Index index) {}
};
