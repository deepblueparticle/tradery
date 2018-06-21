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

#include "propertieslist.h"

class DocumentException {
 private:
  const std::string _message;

 public:
  DocumentException(const std::string& message) : _message(message) {}

  const std::string& message() const { return _message; }
};

class DocumentNode : public Info {
 public:
  enum Type {
    DEFAULTS_ROOT,
    RUNNABLES_ROOT,
    DATASOURCE,
    SYMBOLSSOURCE,
    SIGNALHANDLER,
    RUNNABLE,
    SLIPPAGE,
    COMMISSION,
    MAX_TYPE
  };

 private:
  Type _type;

 public:
  DocumentNode(DocumentNode::Type type, const Info& info)
      : _type(type), Info(info) {}
  ~DocumentNode() {}
  DocumentNode::Type type() const { return _type; }
};

typedef Event<DocumentNode> DocumentEvent;

// this maps unique ids to unique ids
// the reason is to map "real" ids, that are used throughout the system to local
// ids, in order to allow duplicate elements in the document
class IdToIdMap : private std::map<const UniqueId, const UniqueId> {
 private:
  mutable const_iterator _findIter;

 public:
  IdToIdMap() : _findIter(begin()) {}
  const UniqueId* at(const UniqueId& id) const {
    const_iterator i(__super::find(id));
    return i == end() ? 0 : &i->second;
  }

  bool remove(const UniqueId& id) { return erase(id) > 0; }

  bool add(const UniqueId& key, const UniqueId& value) {
    if (__super::find(key) != end())
      return false;
    else {
      __super::insert(value_type(key, value));
      return true;
    }
  }

  // returns the first key corresponding to an id value
  const UniqueId* findFirst(const UniqueId& id) const {
    _findIter = begin();
    return findNext(id);
  }

  // returns the next key corresponding to an id value
  const UniqueId* findNext(const UniqueId& id) const {
    for (const_iterator i = _findIter; i != end(); i++) {
      if (i->second == id) {
        const UniqueId& id = i->first;
        _findIter = ++i;
        return &id;
      }
    }
    return 0;
  }

  // returns true if id is one of the values, (not keys, as find would do)
  bool hasId(const UniqueId& id) const {
    for (const_iterator i = begin(); i != end(); i++) {
      if (i->second == id) return true;
    }
    return false;
  }
};

template <Node::NodeType T>
class DocumentConstants {
 public:
  static const std::string _elementName;
  static const std::string _elementString;
};

template <>
const std::string
    DocumentConstants<Node::NodeType::SIGNALHANDLER>::_elementName =
        "DefaultSignalHandler";
template <>
const std::string DocumentConstants<Node::NodeType::COMMISSION>::_elementName =
    "DefaultCommission";
template <>
const std::string DocumentConstants<Node::NodeType::DATASOURCE>::_elementName =
    "DefaultDataSource";
template <>
const std::string DocumentConstants<Node::NodeType::RUNNABLE>::_elementName =
    "Runnable";
template <>
const std::string DocumentConstants<Node::NodeType::SLIPPAGE>::_elementName =
    "DefaultSlippage";
template <>
const std::string
    DocumentConstants<Node::NodeType::SYMBOLSSOURCE>::_elementName =
        "DefaultSymbolsSource";

template <>
const std::string
    DocumentConstants<Node::NodeType::SIGNALHANDLER>::_elementString =
        "signal handler";
template <>
const std::string
    DocumentConstants<Node::NodeType::COMMISSION>::_elementString =
        "commission";
template <>
const std::string
    DocumentConstants<Node::NodeType::DATASOURCE>::_elementString =
        "data source";
template <>
const std::string DocumentConstants<Node::NodeType::RUNNABLE>::_elementString =
    "runnable";
template <>
const std::string DocumentConstants<Node::NodeType::SLIPPAGE>::_elementString =
    "slippage";
template <>
const std::string
    DocumentConstants<Node::NodeType::SYMBOLSSOURCE>::_elementString =
        "symbols source";

class Cleanable {
 private:
  bool _dirty;

 public:
  Cleanable() : _dirty(false) {}
  virtual ~Cleanable() {}

  virtual void setClean() { _dirty = false; }
  void setDirty() { _dirty = true; }

  virtual bool isDirty() const { return _dirty; }
};

class UpdateReceiver {
 public:
  virtual ~UpdateReceiver() {}

  virtual void update() = 0;
};

class PositionSizingParamsImpl : public PositionSizingParams, public Cleanable {
 private:
  double _initialCapital;
  MaxOpenPositions _maxOpenPos;
  PosSizeType _posSizeType;
  double _posSize;
  PosSizeLimitType _posSizeLimitType;
  double _posSizeLimit;

 protected:
  PositionSizingParamsImpl(double initialCapital,
                           const MaxOpenPositions& maxOpenPos,
                           PosSizeType posSizeType, double posSize,
                           PosSizeLimitType posSizeLimitType,
                           double posSizeLimit)
      : _initialCapital(initialCapital),
        _maxOpenPos(maxOpenPos),
        _posSizeType(posSizeType),
        _posSize(posSize),
        _posSizeLimitType(posSizeLimitType),
        _posSizeLimit(posSizeLimit) {}

 public:
  virtual ~PositionSizingParamsImpl() {}
  PositionSizingParamsImpl()
      : _initialCapital(DEFAULT_INITIAL_CAPITAL),
        _maxOpenPos(DEFAULT_MAX_OPEN_POSITIONS),
        _posSizeType((PosSizeType)DEFAULT_POS_SIZE_TYPE),
        _posSize(DEFAULT_POSITION_SIZE),
        _posSizeLimitType(DEFAULT_POS_SIZE_LIMIT_TYPE),
        _posSizeLimit(DEFAULT_POSITION_SIZE_LIMIT) {}

  PositionSizingParamsImpl& operator=(
      const tradery_thrift_api::PositionSizing& psp) {
    setInitialCapital(psp.initialCapital);
    setMaxOpenPos(psp.maxOpenPositions);
    setPosSizeType((PosSizeType)psp.positionSizeType);
    setPosSize(psp.positionSize);
    setPosSizeLimitType((PosSizeLimitType)psp.positionSizeLimitType);
    setPosSizeLimit(psp.positionSizeLimit);

    return *this;
  }

  double initialCapital() const { return _initialCapital; }
  const MaxOpenPositions maxOpenPos() const { return _maxOpenPos; }
  PosSizeType posSizeType() const { return _posSizeType; }
  double posSize() const { return _posSize; }
  PosSizeLimitType posSizeLimitType() const { return _posSizeLimitType; }
  double posSizeLimit() const { return _posSizeLimit; }

  void setInitialCapital(double capital) {
    __super::setDirty();
    _initialCapital = capital;
  }
  void setMaxOpenPos(const MaxOpenPositions& maxOpenPos) {
    __super::setDirty();
    _maxOpenPos = maxOpenPos;
  }
  void setPosSizeType(PosSizeType posSizeType) {
    __super::setDirty();
    _posSizeType = posSizeType;
  }
  void setPosSize(double posSize) {
    __super::setDirty();
    _posSize = posSize;
  }
  void setPosSizeLimitType(PosSizeLimitType posSizeLimitType) {
    __super::setDirty();
    _posSizeLimitType = posSizeLimitType;
  }
  void setPosSizeLimit(double posSizeLimit) {
    __super::setDirty();
    _posSizeLimit = posSizeLimit;
  }

  void set(const tradery_thrift_api::PositionSizing& psp) {
    __super::setDirty();
    operator=(psp);
  }
};

class RuntimeParamsImpl : public Cleanable, public RuntimeParams {
 private:
  unsigned long _threads;
  ThreadAlgorithm _threadAlgorithm;
  DateTime _startTradesDateTime;
  DateTimeRangePtr _range;
  PositionSizingParamsImpl _posSizing;

  static const unsigned long DEFAULT_THREADS = 2;

  bool _chartsEnabled;
  bool _statsEnabled;
  bool _equityEnabled;
  bool _tradesEnabled;
  bool _signalsEnabled;
  bool _outputEnabled;

 public:
  RuntimeParamsImpl()
      : _threads(DEFAULT_THREADS),
        _range(new DateTimeRange(LocalTimeSec() - Days(30), LocalTimeSec())) {}

  void setRange(DateTimeRangePtr range) {
    __super::setDirty();
    _range = range;
  }

  void setStartTradesDateTime(DateTime startTradesDateTime) {
    __super::setDirty();
    _startTradesDateTime = startTradesDateTime;
  }

  void setPositionSizingParams(const tradery_thrift_api::PositionSizing& psp) {
    __super::setDirty();
    _posSizing.set(psp);
  }

  void setChartsEnabled(bool b) { _chartsEnabled = b; }
  void setEquityEnabled(bool b) { _equityEnabled = b; }
  void setStatsEnabled(bool b) { _statsEnabled = b; }
  void setTradesEnabled(bool b) { _tradesEnabled = b; }

  void setThreads(unsigned long threads) {
    __super::setDirty();
    _threads = threads;
  }

  void setThreadAlgorithm(ThreadAlgorithm ta) { _threadAlgorithm = ta; }

  virtual bool chartsEnabled() const { return _chartsEnabled; }
  virtual bool equityCurveEnabled() const { return _equityEnabled; }
  virtual bool statsEnabled() const { return _statsEnabled; }
  virtual bool tradesEnabled() const { return _tradesEnabled; }
  virtual bool signalsEnabled() const { return _signalsEnabled; }
  virtual bool outputEnabled() const { return _outputEnabled; }

  virtual DateTime startTradesDateTime() const { return _startTradesDateTime; }

  unsigned long getThreads() const { return _threads; }
  ThreadAlgorithm getThreadAlgorithm() const { return _threadAlgorithm; }
  DateTimeRangePtr getRange() const throw(DateTimeRangeException) {
    return _range;
  }

  const PositionSizingParams* positionSizing() const { return &_posSizing; }
  virtual bool isDirty() const {
    return __super::isDirty() || _posSizing.isDirty();
  }

  virtual void setClean() {
    __super::setClean();
    _posSizing.setClean();
  }
  void setDirty() {
    __super::setDirty();
    _posSizing.setDirty();
  }
};

class DocumentBase {
 public:
  virtual ~DocumentBase() {}

  DocumentBase() {}

  virtual const std::string name() = 0;
  virtual RuntimeParamsImpl& getRuntimeParams() = 0;
  virtual const UniqueId* getFirstRunnableId() const = 0;
  virtual const UniqueId* getNextRunnableId() const = 0;
  virtual bool hasDefaultCommission() const throw(DocumentException) = 0;
  virtual const UniqueId* getDefaultCommissionId() const
      throw(DocumentException) = 0;
  virtual bool hasDefaultSlippage() const throw(DocumentException) = 0;
  virtual const UniqueId* getDefaultSlippageId() const
      throw(DocumentException) = 0;
  virtual bool hasDefaultDataSource() const throw(DocumentException) = 0;
  virtual const UniqueId* getDefaultDataSourceId() const
      throw(DocumentException) = 0;
  // string passed to the get method
  virtual const std::vector<std::string>* defaultSymbolsSourceStrings() const
      throw(DocumentException) = 0;
  virtual const std::vector<std::string>* defaultDataSourceStrings() const
      throw(DocumentException) = 0;
  virtual const std::vector<std::string>* defaultSignalHandlerStrings() const
      throw(DocumentException) = 0;
  virtual const std::vector<std::string>* defaultSlippageStrings() const
      throw(DocumentException) = 0;
  virtual const std::vector<std::string>* defaultCommissionStrings() const
      throw(DocumentException) = 0;

  virtual bool hasDefaultSymbolsSource() const throw(DocumentException) = 0;
  virtual size_t runnablesCount() const = 0;
  virtual const UniqueId* getDefaultSymbolsSourceId() const
      throw(DocumentException) = 0;
  virtual bool hasDefaultSignalHandler() const throw(DocumentException) = 0;
  virtual const UniqueId* getDefaultSignalHandlerId() const
      throw(DocumentException) = 0;
  // returns the document's position sizing params
  // will return 0 if no position sizing params
  virtual const PositionSizingParams* positionSizingParams() const = 0;

  virtual chart::ChartManager* chartManager() = 0;

  // todo: for the moment will assume all runnables get the same vector of
  // strings, but we should in fact allow each runnable to receive its own set
  // of strings
  virtual const std::vector<std::string>* getRunnablesStrings() const
      throw(DocumentException) = 0;
  virtual const ExplicitTrades* getExplicitTrades(const UniqueId& id) const = 0;
  virtual const PluginTree& getSessionPluginTree() const = 0;
  virtual PluginTree& getSessionPluginTree() = 0;
  virtual const UniqueId& getSessionId() const = 0;
};
