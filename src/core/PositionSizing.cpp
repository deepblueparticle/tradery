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

#include "stdafx.h"
#include "positions.h"

class Exits {
 private:
  typedef std::multimap<DateTime, tradery::Position> DatePosMap;

  DatePosMap _map;

 public:
  unsigned __int64 openPosCount() const { return _map.size(); }

  void closePositions(DateTime dt, tradery::OrderType type) {
    for (DatePosMap::iterator i = _map.begin(); i != _map.end();) {
      if (i->first < dt)
        _map.erase(i++);
      else if (i->first == dt) {
        // fine tuning of the algorithm if the 2 positions are on the same bar
        OrderType exitOrderType = i->second.getExitOrderType();
        // if the position is exiting at market
        // or the position entrying is at close,
        // then it's safe to close the exiting position, because they won't
        // overlap
        if (exitOrderType == market_order || type == close_order)
          _map.erase(i++);
        else
          i++;
        // other combinations are unsafe, we don't know which occurred first, so
        // we can't close or they may overlap
      } else
        break;
    }
  }

  void addPosition(tradery::Position pos) {
    // if the position is open, it means it will never be closed, so simulate
    // this by setting the associated time to + infinity
    //    std::cout << _T( "in pos sizing, addPosition" ) << std::endl;
    if (pos.isOpen())
      _map.insert(DatePosMap::value_type(PosInfinityDateTime(), pos));
    else
      _map.insert(DatePosMap::value_type(pos.getCloseTime(), pos));
  }
};

class PosSizingHandler : public PositionHandler {
 private:
  unsigned __int64 _maxOpenPos;
  PositionsContainerImpl& _pci;
  Exits _exits;

 public:
  PosSizingHandler(PositionsContainer& pc, unsigned __int64 maxOpenPos)
      : _maxOpenPos(maxOpenPos),
        _pci(dynamic_cast<PositionsContainerImpl&>(pc)) {}

  // called on each position
  virtual void onPosition(tradery::Position pos) {
    assert(_exits.openPosCount() <= _maxOpenPos);

    // entry time stamp
    const DateTime entry(pos.getEntryTime());

    // adjust the collection of still open positions at the date/time of entry
    // of the current position
    _exits.closePositions(entry, pos.getEntryOrderType());

    //      std::cout << _T( "in pos sizing handler, onPosition before test2" )
    //      << std::endl;
    if (_exits.openPosCount() == _maxOpenPos) {
      //        std::cout << _T( "in pos sizing handler, onPosition in test2" )
      //        << std::endl;
      pos.disable();
    } else if (_exits.openPosCount() < _maxOpenPos) {
      _exits.addPosition(pos);
    }
  }
};

PositionSizing::PositionSizing(PositionsContainer& pc,
                               const PositionSizingParams& psp)
    : _pc(pc), _psp(psp) {
  apply();
}

void PositionSizing::apply() {
  LOG(log_debug, "in pos sizing, apply, before sort");
  // first sort by entry date
  _pc.sortByEntryTime();
  LOG(log_debug, "in pos sizing, apply before test unlimited pos");
  if (!_psp.maxOpenPos().unlimited()) {
    LOG(log_debug, "in pos sizing, before pos handler");
    // if not unlimited number of positions, do the position sizing
    // now go through the positions
    PosSizingHandler psh(_pc, _psp.maxOpenPos().get());
    _pc.forEach(psh);
  }
}
