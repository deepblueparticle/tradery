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

#include "core.h"
#include <tokenizer.h>
#include <datasource.h>

namespace tradery {
class ExplicitTradeException {
 private:
  const std::string _message;

 public:
  ExplicitTradeException(const std::string& message) : _message(message) {}

  const std::string& message() const { return _message; }
};

class ExplicitTradesException {
 private:
  const std::string _message;

 public:
  ExplicitTradesException(const std::string& message) : _message(message) {}

  const std::string& message() const { return _message; }
};

enum Type { MARKET, CLOSE, LIMIT, STOP, PRICE };

enum Action { BUY, SELL, SELL_SHORT, COVER, SELL_ALL, COVER_ALL, EXIT_ALL };

class ExplicitTrade {
 private:
  const PositionId _id;
  const std::string _symbol;
  const DateTime _time;
  const Type _type;
  const unsigned long _shares;
  const double _price;
  const Action _action;

 protected:
  Action toAction(const std::string& action);
  Type toType(const std::string& type);

 public:
  ExplicitTrade(const std::string& symbol, const DateTime& time, Action action,
                Type type, unsigned long shares, double price,
                PositionId id = 0)
      : _symbol(symbol),
        _time(time),
        _shares(shares),
        _action(action),
        _type(type),
        _price(price),
        _id(id) {}

 public:
  const std::string& symbol() const { return _symbol; }
  const DateTime& time() const { return _time; }
  Date date() const { return _time.date(); }
  Type type() const { return _type; }
  Action action() const { return _action; }
  double price() const { return _price; }
  unsigned long shares() const { return _shares; }

  void CORE_API processExit(Index barIndex, Positions pos, Bars bars) const;
  void CORE_API processEntry(Index barIndex, Positions pos, Bars bars) const;

  std::string toString() const;

  PositionId id() const { return _id; }
};

typedef ManagedPtr<const ExplicitTrade> ExplicitTradeConstPtr;

typedef std::vector<ExplicitTradeConstPtr> ExplicitTradesVector;

// timestamps to triggers
class TimeToExplicitTrades : public std::map<DateTime, ExplicitTradesVector> {
 public:
  void add(const DateTime& dt, const ExplicitTradeConstPtr et) {
    assert(et);
    //      std::cout <<_T( "[TimeToTriggers::add] " ) << trigger->symbol <<
    //      std::endl;
    iterator i = find(dt);

    if (i == __super::end())
      i = __super::insert(__super::value_type(dt, ExplicitTradesVector()))
              .first;

    i->second.push_back(et);
  }

  std::wstring toString() const {
    std::wstring str;
    for (__super::const_iterator i; i != __super::end(); ++i) {
    }

    return str;
  }
};

// symbols to (timestamps to triggers)
typedef std::map<std::string, TimeToExplicitTrades> SymbolToExplicitTrades;

class ExplicitTrades : public SymbolToExplicitTrades {
 private:
  ExplicitTradesVector _empty;

 private:
  void processEntries(const ExplicitTradesVector& t, Index barIndex,
                      Positions pos, Bars bars) const {
    for (unsigned int n = 0; n < t.size(); n++) {
      const ExplicitTrade& explicitTrade(*t[n]);
      explicitTrade.processEntry(barIndex, pos, bars);
    }
  }

  void processExits(const ExplicitTradesVector& t, Index barIndex,
                    Positions pos, Bars bars) const {
    for (unsigned int n = 0; n < t.size(); n++) {
      const ExplicitTrade& explicitTrade(*t[n]);
      explicitTrade.processExit(barIndex, pos, bars);
    }
  }

 protected:
  void add(const ExplicitTradeConstPtr et) {
    assert(et);
    std::string symbol = to_lower_case(et->symbol());
    //      std::cout << _T( "[tradery::Triggers::add]: " ) << symbol <<
    //      std::endl;

    SymbolToExplicitTrades::iterator i = SymbolToExplicitTrades::find(symbol);

    if (i == __super::end()) {
      //        std::cout << _T( "[tradery::Triggers::add]: " ) << symbol << _T(
      //        " not found" ) << std::endl;
      i = __super::insert(SymbolToExplicitTrades::value_type(
                              symbol, TimeToExplicitTrades()))
              .first;
    }

    i->second.add(et->time(), et);
  }

 public:
  const ExplicitTradesVector& getExplicitTrades(const std::string& symbol,
                                                const DateTime& dt) const {
    SymbolToExplicitTrades::const_iterator i =
        __super::find(to_lower_case(symbol));

    if (i != __super::end()) {
      const TimeToExplicitTrades& triggers(i->second);

      TimeToExplicitTrades::const_iterator i = triggers.find(dt);

      if (i != triggers.end())
        return i->second;
      else
        return _empty;

    } else
      return _empty;
  }

  void process(const std::string& symbol, DateTime time, Index barIndex,
               Positions pos, Bars bars) const {
    const ExplicitTradesVector& t(getExplicitTrades(symbol, time));

    if (t.size() > 0) {
      LOG(log_info, "ExplicitTrades::process " << time.to_simple_string());
    }

    processExits(t, barIndex, pos, bars);
    processEntries(t, barIndex, pos, bars);
  }

  Positions toPositions(Positions pos, const std::string& symbol,
                        tradery::BarsPtr data,
                        DateTime start = NegInfinityDateTime(),
                        DateTime end = PosInfinityDateTime()) const {
    // for now just bars data
    Bars bars(dynamic_cast<const BarsAbstr*>(data.get()));

    if (start.is_not_a_date_time()) start = NegInfinityDateTime();

    if (end.is_not_a_date_time()) end = PosInfinityDateTime();

    assert(bars);
    assert(!symbol.empty());
    assert(start < end);
    assert(bars.size() > 0);

    for (Index i = 0; i < bars.size(); i++) {
      DateTime dt = bars.time(i);
      if (dt >= end) break;

      if (dt >= start) process(symbol, bars.time(i), i, pos, bars);
    }

    return pos;
  }

  bool autoTrigger(const std::wstring& symbol, const DateTime& dt) {}

  virtual bool hasTriggers() const { return true; }

  std::wstring toString() const {}
};

typedef ManagedPtr<const ExplicitTrades> ExplicitTradesPtr;

class FileExplicitTrades : public ExplicitTrades {
 private:
  enum Format {
    csv,  // original format (comma separated strings)
    json  // json format
  };

 private:
  Format _format;

 private:
  // ignore empty line or comments (start with # or with //
  static bool isComment(const std::string& str);
  static bool ignore(const std::string& str);
  static std::string getComment(const std::string& str);

  // processes directives
  // format: !name=value
  void preprocess(const std::string& str);

  void processCSVFormat(const std::string& line, unsigned int lineCt);
  void processJSONFormat(const std::string& line, unsigned int lineCt);

 public:
  CORE_API FileExplicitTrades(const std::string& fileName);
};

}  // namespace tradery