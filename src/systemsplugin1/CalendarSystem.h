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

#pragma once

class CalendarSystem : public BarSystem<CalendarSystem> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  CalendarSystem(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<CalendarSystem>(Info("510D5B99-31C8-4117-8E19-6C08D4B78844",
                                       "May-October - by odelys in WL", ""))

  {}

 public:
  ~CalendarSystem(void) {}

  virtual void run() {
    installProfitTarget(5);
    installStopLoss(5);

    // state 0 - we are somewhere whithin the buying period
    // state 1 - we are somewhere in the shorting period
    int state = 0;

    for (Index bar = 0; bar < size(); bar++) {
      applyAutoStops(bar);

      Date d = date(bar);

      if (d.month() == 5 && state == 1) {
        closeAllShortAtMarket(bar + 1, "closing all short at market");
        buyAtMarket(bar + 1, 1000, "buy at market");

        state = 0;
      }

      if (d.month() == 10 && state == 0) {
        closeAllLongAtMarket(bar + 1, "closing all long");
        shortAtMarket(bar + 1, 1000, "short at market");

        state = 1;
      }

      Series s;

      //		s[5;
    }
  }
};
