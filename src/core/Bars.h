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

#include <algorithm>
#include "cache.h"
#include "seriesimpl.h"
#include <objcounter.h>

using tradery::Bar;

class InvalidBars {
 private:
  // indexes of bars with invalid data
  std::vector<std::string> _invalidBarsMessages;

 public:
  void add(const std::string& message) {
    _invalidBarsMessages.push_back(message);
  }

  operator bool() const { return !_invalidBarsMessages.empty(); }

  std::string toString(unsigned int n = 3) const {
    std::string str;

    std::vector<std::string>::size_type i = 0;
    for (; i < std::min(n, _invalidBarsMessages.size()); i++) {
      if (i > 0) str << "; ";
      str += _invalidBarsMessages[i];
    }

    if (i < _invalidBarsMessages.size()) str += "; ...";
    return str;
  }
};

// TODO: add iterators and other stuff so I can use algorithms on this

class BarsImpl : public tradery::BarsAbstr, public BarsBase, public Ideable {
  OBJ_COUNTER(BarsImpl)
 private:
  // interval between bars in seconds. Usually it goes from 1 minute to 1 month
  const unsigned int _resolution;
  const Type _type;

  Series _lowSeries;
  Series _highSeries;
  Series _openSeries;
  Series _closeSeries;
  Series _volumeSeries;
  Series _openInterest;
  tradery::TimeSeries _timeSeries;
  tradery::ExtraInfoSeries _extraInfoSeries;

  SynchronizerPtr _synchronizer;

  ErrorHandlingMode _errorHandlingMode;

  InvalidBars _invalidBars;

 public:
  // TODO: the bars id is the symbol for now (for testing). Needs to have
  // datasource, range, and other info that shows it's up to date
  // and the various series are created attaching the name of the series
  // will need to be changed to make them foolproof, unique numbers etc
  BarsImpl(const std::string& dataSourceName, const std::string& symbol,
           Type type, unsigned int resolution, DateTimeRangePtr range,
           ErrorHandlingMode errorHandlingMode)
      : _resolution(resolution),
        _type(type),
        Ideable(dataSourceName + " - bars - " + symbol +
                (range == 0 ? "" : " - range: " + range->getId())),
        BarsBase(symbol),
        _errorHandlingMode(errorHandlingMode) {}

  virtual ~BarsImpl() {}

 public:
  virtual void synchronize(Bars bars) {
    _synchronizer = Synchronizer::create(bars, Bars(this));
    _lowSeries.synchronize(_synchronizer);
    _highSeries.synchronize(_synchronizer);
    _openSeries.synchronize(_synchronizer);
    _closeSeries.synchronize(_synchronizer);
    _volumeSeries.synchronize(_synchronizer);
    _openInterest.synchronize(_synchronizer);
    _timeSeries.synchronize(_synchronizer);
  }

  virtual ErrorHandlingMode getErrorHandlingMode() const {
    return _errorHandlingMode;
  }

  virtual const InvalidBars& getInvalidBars() const { return _invalidBars; }

  virtual bool hasInvalidData() const { return (bool)_invalidBars; }

  virtual std::string getInvalidDataAsString() const {
    return _invalidBars.toString();
  }

  bool isSynchronized() const { return _synchronizer; }
  virtual bool syncModified() const {
    return isSynchronized() ? _synchronizer->modified() : false;
  }

  virtual Type type() const { return _type; }

  SynchronizerPtr synchronizer() const { return _synchronizer; }
  virtual size_t unsyncSize() const { return _lowSeries.unsyncSize(); }
  virtual std::pair<DateTime, DateTime> unsyncStartEnd() const {
    return _timeSeries.unsyncStartEnd();
  }

  virtual const Bar getBar(size_t ix) const
      throw(BarException, DataIndexOutOfRangeException) {
    return get(ix);
  }

  virtual const std::string& getSymbol() const { return BarsBase::getSymbol(); }

  virtual const Bar get(size_t index) const
      throw(DataIndexOutOfRangeException) {
    return Bar(
        time(index), open(index), high(index), low(index), close(index),
        volume(index), openInterest(index),
        getBarExtraInfo(index) == 0 ? 0 : getBarExtraInfo(index)->clone());
  }

  virtual unsigned long resolution() const { return _resolution; }

  size_t size() const {
    if (_lowSeries.size() != _highSeries.size()) {
      LOG(log_info, "symbol: " << getSymbol()
                               << "low size: " << _lowSeries.size()
                               << ", high size: " << _highSeries.size());
    }
    assert(_lowSeries.size() == _highSeries.size());
    assert(_lowSeries.size() == _openSeries.size());
    assert(_lowSeries.size() == _closeSeries.size());
    assert(_lowSeries.size() == _volumeSeries.size());
    // todo: handle the time series for synchronized series
    assert(_lowSeries.unsyncSize() == _timeSeries.size());
    assert(_lowSeries.size() == _openInterest.size());
    // todo: handle extra info for synced series
    assert(_lowSeries.unsyncSize() == _extraInfoSeries.size());

    // this works for both sync and unsync bars - if it's synced, we already
    // synced the low series too (see synchronize( .. ) )
    return _lowSeries.size();
  }

  // implemented from base class Addable
  void add(const Bar& bar) {
    if (!bar.isValid()) {
      if (_errorHandlingMode == fatal)
        throw BarException(bar.getStatusAsString());
      else if (_errorHandlingMode == warning)
        // if warning mode, add the index of the current bar (the one that's
        // being added)
        _invalidBars.add(bar.getStatusAsString());
    }
    _lowSeries.push_back(bar.getLow());
    _highSeries.push_back(bar.getHigh());
    _openSeries.push_back(bar.getOpen());
    _closeSeries.push_back(bar.getClose());
    _volumeSeries.push_back(bar.getVolume());
    _openInterest.push_back(bar.getOpenInterest());
    _timeSeries.push_back(bar.time());
    _extraInfoSeries.push_back(bar.getBarExtraInfo());
  }

  virtual void forEach(tradery::BarHandler& barHandler,
                       size_t startBar = 0) const
      throw(BarIndexOutOfRangeException) {
    if (startBar >= size())
      throw BarIndexOutOfRangeException(size(), startBar, getSymbol());

    for (size_t bar = startBar; bar < size(); bar++)
      barHandler.dataHandler(*this, bar);
  }

  double open(size_t barIndex) const throw(BarIndexOutOfRangeException) {
    try {
      return _openSeries[barIndex];
    } catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    } catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    }
  }
  double low(size_t barIndex) const throw(BarIndexOutOfRangeException) {
    try {
      return _lowSeries[barIndex];
    } catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    } catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    }
  }
  double high(size_t barIndex) const throw(BarIndexOutOfRangeException) {
    try {
      return _highSeries[barIndex];
    } catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    } catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    }
  }
  double close(size_t barIndex) const throw(BarIndexOutOfRangeException) {
    try {
      return _closeSeries[barIndex];
    } catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    } catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    }
  }
  unsigned long volume(size_t barIndex) const
      throw(BarIndexOutOfRangeException) {
    try {
      return (unsigned long)_volumeSeries[barIndex];
    } catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    } catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    }
  }

  unsigned long openInterest(size_t barIndex) const
      throw(BarIndexOutOfRangeException) {
    try {
      return (unsigned long)_openInterest[barIndex];
    } catch (const SeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    } catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    }
  }

  Date date(size_t barIndex) const throw(BarIndexOutOfRangeException) {
    return time(barIndex).date();
  }

  DateTime time(size_t barIndex) const throw(BarIndexOutOfRangeException) {
    try {
      return timeSeries()[barIndex];
    } catch (const std::out_of_range&) {
      throw BarIndexOutOfRangeException(size(), barIndex,
                                        BarsBase::getSymbol());
    } catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    }
  }

  const tradery::BarExtraInfo* getBarExtraInfo(size_t barIndex) const
      throw(BarIndexOutOfRangeException) {
    try {
      // todo: fix for synced series
      return 0;
      //      return _extraInfoSeries[ barIndex ] );
    } catch (const std::out_of_range&) {
      throw BarIndexOutOfRangeException(size(), barIndex,
                                        BarsBase::getSymbol());
    } catch (const SynchronizedSeriesIndexOutOfRangeException& e) {
      throw BarIndexOutOfRangeException(e.getSize(), e.getIndex(),
                                        BarsBase::getSymbol());
    }
  }

  const Series openSeries() const { return _openSeries; }

  const Series lowSeries() const { return _lowSeries; }

  const Series highSeries() const { return _highSeries; }

  const Series closeSeries() const { return _closeSeries; }

  const Series volumeSeries() const { return _volumeSeries; }

  const Series openInterestSeries() const { return _volumeSeries; }

  tradery::TimeSeries timeSeries() const {
    return isSynchronized() ? _synchronizer->timeSeries() : _timeSeries;
  }

  virtual const tradery::ExtraInfoSeries& getExtraInfoSeries() const {
    return _extraInfoSeries;
  }

  virtual const Series TrueRange() const;
  virtual const Series AccumDist() const;
  virtual const Series ADX(unsigned int period) const;
  virtual const Series MidPrice(unsigned int period) const;
  virtual const Series SAR(double acceleration, double maximum) const;
  virtual const Series MFI(unsigned int period) const;
  virtual const Series CCI(unsigned int period) const;
  virtual const Series DX(unsigned int period) const;
  virtual const Series PlusDI(unsigned int period) const;
  virtual const Series PlusDM(unsigned int period) const;
  virtual const Series WillR(unsigned int period) const;
  virtual const Series NATR(unsigned int period) const;
  virtual const Series ChaikinAD() const;
  virtual const Series ChaikinADOscillator(unsigned int fastPeriod,
                                           unsigned int slowPeriod) const;
  virtual const Series OBV(const Series& series) const;
  virtual const Series AvgPrice() const;
  virtual const Series MedPrice() const;
  virtual const Series TypPrice() const;
  virtual const Series WclPrice() const;
  virtual const Series MinusDI(unsigned int period) const;
  virtual const Series MinusDM(unsigned int period) const;
  virtual const Series ADXR(unsigned int period) const;
  virtual const Series ATR(unsigned int period) const;
  virtual const Series TR() const;
  virtual const Series StochSlowK(int fastKPeriod, int slowKPeriod,
                                  MAType slowKMAType, int slowDPeriod,
                                  MAType slowDMAType) const;
  virtual const Series StochSlowD(int fastKPeriod, int slowKPeriod,
                                  MAType slowKMAType, int slowDPeriod,
                                  MAType slowDMAType) const;
  virtual const Series StochFastK(int fastKPeriod, int fastDPeriod,
                                  MAType fastDMAType) const;
  virtual const Series StochFastD(int fastKPeriod, int fastDPeriod,
                                  MAType fastDMAType) const;

  virtual const Series BOP() const;

  // pattern recognition
  virtual const Series Cdl3BlackCrows() const;
  virtual const Series CdlAbandonedBaby(double penetration) const;
  virtual const Series CdlDojiStar() const;
  virtual const Series CdlEngulfing() const;
  virtual const Series CdlEveningDojiStar(double penetration) const;
  virtual const Series CdlEveningStar(double penetration) const;
  virtual const Series CdlHammer() const;
  virtual const Series CdlHangingMan() const;
  virtual const Series CdlHarami() const;
  virtual const Series CdlHaramiCross() const;
  virtual const Series CdlHighWave() const;
  virtual const Series CdlIdentical3Crows() const;
  virtual const Series CdlInvertedHammer() const;
  virtual const Series CdlLongLine() const;
  virtual const Series CdlMorningDojiStar(double penetration) const;
  virtual const Series CdlMorningStar(double penetration) const;
  virtual const Series CdlShootingStar() const;
  virtual const Series CdlShortLine() const;
  virtual const Series CdlSpinningTop() const;
  virtual const Series CdlTristar() const;
  virtual const Series CdlUpsideGap2Crows() const;
  virtual const Series CdlHikkake() const;

  // ta-lib 0.1.3
  virtual const Series Cdl2Crows() const;
  virtual const Series Cdl3Inside() const;
  virtual const Series Cdl3LineStrike() const;
  virtual const Series Cdl3WhiteSoldiers() const;
  virtual const Series Cdl3Outside() const;
  virtual const Series CdlInNeck() const;
  virtual const Series CdlOnNeck() const;
  virtual const Series CdlPiercing() const;
  virtual const Series CdlStalled() const;
  virtual const Series CdlThrusting() const;
  virtual const Series CdlAdvanceBlock(double penetration) const;

  // ta-lib 0.1.4

  virtual const Series Cdl3StarsInSouth() const;
  virtual const Series CdlBeltHold() const;
  virtual const Series CdlBreakaway() const;
  virtual const Series CdlClosingMarubuzu() const;
  virtual const Series CdlConcealingBabySwallow() const;
  virtual const Series CdlCounterattack() const;
  virtual const Series CdlDoji() const;
  virtual const Series CdlDragonFlyDoji() const;
  virtual const Series CdlGapSideBySideWhite() const;
  virtual const Series CdlGravestoneDoji() const;
  virtual const Series CdlHomingPigeon() const;
  virtual const Series CdlKicking() const;
  virtual const Series CdlKickingByLength() const;
  virtual const Series CdlLadderBottom() const;
  virtual const Series CdlLongLeggedDoji() const;
  virtual const Series CdlMarubozu() const;
  virtual const Series CdlMatchingLow() const;
  virtual const Series CdlRickshawMan() const;
  virtual const Series CdlRiseFall3Methods() const;
  virtual const Series CdlSeparatingLines() const;
  virtual const Series CdlStickSandwich() const;
  virtual const Series CdlTakuri() const;
  virtual const Series CdlTasukiGap() const;
  virtual const Series CdlUnique3River() const;
  virtual const Series CdlXSideGap3Methods() const;
  virtual const Series CdlMatHold(double penetration) const;

  const std::string locationInfoToXML() const {
    return BarsBase::locationInfoToXML();
  }
};

class Stock : public BarsImpl {};
/*
class Future : public Bars
{
private:
  double _tick;
  std::auto_ptr< Series > _openInterestSeries;

public:
  Future( const std::wstring& dataSourceName, const std::wstring& symbol,
unsigned int resolution, double tick ) : Bars( dataSourceName, symbol,
resolution ), _tick( tick ), _openInterestSeries( Series::create() )
  {
  }
};
*/
