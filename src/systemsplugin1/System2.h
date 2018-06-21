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
#pragma warning(disable : 4482)

#include <macros.h>

/**
 * This sample system does "something" within the open positions handler
 */
class System2 : public BarSystem<System2> {
 private:
  // do something on each open position - close all open positions at limit $25
  virtual bool onOpenPosition(tradery::Position pos, Index bar) {
    sellAtLimit(bar + 1, pos, 25, "sell at limit");

    return true;
  }

 public:
  System2(const std::vector<std::string>* params = 0)
      : BarSystem<System2>(Info("1AB6A6A9-A13F-480e-9DAD-2F4D71D140FE",
                                "System 2 - example of open positions handler",
                                "This is an example of a system class that "
                                "does \"something\" in the open positions "
                                "handler")) {}

  virtual bool init(const std::string& symbol) {
    PrintLine(green << name() << tradery::Control::reset << italic << " in init"
                    << tradery::endl);

    return true;
  }

  virtual void cleanup() {
    PrintLine(blue << name() << tradery::Control::reset << bold << " in cleanup"
                   << endl);
  }

  virtual void onBar(Index bar) {
    // this will call onOpenPosition on each open position
    forEachOpenPosition(bar);
    // entry at limit
    buyAtLimit(bar + 1, low(bar) * 1.05, 1000, "buy at limit");
  }

  virtual void run() {
    // apply a loop for each bar
    forEachBar(0);
  }
};
