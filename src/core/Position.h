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

#pragma warning(disable : 4800)
#pragma warning(default : 4800)

using namespace tradery;

// TODO: should I use exception here?
const size_t NO_POSITION = -1;

using std::pair;

/**
 * Template class for Settable - values that have a "not set" state
 * in addition to their values
 */

class Settable2Double : public Settable<pair<double, double> > {
 public:
  void set(double value1, double value2) {
    Settable<pair<double, double> >::set(pair<double, double>(value1, value2));
  }
  double getValue1() const { return getValue().first; }
  double getValue2() const { return getValue().second; }
};

/**
 * One position leg, open or close.
 */
class PositionLeg {
 private:
  std::string _name;
  double _price;
  // TODO: should slippage be in the position?
  double _slippage;
  double _commission;
  // TODO: bar depends on the price series, so if positions are saved, bar value
  // may become invalid
  size_t _barIndex;
  DateTime _time;
  OrderType _orderType;

 public:
  PositionLeg(OrderType orderType, double price, double slippage,
              double commission, DateTime time, size_t barIndex,
              const std::string& name)
      : _orderType(orderType),
        _price(price),
        _slippage(slippage),
        _commission(commission),
        _time(time),
        _barIndex(barIndex),
        _name(name) {
    if (price == 0) {
      //			throw PositionZeroPriceException();
    }
  }
  virtual ~PositionLeg() {}

 public:
  OrderType getType() { return _orderType; }

  size_t getBarIndex() const { return _barIndex; }
  double getPrice() const { return _price; }
  const std::string& getName() const { return _name; }

  const DateTime& getTime() const { return _time; }

  double getCommission() const { return _commission; }

  double getSlippage() const { return _slippage; }

  std::ostream& dump(std::ostream& os) const {
    std::ostringstream o;
    o << "\t" << _time.to_simple_string() << " " << _barIndex << "\t$"
      << std::fixed << std::setprecision(2) << _price << "\ts: " << _slippage
      << "\tc: " << _commission << "\tname: \"" << _name << "\"";
    return sprint(o, os);
  }

  std::ostream& dumpMin(std::ostream& os) const {
    std::ostringstream o;
    o << "\t" << _time.to_simple_string() << "\t$" << std::fixed
      << std::setprecision(2) << _price << "\ts: " << _slippage
      << "\tc: " << _commission;
    ;
    return sprint(o, os);
  }
};

typedef std::auto_ptr<PositionLeg> PositionLegPtr;

/**
 * PositionExtraInfo - has extra info per position, used for
 * handling that requires holding a staus info in time, for
 * example when doing trailingStop or breakEvenStop processing
 */
class PositionExtraInfo  // : ObjCount
{
 private:
  bool _breakEvenActive;
  bool _breakEvenLongActive;
  bool _breakEvenShortActive;
  bool _reverseBreakEvenActive;
  bool _reverseBreakEvenLongActive;
  bool _reverseBreakEvenShortActive;

  DoubleSettable _TTrailingStop;

 public:
  PositionExtraInfo()
      : _breakEvenActive(false),
        _reverseBreakEvenActive(false),
        _breakEvenLongActive(false),
        _breakEvenShortActive(false),
        _reverseBreakEvenLongActive(false),
        _reverseBreakEvenShortActive(false) {}

  bool isBreakEvenStopActive() const { return _breakEvenActive; }
  bool isBreakEvenStopLongActive() const { return _breakEvenLongActive; }
  bool isBreakEvenStopShortActive() const { return _breakEvenShortActive; }
  bool isReverseBreakEvenStopActive() const { return _reverseBreakEvenActive; }

  bool isReverseBreakEvenStopLongActive() const {
    return _reverseBreakEvenLongActive;
  }
  bool isReverseBreakEvenStopShortActive() const {
    return _reverseBreakEvenShortActive;
  }

  void activateBreakEvenStop() { _breakEvenActive = true; }
  void activateReverseBreakEvenStop() { _reverseBreakEvenActive = true; }
  void activateTrailingStop(double level) { _TTrailingStop.set(level); }
  bool isTrailingStopActive() const { return _TTrailingStop.isSet(); }
  double getTrailingStopLevel() const {
    // TODO: handle this case?
    // can't request the trailing value if it was not set
    if (!_TTrailingStop.isSet()) assert(false);
    return _TTrailingStop.getValue();
  }
};

/**
 * An abstract Position. Concrete positions are short and long
 */

class PositionImpl : public PositionAbstr {
 private:
  // this is the id for all positions, will get incremented after a position is
  // created
  static PositionId _uniqueId;
  const PositionUserData* _data;
  std::string _symbol;
  // number of shares before position sizing
  size_t _initialShares;
  // final number of shares, after position sizing
  size_t _shares;

  const ManagedPtr<const std::string> _userString;

  PositionLegPtr _openLeg;
  PositionLegPtr _closeLeg;

  PositionExtraInfo _extraInfo;
  // each position has an unique id
  const PositionId _id;

  static Mutex _mx;

  const bool _applyPositionSizing;

 private:
  static PositionId getNewId() {
    Lock lock(_mx);

    return _uniqueId++;
  }

 public:
  virtual bool applyPositionSizing() const { return _applyPositionSizing; }
  virtual const ManagedPtr<const std::string> getUserString() const {
    return _userString;
  }
  virtual PositionId getId() const {
    assert(_id > 0);
    return _id;
  }

  // this sets the actual number of shares to 0
  void disable() { _shares = 0; }
  // sets the actual number of shares to a value different than that set
  // initially used during position sizing
  void setShares(size_t shares) { _shares = shares; }
  // if the actual number of shares is 0, the position is disabled
  bool isDisabled() const { return _shares == 0; }
  bool isEnabled() const { return _shares != 0; }
  bool isTrailingStopActive() const {
    return _extraInfo.isTrailingStopActive();
  }
  bool isBreakEvenStopActive() const {
    return _extraInfo.isBreakEvenStopActive();
  }
  bool isBreakEvenStopLongActive() const {
    return _extraInfo.isBreakEvenStopLongActive();
  }
  bool isBreakEvenStopShortActive() const {
    return _extraInfo.isBreakEvenStopShortActive();
  }
  bool isReverseBreakEvenStopActive() const {
    return _extraInfo.isReverseBreakEvenStopActive();
  }
  bool isReverseBreakEvenStopLongActive() const {
    return _extraInfo.isReverseBreakEvenStopLongActive();
  }
  bool isReverseBreakEvenStopShortActive() const {
    return _extraInfo.isReverseBreakEvenStopShortActive();
  }
  double getTrailingStopLevel() const {
    return _extraInfo.getTrailingStopLevel();
  }
  void activateTrailingStop(double level) {
    _extraInfo.activateTrailingStop(level);
  }
  void activateBreakEvenStop() { _extraInfo.activateBreakEvenStop(); }
  void activateReverseBreakEvenStop() {
    _extraInfo.activateReverseBreakEvenStop();
  }
  size_t getShares() const { return _shares; }
  void setPositionUserData(const PositionUserData* data) { _data = data; }
  const PositionUserData* getPositionUserData() const { return _data; }
  virtual bool isLong() const = 0;
  virtual bool isShort() const = 0;
  virtual void closeShort(OrderType orderType, double price, double slippage,
                          double commission, DateTime time, size_t bar,
                          const std::string& name) = 0;
  virtual void closeLong(OrderType orderType, double price, double slippage,
                         double commission, DateTime time, size_t bar,
                         const std::string& name) = 0;

  const std::string& getSymbol() const { return _symbol; }
  const bool isOpen() const { return _closeLeg.get() == 0; }

  const bool isClosed() const { return !isOpen(); }

  virtual OrderType getEntryOrderType() const {
    assert(_openLeg.get() != 0);
    return _openLeg->getType();
  }
  virtual OrderType getExitOrderType() const {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getExitType");
    assert(_closeLeg.get() != 0);
    return _closeLeg->getType();
  }

  const DateTime getEntryTime() const { return _openLeg->getTime(); }

  const DateTime getCloseTime() const
      throw(PositionCloseOperationOnOpenPositionException) {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getCloseTime");
    return _closeLeg->getTime();
  }

  size_t getEntryBar() const {
    assert(_openLeg.get() != 0);
    return _openLeg->getBarIndex();
  }

  size_t getCloseBar() const
      throw(PositionCloseOperationOnOpenPositionException) {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getCloseBar");
    return _closeLeg->getBarIndex();
  }

  double getClosePrice() const
      throw(PositionCloseOperationOnOpenPositionException) {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getClosePrice");
    return _closeLeg->getPrice();
  }

  double getEntryPrice() const {
    assert(_openLeg.get() != 0);
    return _openLeg->getPrice();
  }
  virtual double getEntrySlippage() const {
    assert(_openLeg.get() != 0);
    return _openLeg->getSlippage();
  }

  virtual double getEntryCommission() const {
    assert(_openLeg.get() != 0);
    return _openLeg->getCommission();
  }

  virtual double getCloseSlippage() const
      throw(PositionCloseOperationOnOpenPositionException) {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getCloseSlippage");
    return _closeLeg->getSlippage();
  }
  virtual double getCloseCommission() const
      throw(PositionCloseOperationOnOpenPositionException) {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getCloseCommission");
    return _closeLeg->getCommission();
  }

  virtual const std::string& getEntryName() const {
    assert(_openLeg.get() != 0);
    return _openLeg->getName();
  }

  virtual const std::string& getCloseName() const
      throw(PositionCloseOperationOnOpenPositionException) {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getCloseName");
    return _closeLeg->getName();
  }

  virtual double getPctGain() const
      throw(PositionCloseOperationOnOpenPositionException) {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getPctGain");

    double entryCost = getEntryCost();
    entryCost = entryCost == 0 ? 0.01 : entryCost;

    return getGain() / entryCost * 100;
  }

  virtual double getPctGain(double value) const {
    return getGain(value) / getEntryCost() * 100;
  }

  std::ostream& dump(std::ostream& os) const {
    {
      std::ostringstream o;
      o << _symbol << ", " << _shares << " sh, ";
      sprint(o, os);
    }
    _openLeg->dump(os);

    if (_closeLeg.get() == 0) {
      std::ostringstream o;
      o << "\t-\t- - - - - - - - - - - - - - - - - -";
      sprint(o, os);
    } else {
      std::ostringstream o;
      o << "\t- ";
      sprint(o, os);
      _closeLeg->dump(os);
    }

    return os;
  }

  std::ostream& dumpMin(std::ostream& os) const {
    {
      std::ostringstream o;
      o << _symbol << ", " << _shares << " sh, ";
      sprint(o, os);
      _openLeg->dumpMin(os);
    }

    if (_closeLeg.get() == 0) {
      std::ostringstream o;
      o << "\t-\t- - - - - - - - - - - - - - - - - -";
      sprint(o, os);
    } else {
      {
        std::ostringstream o;
        o << "\t- ";
        sprint(o, os);
        _closeLeg->dumpMin(os);
      }
      {
        std::ostringstream o;
        o << " gain: " << getGain();
        sprint(o, os);
      }
    }

    return os;
  }

 protected:
  PositionImpl(OrderType orderType, const std::string& symbol, size_t shares,
               double price, double slippage, double commission, DateTime time,
               size_t bar, const std::string& name,
               const ManagedPtr<const std::string> userString,
               bool applyPositionSizing, PositionId id)
      : _symbol(symbol),
        _shares(shares),
        _initialShares(shares),
        _openLeg(new PositionLeg(orderType, price, slippage, commission, time,
                                 bar, name)),
        _id(id > 0 ? id : getNewId()),
        _userString(userString),
        _applyPositionSizing(applyPositionSizing) {
    assert(_id != 0);
  }

  void close(
      OrderType orderType, double price, double slippage, double commission,
      DateTime time, size_t bar,
      const std::string& name) throw(ClosingAlreadyClosedPositionException) {
    // TODO:  assert time > open time
    // TODO: warning if exit bar/date is lower or equal to entry bar/date (
    // exception?) - maybe an option that will allow exception to
    // be thrown or not
    // make sure we are closing an open position
    if (!isOpen()) throw ClosingAlreadyClosedPositionException();
    _closeLeg = PositionLegPtr(new PositionLeg(orderType, price, slippage,
                                               commission, time, bar, name));
  }

 public:
  virtual ~PositionImpl() {}
};

/**
 * A concrete short position
 */
class ShortPosition : public PositionImpl {
 public:
  ShortPosition(OrderType orderType, const std::string& symbol, size_t shares,
                double price, double slippage, double commission, DateTime time,
                size_t bar, const std::string& name,
                const ManagedPtr<const std::string> userString,
                bool applyPositionSizing, PositionId id = 0)
      : PositionImpl(orderType, symbol, shares, price, slippage, commission,
                     time, bar, name, userString, applyPositionSizing, id) {}

 public:
  virtual bool isLong() const { return false; }
  virtual bool isShort() const { return true; }
  virtual void closeShort(
      OrderType orderType, double price, double slippage, double commission,
      DateTime time, size_t bar,
      const std::string& name) throw(ClosingAlreadyClosedPositionException) {
    PositionImpl::close(orderType, price, slippage, commission, time, bar,
                        name);
  }

  virtual void closeLong(
      OrderType, double, double, double, DateTime, size_t,
      const std::string&) throw(SellingShortPositionException) {
    // covering instead of selling
    throw SellingShortPositionException();
  }

  virtual double getGain() const
      throw(PositionCloseOperationOnOpenPositionException) {
    return getEntryCost() - getCloseIncome();
  }

  virtual double getGain(double price) const
      throw(PositionCloseOperationOnOpenPositionException) {
    return getEntryCost() - price * getShares();
  }

  // for a short position, the entry cost is in fact an income, as the short
  // position is sold, so commission should be subtracted
  virtual double getEntryCost() const {
    return getEntryPrice() * getShares() - getEntryCommission();
  }

  virtual double getEntryCost(size_t shares) const {
    return getEntryPrice() * shares - getEntryCommission();
  }

  virtual double getCloseIncome() const
      throw(PositionCloseOperationOnOpenPositionException) {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getCloseCost");

    return getClosePrice() * getShares() + getCloseCommission();
  }
};

/**
 * A concrete long position
 */
class LongPosition : public PositionImpl {
 public:
  LongPosition(OrderType orderType, const std::string& symbol,
               unsigned long shares, double price, double slippage,
               double commission, DateTime time, size_t bar,
               const std::string& name,
               const ManagedPtr<const std::string> userString,
               bool applyPositionsSizing, PositionId id = 0)
      : PositionImpl(orderType, symbol, shares, price, slippage, commission,
                     time, bar, name, userString, applyPositionsSizing, id) {}

 public:
  virtual bool isLong() const { return true; }
  virtual bool isShort() const { return false; }

  virtual void closeLong(
      OrderType orderType, double price, double slippage, double commission,
      DateTime time, size_t bar,
      const std::string& name) throw(ClosingAlreadyClosedPositionException) {
    PositionImpl::close(orderType, price, slippage, commission, time, bar,
                        name);
  }

  virtual void closeShort(
      OrderType, double, double, double, DateTime, size_t,
      const std::string&) throw(CoveringLongPositionException) {
    // selling instead of covering
    throw CoveringLongPositionException();
  }

  virtual double getGain() const
      throw(PositionCloseOperationOnOpenPositionException) {
    return getCloseIncome() - getEntryCost();
  }

  virtual double getGain(double price) const {
    return getShares() * price - getEntryCost();
  }

  virtual double getEntryCost(size_t shares) const {
    return getEntryPrice() * shares + getEntryCommission();
  }

  virtual double getEntryCost() const {
    return getEntryPrice() * getShares() + getEntryCommission();
  }

  virtual double getCloseIncome() const
      throw(PositionCloseOperationOnOpenPositionException) {
    if (!isClosed())
      throw PositionCloseOperationOnOpenPositionException("getCloseCost");

    return getClosePrice() * getShares() - getCloseCommission();
  }
};

// defines less entry time predicate
class LessEntryTimePredicate
    : public std::binary_function<ManagedPtr<PositionAbstr>,
                                  ManagedPtr<PositionAbstr>, bool> {
 public:
  virtual bool operator()(const ManagedPtr<PositionAbstr> pos1,
                          const ManagedPtr<PositionAbstr> pos2) const {
    assert(pos1.get() != 0 && pos2.get() != 0);
    if (pos1->getEntryTime() == pos2->getEntryTime()) {
      if (pos1->getEntryOrderType() != pos2->getEntryOrderType())
        return PositionAbstr::orderTypeLower(pos1->getEntryOrderType(),
                                             pos2->getEntryOrderType());
      /*		else if( pos1->getExitOrderType() !=
         pos2->getExitOrderType() ) return PositionAbstr::orderTypeLower(
         pos1->getExitOrderType(), pos2->getExitOrderType() );
      */
      else if (pos1->getSymbol() != pos2->getSymbol())
        return pos1->getSymbol() < pos2->getSymbol();
      else
        return pos1.get() < pos2.get();
    } else
      return pos1->getEntryTime() < pos2->getEntryTime();
  }
};

// defines less close time predicate
class LessCloseTimePredicate
    : public std::binary_function<ManagedPtr<PositionAbstr>,
                                  ManagedPtr<PositionAbstr>, bool> {
 public:
  virtual bool operator()(const ManagedPtr<PositionAbstr> pos1,
                          const ManagedPtr<PositionAbstr> pos2) const {
    assert(pos1.get() != 0 && pos2.get() != 0);
    if (pos1->isOpen())
      return true;
    else if (pos2->isOpen())
      return false;
    else if (pos1->getCloseTime() == pos2->getCloseTime())
      return PositionAbstr::orderTypeLower(pos1->getExitOrderType(),
                                           pos2->getExitOrderType());
    else
      return pos1->getCloseTime() < pos2->getCloseTime();
  }
};

// defines less gain predicate
class LessGainPredicate
    : public std::binary_function<ManagedPtr<PositionAbstr>,
                                  ManagedPtr<PositionAbstr>, bool> {
 public:
  virtual bool operator()(const ManagedPtr<PositionAbstr> pos1,
                          const ManagedPtr<PositionAbstr> pos2) const {
    assert(pos1.get() != 0 && pos2.get() != 0);
    if (pos1->isOpen())
      return true;
    else if (pos2->isOpen())
      return false;
    else
      return pos1->getGain() < pos2->getGain();
  }
};

// adapter for externally defined arbirary predicate
class LessPredicate
    : public std::binary_function<PositionAbstrPtr, PositionAbstrPtr, bool> {
 private:
  PositionLessPredicate& _predicate;

 public:
  LessPredicate(PositionLessPredicate& predicate) : _predicate(predicate) {}

 public:
  bool operator()(const PositionAbstrPtr pos1,
                  const PositionAbstrPtr pos2) const {
    return _predicate.less(pos1, pos2);
  }
};
