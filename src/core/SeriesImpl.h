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

#include "cache.h"

using std::vector;

// TODO: after a lot of thinking, I chose to make this class a vector< double >
// instead of a templetized class, that takes the type of the
// Series element as argument, and this because I don't know how to make this
// usable polymorphically in other classes such as SMASeries, which cannot know
// at compile time all the series that it may need to work on. As every integral
// and double type converts to a double, this will work, at the expense of
// storing more info for things like int or short It could  have been done by
// making vector< T > a member of Series and adding all the needed vector
// members to the class Series to make it usable with algorithms, but the
// process would have been too tedious
class SeriesImpl : public tradery::SeriesAbstr, public Ideable {
 protected:
  mutable vector<double> _v;

 private:
  SynchronizerPtr _synchronizer;

  int getIndex(size_t ix) const {
    return isSynchronized() ? _synchronizer->index(ix) : ix;
  }

 public:
  // copy constructor
  /*  SeriesImpl( const SeriesImpl& series, const Id& id = Id() )
    : vector< double >( series.size() ), Ideable(id )
    {
      std::copy( series.begin(), series.end(), begin() );
    }
  */
  SeriesImpl(size_t size, SynchronizerPtr synchronizer, const Id& id = Id())
      : _v(size), Ideable(id), _synchronizer(synchronizer) {}

  SeriesImpl(const Id& id = Id()) : Ideable(id) {}

  SeriesImpl(const SeriesImpl& series)
      : _v(series.getVector()),
        Ideable(Id()),
        _synchronizer(series.synchronizer()) {}

  virtual ~SeriesImpl() {}

 public:
  virtual void synchronize(SynchronizerPtr synchronizer) {
    _synchronizer = synchronizer;
  }

  virtual bool isSynchronized() const { return _synchronizer; }

  virtual SynchronizerPtr synchronizer() const { return _synchronizer; }

  // not affected by synchronizer as it is used to calculate indicators, which
  // should be based on the unsynced data
  virtual double& at(size_t index) { return _v.at(index); }

  void push_back(double value) {
    // todo: can't modify a synced series?
    _v.push_back(value);
  }

  virtual double setValue(size_t barIndex,
                          double value) throw(SeriesIndexOutOfRangeException) {
    try {
      // todo: should we throw an exception if the series is synced? Probably we
      // don't want to set values then
      if (isSynchronized()) throw OperationNotAllowedOnSynchronizedseries();
      _v.at(barIndex) = value;
      return value;
    } catch (const std::out_of_range&) {
      throw SeriesIndexOutOfRangeException(size(), barIndex);
    }
  }

  virtual SeriesAbstrPtr clone() const {
    return SeriesAbstrPtr(new SeriesImpl(*this));
  }

  virtual double& getRef(size_t ix) throw(SeriesIndexOutOfRangeException) {
    if (ix >= size()) throw SeriesIndexOutOfRangeException(size(), ix);
    return _v[ix];
  }

  virtual double getValue(size_t ix) const
      throw(SeriesIndexOutOfRangeException) {
    try {
      return _v.at(getIndex(ix));
    } catch (const std::out_of_range&) {
      throw SeriesIndexOutOfRangeException(size(), ix);
    }
  }

  // this returns the synchronized size (the same as the actual size if not
  // synchronized)
  virtual size_t size() const {
    return isSynchronized() ? _synchronizer->size() : _v.size();
  }

  virtual size_t unsyncSize() const { return _v.size(); }

  virtual const double* getArray() const {
    return (const double*)(&_v.front());
  }

  virtual const vector<double>& getVector() const { return _v; }

 public:
  SeriesAbstrPtr operator*(SeriesAbstrPtr series) const
      throw(OperationOnUnequalSizeSeriesException) {
    return multiply(series);
  }
  virtual SeriesAbstrPtr operator*(double value) const {
    return multiply(value);
  }
  virtual SeriesAbstr& operator*=(SeriesAbstrPtr series) throw(
      OperationOnUnequalSizeSeriesException) {
    _v = ((*this) * series)->getVector();
    return *this;
  }
  virtual SeriesAbstr& operator*=(double value) {
    _v = ((*this) * value)->getVector();
    return *this;
  }

  virtual SeriesAbstrPtr operator+(SeriesAbstrPtr series) const
      throw(OperationOnUnequalSizeSeriesException) {
    return add(series);
  }
  virtual SeriesAbstr& operator+=(SeriesAbstrPtr series) throw(
      OperationOnUnequalSizeSeriesException) {
    _v = ((*this) + series)->getVector();
    return *this;
  }

  virtual SeriesAbstrPtr operator+(double value) const { return add(value); }
  virtual SeriesAbstr& operator+=(double value) {
    _v = ((*this) + value)->getVector();
    return *this;
  }

  virtual SeriesAbstrPtr operator-(SeriesAbstrPtr series) const
      throw(OperationOnUnequalSizeSeriesException) {
    return subtract(series);
  }
  virtual SeriesAbstr& operator-=(SeriesAbstrPtr series) throw(
      OperationOnUnequalSizeSeriesException) {
    _v = ((*this) - series)->getVector();
    return *this;
  }

  virtual SeriesAbstrPtr operator-(double value) const {
    return subtract(value);
  }
  virtual SeriesAbstr& operator-=(double value) {
    _v = ((*this) - value)->getVector();
    return *this;
  }

  virtual SeriesAbstrPtr operator/(SeriesAbstrPtr series) const
      throw(OperationOnUnequalSizeSeriesException, DivideByZeroException) {
    return divide(series);
  }
  virtual SeriesAbstr& operator/=(SeriesAbstrPtr series) throw(
      OperationOnUnequalSizeSeriesException, DivideByZeroException) {
    _v = ((*this) / series)->getVector();
    return *this;
  }

  virtual SeriesAbstrPtr operator/(double value) const
      throw(DivideByZeroException) {
    return divide(value);
  }
  virtual SeriesAbstr& operator/=(double value) throw(DivideByZeroException) {
    _v = ((*this) / value)->getVector();
    return *this;
  }

  bool crossOver(size_t barIndex, SeriesAbstrPtr series) const
      throw(OperationOnUnequalSizeSeriesException,
            SeriesIndexOutOfRangeException, InvalidIndexForOperationException) {
    if (size() != series->size())
      throw OperationOnUnequalSizeSeriesException(size(), series->size());
    // TODO: see how to handle this case - maybe an exception when barIndex ==
    // 0, as we cannot determine a crossover
    if (barIndex == 0)
      throw InvalidIndexForOperationException(barIndex, "Series::crossOver");

    return (getValue(barIndex - 1) < series->getValue(barIndex - 1)) &&
           (getValue(barIndex) >= series->getValue(barIndex));
  }

  bool crossOver(size_t barIndex, double d) const
      throw(SeriesIndexOutOfRangeException, InvalidIndexForOperationException) {
    if (barIndex == 0)
      throw InvalidIndexForOperationException(barIndex, "Series::crossOver");

    return (getValue(barIndex - 1) < d) && (getValue(barIndex) >= d);
  }

  bool crossUnder(size_t barIndex, SeriesAbstrPtr series) const
      throw(OperationOnUnequalSizeSeriesException,
            SeriesIndexOutOfRangeException, InvalidIndexForOperationException) {
    if (size() != series->size())
      throw OperationOnUnequalSizeSeriesException(size(), series->size());
    // TODO: see how to handle this case - maybe an exception when barIndex ==
    // 0, as we cannot determine a crossover
    if (barIndex == 0)
      throw InvalidIndexForOperationException(barIndex, "Series::crossUnder");

    return (getValue(barIndex - 1) > series->getValue(barIndex - 1)) &&
           (getValue(barIndex) <= series->getValue(barIndex));
  }

  bool crossUnder(size_t barIndex, double d) const
      throw(SeriesIndexOutOfRangeException, InvalidIndexForOperationException) {
    if (barIndex == 0)
      throw InvalidIndexForOperationException(barIndex, "Series::crossUnder");

    return (getValue(barIndex - 1) > d) && (getValue(barIndex) <= d);
  }

 public:
  // subtracts series from the current series and puts the result in dest (does
  // not create a new series)
  /*  void subtractSeries( const Series& series, Series& dest ) const
    {
      //TODO: should this be an exception, as the user may inadverdently use
    unequal series assert( size() == series.size() ); assert( size() ==
    dest.size() );

      std::transform( begin(), end(), series.begin(), dest.begin(), std::minus<
    double >() );
    }
  */

 public:
  bool turnDown(size_t barIndex) const {
    // TODO: see what to do if barIndex is 0 or 1, as in this case it cannot say
    // whether it turned down or not
    // assert for now
    assert(barIndex > 1);

    return (getValue(barIndex - 1) >= getValue(barIndex - 2)) &&
           (getValue(barIndex) < getValue(barIndex - 1));
  }

  bool turnUp(size_t barIndex) const {
    // TODO: see what to do if barIndex is 0 or 1, as in this case it cannot say
    // whether it turned down or not
    // assert for now
    assert(barIndex > 1);

    return (getValue(barIndex - 1) <= getValue(barIndex - 2)) &&
           (getValue(barIndex) > getValue(barIndex - 1));
  }

  // returns the index of the maximum value in the range starting at start and
  // of length "length" the index is relative to the start of the series
  size_t maxIndexInInterval(size_t start, size_t length) const {
    // TODO: validate the inputs
    assert(start < size());
    assert(length > 0);

    double _max = _v.at(start);
    size_t maxIndex = start;

    // TODO: use std algorithm
    for (size_t n = start + 1; n < min2(start + length, size()); n++) {
      if (_v.at(n) > _max) {
        _max = _v.at(n);
        maxIndex = n;
      }
    }
    return maxIndex;
  }

  // returns the index of the minimum value in the range
  // the index is relative to the start of the series
  size_t minIndexInInterval(size_t start, size_t length) const {
    // TODO: validate the inputs
    assert(start < size());
    assert(length > 0);

    double _min = _v.at(start);
    size_t minIndex = start;

    // TODO: use std algorithm
    for (size_t n = start + 1; n < start + length; n++) {
      if (_v.at(n) < _min) {
        _min = _v.at(n);
        minIndex = n;
      }
    }
    return minIndex;
  }

  // TODO:
  virtual SeriesAbstr& operator=(SeriesAbstrPtr series) throw(
      OperationOnUnequalSizeSeriesException) {
    if (size() != series->size())
      throw OperationOnUnequalSizeSeriesException(size(), series->size());

    // todo - can the source and dest be the same?
    _v = series->getVector();
    return *this;
  }

  virtual SeriesAbstrPtr shiftRight(size_t n) const;
  virtual SeriesAbstrPtr shiftLeft(size_t n) const;

  virtual SeriesAbstrPtr multiply(SeriesAbstrPtr series) const
      throw(OperationOnUnequalSizeSeriesException);
  virtual SeriesAbstrPtr multiply(double value) const;
  virtual SeriesAbstrPtr add(SeriesAbstrPtr series) const
      throw(OperationOnUnequalSizeSeriesException);
  virtual SeriesAbstrPtr add(double value) const;
  virtual SeriesAbstrPtr subtract(SeriesAbstrPtr series) const
      throw(OperationOnUnequalSizeSeriesException);
  virtual SeriesAbstrPtr subtract(double value) const;
  virtual SeriesAbstrPtr subtractFrom(double value) const;
  virtual SeriesAbstrPtr divide(SeriesAbstrPtr series) const
      throw(OperationOnUnequalSizeSeriesException, DivideByZeroException);
  virtual SeriesAbstrPtr divide(double value) const
      throw(DivideByZeroException);
  // this is value/series
  virtual SeriesAbstrPtr divideBy(double value) const
      throw(DivideByZeroException);

  // indicators
  virtual SeriesAbstrPtr SMA(unsigned int period) const;
  virtual SeriesAbstrPtr EMA(unsigned int period) const;
  virtual SeriesAbstrPtr EMA(unsigned int period, double exp) const;
  virtual SeriesAbstrPtr WMA(unsigned int period) const;
  virtual SeriesAbstrPtr AroonDown(unsigned int period) const;
  virtual SeriesAbstrPtr AroonUp(unsigned int period) const;
  //  virtual SeriesConstPtr AroonOscillator( const Series& series, unsigned int
  //  period ) const;
  virtual SeriesAbstrPtr ROC(unsigned int period) const;
  //  virtual SeriesConstPtr MACDFix( const Series& series ) const;
  virtual SeriesAbstrPtr BBandUpper(unsigned int period, double stdDev) const;
  virtual SeriesAbstrPtr BBandLower(unsigned int period, double stdDev) const;
  virtual SeriesAbstrPtr DEMA(unsigned int period) const;
  virtual SeriesAbstrPtr HTTrendline() const;
  virtual SeriesAbstrPtr KAMA(unsigned int period) const;
  virtual SeriesAbstrPtr MAMA(double fastLimit, double slowLimit) const;
  virtual SeriesAbstrPtr FAMA(double fastLimit, double slowLimit) const;
  virtual SeriesAbstrPtr Min(unsigned int period) const;
  virtual SeriesAbstrPtr Max(unsigned int period) const;
  virtual SeriesAbstrPtr MinIndex(unsigned int period) const;
  virtual SeriesAbstrPtr MaxIndex(unsigned int period) const;
  virtual SeriesAbstrPtr MidPoint(unsigned int period) const;
  virtual SeriesAbstrPtr PPO(unsigned int fastPeriod, unsigned int slowPeriod,
                             MAType maType) const;
  virtual SeriesAbstrPtr ROCP(unsigned int period) const;
  virtual SeriesAbstrPtr ROCR(unsigned int period) const;
  virtual SeriesAbstrPtr ROCR100(unsigned int period) const;
  virtual SeriesAbstrPtr RSI(unsigned int period) const;
  virtual SeriesAbstrPtr TRIX(unsigned int period) const;
  virtual SeriesAbstrPtr HTDCPeriod() const;
  virtual SeriesAbstrPtr HTDCPhase() const;
  virtual SeriesAbstrPtr HTPhasorPhase() const;
  virtual SeriesAbstrPtr HTPhasorQuadrature() const;
  virtual SeriesAbstrPtr HTSine() const;
  virtual SeriesAbstrPtr HTLeadSine() const;
  virtual SeriesAbstrPtr HTTrendMode() const;
  virtual SeriesAbstrPtr LinearReg(unsigned int period) const;
  virtual SeriesAbstrPtr LinearRegSlope(unsigned int period) const;
  virtual SeriesAbstrPtr LinearRegAngle(unsigned int period) const;
  virtual SeriesAbstrPtr LinearRegIntercept(unsigned int period) const;
  virtual SeriesAbstrPtr StdDev(unsigned int period, double nbDev) const;
  virtual SeriesAbstrPtr Variance(unsigned int period, double nbDev) const;
  virtual SeriesAbstrPtr Correlation(SeriesAbstrPtr series,
                                     unsigned int period) const;
  virtual SeriesAbstrPtr Beta(SeriesAbstrPtr series, unsigned int period) const;
  virtual SeriesAbstrPtr TSF(unsigned int period) const;
  virtual SeriesAbstrPtr CMO(unsigned int period) const;
  virtual SeriesAbstrPtr MOM(unsigned int period) const;
  virtual SeriesAbstrPtr Momentum(unsigned int period) const;
  virtual SeriesAbstrPtr MACD(unsigned int fastPeriod, unsigned int slowPeriod,
                              unsigned int signalPeriod) const;
  virtual SeriesAbstrPtr MACDSignal(unsigned int fastPeriod,
                                    unsigned int slowPeriod,
                                    unsigned int signalPeriod) const;
  virtual SeriesAbstrPtr MACDHist(unsigned int fastPeriod,
                                  unsigned int slowPeriod,
                                  unsigned int signalPeriod) const;
  virtual SeriesAbstrPtr MACDExt(unsigned int fastPeriod, MAType fastMAType,
                                 unsigned int slowPeriod, MAType slowMAType,
                                 unsigned int signalPeriod,
                                 MAType signalMAType) const;
  virtual SeriesAbstrPtr MACDSignalExt(
      unsigned int fastPeriod, MAType fastMAType, unsigned int slowPeriod,
      MAType slowMAType, unsigned int signalPeriod, MAType signalMAType) const;
  virtual SeriesAbstrPtr MACDHistExt(unsigned int fastPeriod, MAType fastMAType,
                                     unsigned int slowPeriod, MAType slowMAType,
                                     unsigned int signalPeriod,
                                     MAType signalMAType) const;
  virtual SeriesAbstrPtr MACDFix(unsigned int period) const;
  virtual SeriesAbstrPtr MACDSignalFix(unsigned int period) const;
  virtual SeriesAbstrPtr MACDHistFix(unsigned int period) const;
  virtual SeriesAbstrPtr APO(unsigned int fastPeriod, unsigned int slowPeriod,
                             MAType maType) const;
  virtual SeriesAbstrPtr T3(unsigned int period, double vFactor) const;
  virtual SeriesAbstrPtr TEMA(unsigned int period) const;
  virtual SeriesAbstrPtr TRIMA(unsigned int period) const;
  virtual SeriesAbstrPtr StochRSIFastK(int period, int fastKPeriod,
                                       int fastDPeriod,
                                       MAType fastDMAType) const;
  virtual SeriesAbstrPtr StochRSIFastD(int period, int fastKPeriod,
                                       int fastDPeriod,
                                       MAType fastDMAType) const;
  virtual SeriesAbstrPtr Sin() const;
  virtual SeriesAbstrPtr Cos() const;
  virtual SeriesAbstrPtr Tan() const;
  virtual SeriesAbstrPtr Cosh() const;
  virtual SeriesAbstrPtr Sinh() const;
  virtual SeriesAbstrPtr Tanh() const;
  virtual SeriesAbstrPtr Acos() const;
  virtual SeriesAbstrPtr Asin() const;
  virtual SeriesAbstrPtr Atan() const;
  virtual SeriesAbstrPtr Ceil() const;
  virtual SeriesAbstrPtr Floor() const;
  virtual SeriesAbstrPtr Exp() const;
  virtual SeriesAbstrPtr Sqrt() const;
  virtual SeriesAbstrPtr Ln() const;
  virtual SeriesAbstrPtr Log10() const;
};
