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

#include <iomanip>

#include <datasource.h>
#include "core.h"
#include "log.h"
#include "colors.h"

using std::ostream;

namespace tradery {
namespace chart {

class Color {
 private:
  bool _default;

  unsigned char _transparency;
  unsigned long _rgb;

 public:
  Color() : _default(true), _transparency(0x00) {}
  Color(unsigned long rgb, unsigned char transparency = 0x00)
      : _default(false), _rgb(rgb), _transparency(transparency) {
    assert(rgb <= 0xffffff);
    assert(transparency <= 0xff);
  }

  bool isDefault() const { return _default; }
  unsigned long rgb() const { return _rgb; }
  unsigned char transparency() const { _transparency; }

  std::string rgbAsString() const {
    if (_default)
      return "default";
    else {
      std::ostringstream os;

      os << std::hex << _rgb;
      return os.str();
    }
  }

  std::string transparencyAsString() const {
    std::ostringstream os;

    os << std::hex << (long)_transparency;
    //          std::cout << "transparency: " << os.str() << std::endl;
    return os.str();
  }
};

class ChartException {
 private:
  const std::string _message;

 public:
  ChartException(const std::string& message) : _message(message) {}

  const std::string& message() const { return _message; }
};

class PaneException {};

class WithPositions {
 public:
  virtual ~WithPositions() {}

  virtual const tradery::PositionsPtrVector& positions() const = 0;
};

class WithBars {
 public:
  virtual ~WithBars() {}
  virtual const Bars bars() = 0;
};

class Component {
 private:
  std::string _name;
  bool _enabled;
  bool _error;
  std::string _errorMessage;

 public:
  Component(const std::string& name)
      : _name(name), _enabled(true), _error(false) {}
  virtual ~Component() {}
  virtual void serialize(ostream& os) = 0;

  const std::string& getName() const { return _name; }
  void setName(const std::string& name) { _name = name; }
  bool enabled() const { return _enabled; }
  void disable() { _enabled = false; }
  void setError(const std::string& message) {
    _error = true;
    _errorMessage = message;
  }
  bool getError() const { return _error; }
  const std::string& getErrorMessage() const { return _errorMessage; }
};

// a line
class Line : public Component {
 private:
  Series _s;
  Color _color;

 public:
  Line(const std::string& name, const Series& series, const Color& color)
      : _s(series), Component(name), _color(color) {}

  const Color& getColor() const { return _color; }
  void setColor(const Color& color) { _color = color; }

 protected:
  const Series& series() const { return _s; }
};

// shows the whole bar data (prices + volume)
// derived classes are candlesticks, or ohlcv

typedef std::vector<ManagedPtr<Component> > Components;

class ComponentContainer : public Component {
 private:
  Components _components;

 public:
  ComponentContainer(const std::string& name) : Component(name) {}
  virtual ~ComponentContainer() {}

  void add(Component* component) {
    //        std::cout << "ComponentContainer::add: " << component <<
    //        std::endl;
    _components.push_back(component);
  }

  Component* get(unsigned int ix) { return _components[ix].get(); }

  size_t count() const { return _components.size(); }

 protected:
  void serializeComponents(ostream& os) {
    //      std::cout << "ComponentContainer - serializeComponents: " <<
    //      _components.size() << std::endl;
    for (Components::iterator i = _components.begin(); i != _components.end();
         i++) {
      Component* component = (*i).get();

      component->serialize(os);
    }
  }
};

class PaneAbstr : public ComponentContainer {
 private:
  const bool _default;
  Color _background;

 public:
  PaneAbstr(const std::string& name, bool def, const Color& background)
      : _default(def), ComponentContainer(name), _background(background) {}

  virtual ~PaneAbstr() {}

  virtual void drawSeries(const std::string& name, const Series& series,
                          const Color& color) = 0;
  bool isDefault() const { return _default; }

  void setBackgroundColor(const Color& color) { _background = color; }

  size_t componentCount() const { return __super::count(); }
  const Color& background() const { return _background; }
};

class BarsPane : public PaneAbstr {
  Bars _bars;

 public:
  BarsPane(const std::string& name, Bars bars, bool def,
           const Color& background)
      : _bars(bars), PaneAbstr(name, def, background) {}
};

typedef ManagedPtr<PaneAbstr> PaneAbstrPtr;

class Pane {
 private:
  PaneAbstr* _pane;

 protected:
  Pane() : _pane(0) {}

 public:
  Pane(PaneAbstr* pane) : _pane(pane) {
    // a pane can be 0 if not chart if being generated
    // the tests in the following functions ensure that
    //        std::cout << "Creating a Pane: " << pane << std::endl;

    //        std::cout << "Creating a Pane after assert: " << std::endl;
  }

  void drawSeries(const std::string& name, const Series& series,
                  const Color color = Color()) {
    //        std::cout << "Drawing series: " << name << std::endl;
    if (_pane != 0) _pane->drawSeries(name, series, color);
    //        std::cout << "After drawing series: " << name << std::endl;
  }

  void setBackgroundColor(const Color color) {
    if (_pane != 0) _pane->setBackgroundColor(color);
  }

  bool isDefault() const {
    if (_pane != 0) return _pane->isDefault();
  }
};

class NullPane : public Pane {
 public:
  NullPane() {}

  virtual void serialize(ostream& os) {}
};

// a chart contains panes elements
class Chart : public ComponentContainer {
 private:
  const std::string _symbol;
  BarsPtr _dr;
  PositionsPtrVector _pos;
  PaneAbstr* _defaultPane;
  Mutex _m;

 public:
  Chart(const std::string& name, const std::string& symbol)
      : _symbol(symbol), _defaultPane(0), ComponentContainer(name) {
    assert(!symbol.empty());
  }
  // this will normally come from a null chart handler
  Chart(const std::string& name) : _defaultPane(0), ComponentContainer(name) {}
  virtual ~Chart() {}

  void init(BarsPtr data, const PositionsContainer* pos) {
    Lock lock(_m);
    //        std::cout << "Chart::init, symbol: " << _symbol << ", _dc: " << dc
    //        << ", _pos: " << pos << std::endl;

    // new data should be the same as old
    assert(_dr.get() == 0 || data.get() == _dr.get());

    // only add data the first time
    if (!_dr) _dr = data;

    _pos.add(pos);
  }

  void addDefaultPane(PaneAbstr* pane) {
    _defaultPane = pane;

    __super::add(pane);
  }

  Pane getDefaultPane() { return Pane(_defaultPane); }

  Pane getPane(unsigned int ix) {
    try {
      return Pane(dynamic_cast<PaneAbstr*>(__super::get(ix)));
    } catch (const std::bad_cast&) {
      assert(false);

      LOG(log_error,
          "could not convert from Component to Pane in Chart::getPane");
    }
  }

  virtual Pane createDefaultPane(const std::string& name,
                                 const Color& background = Color()) = 0;

  virtual Pane createPane(const std::string& name,
                          const Color& backgroun = Color()) = 0;

  //  virtual Pane& addPane() = 0;

  const std::string& getSymbol() const { return _symbol; }

  BarsPtr getDataCollection() const {
    //        std::cout << "[Chart::getDatacollection] " << _symbol << ", " <<
    //        std::hex << _dc << std::endl;
    return _dr;
  }

  const PositionsPtrVector& getPositions() const { return _pos; }

  const size_t getPaneCount() const { return __super::count(); }
};

// this is a chart that applies to a system that runs on bars, and it has a set
// of default positions too (the ones for the default symbols) it should be
// possible to add other positions collections as well, if the system generates
// these for other symbols
class BarsChart : public Chart, public WithBars, public WithPositions {
 public:
  BarsChart(const std::string& name, const std::string& symbol)
      : Chart(name, symbol) {}

  const Bars bars() {
    try {
      // can't create a new Bars object here, or the pointer may get deleted
      // although the cache will think it's still around and this will cause
      // other problems like double deletion etc.
      //          std::cout << "[BarsChart::bars] before dynamic cast" <<
      //          std::endl;
      return Bars(
          dynamic_cast<const BarsAbstr*>(__super::getDataCollection().get()));
      //          std::cout << "[BarsChart::bars] after dynamic cast" <<
      //          std::endl;
    } catch (const std::bad_cast&) {
      LOG(log_error, "[BarsChart::bars] bad cast exception");
      return 0;
    } catch (...) {
      LOG(log_error, "[BarsChart::bars] unknown exception");
      return 0;
    }
  }

  const PositionsPtrVector& positions() const {
    return __super::getPositions();
  }
};

class NullChart : public Chart {
 public:
  NullChart() : Chart("Null chart") {
    LOG(log_info, "creating null chart");
    __super::disable();
  }
  void serialize(ostream& os) { LOG(log_info, "serializing null chart"); }

  virtual Pane createPane(const std::string& name,
                          const Color& background = Color()) {
    //        std::cout << "null chart - create pane" << std::endl;
    return NullPane();
  }

  virtual Pane createDefaultPane(const std::string& name,
                                 const Color& background = Color()) {
    LOG(log_info, "null chart - create default pane");
    return NullPane();
  }
};

class ChartManagerException {};

typedef ManagedPtr<Chart> ChartPtr;

class ChartManager {
 protected:
  typedef std::map<std::string, ChartPtr> Charts;

  RuntimeStats* _rts;

 private:
  Charts _charts;

 public:
  ChartManager() : _rts(0) {}
  virtual ~ChartManager() {}

  void addChart(const std::string& symbol, Chart* ch) {
    // we can't a chart over another one
    // we'll only chart one instance, so if the chart is already
    // there, don't do anything
    LOG(log_info, "symbol: " << symbol);
    if (_charts.find(symbol) == _charts.end())
      _charts.insert(Charts::value_type(symbol, ch));
  }

  Chart* getChart(const std::string& symbol) {
    Charts::iterator i = _charts.find(symbol);

    if (i == _charts.end()) {
      addChart(symbol, new NullChart());
      i = _charts.find(symbol);

      assert(i != _charts.end());
    }

    return (*i).second.get();
  }

  Charts& getCharts() { return _charts; }

  virtual void serialize() = 0;

  void init(RuntimeStats* rts) {
    LOG(log_info, "init");
    _rts = rts;
    // charts take 10% (arbitrary value)
    _rts->addPct(getTotalPct());
  }

 protected:
  RuntimeStats* runtimeStats() { return _rts; }
  double getTotalPct() const { return 10; }
};

typedef ManagedPtr<ChartManager> ChartManagerPtr;

};  // namespace chart
};  // namespace tradery
