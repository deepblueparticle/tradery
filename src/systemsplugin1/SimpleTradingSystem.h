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

#include <macros.h>

/**
 * This sample system does "something" within the open positions handler
 */
class SimpleTradingSystem : public BarSystem<System2> {
 private:
  // do something on each open position - close all open positions at limit $25
  virtual bool onOpenPosition(tradery::Position pos, Index bar) {
    sellAtLimit(bar + 1, pos, 25, "sell at limit");

    return true;
  }

 public:
  SimpleTradingSystem(const std::vector<std::string>* params = 0)
      : BarSystem<System2>(Info("E6D64E9E-3A59-4a99-9C1F-ABD300E9AFE3",
                                "Sample for a simple trading system",
                                "This is an example of a system class that "
                                "does \"something\" in the open positions "
                                "handler")) {}

  virtual void run() {
    // apply a loop for each bar
    for (Index bar = 0; bar < barsCount(); bar++) {
      // positions exit logic
      for (Position pos = getFirstOpenPosition(); pos;
           pos = getNextOpenPosition()) {
        PrintLine("pos entry price" << pos.getEntryPrice() << ", target price: "
                                    << pos.getEntryPrice() * 1.05);
        bool b = sellAtLimit(bar + 1, pos, pos.getEntryPrice() * 1.05,
                             "sell at limit");

        if (b) PrintLine("Position closed at limit: " << pos.getClosePrice());
      }

      // position entry logic
      buyAtLimit(bar + 1, close(bar) * 0.95, 1000, "buy at limit");
    }
  }
};
