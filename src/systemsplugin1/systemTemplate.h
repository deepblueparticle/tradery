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

class SystemTemplate : public BarSystem<SystemTemplate> {
 public:
  SystemTemplate(const std::vector<std::string>* params = 0)
      // set the system name and the holding period to 2 days, profit target to
      // 5%
      : BarSystem<SystemTemplate>(Info("665C0513-4388-4db2-9346-24A4482A123F",
                                       "System template", "")) {}

 private:
  //************************************
  // Optional
  // systemwide variables, if any, here
  //***********************************

  bool init(const String& symbol) {
    std::cout << "SystemTemplate::init" << std::endl;
    //************************************
    // Optional
    // intialization of systemwide variables
    // or other resources used by the system here
    //************************************

    return true;
  }

  void run() {
    //************************************
    // replace 10 with the number of bars
    // the system should wait before starting
    // the processing
    //************************************
    FOR_EACH_BAR(10);
  }

  void onBar(Index bar) {
    APPLY_AUTO_STOPS(bar);
    FOR_EACH_OPEN_POSITION(bar);

    //************************************
    // position entry rules here
    //************************************
  }

  bool onOpenPosition(tradery::Position pos, Index bar) {
    //************************************
    // position exit rules here
    //************************************
    return true;
  }

  void cleanup() {
    //************************************
    // Optional
    // variable or resource cleanup here
    //************************************
  }
};