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

#include "datasource.h"

namespace tradery {
class Bars;
class TimeSeries;
/* @cond */
class CORE_API Synchronizer {
 public:
  static Synchronizer* create(Bars ref, Bars syncd);
  virtual ~Synchronizer() {}
  virtual int index(size_t ix) const = 0;
  virtual size_t size() const = 0;
  virtual bool modified() const = 0;
  virtual bool operator==(const Synchronizer& synchronizer) const = 0;
  virtual bool operator==(const Synchronizer* synchronizer) const = 0;
  bool operator!=(const Synchronizer& synchronizer) const {
    return !(*this == synchronizer);
  }
  bool operator!=(const Synchronizer* synchronizer) const {
    return !(*this == synchronizer);
  }
  virtual const std::string& refSymbol() const = 0;
  virtual TimeSeries timeSeries() const = 0;
};
/* @endcond */

typedef ManagedPtr<Synchronizer> SynchronizerPtr;
}  // namespace tradery