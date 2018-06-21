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

#include "position.h"
#include "bars.h"

using std::list;

// TODO: make this thread safe for the case where multiple threads are writing
// positioins in the same list.

class PosPtrList : public std::list<PositionAbstrPtr> {
 public:
  PosPtrList() {
    //    tsprint( _T( "In PosPtrList constructor\n" ), std::cout );
  }

  virtual ~PosPtrList() {
    /*    std::wostringstream o;

            int n = size();
            o << _T( "In PosPtrList destructor: " ) << size() << _T( "\n" );
            tsprint( o, std::cout );
            int m = 0;
    */
  }
};

typedef PosPtrList BaseContainer;

typedef std::list<PositionAbstrPtr> OpenPosBaseContainer;

/**
 * Vector of open positions.  Contains indexes to the actual positions
 * Gets adjusted as new positions are open and old ones are closed
 */
class OpenPositions : public OpenPosBaseContainer {
 private:
  iterator find(const PositionAbstrPtr pos) {
    assert(pos);
    // take into account that there can be closed positions because we defer
    // deleting of them
    return std::find(begin(), end(), pos);
  }

  const_iterator find(const PositionAbstrPtr pos) const {
    assert(pos);
    // take into account that there can be closed positions because we defer
    // deleting of them
    return std::find(begin(), end(), pos);
  }

  bool hasPosition(const PositionAbstrPtr pos) const {
    assert(pos);
    return find(pos) != end() && pos->isOpen();
  }

 public:
  OpenPositions() {
    /*    std::wostringstream o;

            int n = size();
            o << _T( "In OpenPositions constructor\n" );
            tsprint( o, std::cout );
    */
  }
  void add(PositionAbstrPtr pos) {
    assert(pos);
    // can only add an open position
    assert(pos->isOpen());
    // cannot add the same position twice
    assert(find(pos) == end());
    push_back(pos);
  }

  void append(OpenPositions& openPos) {
    splice(end(), openPos);
    openPos.clear();
  }

  void remove(const PositionAbstrPtr pos) {
    assert(pos);
    // only remove a position after it has been closed
    assert(pos->isClosed());
    // to remove a position, it has to be there in the first place
    assert(find(pos) != end());
    // don't do this now, defer until we need to read the positions, in
    // forEach...
    //    erase( find( pos ) );
  }

  PositionAbstrPtr getLast() {
    // we need to do this as the last position may be closed (we defer removing
    // the closed positions until it's necessary, like now)
    while (!empty()) {
      if (back()->isClosed())
        // if last position has been closed, remove it and repeat the process
        erase(--end());
      else
        // else, return the last open positions
        return back();
    }
    return 0;
  }

  size_t getCount() const {
    OpenPositions* p = const_cast<OpenPositions*>(this);
    // get rid of closed positions before returning the size
    for (iterator i = p->begin(); i != p->end();) {
      if ((*i)->isClosed())
        i = p->erase(i);
      else
        ++i;
    }
    return p->size();
  }

  // calls handler for each open position, and passes bar too.
  // the trick is that it also removes positions that are no longer open, which
  // improves performance tremendously for lists with lots of positions and open
  // positions (instead of doing it every time a position is closed, we defer
  // removing them until it's necessary, and we do it in block)
  void forEachOpenPosition(OpenPositionHandler& openPositionHandler, Bars bars,
                           size_t bar) {
    forEachOpenPosition(openPositionHandler, bars, bar,
                        PositionEqualAllPredicate());
  }

  void forEachOpenPosition(OpenPositionHandler1& openPositionHandler) {
    forEachOpenPosition(openPositionHandler, PositionEqualAllPredicate());
  }

  virtual void forEachOpenPosition(OpenPositionHandler& openPositionHandler,
                                   Bars bars, size_t bar,
                                   const PositionEqualPredicate& pred) {
    for (iterator i = begin(); i != end();) {
      // we do this for positions that are closed - they need to be removed
      // (left from calls to remove, which defers the remove to this method)
      tradery::Position pos(*i);

      assert(pos);

      if (pos.isClosed())
        i = erase(i);
      else {
        if (pred == pos && !pos.isDisabled())
          if (!openPositionHandler.onOpenPosition(pos, bars, bar)) break;
        ++i;
      }
    }
  }

  virtual void forEachOpenPosition(OpenPositionHandler1& openPositionHandler,
                                   const PositionEqualPredicate& pred) {
    for (iterator i = begin(); i != end();) {
      // we do this for positions that are closed - they need to be removed
      // (left from calls to remove, which defers the remove to this method)
      tradery::Position pos(*i);
      assert(pos);

      if (pos.isClosed())
        i = erase(i);
      else {
        if (pred == pos && !pos.isDisabled())
          if (!openPositionHandler.onOpenPosition(pos)) break;
        ++i;
      }
    }
  }

  void clear() {}
};

class OpenPositionsIteratorImpl : public OpenPositionsIteratorAbstr {
 private:
  OpenPositions& _op;
  OpenPositions::iterator _i;

 public:
  OpenPositionsIteratorImpl(OpenPositions& op) : _op(op), _i(op.begin()) {}

  Position getFirst() {
    _i = _op.begin();
    return getNext();
  }

  Position getNext() {
    if (_i != _op.end()) {
      tradery::Position pos(*_i);
      if (pos.isClosed()) {
        _op.erase(_i++);
        return getNext();
      } else {
        ++_i;
        return pos;
      }
    } else
      return tradery::Position();
  }
};

class PositionsIteratorImpl;

typedef std::map<PositionId, PositionAbstrPtr> PositionIdToPositionMap;

// class PositionsPtrList : private PosPtrList, public PositionsContainer
class PositionsContainerImpl : private BaseContainer,
                               public PositionsContainer {
  friend PositionsIteratorImpl;

 private:
  mutable OpenPositions _openPositions;

  // todo: make this an on demand map
  PositionIdToPositionMap _idsToPositions;

 public:
  virtual ~PositionsContainerImpl() {}

  PositionsContainerImpl() {}

  PositionsContainerImpl(const PositionsContainer* pc) {
    PositionsIteratorConst pi(pc);

    for (tradery::Position p = pi.first(); p; p = pi.next()) {
      //      add
    }
  }

 public:
  virtual OpenPositionsIterator getOpenPositionsIterator() {
    return OpenPositionsIterator(ManagedPtr<OpenPositionsIteratorAbstr>(
        new OpenPositionsIteratorImpl(_openPositions)));
  }

  virtual tradery::Position getPosition(PositionId id) {
    PositionIdToPositionMap::iterator i = _idsToPositions.find(id);

    if (i != _idsToPositions.end())
      return (*i).second;
    else
      return 0;
  }

  //  PositionsPtrList() : _posList( new PosPtrList() ){}
  /**
   * Appends the contents of the argument list to the current list. It also
   * empties the argument list in the process, but without deleting anything. It
   * is equivalent to moving the contents of the argument list over to the end
   * of the current list
   *
   * @param posList Source list
   */
  void add(PositionAbstrPtr pos) {
    assert(pos);
    BaseContainer::push_back(pos);
    if (pos->isOpen()) _openPositions.add(pos);
    // make sure the id is unique
    // todo: assert
    //
    //    std::cout << _T( "adding position with id: " ) << pos->getId() <<
    //    std::endl;
    bool b = _idsToPositions
                 .insert(PositionIdToPositionMap::value_type(pos->getId(), pos))
                 .second;

    assert(b);
  }

  virtual void append(PositionsContainer* posContainer) {
    // TODO: throw an exception if posList is 0
    assert(posContainer != 0);
    try {
      PositionsContainerImpl* p =
          dynamic_cast<PositionsContainerImpl*>(posContainer);
      // attach all the elements in the map
      // and erase the original map
      // todo: make this more efficient

      _idsToPositions.insert(p->_idsToPositions.begin(),
                             p->_idsToPositions.end());
      p->_idsToPositions.erase(p->_idsToPositions.begin(),
                               p->_idsToPositions.end());

      _openPositions.append(p->_openPositions);
      BaseContainer::splice(end(), *p);

    } catch (const std::bad_cast&) {
      // TODO: throw an exception for the user to tell him he cannot derive from
      // Position
      assert(false);
    }
  }

  virtual void nonDestructiveAppend(PositionsContainer* posContainer) {
    assert(posContainer != 0);

    try {
      PositionsContainerImpl* p =
          dynamic_cast<PositionsContainerImpl*>(posContainer);
      // todo: does this just make a copy of the elements, or does it remove
      // them from the source sequance?

      PositionsContainerImpl newPC = *p;
      append(&newPC);
    } catch (const std::bad_cast&) {
      // TODO: throw an exception for the user to tell him he cannot derive from
      // Position
      assert(false);
    }
  }

  virtual size_t count() const { return BaseContainer::size(); }

  virtual size_t enabledCount() const {
    size_t n = 0;
    for (BaseContainer::const_iterator i = __super::begin();
         i != __super::end(); i++) {
      if ((*i)->isEnabled()) n++;
    }

    return n;
  }

  virtual tradery::Position getLastPosition() {
    return empty() ? Position() : BaseContainer::back();
  }

  virtual size_t openPositionsCount() const {
    return _openPositions.getCount();
  }

  void close(const PositionAbstrPtr pos) { _openPositions.remove(pos); }

  virtual std::ostream& dumpMin(std::ostream& os) const;
  virtual std::ostream& dump(std::ostream& os) const;

  virtual tradery::Position getLastOpenPosition() {
    return _openPositions.getLast();
  }

  virtual const tradery::Position getLastOpenPosition() const {
    return _openPositions.getLast();
  }

  virtual void clear() {
    BaseContainer::clear();
    _openPositions.clear();
  }

  /**
   * Predefined sort, sorting by position entry time
   *
   * @param ascending true for ascending, false for descending, default
   * ascending
   * @see sort
   */
  void sortByEntryTime(bool ascending = true) {
    LOG(log_info, "Sorting positions by entry time");
    if (ascending)
      BaseContainer::sort(LessEntryTimePredicate());
    else
      BaseContainer::sort(std::not2(LessEntryTimePredicate()));
  }
  /**
   * Predefined sort, sorting by position exit time
   *
   * @param ascending true for ascending, false for descending, default
   * ascending
   * @see sort
   */
  void sortByExitTime(bool ascending = true) {
    if (ascending)
      BaseContainer::sort(LessCloseTimePredicate());
    else
      BaseContainer::sort(std::not2(LessCloseTimePredicate()));
  }
  /**
   * Predefined sort, sorting by position gain (exit price - entry price)
   * Not closed position will be last (or first for descending).
   *
   * @param ascending true for ascending, false for descending, default
   * ascending
   * @see sort
   */
  virtual void sortByGain(bool ascending = true) {
    if (ascending)
      BaseContainer::sort(LessGainPredicate());
    else
      BaseContainer::sort(std::not2(LessGainPredicate()));
  }
  /**
   * Reverses the order of all positions in the list.
   */
  virtual void reverse() { std::reverse(begin(), end()); }
  /**
   * General sort method, that takes a user defined comparison predicate as
   * parameter.
   *
   * In order to use it, a user will have to wite a concrete class derived from
   * PositionLess and to implement the less method.
   *
   * Thus, sorting can be done in any possible order, given the right predicate,
   * including sorting by multiple fields.
   *
   * @param predicate On object implementing the comparison method for the sort.
   * @param ascending true for ascending sorting, false for descending sorting,
   * true default value
   * @see PositionLess
   */
  virtual void sort(PositionLessPredicate& predicate, bool ascending = true) {
    if (ascending)
      BaseContainer::sort(LessPredicate(predicate));
    else
      BaseContainer::sort(std::not2(LessPredicate(predicate)));
  }

  virtual void forEach(PositionHandler& op) {
    for (BaseContainer::iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (!pos->isDisabled()) op.onPosition(pos);
    }
  }

  virtual void forEach(PositionHandler& op,
                       const PositionEqualPredicate& pred) {
    for (BaseContainer::iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (pred == pos && !pos->isDisabled()) op.onPosition(pos);
    }
  }

  virtual void forEachNot(PositionHandler& op,
                          const PositionEqualPredicate& pred) {
    for (BaseContainer::iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (pred != pos && !pos->isDisabled()) op.onPosition(pos);
    }
  }

  virtual void forEach(PositionEqualPredHandler& predHandler) {
    for (BaseContainer::iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (predHandler == pos && !pos->isDisabled()) predHandler.onPosition(pos);
    }
  }

  virtual void forEachNot(PositionEqualPredHandler& predHandler) {
    for (BaseContainer::iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (predHandler != pos && !pos->isDisabled()) predHandler.onPosition(pos);
    }
  }

  virtual void forEachOr(PositionHandler& positionHandler,
                         std::vector<PositionEqualPredicate*> predicates) {
    for (BaseContainer::iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (!pos->isDisabled()) {
        for (size_t n = 0; n < predicates.size(); n++) {
          if ((*predicates[n]) == pos) {
            positionHandler.onPosition(pos);
            break;
          }
        }
      }
    }
  }

  virtual void forEachAnd(PositionHandler& positionHandler,
                          std::vector<PositionEqualPredicate*> predicates) {
    for (BaseContainer::iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (!pos->isDisabled()) {
        bool b = true;
        for (size_t n = 0; n < predicates.size(); n++) {
          if ((*predicates[n]) != pos) {
            b = false;
            break;
          }
        }
        if (b) positionHandler.onPosition(pos);
      }
    }
  }

  virtual void forEachConst(PositionHandler& op) const {
    for (BaseContainer::const_iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (!pos->isDisabled()) op.onPositionConst(pos);
    }
  }

  virtual void forEachConst(PositionHandler& op,
                            const PositionEqualPredicate& pred) const {
    for (BaseContainer::const_iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (pred == pos && !pos->isDisabled()) op.onPositionConst(pos);
    }
  }

  // this goes through all open pos and closes the ones for which "pr" returns
  // true. this should be used as it guarantees the integrity of the list after
  // removing elements
  virtual void forEachOpenPosition(OpenPositionHandler& openPositionHandler,
                                   Bars bars, size_t bar) {
    _openPositions.forEachOpenPosition(openPositionHandler, bars, bar);
  }

  virtual void forEachOpenPosition(OpenPositionHandler1& openPositionHandler) {
    _openPositions.forEachOpenPosition(openPositionHandler);
  }

  virtual void forEachOpenPosition(OpenPositionHandler& openPositionHandler,
                                   Bars bars, size_t bar,
                                   const PositionEqualPredicate& pred) {
    _openPositions.forEachOpenPosition(openPositionHandler, bars, bar, pred);
  }

  virtual void forEachOpenPosition(OpenPositionHandler1& openPositionHandler,
                                   const PositionEqualPredicate& pred) {
    _openPositions.forEachOpenPosition(openPositionHandler, pred);
  }

  virtual void forEachClosed(PositionHandler& op) {
    for (BaseContainer::iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (pos->isClosed() && !pos->isDisabled()) op.onPosition(pos);
    }
  }

  virtual void forEachClosedConst(PositionHandler& op) const {
    for (BaseContainer::const_iterator i = BaseContainer::begin();
         i != BaseContainer::end(); i++) {
      PositionAbstrPtr pos = *i;
      if (pos->isClosed() && !pos->isDisabled()) op.onPositionConst(pos);
    }
  }
};

/**
 * Position trailing stop data
 */
class TrailingStopData : public Settable2Double {
 public:
  void set(double trigger, double level) {
    Settable2Double::set(trigger, level);
  }
  double getTrigger() const { return Settable2Double::getValue1(); }
  double getLevel() const { return Settable2Double::getValue2(); }
};

class SignalImpl : public Signal {
 private:
  const SignalType _type;
  const std::string _symbol;
  const DateTime _time;
  const size_t _bar;
  unsigned int _shares;
  const double _price;
  const tradery::Position _pos;
  const std::string _name;
  const ConstStringPtr _systemName;
  const ConstStringPtr _systemId;
  const bool _applySignalSizing;

 public:
  // for close limit type order
  // in the case, do not apply signal sizing by default (which is done anyway)
  SignalImpl(SignalType type, const std::string& symbol, DateTime time,
             size_t bar, unsigned int shares, double price,
             const tradery::Position pos, const std::string& name,
             const ConstStringPtr systemName, const ConstStringPtr systemId)
      : _type(type),
        _symbol(symbol),
        _time(time),
        _bar(bar),
        _shares(shares),
        _price(price),
        _pos(pos),
        _name(name),
        _systemName(systemName),
        _applySignalSizing(false),
        _systemId(systemId) {
    assert(_systemName);
    assert(_systemId);
  }

  // for open market type order (no price, no position )
  SignalImpl(SignalType type, const std::string& symbol, DateTime time,
             size_t bar, unsigned int shares, const std::string& name,
             const ConstStringPtr systemName, bool applySignalSizing,
             const ConstStringPtr systemId)
      : _type(type),
        _symbol(symbol),
        _time(time),
        _bar(bar),
        _shares(shares),
        _price(0),
        _name(name),
        _systemName(systemName),
        _applySignalSizing(applySignalSizing),
        _systemId(systemId) {
    assert(_systemName);
    assert(_systemId);
  }

  // for close market type order (no price )
  SignalImpl(SignalType type, const std::string& symbol, DateTime time,
             size_t bar, unsigned int shares, const tradery::Position pos,
             const std::string& name, const ConstStringPtr systemName,
             const ConstStringPtr systemId)
      : _type(type),
        _symbol(symbol),
        _time(time),
        _bar(bar),
        _shares(shares),
        _price(0),
        _pos(pos),
        _name(name),
        _systemName(systemName),
        _applySignalSizing(false),
        _systemId(systemId) {
    assert(_systemName);
    assert(_systemId);
  }

  // for open limit type orders ( no position )
  SignalImpl(SignalType type, const std::string& symbol, DateTime time,
             size_t bar, unsigned int shares, double price,
             const std::string& name, const ConstStringPtr systemName,
             bool applySignalSizing, const ConstStringPtr systemId)
      : _type(type),
        _symbol(symbol),
        _time(time),
        _bar(bar),
        _shares(shares),
        _price(price),
        _name(name),
        _systemName(systemName),
        _applySignalSizing(applySignalSizing),
        _systemId(systemId) {
    assert(_systemName);
    assert(_systemId);
  }

  SignalImpl(const Signal& signal)
      : _type(signal.type()),
        _symbol(signal.symbol()),
        _time(signal.time()),
        _bar(signal.bar()),
        _shares(signal.shares()),
        _price(signal.price()),
        _pos(signal.position()),
        _name(signal.name()),
        _systemName(signal.systemName()),
        _applySignalSizing(signal.applySignalSizing()),
        _systemId(signal.systemId()) {
    assert(_systemName);
    assert(_systemId);
  }

  virtual bool applySignalSizing() const { return _applySignalSizing; }

  virtual void disable() { _shares = 0; }
  virtual bool isEnabled() const { return _shares > 0; }
  virtual bool isDisabled() const { return !isEnabled(); }

  virtual const Signal* clone() const { return new SignalImpl(*this); }

  virtual SignalType type() const { return _type; }

  virtual bool isShort() const {
    return _type == SHORT_AT_MARKET || _type == SHORT_AT_CLOSE ||
           _type == SHORT_AT_STOP || _type == SHORT_AT_LIMIT ||
           _type == COVER_AT_MARKET || _type == COVER_AT_CLOSE ||
           _type == COVER_AT_STOP || _type == COVER_AT_LIMIT;
  }

  virtual bool isLong() const { return !isShort(); }

  virtual bool isEntryPosition() const {
    return _type == BUY_AT_MARKET || _type == BUY_AT_CLOSE ||
           _type == BUY_AT_STOP || _type == BUY_AT_LIMIT ||
           _type == SHORT_AT_MARKET || _type == SHORT_AT_CLOSE ||
           _type == SHORT_AT_STOP || _type == SHORT_AT_LIMIT;
  }

  virtual bool isExitPosition() const { return !isEntryPosition(); }

  virtual bool hasPrice() const {
    return _type == BUY_AT_LIMIT || _type == SHORT_AT_LIMIT ||
           _type == BUY_AT_STOP || _type == SHORT_AT_STOP ||
           _type == SELL_AT_LIMIT || _type == COVER_AT_LIMIT ||
           _type == SELL_AT_STOP || _type == COVER_AT_STOP;
  }

  virtual const std::string& symbol() const { return _symbol; }

  virtual DateTime time() const { return _time; }

  virtual size_t bar() const { return _bar; }

  virtual unsigned int shares() const { return _shares; }

  virtual double price() const { return _price; }

  virtual const tradery::Position position() const { return _pos; }

  virtual const std::string& name() const { return _name; }

  virtual const ConstStringPtr systemName() const { return _systemName; }

  virtual void setShares(unsigned int shares) { _shares = shares; }

  virtual const ConstStringPtr systemId() const { return _systemId; }
};

class SignalHandlerCollection : public SignalHandler,
                                public std::vector<SignalHandler*> {
 public:
  SignalHandlerCollection()
      : SignalHandler(Info("45ED02AB-C2A7-4c25-9E66-24DB06E239A2",
                           "Signal handler collection",
                           "Signal handler collection")) {}

  void add(SignalHandler* signalHandler) {
    //  only add non-null signal handlers
    if (signalHandler != 0) push_back(signalHandler);
  }

  virtual void signal(SignalPtr _signal) {
    for (unsigned int n = 0; n < size(); n++) {
      assert(at(n) != 0);
      at(n)->signal(_signal);
    }
  }
};

/**
 * Manages positions associated with a system
 *
 * It is passed a pointer to a positionPtrList object, which
 * will contain the actual positions.
 */
class PositionsManagerImpl : public PositionsManagerAbstr  //, ObjCount
{
  friend class CX;

 private:
  Slippage* _slippage;
  Commission* _commission;
  const DateTime
      _startTrades;  // start time (inclusive) after which generate trades
  const DateTime
      _endTrades;  // end time (exclusive) until which generate trades
  DoubleSettable _breakEvenStop;
  DoubleSettable _breakEvenStopLong;
  DoubleSettable _breakEvenStopShort;
  DoubleSettable _reverseBreakEvenStop;
  DoubleSettable _reverseBreakEvenStopLong;
  DoubleSettable _reverseBreakEvenStopShort;
  DoubleSettable _profitTarget;
  DoubleSettable _profitTargetShort;
  DoubleSettable _profitTargetLong;
  DoubleSettable _stopLoss;
  DoubleSettable _stopLossLong;
  DoubleSettable _stopLossShort;
  IntSettable _timeBasedExitAtMarket;
  IntSettable _timeBasedExitAtClose;
  TrailingStopData _TTrailingStop;
  SignalHandlerCollection _signalHandlers;
  PositionsContainerImpl* _posContainer;
  ConstStringPtr _systemName;
  ConstStringPtr _systemId;

  bool _acceptVolume0;

  //  static const OrderFilter _defaultOrderFilter;

  OrderFilter* _orderFilter;

 private:
  void validateSymbol(Bars bars, Position pos) const {
    if (bars.getSymbol() != pos.getSymbol())
      throw ClosingPostionOnDifferentSymbolException(pos.getSymbol(),
                                                     bars.getSymbol());
  }

  void validateLimitPrice(size_t barIndex, double price) {
    if (price <= 0) throw InvalidLimitPriceException(barIndex, price);
  }

  void validateStopPrice(size_t barIndex, double price) {
    if (price <= 0) throw InvalidStopPriceException(barIndex, price);
  }

 public:
  virtual void setSystemName(const std::string& str) {
    _systemName = new std::string(str);
  }
  virtual void setSystemName(const ConstStringPtr str) { _systemName = str; }
  virtual const ConstStringPtr systemName() const { return _systemName; }
  virtual void setSystemId(const std::string& str) {
    _systemId = new std::string(str);
  }
  virtual void setSystemId(const ConstStringPtr str) { _systemId = str; }
  virtual const ConstStringPtr systemId() const { return _systemId; }

  virtual tradery::Position getPosition(PositionId id) {
    assert(_posContainer != 0);
    return _posContainer->getPosition(id);
  }

  // TODO: enforce that
  /*  PositionsManagerImpl( PositionsContainer* posContainer )
    : _slippage( 0 ), _commission( 0 ), _orderFilter( 0 )
    {
          //TODO: throw exception if pointer is 0
          assert( posContainer != 0 );
          try
          {
            // do the init here, although I always do it in the member
    initializer list, but now we have to catch the bad_cast exception
            _posContainer = dynamic_cast< PositionsContainerImpl*
    >(posContainer);
          }
          catch ( const std::bad_cast& )
          {
            // TODO: throw an exception for the user to tell him he cannot
    derive from PositionsContainer assert( false );
          }
          }
          */

  PositionsManagerImpl(PositionsContainer* posContainer, DateTime startTrades,
                       DateTime endTrades, Slippage* slippage = 0,
                       Commission* commission = 0, bool acceptVolume0 = true)
      : _slippage(slippage),
        _commission(commission),
        _orderFilter(0),
        _posContainer(dynamic_cast<PositionsContainerImpl*>(posContainer)),
        _startTrades(startTrades),
        _endTrades(endTrades),
        _acceptVolume0(acceptVolume0) {
    // TODO: throw exception if pointer is 0
    assert(_posContainer != 0);
  }

  virtual ~PositionsManagerImpl() {}

  virtual void forEachOpenPosition(OpenPositionHandler& openPositionHandler,
                                   Bars bars, size_t bar) {
    _posContainer->forEachOpenPosition(openPositionHandler, bars, bar);
  }

  /*  Slippage* getSlippage() const
  {
	return _slippage;
  }

  Commission* getCommission() const
  {
	return _commission;
  }
*/  // TODO: add parameters to all the filters

  double calculateSlippage(unsigned long shares, unsigned long volume,
                           double price) const {
    return _slippage == 0 ? 0 : _slippage->getValue(shares, volume, price);
  }

  double calculateCommission(unsigned long shares, double price) const {
    return _commission == 0 ? 0 : _commission->getValue(shares, price);
  }

 public:
  /*  virtual const Position* getOpenPositionAt( size_t index ) const throw (
    OpenPositionIndexOutOfRangeException )
    {
          //TODO: throw exception here if _posList is 0?
          assert( _posList != 0 );
          return _posList->openPositionAt( index );
    }

    virtual Position* getOpenPositionAt( size_t index ) throw (
    OpenPositionIndexOutOfRangeException )
    {
          //TODO: throw exception here if _posList is 0?
          assert( _posList != 0 );
          return _posList->openPositionAt( index );
    }
  */

  virtual OrderFilter* registerOrderFilter(OrderFilter* orderFilter) {
    OrderFilter* x = _orderFilter;
    _orderFilter = orderFilter;
    return x;
  }

  // can be 0
  virtual void registerSignalHandler(SignalHandler* al) {
    _signalHandlers.add(al);
  }

  virtual void registerSignalHandlers(std::vector<SignalHandler*> ah) {
    for (unsigned int n = 0; n < ah.size(); n++)
      if (ah[n] != 0) _signalHandlers.push_back(ah[n]);
  }

 public:
  void installTimeBasedExitAtMarket(Index bars) {
    _timeBasedExitAtMarket.set(bars);
  }

  void installTimeBasedExitAtClose(Index bars) {
    _timeBasedExitAtClose.set(bars);
  }

  void installTimeBasedExit(Index bars) { _timeBasedExitAtMarket.set(bars); }

  void installReverseBreakEvenStop(double level) {
    _reverseBreakEvenStop.set(level);
  }

  void installReverseBreakEvenStopLong(double level) {
    _reverseBreakEvenStopLong.set(level);
  }

  void installReverseBreakEvenStopShort(double level) {
    _reverseBreakEvenStopShort.set(level);
  }

  void installStopLoss(double level) { _stopLoss.set(level); }

  void installStopLossLong(double level) { _stopLossLong.set(level); }

  void installStopLossShort(double level) { _stopLossShort.set(level); }

  void installProfitTarget(double level) { _profitTarget.set(level); }

  void installProfitTargetLong(double level) { _profitTargetLong.set(level); }

  void installProfitTargetShort(double level) { _profitTargetShort.set(level); }

  void installTrailingStop(double trigger, double level) {
    _TTrailingStop.set(trigger, level);
  }

  void installBreakEvenStop(double level) { _breakEvenStop.set(level); }

  void installBreakEvenStopLong(double level) { _breakEvenStopLong.set(level); }

  void installBreakEvenStopShort(double level) {
    _breakEvenStopShort.set(level);
  }

 public:
  virtual std::ostream& dump(std::ostream& os) const {
    assert(_posContainer != 0);
    // TODO: fix this to make it available in the exported classes
    return _posContainer->dump(os);
  }

  virtual std::ostream& dumpMin(std::ostream& os) const {
    assert(_posContainer != 0);
    // TODO: fix this to make it available in the exported classes
    return _posContainer->dumpMin(os);
  }

 public:
 private:
  tradery::PositionAbstr* openShort(
      tradery::OrderType orderType, const std::string& symbol, size_t shares,
      double price, double slippage, double commission, DateTime time,
      size_t bar, const std::string& name,
      const ManagedPtr<const std::string> userString, bool applyPositionsSizing,
      PositionId id = 0) {
    assert(_posContainer != 0);
    // TODO: calculate slippage using volume

    tradery::PositionAbstr* pos = new ShortPosition(
        orderType, symbol, shares, price, slippage, commission, time, bar, name,
        userString, applyPositionsSizing, id);
    _posContainer->add(pos);
    return pos;
  }

  void closeShort(
      tradery::OrderType orderType, tradery::Position p, double price,
      double slippage, double commission, DateTime time, size_t bar,
      const std::string& name) throw(ClosingAlreadyClosedPositionException) {
    assert(_posContainer != 0);

    try {
      assert(p);
      PositionImpl& pos = dynamic_cast<PositionImpl&>(*p.getPos());
      if (pos.isClosed()) {
        throw ClosingAlreadyClosedPositionException();
      } else {
        // TODO: make sure we are closing a position of the same type that was
        // opened throw exception if not close the position
        // TODO: calculate slippage using volume
        pos.closeShort(orderType, price, slippage, commission, time, bar, name);

        // this removes the position from open positions
        _posContainer->close(p.getPos());
      }
    } catch (const std::bad_cast&) {
      // TODO: throw an exception for the user to tell him he cannot derive from
      // Position
      assert(false);
    }
  }

  PositionImpl* openLong(tradery::OrderType orderType,
                         const std::string& symbol, unsigned long shares,
                         double price, double slippage, double commission,
                         DateTime time, size_t bar, const std::string& name,
                         const ManagedPtr<const std::string> userString,
                         bool applyPositionsSizing, PositionId id = 0) {
    // TODO: calculate slippage using volume
    assert(_posContainer != 0);
    PositionImpl* pos =
        new LongPosition(orderType, symbol, shares, price, slippage, commission,
                         time, bar, name, userString, applyPositionsSizing, id);
    _posContainer->add(pos);
    return pos;
  }

  void closeLong(
      tradery::OrderType orderType, tradery::Position p, double price,
      double slippage, double commission, DateTime time, size_t bar,
      const std::string& name) throw(ClosingAlreadyClosedPositionException) {
    assert(_posContainer != 0);

    try {
      assert(p);
      PositionImpl& pos = dynamic_cast<PositionImpl&>(*p.getPos());
      if (pos.isClosed()) {
        throw ClosingAlreadyClosedPositionException();
      } else {
        // TODO: make sure we are closing a position of the same type that was
        // opened throw exception if not close the position
        // TODO: calculate slippage using volume
        pos.closeLong(orderType, price, slippage, commission, time, bar, name);

        // remove from open positions
        _posContainer->close(p.getPos());
      }
    } catch (const std::bad_cast&) {
      // TODO: throw an exception for the user to tell him he cannot derive from
      // PositionsContainer
      assert(false);
    }
  }

  void setPositionUserData(tradery::PositionAbstr* pos,
                           const PositionUserData* data) {
    pos->setPositionUserData(data);
  }

  const PositionUserData* getPositionUserData(
      tradery::PositionAbstr* pos) const {
    return pos->getPositionUserData();
  }

 public:
  void applyTimeBased(Bars bars, Index barIndex, tradery::Position pos) throw(
      BarIndexOutOfRangeException) {
    applyTimeBasedAtMarket(bars, barIndex, pos);
  }
  void applyTimeBasedAtMarket(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyTimeBasedAtClose(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyStopLoss(Bars bars, Index barIndex,
                     tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyStopLossLong(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyStopLossShort(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyTrailing(Bars bars, Index barIndex,
                     tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyBreakEvenStop(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyBreakEvenStopLong(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyBreakEvenStopShort(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyReverseBreakEvenStop(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyReverseBreakEvenStopLong(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyReverseBreakEvenStopShort(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyProfitTarget(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException);
  void applyProfitTargetLong(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException,
                                   SellingShortPositionException);
  void applyProfitTargetShort(
      Bars bars, Index barIndex,
      tradery::Position pos) throw(BarIndexOutOfRangeException,
                                   CoveringLongPositionException);
  void applyAutoStops(Bars bars,
                      Index barIndex) throw(BarIndexOutOfRangeException);

 private:
  void applyAutoStops(Bars bs, Index barIndex,
                      tradery::Position pos) throw(BarIndexOutOfRangeException);

 public:
  PositionId buyAtMarket(
      Bars bars, Index barIndex, size_t shares, const std::string& name,
      bool applyPositionSizing = true) throw(BarIndexOutOfRangeException);
  PositionId buyAtClose(
      Bars bars, Index barIndex, size_t shares, const std::string& name,
      bool applyPositionSizing = true) throw(BarIndexOutOfRangeException);
  PositionId buyAtStop(
      Bars bars, Index barIndex, double stopPrice, size_t shares,
      const std::string& name,
      bool applyPositionSizing = true) throw(BarIndexOutOfRangeException);
  virtual PositionId buyAtPrice(
      Bars bars, size_t barIndex, double price, size_t shares,
      const std::string& name, bool applyPositionSizing,
      PositionId id) throw(BarIndexOutOfRangeException);
  virtual PositionId shortAtPrice(
      Bars bars, size_t barIndex, double price, size_t shares,
      const std::string& name, bool applyPositionSizing,
      PositionId id) throw(BarIndexOutOfRangeException);
  PositionId buyAtLimit(
      Bars bars, Index barIndex, double limitPrice, size_t shares,
      const std::string& name,
      bool applyPositionSizing = true) throw(BarIndexOutOfRangeException);
  bool sellAtMarket(
      Bars bars, Index barIndex, tradery::Position pos,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     SellingShortPositionException,
                                     ClosingAlreadyClosedPositionException);
  bool sellAtClose(
      Bars bars, Index barIndex, tradery::Position pos,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     SellingShortPositionException,
                                     ClosingAlreadyClosedPositionException);
  bool sellAtStop(
      Bars bars, Index barIndex, tradery::Position pos, double stopPrice,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     SellingShortPositionException,
                                     ClosingAlreadyClosedPositionException);
  bool sellAtLimit(
      Bars bars, Index barIndex, tradery::Position pos, double limitPrice,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     SellingShortPositionException,
                                     ClosingAlreadyClosedPositionException);
  PositionId shortAtMarket(
      Bars bars, Index barIndex, size_t shares, const std::string& name,
      bool applyPositionSizing = true) throw(BarIndexOutOfRangeException);
  PositionId shortAtClose(
      Bars bars, Index barIndex, size_t shares, const std::string& name,
      bool applyPositionSizing = true) throw(BarIndexOutOfRangeException);
  PositionId shortAtStop(
      Bars bars, Index barIndex, double stopPrice, size_t shares,
      const std::string& name,
      bool applyPositionSizing = true) throw(BarIndexOutOfRangeException);
  PositionId shortAtLimit(
      Bars bars, Index barIndex, double limitPrice, size_t shares,
      const std::string& name,
      bool applyPositionSizing = true) throw(BarIndexOutOfRangeException);
  bool coverAtMarket(
      Bars bars, Index barIndex, tradery::Position pos,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     CoveringLongPositionException,
                                     ClosingAlreadyClosedPositionException);
  bool coverAtClose(
      Bars bars, Index barIndex, tradery::Position pos,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     CoveringLongPositionException,
                                     ClosingAlreadyClosedPositionException);
  bool coverAtStop(
      Bars bars, Index barIndex, tradery::Position pos, double stopPrice,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     CoveringLongPositionException,
                                     ClosingAlreadyClosedPositionException);
  bool coverAtLimit(
      Bars bars, Index barIndex, tradery::Position pos, double limitPrice,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     CoveringLongPositionException,
                                     ClosingAlreadyClosedPositionException);

  virtual bool sellAtMarket(
      Bars bars, size_t barIndex, PositionId pos,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     SellingShortPositionException,
                                     ClosingAlreadyClosedPositionException,
                                     PositionIdNotFoundException);
  virtual bool sellAtClose(
      Bars bars, size_t barIndex, PositionId pos,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     SellingShortPositionException,
                                     ClosingAlreadyClosedPositionException,
                                     PositionIdNotFoundException);
  virtual bool sellAtStop(
      Bars bars, size_t barIndex, PositionId pos, double stopPrice,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     SellingShortPositionException,
                                     ClosingAlreadyClosedPositionException,
                                     PositionIdNotFoundException);
  virtual bool sellAtLimit(
      Bars bars, size_t barIndex, PositionId pos, double limitPrice,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     SellingShortPositionException,
                                     ClosingAlreadyClosedPositionException,
                                     PositionIdNotFoundException);
  virtual bool coverAtMarket(
      Bars bars, size_t barIndex, PositionId pos,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     CoveringLongPositionException,
                                     ClosingAlreadyClosedPositionException,
                                     PositionIdNotFoundException);
  virtual bool coverAtClose(
      Bars bars, size_t barIndex, PositionId pos,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     CoveringLongPositionException,
                                     ClosingAlreadyClosedPositionException,
                                     PositionIdNotFoundException);
  virtual bool coverAtStop(
      Bars bars, size_t barIndex, PositionId pos, double stopPrice,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     CoveringLongPositionException,
                                     ClosingAlreadyClosedPositionException,
                                     PositionIdNotFoundException);
  virtual bool coverAtLimit(
      Bars bars, size_t barIndex, PositionId pos, double limitPrice,
      const std::string& name) throw(BarIndexOutOfRangeException,
                                     CoveringLongPositionException,
                                     ClosingAlreadyClosedPositionException,
                                     PositionIdNotFoundException);

  virtual tradery::Position getLastPosition() {
    assert(_posContainer != 0);
    return _posContainer->getLastPosition();
  }

  /**
   * Returns 0 if there is no open position
   *
   * @return
   */
  const tradery::Position getLastOpenPosition() const {
    assert(_posContainer != 0);
    return _posContainer->getLastOpenPosition();
  }

  tradery::Position getLastOpenPosition() {
    assert(_posContainer != 0);
    return _posContainer->getLastOpenPosition();
  }

  size_t openPositionsCount() const {
    assert(_posContainer != 0);
    return _posContainer->openPositionsCount();
  }

  /**
   * returns NO_POSITION if there are no positions, open or closed.
   *
   * @return
   */
  tradery::Position getLastPosition() const {
    assert(_posContainer != 0);
    return _posContainer->getLastPosition();
  }

  bool isLastPositionOpen() const { return getLastPosition().isOpen(); }

  /*
    // does what WL does - see WL doc
    short marketPosition() const
    {
          tradery::Position pos = getLastOpenPosition();
          return pos == 0 ? 0 : ( pos -> isLong() ? 1 : -1 );
    }
  */
  double positionBasisPrice() const {
    // TODO - positionBasisPrice
  }

  size_t positionCount() const {
    assert(_posContainer != 0);
    return _posContainer->count();
  }

  virtual void reset() {
    assert(_posContainer != 0);
    _posContainer->clear();
    if (_commission != 0) _commission->reset();
    if (_slippage != 0) _slippage->reset();
  }

  virtual void init(PositionsManagerAbstr& positions) {
    // TODO: not entirely ok - what happens with the existing slippage, etc
    // and what happens when multiple positions point to the same slippage etc
    PositionsManagerImpl& p = dynamic_cast<PositionsManagerImpl&>(positions);
    _slippage = p._slippage;
    _commission = p._commission;
    this->registerOrderFilter(p._orderFilter);
  }

  class CloseAtMarketHandler : public OpenPositionHandler {
    const std::string _name;
    PositionsManagerAbstr& _pm;
    bool _first;

   public:
    CloseAtMarketHandler(const std::string& name, PositionsManagerAbstr& pm,
                         bool first = false)
        : _name(name), _pm(pm), _first(first) {}

    virtual bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) {
      if (pos.isLong())
        _pm.sellAtMarket(bars, bar, pos, _name);
      else
        _pm.coverAtMarket(bars, bar, pos, _name);

      // if first, return false so we won't do any more iterations
      return !_first;
    }
  };

  class CloseAtCloseHandler : public OpenPositionHandler {
    const std::string _name;
    PositionsManagerAbstr& _pm;

   public:
    CloseAtCloseHandler(const std::string& name, PositionsManagerAbstr& pm)
        : _name(name), _pm(pm) {}

    virtual bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) {
      if (pos.isLong())
        _pm.sellAtClose(bars, bar, pos, _name);
      else
        _pm.coverAtClose(bars, bar, pos, _name);

      return true;
    }
  };

  class CloseAtLimitHandler : public OpenPositionHandler {
    const std::string _name;
    PositionsManagerAbstr& _pm;
    const double _price;

   public:
    CloseAtLimitHandler(const std::string& name, PositionsManagerAbstr& pm,
                        double price)
        : _name(name), _pm(pm), _price(price) {}

    virtual bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) {
      if (pos.isLong())
        _pm.sellAtLimit(bars, bar, pos, _price, _name);
      else
        _pm.coverAtLimit(bars, bar, pos, _price, _name);

      return true;
    }
  };

  class CloseAtStopHandler : public OpenPositionHandler {
    const std::string _name;
    PositionsManagerAbstr& _pm;
    const double _price;

   public:
    CloseAtStopHandler(const std::string& name, PositionsManagerAbstr& pm,
                       double price)
        : _name(name), _pm(pm), _price(price) {}

    virtual bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) {
      if (pos.isLong())
        _pm.sellAtStop(bars, bar, pos, _price, _name);
      else
        _pm.coverAtStop(bars, bar, pos, _price, _name);

      return true;
    }
  };

  class PositionShortEqualSharesPredicate : public PositionEqualPredicate {
   private:
    unsigned long _shares;

   public:
    PositionShortEqualSharesPredicate(unsigned long shares) : _shares(shares) {}

    /**
     * Returns true if the position is short
     *
     * @param position The position to be tested
     *
     * @return true if the position is short, false otherwise
     */
    virtual bool operator==(const tradery::Position position) const {
      return position.getShares() == _shares && position.isShort();
    }
  };

  class PositionLongEqualSharesPredicate : public PositionEqualPredicate {
   private:
    unsigned long _shares;

   public:
    PositionLongEqualSharesPredicate(unsigned long shares) : _shares(shares) {}

    /**
     * Returns true if the position is short
     *
     * @param position The position to be tested
     *
     * @return true if the position is short, false otherwise
     */
    virtual bool operator==(const tradery::Position position) const {
      return position.getShares() == _shares && position.isLong();
    }
  };

  virtual void closeAllAtMarket(
      Bars bars, size_t barIndex,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    forEachOpenPosition(CloseAtMarketHandler(name, *this), bars, barIndex);
  }
  virtual void closeAllShortAtMarket(
      Bars bars, size_t barIndex,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    _posContainer->forEachOpenPosition(CloseAtMarketHandler(name, *this), bars,
                                       barIndex, PositionEqualShortPredicate());
  }
  virtual void closeAllLongAtMarket(
      Bars bars, size_t barIndex,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    _posContainer->forEachOpenPosition(CloseAtMarketHandler(name, *this), bars,
                                       barIndex, PositionEqualLongPredicate());
  }

  virtual void closeAllAtClose(
      Bars bars, size_t barIndex,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    forEachOpenPosition(CloseAtCloseHandler(name, *this), bars, barIndex);
  }
  virtual void closeAllShortAtClose(
      Bars bars, size_t barIndex,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    _posContainer->forEachOpenPosition(CloseAtCloseHandler(name, *this), bars,
                                       barIndex, PositionEqualShortPredicate());
  }
  virtual void closeAllLongAtClose(
      Bars bars, size_t barIndex,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    _posContainer->forEachOpenPosition(CloseAtCloseHandler(name, *this), bars,
                                       barIndex, PositionEqualLongPredicate());
  }

  virtual void closeAllShortAtLimit(
      Bars bars, size_t barIndex, double price,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    _posContainer->forEachOpenPosition(CloseAtLimitHandler(name, *this, price),
                                       bars, barIndex,
                                       PositionEqualShortPredicate());
  }
  virtual void closeAllLongAtLimit(
      Bars bars, size_t barIndex, double price,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    _posContainer->forEachOpenPosition(CloseAtLimitHandler(name, *this, price),
                                       bars, barIndex,
                                       PositionEqualLongPredicate());
  }

  virtual void closeAllShortAtStop(
      Bars bars, size_t barIndex, double price,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    _posContainer->forEachOpenPosition(CloseAtStopHandler(name, *this, price),
                                       bars, barIndex,
                                       PositionEqualShortPredicate());
  }
  virtual void closeAllLongAtStop(
      Bars bars, size_t barIndex, double price,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    _posContainer->forEachOpenPosition(CloseAtStopHandler(name, *this, price),
                                       bars, barIndex,
                                       PositionEqualLongPredicate());
  }

  class CloseFirstAtMarketHandler : public OpenPositionHandler {
    const std::string _name;
    PositionsManagerAbstr& _pm;

   public:
    CloseFirstAtMarketHandler(const std::string& name,
                              PositionsManagerAbstr& pm)
        : _name(name), _pm(pm) {}

    virtual bool onOpenPosition(tradery::Position pos, Bars bars, size_t bar) {
      if (pos.isLong())
        _pm.sellAtMarket(bars, bar, pos, _name);
      else
        _pm.coverAtMarket(bars, bar, pos, _name);

      return true;
    }
  };

  virtual void closeFirstLongAtMarketByShares(
      size_t shares, Bars bars, size_t barIndex,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    _posContainer->forEachOpenPosition(
        CloseAtMarketHandler(name, *this, true), bars, barIndex,
        PositionLongEqualSharesPredicate(shares));
  }

  virtual void closeFirstShortAtMarketByShares(
      size_t shares, Bars bars, size_t barIndex,
      const std::string& name) throw(BarIndexOutOfRangeException) {
    _posContainer->forEachOpenPosition(
        CloseAtMarketHandler(name, *this, true), bars, barIndex,
        PositionShortEqualSharesPredicate(shares));
  }

  virtual OpenPositionsIterator getOpenPositionsIterator() {
    return _posContainer->getOpenPositionsIterator();
  }
};
