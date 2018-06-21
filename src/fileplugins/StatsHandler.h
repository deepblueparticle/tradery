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

#include <log.h>

class StatsHandler : public SignalHandler, public CurrentPriceSource {
 private:
  StatsCalculator _totalStats;
  StatsCalculator _shortStats;
  StatsCalculator _longStats;
  StatsCalculator _buyHoldStats;

  std::auto_ptr<EquityCurve> _ec;
  std::auto_ptr<EquityCurve> _bhEc;

  std::auto_ptr<DrawdownCurve> _totalDC;
  std::auto_ptr<DrawdownCurve> _shortDC;
  std::auto_ptr<DrawdownCurve> _longDC;
  std::auto_ptr<DrawdownCurve> _bhDC;
  mutable PositionsContainerPtr _bhPos;

 public:
  StatsHandler(const Info& info)
      : SignalHandler(info),
        _totalStats(*this),
        _shortStats(*this),
        _longStats(*this),
        _buyHoldStats(*this) {
    //    std::cout << "in StatsHandler constr" << std::endl;
  }

#define DD_STEPS 4
#define STAT_STEPS 4
#define EQ_STEPS 2

  double getEqPct() const { return 16; }

  double getStatsPct() const { return 4; }

  double getPct() const { return getEqPct() + getStatsPct(); }

  double getStatsStep() const { return getStatsPct() / 4; }

  double getDateRangeStep() const { return getEqPct() / 8; }

  double getEqStep() const { return getEqPct() * 5.0 / 16.0; }

  double getDDStep() const { return getEqPct() / 16; }

  virtual void sessionStarted() {
    // this will make this module show span 20%
    if (sessionInfo().runtimeParams()->statsEnabled()) {
      LOG(log_info, "stats enabled");
      sessionInfo().runtimeStats()->addPct(getPct());
    } else if (sessionInfo().runtimeParams()->equityCurveEnabled() ||
               sessionInfo().runtimeParams()->tradesEnabled()) {
      LOG(log_info, "only equity enabled");
      sessionInfo().runtimeStats()->addPct(getEqPct());
    }
  }

  /*
  void sessionEnded( PositionsContainer& positions )
  {
    RuntimeStats& rts = *sessionInfo().runtimeStats();
    std::cout << _T( "Calculating equity date range" ) << std::endl;
    rts.setStatus( _T( "Calculating equity date range" ) );
    EquityDateRange edr( __super::sessionInfo() );
    rts.step( getDateRangeStep() );

    std::cout << _T( "[StatsHandler::sessionEnded] 1" ) << std::endl;
    calcEqCurve( edr, positions );
    std::cout << _T( "[StatsHandler::sessionEnded] 2" ) << std::endl;
    calcStats( edr, positions );
    std::cout << _T( "[StatsHandler::sessionEnded] 3" ) << std::endl;
    calcScore();

    // calculate eq curve eventually
  }
  */

  void calcScore() {
    LOG(log_info, "Calculating scores");
    _totalStats.calculateScore(_ec->getTotalPctExposure(),
                               _totalDC->ulcerIndex());
    _shortStats.calculateScore(_ec->getShortPctExposure(),
                               _shortDC->ulcerIndex());
    _longStats.calculateScore(_ec->getLongPctExposure(), _longDC->ulcerIndex());
    _buyHoldStats.calculateScore(_bhEc->getTotalPctExposure(),
                                 _bhDC->ulcerIndex());
  }

  void calcStats(const DateRange& dateRange, PositionsContainer& positions) {
    LOG(log_info, "Calculating stats");
    sessionInfo().runtimeStats()->setMessage("Calculating statistics");

    double initialCapital =
        sessionInfo().runtimeParams()->positionSizing()->initialCapital();

    Timer timer;
    LOG(log_info, "calculating long + short stats");
    //    sessionInfo().runtimeStats()->setStatus( _T( "Calculating overall
    //    stats" ) );
    _totalStats.setDateRange(dateRange);
    _totalStats.setInitialCapital(initialCapital);
    _totalStats.calculateAll(positions);
    _totalStats.setEndingCapital(_ec->getEndingTotalEquity());
    sessionInfo().runtimeStats()->step(getStatsStep());

    LOG(log_info, "done long + short: " << timer.elapsed());
    //    sessionInfo().runtimeStats()->setStatus( _T( "Calculating long stats"
    //    ) );
    std::cout << "calculating long stats" << std::endl;
    _longStats.setDateRange(dateRange);
    _longStats.setInitialCapital(initialCapital);
    _longStats.calculateLong(positions);
    _longStats.setEndingCapital(_ec->getEndingLongEquity());
    sessionInfo().runtimeStats()->step(getStatsStep());

    LOG(log_info, "done long: " << timer.elapsed());
    //    sessionInfo().runtimeStats()->setStatus( _T( "Calculating short stats"
    //    ) );
    std::cout << "calculating short stats" << std::endl;
    _shortStats.setDateRange(dateRange);
    _shortStats.setInitialCapital(initialCapital);
    _shortStats.calculateShort(positions);
    _shortStats.setEndingCapital(_ec->getEndingShortEquity());
    sessionInfo().runtimeStats()->step(getStatsStep());

    LOG(log_info, "done short: " << timer.elapsed());
    //    sessionInfo().runtimeStats()->setStatus( _T( "Calculating buy and hold
    //    stats" ) );
    std::cout << "calculating b&h stats" << std::endl;
    _buyHoldStats.setDateRange(dateRange);
    _buyHoldStats.setInitialCapital(initialCapital);
    _buyHoldStats.calculateAll(getBHPositions());
    _buyHoldStats.setEndingCapital(_bhEc->getEndingTotalEquity());
    sessionInfo().runtimeStats()->step(getStatsStep());

    LOG(log_info, "done b&h: " << timer.elapsed());
  }

  void calcEqCurve(const DateRange& dateRange, PositionsContainer& positions) {
    assert(sessionInfo().runtimeStats() != 0);
    RuntimeStats& rts = *sessionInfo().runtimeStats();

    if (_ec.get() == 0) {
      rts.setMessage("Calculating equity curve");
      rts.setStatus(RuntimeStatus::RUNNING);
      LOG(log_info, "Calculating equity curve for all positions");
      //      rts.setStatus( _T( "Calculating equity curve for generated
      //      positions" ) );
      _ec = std::auto_ptr<EquityCurve>(
          new EquityCurve(dateRange, sessionInfo(), positions, true));
      rts.step(getEqStep());
    }

    if (_bhEc.get() == 0) {
      rts.setMessage("Calculating Buy and Hold equity curve");
      rts.setStatus(RuntimeStatus::RUNNING);
      LOG(log_info, "Calculating Buy & Hold equity curve: ");
      //      rts.setStatus( _T( "Calculating equity curve for buy and hold" )
      //      );
      _bhEc = std::auto_ptr<EquityCurve>(
          new EquityCurve(dateRange, sessionInfo(), getBHPositions(), false));
      rts.step(getEqStep());
    }

    if (_totalDC.get() == 0) {
      rts.setMessage("Calculating total drawdown");
      rts.setStatus(RuntimeStatus::RUNNING);
      LOG(log_info, "Calculating total drawdown");
      //      rts.setStatus( _T( "Calculating drawdown for all positions" ) );
      _totalDC = std::auto_ptr<DrawdownCurve>(new TotalDrawdownCurve(*_ec));
      rts.step(getDDStep());
    }
    if (_shortDC.get() == 0) {
      rts.setMessage("Calculating short drawdown");
      rts.setStatus(RuntimeStatus::RUNNING);
      LOG(log_info, "Calculating short drawdown");
      //      rts.setStatus( _T( "Calculating drawdown for short positions" ) );
      _shortDC = std::auto_ptr<DrawdownCurve>(new ShortDrawdownCurve(*_ec));
      rts.step(getDDStep());
    }

    if (_longDC.get() == 0) {
      rts.setMessage("Calculating long drawdown");
      LOG(log_info, "Calculating long drawdown");
      //      rts.setStatus( _T( "Calculating drawdown for long positions" ) );
      _longDC = std::auto_ptr<DrawdownCurve>(new LongDrawdownCurve(*_ec));
      rts.step(getDDStep());
    }
    if (_bhDC.get() == 0) {
      rts.setMessage("Calculating Buy and Hold drawdown");
      rts.setStatus(RuntimeStatus::RUNNING);
      LOG(log_info, "Calculating b&h drawdown");
      //      rts.setStatus( _T( "Calculating drawdown for buy and hold" ) );
      _bhDC = std::auto_ptr<DrawdownCurve>(new TotalDrawdownCurve(*_bhEc));
      rts.step(getDDStep());
    }
  }

  PositionsContainerPtr makeBHPositions() const {
    Timer totalTimer;
    Timer dataTimer;

    double dataTime = 0;

    tradery::SymbolsIterator* si = symbolsIterator();
    SymbolConstPtr symbol;

    // reset the symbols iterator
    si->reset();

    // create a positions container and positions manager
    // they will contain positions that will correspond to buy and hold for the
    // whole back-testing period
    PositionsContainerPtr pc(PositionsContainer::create());
    PositionsManagerAbstrPtr pm(PositionsManagerAbstr::create(
        pc.get(), NotADateTime(), NotADateTime()));

    // the number of symbols
    unsigned int symbolsCount = 0;

    // todo: we should not be counting symbols this way, instead he symbols
    // source should return the size  todo: this will not work in the case of
    // dynamic datasources, or the results may be false
    // calculate the number of symbols
    while ((symbol = si->getNext()).get() != 0) symbolsCount++;

    // reset the symbols iterator again
    si->reset();

    // use a simple b&h position sizing strategy - each position has a size =
    // total capital/ total symbols
    double capitalPerPosition =
        sessionInfo().runtimeParams()->positionSizing()->initialCapital() /
        symbolsCount;

    // go through the list of all symbols
    while ((symbol = si->getNext()).get() != 0) {
      // get data for the current symbol
      try {
        dataTimer.restart();
        BarsPtr data = sessionInfo().getData(symbol->symbol());
        Bars bars(dynamic_cast<const BarsAbstr*>(data.get()));
        dataTime += dataTimer.elapsed();

        if (bars) {
          // calculate position size
          double posSize = capitalPerPosition / bars.open(0);
          // buy at market on the first bar
          pm->buyAtMarket(bars, 0, (unsigned long)posSize, "");
          // sell at close on the last bar
          if (pc->getLastPosition())
            pm->sellAtClose(bars, bars.size() - 1, pc->getLastPosition(), "");
        }
      } catch (...) {
        // todo: handle data errors - maybe catch all exceptions and ignore
        // them, we don't care why the data is not available, just don't add
        // that data to final stats
      }
      // for now just assume everything is bars
      // todo: will have to see how to handle an arbitrary datatype
    }
    return pc;
  }

  PositionsContainer& getBHPositions() const {
    if (!_bhPos) _bhPos = makeBHPositions();

    return *_bhPos;
  }

  virtual double get(const std::string& symbol) const
      throw(DataNotAvailableForSymbolException) {
    // make sure this is called after sessionStarted
    BarsPtr data = getData(symbol);
    // we'll assume data is bars for now
    // todo: make it work for all types of data
    Bars bars(dynamic_cast<const BarsAbstr*>(data.get()));
    if (bars)
      return bars.close(bars.size() - 1);
    else
      throw DataNotAvailableForSymbolException();
  }

  const Stats& totalStats() const { return _totalStats; }
  const Stats& shortStats() const { return _shortStats; }
  const Stats& longStats() const { return _longStats; }
  const Stats& bhStats() const { return _buyHoldStats; }

  const EquityCurve& equityCurve() const {
    assert(_ec.get() != 0);
    return *_ec;
  }
  const EquityCurve& bhEquityCurve() const {
    assert(_bhEc.get() != 0);
    return *_bhEc;
  }

  const DrawdownCurve& totalDrawdownCurve() const {
    assert(_totalDC.get() != 0);
    return *_totalDC;
  }
  const DrawdownCurve& shortDrawdownCurve() const {
    assert(_shortDC.get() != 0);
    return *_shortDC;
  }
  const DrawdownCurve& longDrawdownCurve() const {
    assert(_longDC.get() != 0);
    return *_longDC;
  }
  const DrawdownCurve& bhDrawdownCurve() const {
    assert(_bhDC.get() != 0);
    return *_bhDC;
  }
};
