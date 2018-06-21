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

class SimpleSlippage : public Slippage,
                       public ClonableImpl<Slippage, SimpleSlippage> {
 private:
  double _slippage;

 public:
  SimpleSlippage(const std::vector<std::string>* params = 0)
      : Slippage(Info("6B4C1ADB-3C98-416a-A026-78494EE08729",
                      "A simple slippage configuration",
                      "A simple slippage, with the slippage value (in %) "
                      "passed in the params first element")) {
    if (params != 0) {
      std::istringstream i((*params)[0]);
      i >> _slippage;
    } else
      _slippage = 0;

    //    std::cout << "slippage constructor: " << _slippage << std::endl;
  }

  virtual ~SimpleSlippage(void) {}

  virtual double getValue(unsigned long shares, unsigned long volume,
                          double price) const {
    return price * _slippage / 100.0;
  }
};

////////////////////////////////////////////////////////////////
// slippage plugin
class SimpleSlippagePlugin : public SimplePlugin<Slippage> {
 public:
  virtual void init() { insert<SimpleSlippage>(); }

  SimpleSlippagePlugin()
      : SimplePlugin<Slippage>(Info("3FD34BD4-68CB-4ee3-B3F8-A79FFC8F6506",
                                    "Simple slippage plugin",
                                    "Simple slippage plugin - for the moment "
                                    "just one very simple configuration")) {}
};
