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

#include "datetime.h"
namespace tradery {
/** @endcond */

/**
 * \brief The base class for all specific elements of data, such as Bar, Tick
 * and others
 *
 * Represents a generic unit of data, which has a time stamp. Specific units of
 * data such as bars and ticks build on this and add various other fields that
 * define them
 */
class DataUnit {
 private:
  const DateTime _time;

 public:
  virtual ~DataUnit() {}

  /**
   * Constructor - takes a time stamp as argument
   *
   * @param time   The timestamp of the data unit
   */
  DataUnit(const DateTime& time) : _time(time) {}

  /**
   * Returns the time stamp of the data unit
   *
   * @return time stamp
   */
  const DateTime& time() const { return _time; }

  const Date date() const { return _time.date(); }
};
}  // namespace tradery