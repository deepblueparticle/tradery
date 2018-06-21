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

#include <charthandler.h>
#include <set>
#include <iomanip>

#include <log.h>

using namespace tradery::chart;
using std::ostream;
#define SEP " ";

std::string getUniqueFileName();

// inline std::string elementEnd( const std::string

class Attribute {
 private:
  std::string _name;
  std::string _value;

 public:
  Attribute(const std::string& name, const std::string& value)
      : _name(name), _value(value) {}

  Attribute(const std::string& name, size_t value) : _name(name) {
    std::ostringstream os;
    os << value;

    _value = os.str();
  }

  operator std::string() const {
    std::ostringstream os;

    os << " " << _name << "=\"" << _value << "\"";
    return os.str();
  }

  const Attribute(const Attribute& attribute)
      : _name(attribute._name), _value(attribute._value) {}
};

class Attributes : public std::vector<Attribute> {
 public:
  void add(const std::string& name, const std::string& value) {
    __super::push_back(Attribute(name, value));
  }

  void add(const std::string& name, size_t value) {
    __super::push_back(Attribute(name, value));
  }

  void add(const Attribute& attr) { __super::push_back(attr); }

  Attributes& operator<<(const Attribute& attr) {
    add(attr);
    return *this;
  }

  operator std::string() const {
    std::ostringstream os;

    for (size_t n = 0; n < __super::size(); n++)
      os << (std::string) __super::at(n);

    return os.str();
  }
};

class XMLSerializer {
 private:
  ostream& _os;
  const std::string _elementName;

 public:
  virtual ~XMLSerializer() {
    // closing tag
    closingTag();
  }

  XMLSerializer(ostream& os, const std::string& elementName,
                const Attributes& attributes, bool hdr = false)
      : _os(os), _elementName(elementName) {
    if (hdr) header();

    _os << "<" << _elementName;
    _os << (std::string)attributes;
    _os << ">";
  }

  XMLSerializer(ostream& os, const std::string& elementName,
                const std::string& attributeName,
                const std::string& attributeValue, bool hdr = false)
      : _os(os), _elementName(elementName) {
    if (hdr) header();

    _os << "<" << _elementName;
    attribute(attributeName, attributeValue);
    _os << ">";
  }

  XMLSerializer(ostream& os, const std::string& elementName, bool hdr = false)
      : _os(os), _elementName(elementName) {
    if (hdr) header();

    _os << "<" << _elementName << ">";
  }

 private:
  void header() {
    _os << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        << std::endl;
  }
  void closingTag() { _os << "</" << _elementName << ">" << std::endl; }
  void attribute(const std::string& attributeName,
                 const std::string& attributeValue) {
    if (!attributeName.empty()) {
      _os << " " << attributeName << "=\"" << attributeValue << "\"";
    }
  }

 public:
  operator std::ostream&() { return _os; }
};

inline std::ostream& operator<<(std::ostream& os, XMLSerializer& serializer) {
  os << serializer;
  return os;
}

class ChartSerializer : public XMLSerializer {
 public:
  ChartSerializer(ostream& os) : XMLSerializer(os, "chart") {}
};

class XMLCDATASerializer : public XMLSerializer {
 private:
  void toCDATA(const std::string& data) {
    (ostream&)(*this) << "<![CDATA[" << data << "]]>";
  }

 public:
  XMLCDATASerializer(ostream& os, const std::string& elementName,
                     const std::string& data, const std::string& attribute,
                     const std::string& attributeValue)
      : XMLSerializer(os, elementName, attribute, attributeValue, false) {
    toCDATA(data);
  }

  XMLCDATASerializer(ostream& os, const std::string& elementName,
                     const std::string& data, const Attributes& attributes)
      : XMLSerializer(os, elementName, attributes, false) {
    toCDATA(data);
  }

  XMLCDATASerializer(ostream& os, const std::string& elementName,
                     const std::string& data)
      : XMLSerializer(os, elementName, false) {
    //      std::cout << "cdata: " << data << std::endl;
    toCDATA(data);
  }
};

#define sep ","

class PositionsSerializer : public PositionHandler {
 private:
  ostream& _os;

 public:
  PositionsSerializer(ostream& os) : _os(os) {
    // writing data format line:
    _os << "long/short,shares,entry date,entry bar,entry "
           "price,open/closed,exit date,exit bar,exit price,gain,pct gain"
        << std::endl;
  }

  virtual void onPositionConst(const tradery::Position pos) {
    //		COUT << "Position: " << pos.toMinString() << std::endl;
    _os << pos.isLong() << sep << pos.getShares() << sep
        << pos.getEntryTime().date().to_iso_extended_string() << sep
        << pos.getEntryBar() << sep << pos.getEntryPrice() << sep
        << pos.isOpen();

    if (pos.isClosed()) {
      _os << sep << pos.getCloseTime().date().to_iso_extended_string() << sep
          << pos.getCloseBar() << sep << pos.getClosePrice() << sep
          << pos.getGain() << sep << pos.getPctGain();
    }
    _os << std::endl;
  }
};

class WebComponent {
 private:
  const std::string& _path;

 protected:
  WebComponent(const std::string& path) : _path(path) {}

  //    const std::string& path() { return _path; }

  void serializeSeries(ostream& os, const std::string& name,
                       const Series& series, const std::string& fnPostfix) {
    //      std::cout << "in serializeSeries: " << series.size() << std::endl;
    std::string fileName = addFSlash(_path) + name + fnPostfix + ".txt";

    std::ofstream of(fileName.c_str());
    //      std::cout << "in serializeSeries 1" << std::endl;

    if (!of) {
      // handle error
    } else {
      //        std::cout << "in serializeSeries 2" << std::endl;
      for (size_t n = 0; n < series.size(); n++)
        of << fixed << series[n] << SEP;

      //        std::cout << "in serializeSeries 3" << std::endl;
      XMLCDATASerializer x(os, name, fileName);
      //        std::cout << "in serializeSeries 4" << std::endl;
    }
  }

  void serializePositions(ostream& os, const std::string& name,
                          const PositionsPtrVector& pos,
                          const std::string& fnPostfix) {
    std::string fileName = addFSlash(_path) + name + fnPostfix + ".txt";

    //    std::cout << "Serializing positions to file: " << fileName <<
    //    std::endl;

    std::ofstream of(fileName.c_str());

    if (!of) {
      // todo: handle error
    } else {
      PositionsSerializer ps(of);
      if (!pos.hasEnablePositions()) {
        LOG(log_info, "No positions");
      } else {
        //		  COUT << "positions: " << pos.enabledCount() <<
        // std::endl;
        for (size_t n = 0; n < pos.size(); n++) pos[n]->forEachConst(ps);
        XMLCDATASerializer x(os, name, fileName);
      }
    }
  }
};

class WebPositionsComponent : public WebComponent, public chart::Component {
 private:
  WithPositions& _wp;
  const bool _reduced;

 public:
  WebPositionsComponent(const std::string& name, const std::string& path,
                        WithPositions& wp, bool reduced)
      : _wp(wp),
        WebComponent(path),
        Chart::Component(name),
        _reduced(reduced) {}

 protected:
  void serialize(ostream& os) {
    if (_wp.positions().hasEnablePositions()) {
      //    	  COUT << "serializing WebPositionsComponent" << std::endl;
      Attributes attrs;
      attrs << Attribute("type", "positions")
            << Attribute("name", __super::getName());
      XMLSerializer y(os, "component", attrs);

      // ohlcv + timestamp
      std::string postFix = getUniqueFileName();
      __super::serializePositions(os, "positions", _wp.positions(), postFix);
    } else {
      //        std::cout << "positions is 0" << std::endl;
      return;
    }
  }
};

class WebBarsComponent : public WebComponent, public chart::Component {
 private:
  WithBars& _wb;
  const bool _reduced;

 public:
  WebBarsComponent(const std::string& name, const std::string& path,
                   WithBars& wb, bool reduced)
      : _wb(wb), WebComponent(path), chart::Component(name), _reduced(reduced) {
    //      std::cout << "web bars component constructor" << std::endl;
  }

 private:
 protected:
  void serialize(ostream& os) {
    //      COUT << "serializing web bars component" << std::endl;

    try {
      if (_wb.bars()) {
        Attributes attrs;
        attrs << Attribute("type", "bars")
              << Attribute("name", __super::getName());
        XMLSerializer y(os, "component", attrs);

        /*
// ohlcv + timestamp
std::string postFix = getUniqueFileName();
__super::serializeSeries( os, "open", _wb.bars().openSeries(), postFix );
__super::serializeSeries( os, "high", _wb.bars().highSeries(), postFix );
__super::serializeSeries( os, "low", _wb.bars().lowSeries(), postFix );
__super::serializeSeries( os, "close", _wb.bars().closeSeries(), postFix );
__super::serializeSeries( os, "volume", _wb.bars().volumeSeries(), postFix );
        */
      } else {
        LOG(log_info, "bars is 0");
        return;
      }
    } catch (const std::bad_cast&) {
      LOG(log_info, "bad cast in bars()");
      assert(false);
    }
  }

 public:
};

class WebLine : public chart::Line, public WebComponent {
 private:
  const bool _reduced;

 public:
  WebLine(const std::string& name, const Series& series,
          const std::string& path, const Color& color, bool reduced)
      : chart::Line(name, series, color),
        WebComponent(path),
        _reduced(reduced) {}

  virtual void serialize(ostream& os) {
    if (_reduced) return;
    try {
      Attributes attrs;
      attrs.add("type", "series");
      attrs.add("name", __super::getName());
      attrs.add("color", __super::getColor().rgbAsString());
      attrs.add("transparency", __super::getColor().transparencyAsString());

      XMLSerializer y(os, "component", attrs);

      // ohlcv + timestamp
      std::string postFix = getUniqueFileName();
      __super::serializeSeries(os, "series", __super::series(), postFix);
    } catch (const std::bad_cast& e) {
      LOG(log_error, "bad cast in bars: " << e.what());
      assert(false);
    }
  }
};

class WebPane : public tradery::chart::PaneAbstr {
 private:
  const std::string _path;
  const bool _reduced;

 public:
  WebPane(const std::string& name, const Color& background,
          const std::string& path, bool def, bool reduced)
      : _path(path), PaneAbstr(name, def, background), _reduced(reduced) {
    //      std::cout << "WebPane constructor: " << name << std::endl;
  }
  virtual void serialize(ostream& os) {
    // only serialize if not reduced
    bool def(__super::isDefault());
    if (!_reduced || def) {
      //		  COUT << "serializing " << ( def ? "default" : "non
      // default" ) << " web pane" << std::endl;
      Attributes attrs;

      attrs.add("name", __super::getName());
      attrs.add("default", def ? "true" : "false");
      if (!_reduced) {
        // only draw the background if not reduced - in reduced state, we only
        // show the basic info, no system specific stuff like background etc
        attrs.add("background", __super::background().rgbAsString());
        attrs.add("transparency", __super::background().transparencyAsString());
      }

      //!!! XMLserializer serializes on destructor, so careful with the scoping
      //!!!!
      XMLSerializer x(os, "pane", attrs);
      __super::serializeComponents(os);
    } else
      __super::serializeComponents(os);
  }

  virtual void drawSeries(const std::string& name, const Series& series,
                          const Color& color = Color()) {
    // only draw if not reduced
    if (!_reduced) {
      //      std::cout << "WebPane::drawSeries: " << name << std::endl;
      if (__super::componentCount() > 5) {
        std::ostringstream os;

        os << "Too many components in pane: " << __super::getName();
        throw ChartException(os.str());
      } else {
        //        std::cout << "WebPane::drawSeries: adding line " << name <<
        //        std::endl;
        __super::add(new WebLine(name, series, _path, color, _reduced));
        //        std::cout << "WebPane::drawSeries: after adding line" <<
        //        std::endl;
      }
    }
  }
};

class WebBarsPane : public WebPane {
 public:
  WebBarsPane(const std::string& name, const Color& background,
              const std::string& path, bool def, WithBars& wb,
              WithPositions& wp, bool reduced = false)
      : WebPane(name, background, path, def, reduced) {
    //    std::cout << "web bars pane constructor" << std::endl;
    add(new WebBarsComponent("", path, wb, reduced));
    add(new WebPositionsComponent("", path, wp, reduced));
  }
};

// a chart with a bars pane
class WebBarsChart : public tradery::chart::BarsChart {
 private:
  const std::string _path;
  const bool _reduced;

 public:
  WebBarsChart(const std::string& name, const std::string& symbol,
               const std::string& path, bool reduced = false)
      : BarsChart(name, symbol), _path(path), _reduced(reduced) {
    //      std::cout << "Web bars chart constructor" << std::endl;
  }

  Pane createPane(const std::string& name, const Color& background = Color()) {
    if (__super::getPaneCount() > 10)
      throw ChartException("Too many chart panes");

    //      std::cout << "creating web pane" << std::endl;
    PaneAbstr* pane(new WebPane(name, background, path(), false, _reduced));
    //      std::cout << "adding web pane" << std::endl;
    __super::add(pane);
    //      std::cout << "after adding web pane" << std::endl;
    return Pane(pane);
  }

  virtual Pane createDefaultPane(const std::string& name,
                                 const Color& background = Color()) {
    //      std::cout << "Creating default pane for WebBarschart, symbol: " <<
    //      __super::getSymbol() << std::endl;
    PaneAbstr* pane(new WebBarsPane(name, background, path(), true, *this,
                                    *this, _reduced));
    //      std::cout << "Adding default pane" << std::endl;
    __super::addDefaultPane(pane);
    //      std::cout << "After adding default pane" << std::endl;
    return Pane(pane);
  }

  void serialize(ostream& os) {
    //    std::cout << "Serializing webchart" << std::endl;
    const Bars bars = __super::bars();
    //    std::cout << "Serializing webchart - bars ptr: " << hex << bars <<
    //    std::endl;

    if (bars != 0) {
      Attributes attrs;

      //      std::cout << "Serializing webchart - adding symbol attribute: " <<
      //      __super::getSymbol() << std::endl;
      attrs.add("symbol", __super::getSymbol());
      //      std::cout << "Serializing webchart - adding name attribute: " <<
      //      __super::getName() << std::endl;
      attrs.add("name", __super::getName());
      //      std::cout << "Serializing webchart - adding datacount attribute: "
      //      << __super::bars()->size() << std::endl;

      // need to get the unsync parameters that refers to the actual or
      // otherwise the chart will be wrong if the bars has been synced
      attrs.add("datacount", bars.unsyncSize());

      // need to get the real
      std::pair<DateTime, DateTime> startEnd = bars.unsyncStartEnd();

      LOG(log_debug, "range: " << startEnd.first.to_simple_string() << " - "
                               << startEnd.second.to_simple_string());

      Date start = startEnd.first.date();
      attrs.add("startDate", std::string()
                                 << start.month() << "/" << start.day() << "/"
                                 << start.year());

      Date end = startEnd.second.date();
      attrs.add("endDate", std::string() << end.month() << "/" << end.day()
                                         << "/" << end.year());
      //      std::cout << "WebChart, before XMLSerializer" << std::endl;
      XMLSerializer x(os, "chart", attrs, true);
      //      std::cout << "WebChart, after XMLSerializer" << std::endl;

      if (__super::getError()) {
        LOG(log_error,
            "serialize, error message:" << __super::getErrorMessage());
        XMLSerializer z(os, "error");
        os << __super::getErrorMessage();
      }

      os << bars.locationInfoToXML();

      //      std::cout << "[WebChart::serialize] before call to
      //      serializeComponents" << std::endl;
      __super::serializeComponents(os);
      //      std::cout << "[WebChart::serialize] after call to
      //      serializeComponents" << std::endl;
    } else {
      LOG(log_error,
          "writing chart error - no data for symbol: " << __super::getSymbol());
      XMLSerializer z(os, "error");
      os << "No data for symbol: " << __super::getSymbol();
    }

    //    std::cout << "End serializing webchart" << std::endl;
  }

 protected:
  const std::string& path() const { return _path; }
};

class WebChartManagerSerializer : public XMLSerializer {};

class WebChartManager : public chart::ChartManager {
 private:
  const std::string _chartRootPath;
  const std::string _chartsDescriptionFile;
  const bool _reduced;

 private:
  WebBarsChart* createWebChart(const std::string& name,
                               const std::string& symbol);

 public:
  // reduced is used when we only want to display price bars and position, no
  // lines, panes etc. this is used for multi-system run
  WebChartManager(const std::string& name,
                  const std::string& symbolsToChartFile,
                  const std::string& chartRootPath,
                  const std::string& chartsDescriptionFile,
                  bool reduced = false);

  const std::string& chartRootPath() const { return _chartRootPath; }

  virtual void serialize() {
    LOG(log_info,
        "Web chart manager, serializing charts, opening chart description "
        "file: "
            << _chartsDescriptionFile);

    if (_chartsDescriptionFile.empty()) return;

    std::ofstream os(_chartsDescriptionFile.c_str());
    if (os) {
      // create the header and root element "charts"
      XMLSerializer y(os, "charts", true);
      Charts& charts(__super::getCharts());
      for (Charts::iterator i = charts.begin(); i != charts.end(); i++) {
        ChartPtr chart = (*i).second;
        if (!chart->enabled()) continue;
        std::string symbol = (*i).first;
        // todo: this test (hasPositions) should be implemented in a base class
        // or in chart, and not here
        //        std::cout << "Web chart manager, testing positions" <<
        //        std::endl;
        bool hasPositions = chart->getPositions().hasEnablePositions();
        //        std::cout << "Web chart manager, after testing positions" <<
        //        std::endl;
        // todo: handle invalid path characters in symbol name
        // the description for a symbol is <path>\symbol.xml
        std::string fileName =
            addFSlash(_chartRootPath) + chart->getSymbol() + ".xml";
        ofstream symbolDescFile(fileName.c_str());
        if (!symbolDescFile) {
          LOG(log_info,
              "WebChartManager:;serialize Can't create description file for "
              "writing: "
                  << fileName);
        } else {
          // write the file name in the manager file

          // create an element "chart" with the attribute the symbol, and the
          // value the file name that contains the actual description of the
          // chart
          Attributes attrs;

          attrs.add("symbol", symbol);
          attrs.add("positions", hasPositions);
          XMLCDATASerializer x(os, "chart", fileName, attrs);
          // now finalize the actual chart

          __super::runtimeStats()->step(__super::getTotalPct() /
                                        ((double)charts.size()));
          __super::runtimeStats()->setMessage(
              std::string("Generating chart(s) for \"") << symbol << "\"");
          __super::runtimeStats()->setStatus(RuntimeStatus::RUNNING);

          std::ofstream ofs(fileName.c_str());

          if (ofs) {
            //            std::cout << "Web chart manager, before serializing
            //            chart: " << symbol << std::endl;
            chart->serialize(ofs);
            //            std::cout << "Web chart manager, after serializing
            //            chart: " << symbol << std::endl;
          } else {
            // todo - handle file open error
          }
        }
      }
    } else {
      // could not open chart description file for writing
      LOG(log_info, "could not open chart description file for writing");
      throw ChartManagerException();
    }
    LOG(log_info, "Web chart manager, exiting serialize");
  }
};

// turns a collection of bar data into a csv string
inline std::ostream& operator<<(std::ostream& os, Bars bars) {
  for (unsigned int n = 0; n < bars.size(); n++) {
    Bar bar(bars.getBar(n));
  }

  return os;
}
