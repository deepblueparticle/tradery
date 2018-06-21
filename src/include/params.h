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

#include <boost/shared_ptr.hpp>
#include "datetime.h"
#include "misc.h"
#include "defaults.h"
#include "datetimerange.h"

class MaxOpenPositions {
 private:
  unsigned __int64 _maxOpenPos;

 public:
  MaxOpenPositions() : _maxOpenPos(0) {}
  MaxOpenPositions(__int64 mop) : _maxOpenPos(mop) {}

  bool unlimited() const { return _maxOpenPos == 0; }
  unsigned __int64 get() const {
    assert(!unlimited());
    return _maxOpenPos;
  }
  unsigned __int64 getValue() const { return _maxOpenPos; }
  void setUnlimited() { _maxOpenPos = 0; }
  void set(unsigned __int64 maxOpenPos) { _maxOpenPos = maxOpenPos; }

  std::wstring toString() const {
    std::wostringstream os;

    if (unlimited())
      os << "unlimited";
    else
      os << _maxOpenPos;

    return os.str();
  }
};

class PositionSizingParams {
 public:
  enum PosSizeType { system, shares, size, pctEquity, pctCash };

  enum PosSizeLimitType { none, pctVolume, limit };

  virtual ~PositionSizingParams() {}
  virtual double initialCapital() const = 0;
  virtual void setInitialCapital(double capital) = 0;
  virtual void setMaxOpenPos(const MaxOpenPositions& maxOpenPos) = 0;
  virtual const MaxOpenPositions maxOpenPos() const = 0;
  virtual PosSizeType posSizeType() const = 0;
  virtual double posSize() const = 0;
  virtual PosSizeLimitType posSizeLimitType() const = 0;
  virtual double posSizeLimit() const = 0;

  /*std::ostream& dump( std::ostream& os ) const
  {
    os << _T( "initial capital: " ) << _initialCapital << std::endl <<
          _T( "max open pos: " ) << _maxOpenPos.toString() << std::endl <<
          _T( "pos size type: " ) << _posSizeType << std::endl <<
          _T( "pos size: " ) << _posSize << std::endl <<
          _T( "pos size limit type: " ) << _posSizeLimitType << std::endl <<
          _T( "pos size limit: " ) << _posSizeLimit << std::endl;

    return os;
  }
  */
};

namespace tradery {
class SymbolsIterator;
}
/**
 * Abstract class containing various runtime parameters (range, threads,
 * position sizing) as set in the session runtime parameters.
 *
 * @see SessionInfo
 * @see PositionSizing
 */
class RuntimeParams {
 public:
  virtual ~RuntimeParams() {}

  virtual DateTime startTradesDateTime() const = 0;

  /**
   * Gets the number of threads to run the session in
   *
   * @return The number of threads
   */
  virtual unsigned long getThreads() const = 0;
  /**
   * Returns a pointer to the session range
   *
   * @return Pointer to range
   * @exception DateTimeRangeException
   * @see Range
   * @see DateTime
   */
  virtual DateTimeRangePtr getRange() const
      throw(tradery::DateTimeRangeException) = 0;
  /**
   * Returns a pointer to the current session position sizing parameters
   *
   * @return Pointer to a PositionSizing
   * @see PositionSizing
   */
  virtual const PositionSizingParams* positionSizing() const = 0;

  virtual bool chartsEnabled() const = 0;
  virtual bool equityCurveEnabled() const = 0;
  virtual bool statsEnabled() const = 0;
  virtual bool tradesEnabled() const = 0;
  virtual bool signalsEnabled() const = 0;
  virtual bool outputEnabled() const = 0;
};
