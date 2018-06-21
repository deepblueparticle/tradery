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

#include "cache.h"
#include "seriesimpl.h"

class TicksImpl : public Ticks, public Ideable {
  std::auto_ptr<SeriesImpl> _price;
  std::auto_ptr<SeriesImpl> _size;
  std::auto_ptr<TickTypeSeries> _type;
  std::auto_ptr<ExchangeSeries> _exchange;
  TimeSeries _time;

 public:
  TicksImpl(const std::string& dataSourceName, const std::string& symbol,
            const Range* range)
      : Ticks(symbol),
        Ideable(dataSourceName + " - ticks - " + symbol +
                (range == 0 ? "" : " - range: " + range->getId())),
        _price(new SeriesImpl(getId() + " - tick price - ")),
        _size(new SeriesImpl(getId() + " - tick size - ")),
        _type(new TickTypeSeries()),
        _exchange(new ExchangeSeries()) {}

  virtual ~TicksImpl() {}

  void add(const Tick& tick) {
    _price->push_back(tick.price());
    _size->push_back(tick.size());
    _type->push_back(tick.type());
    _exchange->push_back(tick.exchange());
    _time.push_back(tick.time());
  }

  size_t size() const {
    assert(_price->size() == _size->size());
    assert(_price->size() == _type->size());
    assert(_price->size() == _exchange->size());
    assert(_price->size() == _time.size());

    return _price->size();
  }

  virtual void forEach(TickHandler& tickHandler, size_t startBar = 0) const
      throw(TickIndexOutOfRangeException) {
    if (startBar >= size())
      throw TickIndexOutOfRangeException(size(), startBar, getSymbol());

    for (size_t bar = startBar; bar < size(); bar++)
      tickHandler.dataHandler(*this, bar);
  }

  virtual const Tick get(size_t index) const {
    return Tick(_time.at(index), _price->at(index),
                (unsigned long)_size->at(index), _type->at(index),
                _exchange->at(index));
  }

  bool hasInvalidData() const { return false; }

  std::string getInvalidDataAsString() const { return ""; }
};