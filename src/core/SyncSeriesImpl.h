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

#include "seriesimpl.h"
class SeriesException {};
/*
class SyncSeriesImpl : public SeriesImpl
{
private:
  // the synchronizer
  SynchronizerPtr _synchronizer;
  // the series to syncronize
  Series _sourceSeries;
  mutable bool _vUpToDate;

private:
  void copy() const
  {
    if( !_vUpToDate )
    {
      for( size_t n = 0; n < _synchronizer->size(); n++ )
      {
        int ix = _synchronizer->index( n );
        _v[ n ] = ix < 0 ? 0 : _sourceSeries[ ix ];
      }
      _vUpToDate = true;
    }
    else
      assert( _v.size() == _synchronizer->size() );
  }

public:
  SyncSeriesImpl( SynchronizerPtr synchronizer, Series syncSeries )
  : _synchronizer( synchronizer ), _sourceSeries( syncSeries ), _vUpToDate(
false )
  {
  }

  virtual bool isSynchronized() const
  {
    return true;
  }
  virtual SynchronizerPtr synchronizer() const
  {
    return _synchronizer;
  }

  virtual Series syncSeries() const
  {
    return _sourceSeries;
  }

  virtual SeriesAbstrPtr clone() const
  {
    assert( _synchronizer );
    // todo: make sure the synchronizer and syncseries have the same size,
    // if not, throw exception
    // do this for each call
    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries ) );
  }

  virtual double getValue( size_t index ) const throw
(SeriesIndexOutOfRangeException)
  {
    assert( _synchronizer );
    return _sourceSeries[ _synchronizer->index( index ) ];
  }

  virtual double& getRef( size_t index ) throw (SeriesIndexOutOfRangeException)
  {
    assert( _synchronizer );

        // invalidate the synchronized vector, in case the data is modified
    _vUpToDate = false;
    // return a ref to the source series

    return _sourceSeries[ _synchronizer->index( index ) ];
  }

  virtual size_t size() const
  {
    assert( _synchronizer );
    return _synchronizer->size();
  }

  virtual void push_back( double value )
  {
    //
    // throw exception
    // can't modify synchronized series
    throw SeriesException();
  }

  virtual const double* getArray() const
  {
    assert( _synchronizer );

    // make sure the local vector has the data
    copy();

    return (const double* )( &front() );
  }

  virtual const std::vector< double >& getVector() const
  {
    assert( _synchronizer );

    // update the synchronized vector
    copy();
    return _v;
  }

  virtual SeriesAbstr& operator=( SeriesAbstrPtr series ) throw (
OperationOnUnequalSizeSeriesException )
  {
    assert( _synchronizer );

        //todo don't allow copy on sync series - throw exception
    throw SeriesException();
  }

  virtual SeriesAbstrPtr operator*( SeriesAbstrPtr series ) const throw(
OperationOnUnequalSizeSeriesException )
  {
    assert( _synchronizer.get() );
    







    // todo - make sure it's synchronized to the same series
    if( series->isSynchronized() )
      return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries *
Series( series ) ) ); else
    {
      copy();
      return __super::operator*( series );
    }
  }

  virtual SeriesAbstr& operator*=( SeriesAbstrPtr series ) throw (
OperationOnUnequalSizeSeriesException )
  {
    // can't assign to a synchronized series
    throw SeriesException();
  }

  virtual SeriesAbstrPtr operator*( double value ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries *
value ) );
  }

  virtual SeriesAbstr& operator*=( double value )
  {
    throw SeriesException();
  }

  virtual SeriesAbstrPtr operator+( SeriesAbstrPtr series ) const throw
(OperationOnUnequalSizeSeriesException)
  {
    assert( _synchronizer );
    







    if( series->isSynchronized() )
      return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries +
Series( series ) ) ); else
    {
      copy();
      return __super::operator+( series );
    }
  }

  virtual SeriesAbstr& operator+=( SeriesAbstrPtr series ) throw
(OperationOnUnequalSizeSeriesException)
  {
    throw SeriesException();
  }

  virtual SeriesAbstrPtr operator+( double value ) const
  {
    assert( _synchronizer );
    







    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries +
value ) );
  }
  virtual SeriesAbstr& operator+=( double value )
  {
    throw SeriesException();
  }
  virtual SeriesAbstrPtr operator-( SeriesAbstrPtr series ) const throw(
OperationOnUnequalSizeSeriesException)
  {
    assert( _synchronizer );
    







    if( series->isSynchronized() )
      return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries -
Series( series) ) ); else
    {
      copy();
      return __super::operator-( series );
    }
  }

  virtual SeriesAbstr& operator-=( SeriesAbstrPtr series ) throw(
OperationOnUnequalSizeSeriesException)
  {
    throw SeriesException();
  }

  virtual SeriesAbstrPtr operator-( double value ) const
  {
    assert( _synchronizer );

    return __super::operator-( value );
  }

  virtual SeriesAbstr& operator-=( double value )
  {
    throw SeriesException();
  }

  virtual SeriesAbstrPtr operator/( SeriesAbstrPtr series ) const throw (
OperationOnUnequalSizeSeriesException, DivideByZeroException )
  {
    assert( _synchronizer );

    if( series->isSynchronized() )
      return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries /
Series( series ) ) ); else
    {
      copy();
      return __super::operator/( series );
    }
  }

  virtual SeriesAbstr& operator/=( SeriesAbstrPtr series ) throw (
OperationOnUnequalSizeSeriesException, DivideByZeroException )
  {
    throw SeriesException();
  }

  virtual SeriesAbstrPtr operator/( double value ) const throw (
DivideByZeroException )
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries /
value ) );
  }

  virtual SeriesAbstr& operator/=( double value ) throw ( DivideByZeroException
)
  {
    throw SeriesException();
  }

  virtual SeriesAbstrPtr Min( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.Min(
period )  ) );
  }

  virtual SeriesAbstrPtr Max( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.Max(
period )  ) );
  }

  virtual bool crossOver( size_t index, SeriesAbstrPtr series ) const throw
(OperationOnUnequalSizeSeriesException, SeriesIndexOutOfRangeException)
  {
    assert( _synchronizer );

    if( series->isSynchronized() )
      return _sourceSeries.crossOver( index, Series( series ) );
    else
    {
      copy();
      return __super::crossOver( index, series );
    }
  }

  virtual bool crossOver( size_t index, double d ) const throw
(SeriesIndexOutOfRangeException)
  {
    assert( _synchronizer );

    return _sourceSeries.crossOver( index, d );
  }

  virtual bool crossUnder( size_t index, SeriesAbstrPtr series ) const throw
(OperationOnUnequalSizeSeriesException, SeriesIndexOutOfRangeException)
  {
    assert( _synchronizer );

    if( series->isSynchronized() )
      return _sourceSeries.crossUnder( index, Series( series ) );
    else
    {
      copy();
      return __super::crossUnder( index, series );
    }
  }

  virtual bool crossUnder( size_t index, double d ) const throw
(SeriesIndexOutOfRangeException)
  {
    assert( _synchronizer );

    return _sourceSeries.crossUnder( index, d );
  }

  virtual bool turnDown( size_t index ) const throw
(SeriesIndexOutOfRangeException)
  {
    assert( _synchronizer );

    return _sourceSeries.turnDown( index );
  }

  virtual bool turnUp( size_t index ) const throw
(SeriesIndexOutOfRangeException)
  {
    assert( _synchronizer );

    return _sourceSeries.turnUp( index );
  }

  virtual SeriesAbstrPtr SMA( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.SMA(
period ) ) );
  }

  virtual SeriesAbstrPtr EMA( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.EMA(
period ) ) );
  }

  virtual SeriesAbstrPtr EMA( unsigned int period, double exp ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.EMA(
period, exp ) ) );
  }

  virtual SeriesAbstrPtr WMA( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.WMA(
period ) ) );
  }

  virtual SeriesAbstrPtr AroonDown( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.AroonDown( period ) ) );
  }

  virtual SeriesAbstrPtr AroonUp( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.AroonUp( period ) ) );
  }

  virtual SeriesAbstrPtr ROC( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.ROC(
period ) ) );
  }

  virtual SeriesAbstrPtr BBandUpper( unsigned int period, double stdDev ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.BBandUpper( period, stdDev ) ) );
  }

  virtual SeriesAbstrPtr BBandLower( unsigned int period, double stdDev ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.BBandLower( period, stdDev ) ) );
  }

  virtual SeriesAbstrPtr DEMA( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.DEMA( period ) ) );
  }

  virtual SeriesAbstrPtr HTTrendline() const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.HTTrendline() ) );
  }

  virtual SeriesAbstrPtr KAMA( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.KAMA( period ) ) );
  }

  virtual SeriesAbstrPtr MAMA( double fastLimit, double slowLimit ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MAMA( fastLimit, slowLimit ) ) );
  }

  virtual SeriesAbstrPtr FAMA( double fastLimit, double slowLimit ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.FAMA( fastLimit, slowLimit ) ) );
  }

  virtual SeriesAbstrPtr MidPoint( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MidPoint( period ) ) );
  }

  virtual SeriesAbstrPtr PPO( unsigned int fastPeriod, unsigned int slowPeriod,
MAType maType ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.PPO(
fastPeriod, slowPeriod, maType ) ) );
  }

  virtual SeriesAbstrPtr ROCP( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.ROCP( period ) ) );
  }

  virtual SeriesAbstrPtr ROCR( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.ROCR( period ) ) );
  }

  virtual SeriesAbstrPtr ROCR100( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.ROCR100( period ) ) );
  }

  virtual SeriesAbstrPtr RSI( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.RSI(
period ) ) );
  }

  virtual SeriesAbstrPtr TRIX( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.TRIX( period ) ) );
  }

  virtual SeriesAbstrPtr HTDCPeriod() const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.HTDCPeriod() ) );
  }

  virtual SeriesAbstrPtr HTDCPhase() const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.HTDCPhase() ) );
  }

  virtual SeriesAbstrPtr HTPhasorPhase() const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.HTPhasorPhase() ) );
  }

  virtual SeriesAbstrPtr HTPhasorQuadrature() const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.HTPhasorQuadrature() ) );
  }

  virtual SeriesAbstrPtr HTSine() const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.HTSine() ) );
  }

  virtual SeriesAbstrPtr HTLeadSine() const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.HTLeadSine() ) );
  }

  virtual SeriesAbstrPtr HTTrendMode() const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.HTTrendMode() ) );
  }

  virtual SeriesAbstrPtr LinearReg( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.LinearReg( period ) ) );
  }

  virtual SeriesAbstrPtr LinearRegSlope( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.LinearRegSlope( period ) ) );
  }

  virtual SeriesAbstrPtr LinearRegAngle( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.LinearRegAngle( period ) ) );
  }

  virtual SeriesAbstrPtr LinearRegIntercept( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.LinearRegIntercept( period ) ) );
  }

  virtual SeriesAbstrPtr StdDev( unsigned int period, double nbDev ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.StdDev( period, nbDev ) ) );
  }

  virtual SeriesAbstrPtr Variance( unsigned int period, double nbDev ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.Variance( period, nbDev ) ) );
  }

  virtual SeriesAbstrPtr MOM( unsigned int period ) const
  {
    assert( _synchronizer );
 






    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.MOM(
period ) ) );
  }

  virtual SeriesAbstrPtr Momentum( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.Momentum( period ) ) );
  }

  virtual SeriesAbstrPtr MACD( unsigned int fastPeriod, unsigned int slowPeriod,
unsigned int signalPeriod ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MACD( fastPeriod, slowPeriod, signalPeriod ) ) );
  }

  virtual SeriesAbstrPtr MACDSignal( unsigned int fastPeriod, unsigned int
slowPeriod, unsigned int signalPeriod ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MACDSignal( fastPeriod, slowPeriod, signalPeriod ) ) );
  }

  virtual SeriesAbstrPtr MACDHist( unsigned int fastPeriod, unsigned int
slowPeriod, unsigned int signalPeriod ) const
  {
    assert( _synchronizer );
 






    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MACDHist( fastPeriod, slowPeriod, signalPeriod ) ) );
  }

  virtual SeriesAbstrPtr MACDExt( unsigned int fastPeriod, MAType fastMAType,
unsigned int slowPeriod, MAType slowMAType, unsigned int signalPeriod, MAType
signalMAType ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MACDExt( fastPeriod, fastMAType, slowPeriod, slowMAType,
signalPeriod, signalMAType ) ) );
  }

  virtual SeriesAbstrPtr MACDSignalExt( unsigned int fastPeriod, MAType
fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int
signalPeriod, MAType signalMAType ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MACDSignalExt( fastPeriod, fastMAType, slowPeriod, slowMAType,
signalPeriod, signalMAType ) ) );
  }

  virtual SeriesAbstrPtr MACDHistExt( unsigned int fastPeriod, MAType
fastMAType, unsigned int slowPeriod, MAType slowMAType, unsigned int
signalPeriod, MAType signalMAType ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MACDHistExt( fastPeriod, fastMAType, slowPeriod, slowMAType,
signalPeriod, signalMAType ) ) );
  }

  virtual SeriesAbstrPtr MACDFix( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MACDFix( period ) ) );
  }

  virtual SeriesAbstrPtr MACDSignalFix( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MACDSignalFix( period ) ) );
  }

  virtual SeriesAbstrPtr MACDHistFix( unsigned int period ) const
  {
    assert( _synchronizer );
 






    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.MACDHistFix( period ) ) );
  }

  virtual SeriesAbstrPtr APO( unsigned int fastPeriod, unsigned int slowPeriod,
MAType maType ) const
  {
    assert( _synchronizer );

        return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.APO( fastPeriod, slowPeriod, maType ) ) );
  }

  virtual SeriesAbstrPtr T3( unsigned int period, double vFactor ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer, _sourceSeries.T3(
period, vFactor ) ) );
  }

  virtual SeriesAbstrPtr TEMA( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.TEMA( period ) ) );
  }

  virtual SeriesAbstrPtr TRIMA( unsigned int period ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.TRIMA( period ) ) );
  }

  virtual SeriesAbstrPtr StochRSIFastK( int period, int fastKPeriod, int
fastDPeriod, MAType fastDMAType ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.StochRSIFastK( period, fastKPeriod, fastDPeriod, fastDMAType ) )
);
  }

  virtual SeriesAbstrPtr StochRSIFastD( int period, int fastKPeriod, int
fastDPeriod, MAType fastDMAType ) const
  {
    assert( _synchronizer );

    return SeriesAbstrPtr( new SyncSeriesImpl( _synchronizer,
_sourceSeries.StochRSIFastD( period, fastKPeriod, fastDPeriod, fastDMAType ) )
);
  }

  virtual void shiftRight( size_t n )
  {
    assert( _synchronizer );

    _sourceSeries.shiftRight( n );
    _vUpToDate = false;
  }
  virtual void shiftLeft( size_t n )
  {
    assert( _synchronizer );

    _sourceSeries.shiftLeft( n );
    _vUpToDate = false;
  }
};
*/
