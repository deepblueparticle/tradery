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

class AdaptiveBBands : public BarSystem<AdaptiveBBands> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  AdaptiveBBands(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<AdaptiveBBands>(Info("563D4F4A-7BA8-432f-9F49-71EF3455DCBB",
                                       "AdaptiveBBands - WL - A MultiPosition "
                                       "Buy-At-Limit System using Adaptive "
                                       "BBands",
                                       ""))

  {}

 public:
  ~AdaptiveBBands(void) {}

  virtual void run() {
    Series mt2(size());
    double de = 1.5;
    int period = 3;
    double pp = ((double)(period - 1)) / ((double)(period + 1));

    Series ut = closeSeries().EMA(period).EMA(period);

    PrintLine(0);
    double x1 = ((double)(2 * period)) / ((double)(period + 1));
    Series dt = ((ut * x1) - ut) / pp;

    Series xseries = closeSeries() - dt;

    for (Index bar = period + 1; bar < size(); bar++) {
      double x = abs(xseries[bar]);
      mt2[bar] = x;
    }
    PrintLine(2);

    Series dt2 = mt2.EMA(period).EMA(period) * (x1 - 1) / pp;

    Series xSeries = dt2 * de;
    Series BBUpper = dt + xSeries;
    Series BBLower = dt - xSeries;

    double lowFactor = 0.85;
    double highFactor = 1.15;

    Series buyLimitPrice = BBLower * lowFactor;
    Series shortLimitPrice = BBUpper * highFactor;

    // charting the
    Pane defPane = getDefaultPane();
    defPane.drawSeries("buy limit price", buyLimitPrice);
    defPane.drawSeries("short limit price", shortLimitPrice);

    installProfitTarget(5);

    for (Index bar = 20; bar < size(); bar++) {
      // first apply exit rules
      applyAutoStops(bar);

      for (Position pos = getFirstOpenPosition(); pos;
           pos = getNextOpenPosition()) {
        if (pos.isOpen()) {
          if (pos.isLong() AND close(bar) >= dt[bar])
            sellAtMarket(bar + 1, pos, "Sell at market");

          if (pos.isShort() AND close(bar) <= dt[bar])
            coverAtMarket(bar + 1, pos, "Cover at market");
        }
      }

      buyAtLimit(bar + 1, buyLimitPrice[bar], 1000, "");
      shortAtLimit(bar + 1, shortLimitPrice[bar], 1000, "");
    }
  }
};
