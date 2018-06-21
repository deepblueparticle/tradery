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

class MFISystem : public BarSystem<MFISystem> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  MFISystem(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<MFISystem>(Info("A39D47BC-8AFA-422b-B04E-6E0D31C2F86E",
                                  "Money Flow Index - WL", ""))

  {}

 public:
  ~MFISystem(void) {}

  virtual void run() {
    unsigned int period = 10;
    int lowThreshold = 5;
    int highThreshold = 100 - lowThreshold;

    Series mfi = MFI(period);

    installProfitTarget(5);
    installStopLoss(5);

    for (Index bar = period; bar < size(); bar++) {
      applyAutoStops(bar);

      bool highCond = mfi[bar - 1] <= lowThreshold AND mfi[bar] > lowThreshold;

      bool lowCond = mfi[bar - 1] >= highThreshold AND mfi[bar] < highThreshold;

      if (hasOpenPositions()) {
        Position pos = getLastOpenPosition();
        if (highCond AND pos.isLong())
          sellAtMarket(bar + 1, pos, "sell at market");

        if (lowCond AND pos.isShort())
          coverAtMarket(bar + 1, pos, "cover at market");
      } else {
        if (highCond) shortAtMarket(bar + 1, 1000, "buy at market");
        if (lowCond) buyAtMarket(bar + 1, 1000, "short at market");
      }
    }
  }
};
