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

class SampleSlippage : public Slippage,
                       public ClonableImpl<Slippage, SampleSlippage> {
 public:
  SampleSlippage(const std::vector<std::string>* params = 0)
      : Slippage(Info("37124D71-5B28-4516-B07F-2835AA818AD6",
                      "A simple slippage",
                      "A simple implementation meant to illustrate the "
                      "createion of a slippage plugin")) {}

  virtual ~SampleSlippage(void) {}

  // just return a const value for now
  virtual double getValue(unsigned long shares, unsigned long volume,
                          double price) const {
    return 0.01;
  }
};
