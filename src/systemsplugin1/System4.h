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

/**
 * IMPORTANT DISCLAIMER
 *
 * These sample trading systems are presented for illustrative purpose only and
 * are not intended for use in real trading.
 *
 */

#pragma once

/**
 * Illustrates shows that even a fatal error will not crash the framework or
 * affect any other systems at runtime This system tries to dreference a null
 * pointer, thus triggering a null pointer exception
 */
class System4 : public BarSystem<System4> {
 public:
  System4(const std::vector<std::string>* params = 0)
      : BarSystem<System4>(
            Info("4CC8D610-756F-457e-A0E5-468306E8F23A",
                 "System 4 - intentional \"Access violation error\"",
                 "Illustrates how usually fatal errors (such as access "
                 "violation) in one system will not crash the framework or "
                 "affect the other running systems")) {}

  virtual void onBar(Index bar) {
    // define a pointer to a char, and setting it to point to address 0
    char* p = 0;
    // trying to set the char at address 0 to 'a' will trigger a system
    // exception
    *p = 'a';
  }

  virtual void run() {
    // run the data handler for each bar
    FOR_EACH_BAR(0);
  }
};
