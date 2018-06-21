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
 * Illustrates how to set an automatic break even stop exit strategy
 */
class System3 : public BarSystem<System3> {
 public:
  System3(const std::vector<std::string>* params = 0)
      : BarSystem<System3>(Info("5BCB483E-5D13-4934-AA37-00D8B28D0EA6",
                                "System 3 - automatic exit strategy",
                                "Illustrates how to install one of the "
                                "automatic exit strategies, in this case break "
                                "even stop")) {}

  virtual void onBar(Index bar) {
    // processes all auto exit strategies, in this case we only have break even
    // stop 1%
    APPLY_AUTO_STOPS(bar);
    // entry a position at market
    SHORT_AT_MARKET(bar + 1, 1000, "short entry");

    if (CLOSE_SERIES.SMA(12).crossUnder(bar, CLOSE_SERIES)) {
    }
  }

  virtual void run() {
    // install the break even stop 1%, will be executed in the dataHandler
    INSTALL_BREAK_EVEN_STOP(1);
    // run the data handler for each bar
    FOR_EACH_BAR(0);
  }
};
