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

class StockFilter : public BarSystem<StockFilter> {
 public:
  DipBuyer(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<DipBuyer>(
            Info("50BF8555-934E-4e31-A68B-A76F69ECF021", "DipBuyer - ", ""))

            ~StockFilter(void) {}

  virtual bool init(const std::wstring& symbol) {
    if( DEF_BARS.AvgPrice().LinearRegAngle( 10 ) >
      PRINT_LINE( symbol )


      return false;
  }

  // the run method - this is called by the framework after a succesful
  // initialization it just does forEachBar which in turn calls dataHandler for
  // all the available bars
  virtual void run() { FOR_EACH_BAR(5); }

  // called for each bar
  virtual void onBar(INDEX bar) {
    // apply auto stops - so check for profit target and holding period in our
    // case
    APPLY_AUTO_STOPS(bar);

    BUY_AT_LIMIT(bar + 1, (1 - dip / 100) * close(bar), 1000,
                 "Buy at limit on dip");
    SHORT_AT_LIMIT(bar + 1, (1 + jump / 100) * close(bar), 1000,
                   "Short at limit on jump");

    //    FOR_EACH_OPEN_POSITION( bar );
  }

  /*	virtual bool onOpenPosition(Position pos, INDEX bar )
          {
      if( QQV.crossUnder( bar, QQVMedianCloseLong ) )
        SELL_AT_LIMIT( bar + 1, pos, high( bar ), "Sell at limit" );
      else if( QQV.crossUnder( bar, QQVMedianCloseShort ) )
        COVER_AT_LIMIT( bar + 1, pos, low( bar ), "Cover at limit" );

      return true;
          }
  */
};
