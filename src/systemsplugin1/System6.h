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

class System6 : public BarSystem<System6> {
 public:
  //  constructor of the System1 class
  // does one time initialization of the class, which whill hold for the life of
  // the class, indicated by the "const" qualifier associated with the
  // variables. Per run intialization should be done in the method "init".
  System6(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<System6>(Info("8C12F9DE-90B7-4080-BCAA-C2ED4F9D6CF3",
                                "System 6 - ATR band system with anticipation",
                                ""))

  {}

  ~System6() {}

  unsigned int _days;
  double _multiplier;
  int _emaPeriod;
  int _atrPeriod;
  int _profitTarget;
  Series _atrLow;
  Series _trigger;
  int _start;

  // this is called by the framework before running the system.
  // used to do initializations for each run on a symbol
  virtual bool init(const std::wstring& symbol) {
    _days = 3;
    _multiplier = 0.83;
    _emaPeriod = 9;
    _atrPeriod = 30;
    _profitTarget = 13;
    _start = _emaPeriod + _atrPeriod;

    _atrLow = DEF_BARS.AvgPrice().EMA(_emaPeriod) - DEF_BARS.ATR(_atrPeriod);
    _trigger = _atrLow * _multiplier;

    INSTALL_STOP_LOSS(20);
    INSTALL_PROFIT_TARGET(_profitTarget);
    return true;
  }

  // the run method - this is called by the framework after a succesful
  // initialization it just does forEachBar which in turn calls dataHandler for
  // all the available bars
  virtual void run() { FOR_EACH_BAR(_start); }

  // this is the buy at limit dataHandler - it is called before buy at limit is
  // submitted and gives the system a last chance to change it or even not
  // submit it at all
  // the return will indicate how many shares this order should be actually
  // placed for. If 0, no order will be submitted
  unsigned int onBuyAtLimit(size_t bar, unsigned int shares,
                            double price) const {
    return shares;
  }

  // called for each bar
  virtual void onBar(Index bar) {
    // apply auto stops - so check for profit target and holding period in our
    // case
    APPLY_AUTO_STOPS(bar);
    FOR_EACH_OPEN_POSITION(bar);

    double buyLimitPrice = _trigger[bar] + _trigger.Momentum(1)[bar];
    BUY_AT_LIMIT(bar + 1, buyLimitPrice, 1000, "");
  }

  virtual bool onOpenPosition(tradery::Position pos, Index bar) {
    if (bar - pos.getEntryBar() >= _days && pos.isLong())
      SELL_AT_MARKET(bar + 1, pos, "Long days expired");
    return true;
  }
};
