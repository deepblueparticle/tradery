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

class LongShortKeltnerBands : public BarSystem<LongShortKeltnerBands> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  LongShortKeltnerBands(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<LongShortKeltnerBands>(Info(
            "00434C1B-A8B3-4565-B096-1E38C713294D", "LongShortKeltnerBands - ",
            "WL name: A lindq type long/short limit system using Keltner "
            "Bands"))

  {}

 public:
  ~LongShortKeltnerBands(void) {}

  void run() {
    int days = 3;
    int var1 = 30;
    int var2 = 9;
    double profitTarget = 14;
    int start = var1 + var2;

    Series factor1 = highSeries() - lowSeries();
    Series factor2 = (closeSeries() + highSeries() + lowSeries()) / 3;
    Series factor3 = factor1.EMA(var1);
    Series factor4 = factor2.EMA(var2);

    Series kUp = factor4 + factor3;
    Series kDown = factor4 - factor3;

    Pane defPane = getDefaultPane();
    defPane.drawSeries("kDown", kDown);
    defPane.drawSeries("kUp", kUp);

    Series buyLimitPrice = kDown * 0.85;
    Series shortLimitPrice = kUp * 1.12;

    installProfitTarget(profitTarget);
    installTimeBasedExit(days);

    for (Index bar = start + 1; bar < size(); bar++) {
      applyAutoStops(bar);

      buyAtLimit(bar + 1, buyLimitPrice[bar], 1000, "buy at limit");
      shortAtLimit(bar + 1, shortLimitPrice[bar], 1000, "short at limit");
    }
  }
};
