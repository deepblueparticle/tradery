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

#pragma warning(disable : 4786)
#include "stdafx.h"
#include "Positions.h"

/*
#ifdef _DEBUG
  #undef THIS_FILE
static char THIS_FILE[]=__FILE__;
  #define new DEBUG_NEW
#endif
*/

PositionId PositionImpl::_uniqueId = 1;
Mutex PositionImpl::_mx;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// TODO: implement the whole slippage thing - in case of stop or limit orders
// TODO: implement the exception handling thing for all errors such as trying to
// read
//  past the end of the series, or closing the wrong pos etc.
// TODO: do this for a limited number of bars,

void PositionsManagerImpl::applyTimeBasedAtMarket(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  int bars = barIndex - pos.getEntryBar();

  if (pos.isLong()) {
    if (bars >= _timeBasedExitAtMarket.getValue())
      sellAtMarket(bs, barIndex, pos, "Time based at market");
  } else {
    if (bars >= _timeBasedExitAtMarket.getValue())
      coverAtMarket(bs, barIndex, pos, "Time based at market");
  }
}

void PositionsManagerImpl::applyTimeBasedAtClose(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  int bars = barIndex - pos.getEntryBar();

  if (pos.isLong()) {
    if (bars >= _timeBasedExitAtClose.getValue())
      sellAtClose(bs, barIndex, pos, "Time based at close");
  } else {
    if (bars >= _timeBasedExitAtClose.getValue())
      coverAtClose(bs, barIndex, pos, "Time based at close");
  }
}

void PositionsManagerImpl::applyStopLoss(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  double EntryPrice = pos.getEntryPrice();

  if (pos.isLong()) {
    double stopLossPrice = EntryPrice * (1 - _stopLoss.getValue() / 100);

    if (pos.getEntryBar() != barIndex)
      sellAtStop(bs, barIndex, pos, stopLossPrice, "Stop loss");
  } else {
    double stopLossPrice = EntryPrice * (1 + _stopLoss.getValue() / 100);

    if (pos.getEntryBar() != barIndex)
      coverAtStop(bs, barIndex, pos, stopLossPrice, "Stop Loss");
  }
}

void PositionsManagerImpl::applyStopLossLong(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  double EntryPrice = pos.getEntryPrice();

  if (pos.isLong()) {
    double stopLossPrice = EntryPrice * (1 - _stopLossLong.getValue() / 100);

    if (pos.getEntryBar() != barIndex)
      sellAtStop(bs, barIndex, pos, stopLossPrice, "Stop loss long");
  } else
    throw SellingShortPositionException(
        "Selling short position in applyStopLossLong");
}

void PositionsManagerImpl::applyStopLossShort(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  double EntryPrice = pos.getEntryPrice();

  if (pos.isShort()) {
    double stopLossPrice = EntryPrice * (1 + _stopLossShort.getValue() / 100);

    if (pos.getEntryBar() != barIndex)
      coverAtStop(bs, barIndex, pos, stopLossPrice, "Stop loss short");
  } else
    throw SellingShortPositionException(
        "Covering long position in applyStopLossShort");
}

void PositionsManagerImpl::applyTrailing(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  double entryPrice = pos.getEntryPrice();
  if (pos.isLong()) {
    // apply trailing stop to long position
    if (pos.isTrailingStopActive()) {
      // if trailing stop has already been activated
      double level = pos.getTrailingStopLevel();
      // calculate the stop
      double stop =
          level - (level - entryPrice) * _TTrailingStop.getLevel() / 100;
      if (barIndex != pos.getEntryBar()) {
        if (!sellAtStop(bs, barIndex, pos, stop, "Trailing Stop")) {
          double newLevel =
              max2(bs.close(barIndex), pos.getTrailingStopLevel());
          pos.activateTrailingStop(newLevel);
        }
      }
    }
    // if trailing stop is not active, see if we need to activate
    // if the closing price at current bar is higher or equal than the entry
    // price
    else if (barIndex != pos.getEntryBar() &&
             (bs.close(barIndex) >=
              (entryPrice * (1 + _TTrailingStop.getTrigger() / 100))))
      pos.activateTrailingStop(bs.close(barIndex));
  } else {
    if (pos.isTrailingStopActive()) {
      double level = pos.getTrailingStopLevel();
      double stop =
          level + (entryPrice - level) * _TTrailingStop.getLevel() / 100;
      if (barIndex != pos.getEntryBar()) {
        if (!coverAtStop(bs, barIndex, pos, stop, "Trailing Stop")) {
          double newLevel =
              max2(bs.close(barIndex), pos.getTrailingStopLevel());
          pos.activateTrailingStop(newLevel);
        }
      }
    } else if (barIndex != pos.getEntryBar() &&
               (bs.close(barIndex) <=
                (entryPrice * (1 - _TTrailingStop.getTrigger() / 100))))
      pos.activateTrailingStop(bs.close(barIndex));
  }
}

void PositionsManagerImpl::applyBreakEvenStop(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  bool active = pos.isBreakEvenStopActive();
  double entryPrice = pos.getEntryPrice();

  if (pos.isLong()) {
    if (active) {
      if (barIndex != pos.getEntryBar())
        sellAtStop(bs, barIndex, pos, entryPrice, "Break even stop");

    } else if (barIndex != pos.getEntryBar()) {
      double trigger = entryPrice * (1 + _breakEvenStop.getValue() / 100);
      if (bs.close(barIndex) >= trigger) pos.activateBreakEvenStop();
    }
  } else {
    if (active) {
      if (barIndex != pos.getEntryBar())
        coverAtStop(bs, barIndex, pos, entryPrice, "Break even stop");
    } else if (barIndex != pos.getEntryBar()) {
      double trigger = entryPrice * (1 - _breakEvenStop.getValue() / 100);
      if (bs.close(barIndex) <= trigger) pos.activateBreakEvenStop();
    }
  }
}

void PositionsManagerImpl::applyBreakEvenStopLong(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  bool active = pos.isBreakEvenStopLongActive();
  double entryPrice = pos.getEntryPrice();

  if (pos.isLong()) {
    if (active) {
      if (barIndex != pos.getEntryBar())
        sellAtStop(bs, barIndex, pos, entryPrice, "Break even stop long");

    } else if (barIndex != pos.getEntryBar()) {
      double trigger = entryPrice * (1 + _breakEvenStop.getValue() / 100);
      if (bs.close(barIndex) >= trigger) pos.activateBreakEvenStop();
    }
  } else
    throw SellingShortPositionException(
        "Selling short position in applyBreakEvenStopLong");
}

void PositionsManagerImpl::applyBreakEvenStopShort(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  bool active = pos.isBreakEvenStopShortActive();
  double entryPrice = pos.getEntryPrice();

  if (pos.isShort()) {
    if (active) {
      if (barIndex != pos.getEntryBar())
        coverAtStop(bs, barIndex, pos, entryPrice, "Break even stop short");
    } else if (barIndex != pos.getEntryBar()) {
      double trigger = entryPrice * (1 - _breakEvenStop.getValue() / 100);
      if (bs.close(barIndex) <= trigger) pos.activateBreakEvenStop();
    }
  } else
    throw CoveringLongPositionException(
        "Covering long position in applyBreakEvenStopShort");
}

void PositionsManagerImpl::applyReverseBreakEvenStop(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  bool active = pos.isBreakEvenStopActive();
  double EntryPrice = pos.getEntryPrice();

  if (pos.isLong()) {
    if (active) {
      if (barIndex != pos.getEntryBar())
        sellAtLimit(bs, barIndex, pos, EntryPrice, "Reverse break even stop");
    } else if (barIndex != pos.getEntryBar()) {
      double trigger =
          EntryPrice * (1 - _reverseBreakEvenStop.getValue() / 100);
      if (bs.close(barIndex) <= trigger) pos.activateBreakEvenStop();
    }
  } else {
    if (active) {
      if (barIndex != pos.getEntryBar())
        coverAtLimit(bs, barIndex, pos, EntryPrice, "Reverse break even stop");
    } else if (barIndex != pos.getEntryBar()) {
      double trigger =
          EntryPrice * (1 + _reverseBreakEvenStop.getValue() / 100);
      if (bs.close(barIndex) >= trigger) pos.activateBreakEvenStop();
    }
  }
}

void PositionsManagerImpl::applyReverseBreakEvenStopLong(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException,
                                 SellingShortPositionException) {
  bool active = pos.isBreakEvenStopLongActive();
  double EntryPrice = pos.getEntryPrice();

  if (pos.isLong()) {
    if (active) {
      if (barIndex != pos.getEntryBar())
        sellAtLimit(bs, barIndex, pos, EntryPrice,
                    "Reverse break even stop long");
    } else if (barIndex != pos.getEntryBar()) {
      double trigger =
          EntryPrice * (1 - _reverseBreakEvenStop.getValue() / 100);
      if (bs.close(barIndex) <= trigger) pos.activateBreakEvenStop();
    }
  } else
    throw SellingShortPositionException(
        "Selling short position in applyReverseBreakEvenStopLong");
}

void PositionsManagerImpl::applyReverseBreakEvenStopShort(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException,
                                 CoveringLongPositionException) {
  bool active = pos.isBreakEvenStopShortActive();
  double EntryPrice = pos.getEntryPrice();

  if (pos.isShort()) {
    if (active) {
      if (barIndex != pos.getEntryBar())
        coverAtLimit(bs, barIndex, pos, EntryPrice,
                     "Reverse break even stop short");
    } else if (barIndex != pos.getEntryBar()) {
      double trigger =
          EntryPrice * (1 + _reverseBreakEvenStop.getValue() / 100);
      if (bs.close(barIndex) >= trigger) pos.activateBreakEvenStop();
    }
  } else
    throw CoveringLongPositionException(
        "Covering long position in applyReverseBreakEvenStopShort");
}

void PositionsManagerImpl::applyProfitTarget(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  if (pos.isLong()) {
    double targetPrice =
        pos.getEntryPrice() * (1 + _profitTarget.getValue() / 100);
    if (pos.getEntryBar() != barIndex)
      sellAtLimit(bs, barIndex, pos, targetPrice, "Profit target");
  } else {
    double targetPrice =
        pos.getEntryPrice() * (1 - _profitTarget.getValue() / 100);
    if (pos.getEntryBar() != barIndex)
      coverAtLimit(bs, barIndex, pos, targetPrice, "Profit target");
  }
}

void PositionsManagerImpl::applyProfitTargetLong(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException,
                                 SellingShortPositionException) {
  if (pos.isLong()) {
    double targetPrice =
        pos.getEntryPrice() * (1 + _profitTargetLong.getValue() / 100);
    if (pos.getEntryBar() != barIndex)
      sellAtLimit(bs, barIndex, pos, targetPrice, "Profit target long");
  } else
    throw SellingShortPositionException(
        "Selling short position in applyProfitTargetLong");
}

void PositionsManagerImpl::applyProfitTargetShort(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException,
                                 CoveringLongPositionException) {
  if (pos.isShort()) {
    double targetPrice =
        pos.getEntryPrice() * (1 - _profitTargetShort.getValue() / 100);
    if (pos.getEntryBar() != barIndex)
      coverAtLimit(bs, barIndex, pos, targetPrice, "Profit target short");
  } else
    throw CoveringLongPositionException(
        "Covering long position in applyProfitTargetShort");
}

void PositionsManagerImpl::applyAutoStops(
    Bars bs, size_t barIndex,
    tradery::Position pos) throw(BarIndexOutOfRangeException) {
  try {
    if (_timeBasedExitAtMarket.isSet())
      applyTimeBasedAtMarket(bs, barIndex, pos);
    if (pos.isClosed()) return;

    // apply stop loss strtegies (all, short long)
    if (_stopLoss.isSet()) applyStopLoss(bs, barIndex, pos);
    if (pos.isClosed()) return;

    if (_stopLossLong.isSet()) {
      try {
        applyStopLossLong(bs, barIndex, pos);
      } catch (const SellingShortPositionException&) {
      }
    }
    if (pos.isClosed()) return;

    if (_stopLossShort.isSet()) {
      try {
        applyStopLossShort(bs, barIndex, pos);
      } catch (const CoveringLongPositionException&) {
      }
    }
    if (pos.isClosed()) return;

    if (_TTrailingStop.isSet()) applyTrailing(bs, barIndex, pos);
    if (pos.isClosed()) return;

    if (_breakEvenStop.isSet()) applyBreakEvenStop(bs, barIndex, pos);
    if (pos.isClosed()) return;

    if (_breakEvenStopLong.isSet()) {
      try {
        applyBreakEvenStopLong(bs, barIndex, pos);
      } catch (const SellingShortPositionException&) {
      }
    }
    if (pos.isClosed()) return;

    if (_breakEvenStopShort.isSet()) {
      try {
        applyBreakEvenStopShort(bs, barIndex, pos);
      } catch (const CoveringLongPositionException&) {
      }
    }
    if (pos.isClosed()) return;

    if (_reverseBreakEvenStop.isSet())
      applyReverseBreakEvenStop(bs, barIndex, pos);
    if (pos.isClosed()) return;

    if (_reverseBreakEvenStopLong.isSet()) {
      try {
        applyReverseBreakEvenStopLong(bs, barIndex, pos);
      } catch (const SellingShortPositionException&) {
      }
    }
    if (pos.isClosed()) return;

    if (_reverseBreakEvenStopShort.isSet()) {
      try {
        applyReverseBreakEvenStopShort(bs, barIndex, pos);
      } catch (const CoveringLongPositionException&) {
      }
    }
    if (pos.isClosed()) return;

    if (_profitTargetLong.isSet()) {
      try {
        applyProfitTargetLong(bs, barIndex, pos);
      } catch (const SellingShortPositionException&) {
      }
    }
    if (pos.isClosed()) return;

    if (_profitTargetShort.isSet()) {
      try {
        applyProfitTargetShort(bs, barIndex, pos);
      } catch (const CoveringLongPositionException&) {
      }
    }
    if (pos.isClosed()) return;

    if (_profitTarget.isSet()) applyProfitTarget(bs, barIndex, pos);
    if (pos.isClosed()) return;

    if (_timeBasedExitAtClose.isSet()) applyTimeBasedAtClose(bs, barIndex, pos);
  } catch (const BarIndexOutOfRangeException&) {
    // if it is not on the bar after last or if there are no signal listeners
    // registered, rethrow
    if (_signalHandlers.size() == 0 || barIndex != bs.size()) throw;
    // silently kill this exception in the case of a signal - we are just
    // running this on the last bar + 1 to see if there are signals even if we
    // test for signal listeners (see the ...empty() ), this exceptio may be
    // thrown anyways in the different apply... methods when for example
    // bs.close( barIndex ) is called (see applyBreakEvenStop for example).

    // I guess this whole thing is not totally right, as we should not be
    // arbitrarily testing for signals when we are on the last bar. The
    // assumption is that the user is running some sort of loop on barIndex, and
    // this may be right in 99.99% of cases, but still, it should maybe be the
    // explicit action of the user asking for this to be run on the last bar + 1
    // or this should always be run on the barIndex + 1.
  }
}

class CX : public OpenPositionHandler {
 private:
  PositionsManagerImpl& _positions;
  Bars _bars;

 public:
  CX(PositionsManagerImpl& positions, Bars bars)
      : _positions(positions), _bars(bars) {}
  virtual bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) {
    _positions.applyAutoStops(_bars, bar, pos);
    return true;
  }
};

void PositionsManagerImpl::applyAutoStops(Bars bs, size_t barIndex) throw(
    BarIndexOutOfRangeException) {
  assert(_posContainer != 0);

  forEachOpenPosition(CX(*this, bs), bs, barIndex);
  if (_signalHandlers.size() > 0 && barIndex == bs.size() - 1)
    // check for signals only if there are registered signal listeners and we
    // are on the last bar
    applyAutoStops(bs, barIndex + 1);
}

  // we're running this here because we want to handle signals by throwing a
  // BarIndexOutOfRangeException, by a bar method that is called before if this
  // was called before, it would throw  a TimeSeriesOutOfRangeException, so
  // things would become more complicated
#define CHECK_TRADE_RANGE(ret)                                            \
  if (!_startTrades.is_not_a_date_time() &&                               \
          bs.time(barIndex) < _startTrades ||                             \
      _endTrades.is_not_a_date_time() && bs.time(barIndex) >= _endTrades) \
    return ret;

#define CHECK_VOLUME_0(ret) \
  if (!_acceptVolume0 && bs.volume(barIndex) == 0) return ret;

#define CHECKS(ret)      \
  CHECK_TRADE_RANGE(ret) \
  CHECK_VOLUME_0(ret)

PositionId PositionsManagerImpl::buyAtMarket(
    Bars bs, size_t barIndex, size_t shares, const std::string& name,
    bool applyPositionSizing) throw(BarIndexOutOfRangeException) {
  if (_orderFilter == 0 ||
      (shares = _orderFilter->onBuyAtMarket(barIndex, shares)) > 0) {
    try {
      double slippage =
          calculateSlippage(shares, bs.volume(barIndex), bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(0)

      double price = min2(bs.open(barIndex) + slippage, bs.high(barIndex));
      double commission = calculateCommission(shares, price);
      return openLong(market_order, bs.getSymbol(), shares, price, slippage,
                      commission, bs.time(barIndex), barIndex, name,
                      systemName(), applyPositionSizing)
          ->getId();
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::BUY_AT_MARKET,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, shares, name, systemName(),
                                        applyPositionSizing, systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return 0;
      } else
        throw;
    }
  } else
    return 0;
}

PositionId PositionsManagerImpl::buyAtClose(
    Bars bs, size_t barIndex, size_t shares, const std::string& name,
    bool applyPositionSizing) throw(BarIndexOutOfRangeException) {
  if (_orderFilter == 0 ||
      (shares = _orderFilter->onBuyAtClose(barIndex, shares)) > 0) {
    try {
      double slippage =
          calculateSlippage(shares, bs.volume(barIndex), bs.close(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(0)

      double price = min2(bs.close(barIndex) + slippage, bs.high(barIndex));
      double commission = calculateCommission(shares, price);
      return openLong(close_order, bs.getSymbol(), shares, price, slippage,
                      commission, bs.time(barIndex), barIndex, name,
                      systemName(), applyPositionSizing)
          ->getId();
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::BUY_AT_CLOSE,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, shares, name, systemName(),
                                        applyPositionSizing, systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return 0;
      } else
        throw;
    }
  } else
    return 0;
}

PositionId PositionsManagerImpl::buyAtStop(
    Bars bs, size_t barIndex, double price, size_t shares,
    const std::string& name,
    bool applyPositionSizing) throw(BarIndexOutOfRangeException) {
  validateStopPrice(barIndex, price);
  if (_orderFilter == 0 ||
      (shares = _orderFilter->onBuyAtStop(barIndex, shares, price)) > 0) {
    try {
      double slippage =
          calculateSlippage(shares, bs.volume(barIndex), bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(0)

      double stopPrice = price + slippage;
      // TODO: what price should commission get, adjusted or unadjusted?
      double commission = calculateCommission(shares, stopPrice);
      if (bs.open(barIndex) >= stopPrice) {
        // in this case, slippage was 0
        return openLong(stop_order, bs.getSymbol(), shares, bs.open(barIndex),
                        0, commission, bs.time(barIndex), barIndex, name,
                        systemName(), applyPositionSizing)
            ->getId();
      } else if (stopPrice <= bs.high(barIndex))
        return openLong(stop_order, bs.getSymbol(), shares, stopPrice, slippage,
                        commission, bs.time(barIndex), barIndex, name,
                        systemName(), applyPositionSizing)
            ->getId();
      else
        return 0;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        // the signal gets the slippage unadjusted stop - slippage is only for
        // backtesting
        SignalPtr signal(
            new SignalImpl(Signal::SignalType::BUY_AT_STOP, bs.getSymbol(),
                           bs.time(barIndex - 1), barIndex, shares, price, name,
                           systemName(), applyPositionSizing, systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return 0;
      } else
        throw;
    }
  } else
    return 0;
}

PositionId PositionsManagerImpl::buyAtPrice(
    Bars bs, size_t barIndex, double price, size_t shares,
    const std::string& name, bool applyPositionSizing,
    PositionId id) throw(BarIndexOutOfRangeException) {
  CHECKS(0)
  double slippage = 0;
  double commission = calculateCommission(shares, price);
  return openLong(price_order, bs.getSymbol(), shares, price, 0, commission,
                  bs.time(barIndex), barIndex, name, systemName(),
                  applyPositionSizing, id)
      ->getId();
}
PositionId PositionsManagerImpl::shortAtPrice(
    Bars bs, size_t barIndex, double price, size_t shares,
    const std::string& name, bool applyPositionSizing,
    PositionId id) throw(BarIndexOutOfRangeException) {
  CHECKS(0)
  double slippage = 0;
  double commission = calculateCommission(shares, price);
  return openShort(price_order, bs.getSymbol(), shares, price, 0, commission,
                   bs.time(barIndex), barIndex, name, systemName(),
                   applyPositionSizing, id)
      ->getId();
}

PositionId PositionsManagerImpl::buyAtLimit(
    Bars bs, size_t barIndex, double limitPrice, size_t shares,
    const std::string& name,
    bool applyPositionSizing) throw(BarIndexOutOfRangeException) {
  validateLimitPrice(barIndex, limitPrice);

  if (_orderFilter == 0 ||
      (shares = _orderFilter->onBuyAtLimit(barIndex, shares, limitPrice)) > 0) {
    try {
      double slippage =
          calculateSlippage(shares, bs.volume(barIndex), bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(0)

      double l = limitPrice - slippage;
      double commission = calculateCommission(shares, limitPrice);
      if (l < bs.low(barIndex))
        // if the adjusted price is lower than the low, than no trade
        return 0;
      else if (bs.open(barIndex) <= limitPrice) {
        // for a limit order, slippage is 0
        return openLong(limit_order, bs.getSymbol(), shares, bs.open(barIndex),
                        0, commission, bs.time(barIndex), barIndex, name,
                        systemName(), applyPositionSizing)
            ->getId();
      } else if (limitPrice >= bs.low(barIndex))
        // for a limit order, slippage is 0
        return openLong(limit_order, bs.getSymbol(), shares, limitPrice, 0,
                        commission, bs.time(barIndex), barIndex, name,
                        systemName(), applyPositionSizing)
            ->getId();
      else
        return 0;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(
            Signal::SignalType::BUY_AT_LIMIT, bs.getSymbol(),
            bs.time(barIndex - 1), barIndex, shares, limitPrice, name,
            systemName(), applyPositionSizing, systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return 0;
      } else
        throw;
    }
  } else
    return 0;
}
bool PositionsManagerImpl::sellAtMarket(
    Bars bs, size_t barIndex, tradery::Position pos,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   SellingShortPositionException,
                                   ClosingAlreadyClosedPositionException) {
  validateSymbol(bs, pos);

  if (_orderFilter == 0 || _orderFilter->onSellAtMarket(barIndex)) {
    try {
      double slippage = calculateSlippage(pos.getShares(), bs.volume(barIndex),
                                          bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(false)

      double price = max2(bs.open(barIndex) - slippage, bs.low(barIndex));
      double commission = calculateCommission(pos.getShares(), price);
      closeLong(market_order, pos, price, slippage, commission,
                bs.time(barIndex), barIndex, name);
      return true;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::SELL_AT_MARKET,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, pos.getShares(), pos, name,
                                        systemName(), systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return false;
      } else
        throw;
    }
  } else
    return false;
}

bool PositionsManagerImpl::sellAtClose(
    Bars bs, size_t barIndex, tradery::Position pos,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   SellingShortPositionException,
                                   ClosingAlreadyClosedPositionException) {
  validateSymbol(bs, pos);

  if (_orderFilter == 0 || _orderFilter->onSellAtClose(barIndex)) {
    try {
      double slippage = calculateSlippage(pos.getShares(), bs.volume(barIndex),
                                          bs.close(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(false)

      double price = max2(bs.close(barIndex) - slippage, bs.low(barIndex));
      double commission = calculateCommission(pos.getShares(), price);
      closeLong(close_order, pos, price, slippage, commission,
                bs.time(barIndex), barIndex, name);
      return true;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::SELL_AT_CLOSE,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, pos.getShares(), pos, name,
                                        systemName(), systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return false;
      } else
        throw;
    }
  } else
    return false;
}

bool PositionsManagerImpl::sellAtStop(
    Bars bs, size_t barIndex, tradery::Position pos, double price,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   SellingShortPositionException,
                                   ClosingAlreadyClosedPositionException) {
  validateStopPrice(barIndex, price);
  validateSymbol(bs, pos);

  if (_orderFilter == 0 || _orderFilter->onSellAtStop(barIndex, price)) {
    try {
      double slippage = calculateSlippage(pos.getShares(), bs.volume(barIndex),
                                          bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(false)

      double stopPrice = price - slippage;
      double commission = calculateCommission(pos.getShares(), stopPrice);

      if (bs.open(barIndex) <= stopPrice) {
        // in this case slippage is 0
        closeLong(stop_order, pos, bs.open(barIndex), 0, commission,
                  bs.time(barIndex), barIndex, name);
        return true;
      } else if (stopPrice >= bs.low(barIndex)) {
        closeLong(stop_order, pos, stopPrice, slippage, commission,
                  bs.time(barIndex), barIndex, name);
        // the pos is open
        return true;
      } else
        return false;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::SELL_AT_STOP,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, pos.getShares(), price, pos,
                                        name, systemName(), systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return false;
      } else
        throw;
    }
  } else
    return false;
}

bool PositionsManagerImpl::sellAtLimit(
    Bars bs, size_t barIndex, tradery::Position pos, double limitPrice,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   SellingShortPositionException,
                                   ClosingAlreadyClosedPositionException) {
  validateLimitPrice(barIndex, limitPrice);
  validateSymbol(bs, pos);

  if (_orderFilter == 0 || _orderFilter->onSellAtLimit(barIndex, limitPrice)) {
    // TODO: commission, slippage
    try {
      double slippage = calculateSlippage(pos.getShares(), bs.volume(barIndex),
                                          bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(false)

      double l = limitPrice + slippage;
      double commission = calculateCommission(pos.getShares(), limitPrice);
      if (l > bs.high(barIndex))
        // if the adjusted price is highre than the high, than no trade
        return false;
      else if (bs.open(barIndex) >= limitPrice) {
        closeLong(limit_order, pos, bs.open(barIndex), 0, commission,
                  bs.time(barIndex), barIndex, name);
        return true;
      } else if (limitPrice <= bs.high(barIndex)) {
        closeLong(limit_order, pos, limitPrice, 0, commission,
                  bs.time(barIndex), barIndex, name);
        // the pos is open
        return true;
      } else
        return false;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::SELL_AT_LIMIT,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, pos.getShares(), limitPrice,
                                        pos, name, systemName(), systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return false;
      } else
        throw;
    }
  } else
    return false;
}

PositionId PositionsManagerImpl::shortAtMarket(
    Bars bs, size_t barIndex, size_t shares, const std::string& name,
    bool applyPositionSizing) throw(BarIndexOutOfRangeException) {
  if (_orderFilter == 0 ||
      (shares = _orderFilter->onShortAtMarket(barIndex, shares)) > 0) {
    try {
      double slippage =
          calculateSlippage(shares, bs.volume(barIndex), bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(0)

      double price = max2(bs.open(barIndex) - slippage, bs.low(barIndex));
      double commission = calculateCommission(shares, price);
      return openShort(market_order, bs.getSymbol(), shares, price, slippage,
                       commission, bs.time(barIndex), barIndex, name,
                       systemName(), applyPositionSizing)
          ->getId();
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::SHORT_AT_MARKET,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, shares, name, systemName(),
                                        applyPositionSizing, systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return 0;
      } else
        throw;
    }
  } else
    return 0;
}

PositionId PositionsManagerImpl::shortAtClose(
    Bars bs, size_t barIndex, size_t shares, const std::string& name,
    bool applyPositionSizing) throw(BarIndexOutOfRangeException) {
  if (_orderFilter == 0 ||
      (shares = _orderFilter->onShortAtClose(barIndex, shares)) > 0) {
    try {
      double slippage =
          calculateSlippage(shares, bs.volume(barIndex), bs.close(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(0)

      double price = max2(bs.close(barIndex) - slippage, bs.low(barIndex));
      double commission = calculateCommission(shares, price);
      return openShort(close_order, bs.getSymbol(), shares, price, slippage,
                       commission, bs.time(barIndex), barIndex, name,
                       systemName(), applyPositionSizing)
          ->getId();
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::SHORT_AT_CLOSE,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, shares, name, systemName(),
                                        applyPositionSizing, systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return 0;
      } else
        throw;
    }
  } else
    return 0;
}

PositionId PositionsManagerImpl::shortAtStop(
    Bars bs, size_t barIndex, double price, size_t shares,
    const std::string& name,
    bool applyPositionSizing) throw(BarIndexOutOfRangeException) {
  validateStopPrice(barIndex, price);
  if (_orderFilter == 0 ||
      (shares = _orderFilter->onShortAtStop(barIndex, shares, price)) > 0) {
    try {
      double slippage =
          calculateSlippage(shares, bs.volume(barIndex), bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(0)

      double stopPrice = price - slippage;
      double commission = calculateCommission(shares, stopPrice);
      if (bs.open(barIndex) <= stopPrice) {
        // in this case slippage is 0
        return openShort(stop_order, bs.getSymbol(), shares, bs.open(barIndex),
                         0, commission, bs.time(barIndex), barIndex, name,
                         systemName(), applyPositionSizing)
            ->getId();
      } else if (stopPrice >= bs.low(barIndex)) {
        return openShort(stop_order, bs.getSymbol(), shares, stopPrice,
                         slippage, commission, bs.time(barIndex), barIndex,
                         name, systemName(), applyPositionSizing)
            ->getId();
        // the pos is open
      } else
        return 0;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(
            new SignalImpl(Signal::SignalType::SHORT_AT_STOP, bs.getSymbol(),
                           bs.time(barIndex - 1), barIndex, shares, price, name,
                           systemName(), applyPositionSizing, systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return 0;
      } else
        throw;
    }
  } else
    return 0;
}

PositionId PositionsManagerImpl::shortAtLimit(
    Bars bs, size_t barIndex, double limitPrice, size_t shares,
    const std::string& name,
    bool applyPositionSizing) throw(BarIndexOutOfRangeException) {
  validateLimitPrice(barIndex, limitPrice);

  if (_orderFilter == 0 || (shares = _orderFilter->onShortAtLimit(
                                barIndex, shares, limitPrice)) > 0) {
    try {
      double slippage =
          calculateSlippage(shares, bs.volume(barIndex), bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(0)

      double l = limitPrice + slippage;
      double commission = calculateCommission(shares, limitPrice);
      if (l > bs.high(barIndex))
        // if the adjusted price is higher than the high, than no trade
        return 0;
      if (bs.open(barIndex) >= limitPrice) {
        // for a limit order, slippage is 0
        return openShort(limit_order, bs.getSymbol(), shares, bs.open(barIndex),
                         0, commission, bs.time(barIndex), barIndex, name,
                         systemName(), applyPositionSizing)
            ->getId();
      } else if (limitPrice <= bs.high(barIndex)) {
        return openShort(limit_order, bs.getSymbol(), shares, limitPrice, 0,
                         commission, bs.time(barIndex), barIndex, name,
                         systemName(), applyPositionSizing)
            ->getId();
        // the pos is open
      } else
        return 0;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(
            Signal::SignalType::SHORT_AT_LIMIT, bs.getSymbol(),
            bs.time(barIndex - 1), barIndex, shares, limitPrice, name,
            systemName(), applyPositionSizing, systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return 0;
      } else
        throw;
    }
  } else
    return 0;
}

bool PositionsManagerImpl::coverAtMarket(
    Bars bs, size_t barIndex, tradery::Position pos,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   CoveringLongPositionException,
                                   ClosingAlreadyClosedPositionException) {
  validateSymbol(bs, pos);

  if (_orderFilter == 0 || _orderFilter->onCoverAtMarket(barIndex)) {
    try {
      double slippage = calculateSlippage(pos.getShares(), bs.volume(barIndex),
                                          bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(false)

      double price = min2(bs.open(barIndex) + slippage, bs.high(barIndex));
      double commission = calculateCommission(pos.getShares(), price);
      closeShort(market_order, pos, price, slippage, commission,
                 bs.time(barIndex), barIndex, name);
      return true;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::COVER_AT_MARKET,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, pos.getShares(), pos, name,
                                        systemName(), systemId()));
        _signalHandlers.signal(signal);

        // generated an signal, but no position opened, so return false
        return false;
      } else
        throw;
    }
  } else
    return false;
}

bool PositionsManagerImpl::coverAtClose(
    Bars bs, size_t barIndex, tradery::Position pos,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   CoveringLongPositionException,
                                   ClosingAlreadyClosedPositionException) {
  validateSymbol(bs, pos);

  if (_orderFilter == 0 || _orderFilter->onCoverAtClose(barIndex)) {
    try {
      double slippage = calculateSlippage(pos.getShares(), bs.volume(barIndex),
                                          bs.close(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(false)

      double price = min2(bs.close(barIndex) + slippage, bs.high(barIndex));
      double commission = calculateCommission(pos.getShares(), price);
      closeShort(close_order, pos, price, slippage, commission,
                 bs.time(barIndex), barIndex, name);
      return true;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::COVER_AT_CLOSE,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, pos.getShares(), pos, name,
                                        systemName(), systemId()));
        _signalHandlers.signal(signal);

        // generated an signal, but no position opened, so return false
        return false;
      } else
        throw;
    }
  } else
    return false;
}

bool PositionsManagerImpl::coverAtStop(
    Bars bs, size_t barIndex, tradery::Position pos, double price,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   CoveringLongPositionException,
                                   ClosingAlreadyClosedPositionException) {
  validateStopPrice(barIndex, price);
  validateSymbol(bs, pos);

  if (_orderFilter == 0 || _orderFilter->onCoverAtStop(barIndex, price)) {
    try {
      double slippage = calculateSlippage(pos.getShares(), bs.volume(barIndex),
                                          bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(false)

      double stopPrice = price + slippage;
      double commission = calculateCommission(pos.getShares(), stopPrice);
      if (bs.open(barIndex) >= stopPrice) {
        // in this case slippage is 0
        closeShort(stop_order, pos, bs.open(barIndex), 0, commission,
                   bs.time(barIndex), barIndex, name);
        return true;
      } else if (stopPrice <= bs.high(barIndex)) {
        closeShort(stop_order, pos, stopPrice, slippage, commission,
                   bs.time(barIndex), barIndex, name);
        // the pos is open
        return true;
      } else
        return false;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::COVER_AT_STOP,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, pos.getShares(), price, pos,
                                        name, systemName(), systemId()));
        _signalHandlers.signal(signal);

        // generated an signal, but no position opened, so return false
        return false;
      } else
        throw;
    }
  } else
    return false;
}

bool PositionsManagerImpl::coverAtLimit(
    Bars bs, size_t barIndex, tradery::Position pos, double limitPrice,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   CoveringLongPositionException,
                                   ClosingAlreadyClosedPositionException) {
  validateLimitPrice(barIndex, limitPrice);
  validateSymbol(bs, pos);

  if (_orderFilter == 0 || _orderFilter->onCoverAtLimit(barIndex, limitPrice)) {
    try {
      double slippage = calculateSlippage(pos.getShares(), bs.volume(barIndex),
                                          bs.open(barIndex));

      // this has to be here to allow out of range exception to be caught,
      // otherwise there are no signals
      CHECKS(false)

      double l = limitPrice - slippage;
      double commission = calculateCommission(pos.getShares(), limitPrice);
      if (l < bs.low(barIndex))
        // if the slippage adjusted limit price is lower than the low, then no
        // trade
        return false;
      else if (bs.open(barIndex) <= limitPrice) {
        // slippage is 0
        closeShort(limit_order, pos, bs.open(barIndex), 0, commission,
                   bs.time(barIndex), barIndex, name);
        return true;
      } else if (limitPrice >= bs.low(barIndex)) {
        // slippage is 0
        closeShort(limit_order, pos, limitPrice, 0, commission,
                   bs.time(barIndex), barIndex, name);
        // the pos is open
        return true;
      } else
        return false;
    } catch (const BarIndexOutOfRangeException&) {
      // TODO: see if this scheme makes sense
      // if the exception is for the bar past the last bar, if there are
      // registered signalables, notify them, if not (in case signal returns
      // false), rethrow the exception
      if (barIndex == bs.size() && _signalHandlers.size() > 0) {
        SignalPtr signal(new SignalImpl(Signal::SignalType::COVER_AT_LIMIT,
                                        bs.getSymbol(), bs.time(barIndex - 1),
                                        barIndex, pos.getShares(), limitPrice,
                                        pos, name, systemName(), systemId()));
        _signalHandlers.signal(signal);
        // generated an signal, but no position opened, so return false
        return false;
      } else
        throw;
    }
  } else
    return false;
}

bool PositionsManagerImpl::sellAtMarket(
    Bars bars, size_t barIndex, PositionId pos,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   SellingShortPositionException,
                                   ClosingAlreadyClosedPositionException,
                                   PositionIdNotFoundException) {
  tradery::Position p(getPosition(pos));
  if (p)
    return sellAtMarket(bars, barIndex, p, name);
  else
    throw PositionIdNotFoundException(pos);
}

bool PositionsManagerImpl::sellAtClose(
    Bars bars, size_t barIndex, PositionId pos,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   SellingShortPositionException,
                                   ClosingAlreadyClosedPositionException,
                                   PositionIdNotFoundException) {
  tradery::Position p(getPosition(pos));
  if (p)
    return sellAtClose(bars, barIndex, p, name);
  else
    throw PositionIdNotFoundException(pos);
}

bool PositionsManagerImpl::sellAtStop(
    Bars bars, size_t barIndex, PositionId pos, double stopPrice,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   SellingShortPositionException,
                                   ClosingAlreadyClosedPositionException,
                                   PositionIdNotFoundException) {
  tradery::Position p = getPosition(pos);
  if (p)
    return sellAtStop(bars, barIndex, p, stopPrice, name);
  else
    throw PositionIdNotFoundException(pos);
}

bool PositionsManagerImpl::sellAtLimit(
    Bars bars, size_t barIndex, PositionId pos, double limitPrice,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   SellingShortPositionException,
                                   ClosingAlreadyClosedPositionException,
                                   PositionIdNotFoundException) {
  tradery::Position p(getPosition(pos));
  if (p)
    return sellAtLimit(bars, barIndex, p, limitPrice, name);
  else
    throw PositionIdNotFoundException(pos);
}

bool PositionsManagerImpl::coverAtMarket(
    Bars bars, size_t barIndex, PositionId pos,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   CoveringLongPositionException,
                                   ClosingAlreadyClosedPositionException,
                                   PositionIdNotFoundException) {
  tradery::Position p(getPosition(pos));
  if (p)
    return coverAtMarket(bars, barIndex, p, name);
  else
    throw PositionIdNotFoundException(pos);
}

bool PositionsManagerImpl::coverAtClose(
    Bars bars, size_t barIndex, PositionId pos,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   CoveringLongPositionException,
                                   ClosingAlreadyClosedPositionException,
                                   PositionIdNotFoundException) {
  tradery::Position p(getPosition(pos));
  if (p)
    return coverAtClose(bars, barIndex, p, name);
  else
    throw PositionIdNotFoundException(pos);
}

bool PositionsManagerImpl::coverAtStop(
    Bars bars, size_t barIndex, PositionId pos, double stopPrice,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   CoveringLongPositionException,
                                   ClosingAlreadyClosedPositionException,
                                   PositionIdNotFoundException) {
  tradery::Position p(getPosition(pos));
  if (p)
    return coverAtStop(bars, barIndex, p, stopPrice, name);
  else
    throw PositionIdNotFoundException(pos);
}

bool PositionsManagerImpl::coverAtLimit(
    Bars bars, size_t barIndex, PositionId pos, double limitPrice,
    const std::string& name) throw(BarIndexOutOfRangeException,
                                   CoveringLongPositionException,
                                   ClosingAlreadyClosedPositionException,
                                   PositionIdNotFoundException) {
  tradery::Position p(getPosition(pos));
  if (p)
    return coverAtLimit(bars, barIndex, p, limitPrice, name);
  else
    throw PositionIdNotFoundException(pos);
}

std::ostream& PositionsContainerImpl::dumpMin(std::ostream& os) const {
  Lock lock(m);
  {
    std::ostringstream o;
    o << "Trades: " << size() << ", open: " << openPositionsCount()
      << ", closed:" << (size() - openPositionsCount()) << std::endl;
    sprint(o, os);
  }
  for (BaseContainer::const_iterator i = begin(); i != end(); i++) {
    std::ostringstream o;
    (*i)->dumpMin(os);
    o << std::endl;
    sprint(o, os);
  }
  return os;
}

std::ostream& PositionsContainerImpl::dump(std::ostream& os) const {
  Lock lock(m);
  {
    std::ostringstream o;
    o << "Trades: " << size() << ", open: " << openPositionsCount()
      << ", closed:" << (size() - openPositionsCount()) << std::endl;
    sprint(o, os);
  }
  for (BaseContainer::const_iterator i = begin(); i != end(); i++) {
    std::ostringstream o;
    (*i)->dump(os);
    o << std::endl;
    sprint(o, os);
  }
  return os;
}

///////////////////////////////////////////
//

class PositionsIteratorImpl : public PositionsIteratorAbstr {
 private:
  const PositionsContainerImpl* _pc;
  PositionsContainerImpl::const_iterator _i;

 public:
  PositionsIteratorImpl(const PositionsContainer* pc)
      : _pc(dynamic_cast<const PositionsContainerImpl*>(pc)),
        _i(_pc->begin()) {}

  /*
    virtual Position operator ++()
    {
      return *(++_i);
    }

    virtual Position operator ++(int)
    {
      return *(_i++);
    }
  */

  virtual Position first() {
    reset();
    return next();
  }

  virtual Position next() {
    return _i == _pc->end() ? tradery::Position() : Position((_i++)->get());
  }

  virtual void reset() { _i = _pc->begin(); }
};

PositionsIteratorAbstr* PositionsIteratorAbstr::create(
    const PositionsContainer* pc) {
  assert(pc != 0);
  return new PositionsIteratorImpl(pc);
}
