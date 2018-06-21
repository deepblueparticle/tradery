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

class SimpleCommission : public ClonableImpl<Commission, SimpleCommission>,
                         public Commission {
 private:
  const double _commission;

 private:
  double init(const std::vector<std::string>* params) {
    if (params != 0) {
      double commission;
      std::istringstream i((*params)[0]);
      i >> commission;
      return commission;
    } else
      return 0;
  }

 public:
  // constructor - takes a bool indicating whether it will be applied to an
  // account that is charged an API fee
  SimpleCommission(const std::vector<std::string>* params = 0)
      : _commission(init(params)),
        Commission(Info("56EF85F7-2F49-4a8b-8F67-35292E67AA84",
                        "Simple commission",
                        "Simple commission, just returns the value")) {}

  // calculates the commission
  virtual double getValue(unsigned long shares, double price) const {
    return _commission;
  }
};

// commission plugin
class SimpleCommissionPlugin : public SimplePlugin<Commission> {
 public:
  virtual void init() { insert<SimpleCommission>(); }

  SimpleCommissionPlugin()
      : SimplePlugin<Commission>(Info(
            "52C4DB04-DDD9-480b-9FEE-9169E83A6A4E", "Sample commission plugin",
            "Sample commission plugin - for the moment just an Interactive "
            "Brokers commission model")) {}
};
