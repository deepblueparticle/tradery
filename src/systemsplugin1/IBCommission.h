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

class IBCommission : public ClonableImpl<Commission, IBCommission>,
                     public Commission {
 private:
  const bool _apiFee;

 public:
  // constructor - takes a bool indicating whether it will be applied to an
  // account that is charged an API fee
  IBCommission(const std::vector<std::string>* params = 0)
      : _apiFee(false),
        Commission(Info("12601F2C-5C44-45bb-8646-0AA07816EDE6",
                        "Interactive Brokers commission",
                        "Implements the current commission schedule for "
                        "Interactive brokers, which is dependednt on the "
                        "number of shares traded")) {}

  // calculates the commission
  virtual double getValue(unsigned long shares, double price) const {
    // IB commission in the US has three components
    double commission1 = 0;
    double commission2 = 0;
    double commission3 = 0;

    unsigned long shares1;
    unsigned long shares2;

    if (shares > 500) {
      // if the number of shares is > 500
      // the first component is calculated on the first 500 shares
      // and the second, on everything above 500
      shares1 = 500;
      shares2 = shares - 500;
    } else {
      // if less than 500 shares, it's all component 1
      shares1 = shares;
      shares2 = 0;
    }

    // component 1 is $0.01 * shares of component 1
    commission1 = 0.01 * shares1;

    // there is a minimum of $1
    if (commission1 < 1) commission1 = 1;

    // there is a max of 2% of the value of the trade
    if (commission1 > shares * price * 0.02)
      commission1 = shares * price * 0.02;

    // component 2 is $0.005 * shares above 500
    commission2 = 0.005 * shares2;

    // there is a max of 2% of the value of the trade
    if (commission2 > shares * price * 0.02)
      commission2 = shares * price * 0.02;

    if (_apiFee) {
      // if there is an API fee, add $0.003 for each share of the order
      commission3 = 0.03 * shares;

      // thre is a max of 2% of the value of the trade
      if (commission3 > shares * price * 0.02)
        commission3 = shares * price * 0.02;
    }

    // return total commission for the order
    return commission1 + commission2 + commission3;
  }
};