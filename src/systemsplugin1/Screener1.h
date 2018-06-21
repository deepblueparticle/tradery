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

class Screener1 : public BarSystem<Screener1> {
 public:
  Screener1(void) {}

  ~Screener1(void) {}

  void run() {
    // if volume on the last bar (which is barsCount() - 1 )
    // is 1.1 times the volume of the previous bar - barsCount() - 2
    // and (the && operator)
    // if volume of the previous bar is 1.1 times
    // the volume 2 bars ago,
    // then print the current default symbol in the output window

    if (volume(barsCount() - 1) > 1.1 * volume(barsCount() - 2) &&
        volume(barsCount() - 2) > 1.1 * volume(barsCount() - 3))
      PRINT_LINE(defSymbol());
  }
};
