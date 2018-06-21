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

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file for PCH
#endif

#include <log.h>
#include "resource.h"  // main symbols
#include <chartdir.h>
#include "financechart.h"

#include <boost\shared_array.hpp>

#define DATASOURCE_FORMAT1_NAME "Data source plugin format 1"
#define DATASOURCE_FORMAT3_NAME "Data source plugin format 3"

// Cfileplugins1App
// See fileplugins1.cpp for the implementation of this class
//

// we want the each data source instance to have a different id, now that all
// runs are inside the same process to avoid id collisions in data manager.
const Info dataSourceInfoFormat1("6188C38F-78FB-470c-A90D-C03FE37D5DEC",
                                 DATASOURCE_FORMAT1_NAME, "");
const Info dataSourceInfoFormat1NewId(DATASOURCE_FORMAT1_NAME, "");
const Info dataSourceInfoFormat3("3F8D0DAA-C11E-452c-A097-20127C0673E0",
                                 DATASOURCE_FORMAT3_NAME, "");
const Info dataSourceInfoFormat3NewId(DATASOURCE_FORMAT3_NAME, "");
const Info symbolsSourceInfo(
    "E32C975A-ECE1-4e7f-BB49-A604F2EE8083",
    "Symbols Source plugin - symbols file specified dynamically", "");
const Info statsInfo("4B6632DE-CD7B-43c6-932B-13D098E1E287", "Stats plugin",
                     "Implemented as a signal handler plugin, using only the "
                     "session notifications to calculate the stats");

#define CHART_TITLE "<*block,valign=absmiddle*>Equity Curve<*/*>"
#define CHART_WIDTH 850
#define CHART1_HEIGHT 500
#define LEFT_SPACE 70
#define RIGHT_SPACE 3
#define CHART1_TOP_SPACE 0
#define CHART1_BOTTOM_SPACE 0

#define CHART_PLOT_X CHART_WIDTH - LEFT_SPACE - RIGHT_SPACE
#define CHART1_PLOT_Y CHART1_HEIGHT - CHART1_TOP_SPACE - CHART1_BOTTOM_SPACE
#define CHART1_PLOT_BOTTOM CHART1_HEIGHT - CHART1_BOTTOM_SPACE
#define CHART_PLOT_RIGHT CHART_WIDTH - RIGHT_SPACE
#define CHART_PLOT_WIDTH CHART_PLOT_RIGHT - LEFT_SPACE
#define LEGEND_X LEFT_SPACE
#define LEGEND_Y 0
#define LEGEND_WIDTH CHART_X_SIZE
#define LEGEND_HEIGHT 25
#define EQUITY_START_COLOR 0x8000FF00
#define EQUITY_END_COLOR 0x80FFFFFF
#define LEGEND_TEXT_SHORT "Short equity"
#define LEGEND_TEXT_LONG "Long equity"
#define LEGEND_TEXT_CASH "Cash"
#define LEGEND_TEXT_BH "Buy & Hold equity"
#define LEGEND_TEXT_EQUITY "Total equity"
#define LEGEND_TOTAL_DD "Total"
#define LEGEND_SHORT_DD "Short"
#define LEGEND_LONG_DD "Long"
#define LEGEND_BH_DD "Buy & Hold"
#define LEGEND_TOTAL_DD_DURATION "Total"
#define LEGEND_LONG_DD_DURATION "Long"
#define LEGEND_SHORT_DD_DURATION "Short"
#define LEGEND_BH_DD_DURATION "Buy & Hold"
#define X_LABEL_COUNT 30
#define X_LABEL_ANGLE 45
#define COPYRIGHT_TEXT "© Tradery.com"
#define COPYRIGHT_X CHART_PLOT_RIGHT - 80
#define X_LABEL_GAP 3

#define STATS_X CHART_PLOT_RIGHT - 200
#define STATS_Y LEGEND_Y + 10

#define TREND_COLOR 0X000000
#define STATS_COLOR 0X003F00

#define CHART2_TOP_SPACE 0
#define CHART2_BOTTOM_SPACE 0
#define CHART2_HEIGHT 150
#define CHART2_PLOT_Y CHART2_HEIGHT - CHART2_TOP_SPACE - CHART2_BOTTOM_SPACE

#define CHART22_HEIGHT CHART2_HEIGHT

#define CHART3_HEIGHT 220
#define CHART3_TOP_SPACE 0
#define CHART3_BOTTOM_SPACE 70
#define CHART3_PLOT_Y CHART3_HEIGHT - CHART3_TOP_SPACE - CHART3_BOTTOM_SPACE

#define TITLE_X LEFT_SPACE
#define TITLE_Y 0
#define TITLE_WIDTH CHART_WIDTH - LEFT_SPACE - RIGHT_SPACE
#define TITLE_HEIGHT 18

#define TITLE_BACKGROUND_COLOR 0Xc08080ff

#define GAP 0
#define TITLE1_POS_Y 0
#define CHART1_POS_Y TITLE_HEIGHT

#define TITLE2_POS_Y TITLE_HEIGHT + CHART1_HEIGHT + GAP
#define CHART2_POS_Y TITLE2_POS_Y + TITLE_HEIGHT

#define TITLE22_POS_Y CHART2_POS_Y + CHART2_HEIGHT + GAP
#define CHART22_POS_Y TITLE22_POS_Y + TITLE_HEIGHT

#define TITLE3_POS_Y CHART22_POS_Y + CHART22_HEIGHT + GAP
#define CHART3_POS_Y TITLE3_POS_Y + TITLE_HEIGHT

#define SHORT_COLOR 0x00c00000
#define LONG_COLOR 0x000000c0
#define BH_COLOR 0X00C000C0
#define TOTAL_COLOR 0x8080ff80
#define CASH_COLOR 0X00008000

#define CHART1_BK_COLOR 0x80fff0f0
#define CHART21_BK_COLOR 0xf0fff0
#define CHART22_BK_COLOR 0xf0ffff
#define CHART3_BK_COLOR 0xf0f0ff

#define TITLE_TEXT_COLOR 0xffffff
#define TITLE_STYLE "<*font=bold,size=10,color=0X200000*>"

#define MULTICHART_HEIGHT CHART3_POS_Y + CHART3_HEIGHT

// this implements a dynamic array of arrays, used by the chart library
// the base class is the array itself, and the shared_array is just to make sure
// they get deleted at the end
class XLabels : public std::vector<const char*> {
 public:
  std::vector<boost::shared_array<char> > _v;

 public:
  XLabels(const EquityCurve& em) {
    for (EquityCurve::const_iterator i = em.begin(); i != em.end(); i++) {
      std::string str((*i).first.to_simple_string());
      char* p = new char[str.length() + 1];
      strcpy(p, str.c_str());
      _v.push_back(boost::shared_array<char>(p));
      __super::push_back(p);
    }
  }

 public:
  operator const char* const*() const {
    //    return reinterpret_cast< const TCHAR** >( &(__super::front() ) );
    return &(front());
  }

  const char* operator[](size_t index) {
    return std::vector<const char*>::operator[](index);
  }
};

class Chart1 : public XYChart {
 public:
  Chart1(const EquityCurve& ec, const EquityCurve& bhec, const XLabels& labels)
      : XYChart(CHART_WIDTH, CHART1_HEIGHT, Chart::Transparent,
                Chart::Transparent, 0) {
    assert(ec.getSize() > 0);

    const double* total(ec.getTotal());
    const double* sh(ec.getShort());
    const double* lg(ec.getLong());
    const double* cash(ec.getCash());
    const double* bh(bhec.getTotal());

    // Set the plotarea at (50, 30) and of size 240 x 140 pixels. Use white
    // (0xffffff) background.
    PlotArea* pa =
        setPlotArea(LEFT_SPACE, CHART1_TOP_SPACE, CHART_PLOT_X, CHART1_PLOT_Y,
                    CHART1_BK_COLOR, -1, -1, 0xcccccc, 0xcccccc);
    pa->setBackground("background.gif");

    xAxis()->setColors(0x000000, Chart::Transparent);

    // Add a legend box at (50, 185) (below of plot area) using horizontal
    // layout. Use 8 pts Arial font with Transparent background.
    LegendBox* legendBox = addLegend(80, 15, true, "", 8);
    legendBox->setReverse();
    legendBox->setBackground(Chart::Transparent, Chart::Transparent);

    // Add keys to the legend box to explain the color zones
    /*    legendBox->addKey( LEGEND_TEXT_SHORT, LONG_COLOR );
        legendBox->addKey( LEGEND_TEXT_LONG, SHORT_COLOR);
    */
    // Add a title box to the chart using 8 pts Arial Bold font, with yellow
    // (0xffff40) background and a black border (0x0)
    //    c->addTitle("Equity Curve", "arialbd.ttf",
    //    10)->setBackground(0xffff40, 0); c1.addTitle( CHART_TITLE, 0, 10,
    //    0xffffff )->setBackground(0xc00000, -1,
    //    Chart::softLighting(Chart::Right));

    // Set the y axis label format to US$nnnn

    TextBox* tb = addText(COPYRIGHT_X, CHART1_PLOT_Y - 20, COPYRIGHT_TEXT);

    std::ostringstream statsText;

    double start = total[0];
    double end = total[ec.getSize() - 1];
    double pctGain = total[0] != 0 ? (end - start) / start * 100 : 0;
    statsText << "Starting equity: " << std::fixed << std::setprecision(2)
              << start << std::endl;
    statsText << "Ending equity: " << std::fixed << std::setprecision(2)
              << total[ec.getSize() - 1];
    statsText << std::endl
              << "Gain: " << std::fixed << std::setprecision(2) << pctGain
              << " %";
    TextBox* stats = addText(STATS_X, STATS_Y, statsText.str().c_str());
    stats->setFontColor(STATS_COLOR);

    addTrendLayer(DoubleArray(total, (int)ec.getSize()), TREND_COLOR,
                  "Trend Line")
        ->setLineWidth(2);
    LineLayer* layer1 = addLineLayer();
    // Set the default line width to 2 pixels

    // Add the three data sets to the line layer. For demo purpose, we use a
    // dash line color for the last line
    layer1->addDataSet(DoubleArray(sh, (int)ec.size()), SHORT_COLOR,
                       LEGEND_TEXT_SHORT);
    layer1->addDataSet(DoubleArray(lg, (int)ec.size()), LONG_COLOR,
                       LEGEND_TEXT_LONG);
    //    layer1->addDataSet(DoubleArray( cash, (int)ec.size() ), CASH_COLOR,
    //    LEGEND_TEXT_CASH );
    DataSet* ds = layer1->addDataSet(DoubleArray(bh, (int)bhec.size()),
                                     dashLineColor(BH_COLOR, Chart::DotLine),
                                     LEGEND_TEXT_BH);

    AreaLayer* layer = addAreaLayer();
    layer->addDataSet(DoubleArray(total, (int)ec.getSize()), TOTAL_COLOR,
                      LEGEND_TEXT_EQUITY);
    layer->setLineWidth(1);

    AreaLayer* layerCash = addAreaLayer();
    layerCash->addDataSet(DoubleArray(cash, (int)ec.getSize()), CASH_COLOR,
                          LEGEND_TEXT_CASH);
    layerCash->setLineWidth(1);

    //    layer1->addDataSet(DoubleArray(cash, ec.size() ), 0x00ff00, "Cash");

    xAxis()
        ->setLabels(StringArray(labels, (int)ec.getSize()))
        ->setFontAngle(X_LABEL_ANGLE);

    xAxis()->setLabelOffset(-5);
    xAxis()->setLabelStep((int)(ec.getSize() / X_LABEL_COUNT));
    xAxis()->setLabelGap(X_LABEL_GAP);
    xAxis()->setLabelStep(2, 1);
    yAxis()->setTitle("Equity value");
  }
};

class Chart2 : public XYChart {
 public:
  Chart2(const EquityCurve& em, const double* total, const double* longs,
         const double* shorts, const double* bh, const XLabels& labels,
         int bkcolor, const char* ytext, bool pct)
      : XYChart(CHART_WIDTH, CHART2_HEIGHT, Chart::Transparent,
                Chart::Transparent, 0) {
    //***********************************

    //    std::cout<< "a" << std::endl;
    setPlotArea(LEFT_SPACE, CHART2_TOP_SPACE, CHART_PLOT_X, CHART2_PLOT_Y,
                bkcolor, -1, -1, 0xcccccc, 0xcccccc);

    // Set the labels on the x axis.
    //    std::cout<< "b" << std::endl;
    xAxis()
        ->setLabels(StringArray(labels, (int)em.getSize()))
        ->setFontAngle(X_LABEL_ANGLE);

    //    std::cout<< "c" << std::endl;
    xAxis()->setLabelOffset(-5);
    xAxis()->setLabelStep((int)(em.getSize() / X_LABEL_COUNT));
    xAxis()->setLabelGap(X_LABEL_GAP);
    xAxis()->setLabelStep(2, 1);
    yAxis()->addZone(CHART2_PLOT_Y - 10, CHART2_PLOT_Y, 0x99ff99);
    yAxis()->setTitle(ytext);
    if (pct)
      yAxis()->setLabelFormat("{value}%");
    else
      yAxis()->setLabelFormat("{value}");

    //    std::cout<< "d" << std::endl;

    LineLayer* layer1 = addLineLayer();
    // Set the default line width to 2 pixels

    // Add the three data sets to the line layer. For demo purpose, we use a
    // dash line color for the last line

    if (longs != 0)
      layer1->addDataSet(DoubleArray(longs, (int)em.size()), LONG_COLOR,
                         LEGEND_LONG_DD);
    //    std::cout<< "e" << std::endl;

    if (shorts != 0)
      layer1->addDataSet(DoubleArray(shorts, (int)em.size()), SHORT_COLOR,
                         LEGEND_SHORT_DD);
    //    std::cout<< "f" << std::endl;

    if (bh != 0)
      layer1->addDataSet(DoubleArray(bh, (int)em.size()),
                         dashLineColor(BH_COLOR, Chart::DotLine), LEGEND_BH_DD);

    AreaLayer* layer = addAreaLayer();
    //    layer->setLineWidth( 0 );

    if (total != 0)
      layer->addDataSet(DoubleArray(total, (int)em.size()), TOTAL_COLOR,
                        LEGEND_TOTAL_DD);

    LegendBox* legendBox = addLegend(80, 70, true, "", 8);
    legendBox->setReverse();
    legendBox->setBackground(Chart::Transparent, Chart::Transparent);
    TextBox* tb = addText(COPYRIGHT_X, CHART2_PLOT_Y - 20, COPYRIGHT_TEXT);
    //    std::cout<< "g" << std::endl;
  }
};

class Chart21 : public Chart2 {
 public:
  Chart21(const EquityCurve& em, const double* total, const double* longs,
          const double* shorts, const double* bh, const XLabels& labels)
      : Chart2(em, total, longs, shorts, bh, labels, CHART21_BK_COLOR,
               "Drawdown pct", true) {}
};

class Chart22 : public Chart2 {
 public:
  Chart22(const EquityCurve& em, const double* total, const double* longs,
          const double* shorts, const double* bh, const XLabels& labels)
      : Chart2(em, total, longs, shorts, bh, labels, CHART22_BK_COLOR,
               "Drawdown value", false) {}
};

class Chart3 : public XYChart {
 public:
  Chart3(const EquityCurve& em, const DrawdownCurve& totalDC,
         const DrawdownCurve& longDC, const DrawdownCurve& shortDC,
         const DrawdownCurve& bhDC, const XLabels& labels)
      : XYChart(CHART_WIDTH, CHART3_HEIGHT, Chart::Transparent,
                Chart::Transparent, 0) {
    setPlotArea(LEFT_SPACE, CHART3_TOP_SPACE, CHART_PLOT_X, CHART3_PLOT_Y,
                CHART3_BK_COLOR, -1, -1, 0xcccccc, 0xcccccc);

    LineLayer* layer1 = addLineLayer();
    // Set the default line width to 2 pixels

    // Add the three data sets to the line layer. For demo purpose, we use a
    // dash line color for the last line

    if (longDC.getBarsArray() != 0)
      layer1->addDataSet(DoubleArray(longDC.getBarsArray(), (int)em.size()),
                         LONG_COLOR, LEGEND_LONG_DD_DURATION);
    if (shortDC.getBarsArray() != 0)
      layer1->addDataSet(DoubleArray(shortDC.getBarsArray(), (int)em.size()),
                         SHORT_COLOR, LEGEND_SHORT_DD_DURATION);
    if (bhDC.getBarsArray() != 0)
      layer1->addDataSet(DoubleArray(bhDC.getBarsArray(), (int)em.size()),
                         dashLineColor(BH_COLOR, Chart::DotLine),
                         LEGEND_BH_DD_DURATION);

    AreaLayer* layer = addAreaLayer();
    if (totalDC.getBarsArray() != 0)
      layer->addDataSet(DoubleArray(totalDC.getBarsArray(), (int)em.size()),
                        TOTAL_COLOR, LEGEND_TOTAL_DD_DURATION);
    //    layer->setLineWidth( 0 );
    yAxis()->setLabelFormat("{value}");
    yAxis()->setTitle("Drawdown duration (days)");

    // Set the labels on the x axis.
    xAxis()
        ->setLabels(StringArray(labels, (int)em.getSize()))
        ->setFontAngle(X_LABEL_ANGLE);

    xAxis()->setLabelOffset(-5);
    xAxis()->setLabelStep((int)(em.getSize() / X_LABEL_COUNT));
    xAxis()->setLabelGap(X_LABEL_GAP);
    xAxis()->setLabelStep(2, 1);

    LegendBox* legendBox = addLegend(80, 15, true, "", 8);
    legendBox->setReverse();
    legendBox->setBackground(Chart::Transparent, Chart::Transparent);

    TextBox* tb = addText(COPYRIGHT_X, CHART3_TOP_SPACE + 7, COPYRIGHT_TEXT);
  }
};

class AllChart : public MultiChart {
 public:
  AllChart(const EquityCurve& ec, const EquityCurve& bhec,
           const DrawdownCurve& totalDC, const DrawdownCurve& longDC,
           const DrawdownCurve& shortDC, const DrawdownCurve& bhDC,
           const XLabels& labels, const std::string& fileNameBase)
      : MultiChart(CHART_WIDTH, MULTICHART_HEIGHT) {
    Chart1 c1(ec, bhec, labels);
    Chart21 c21(ec, totalDC.getDDPercentArray(), longDC.getDDPercentArray(),
                shortDC.getDDPercentArray(), bhDC.getDDPercentArray(), labels);
    Chart22 c22(ec, totalDC.getDDArray(), longDC.getDDArray(),
                shortDC.getDDArray(), bhDC.getDDArray(), labels);
    Chart3 c3(ec, totalDC, longDC, shortDC, bhDC, labels);

    TextBox* title = addText(TITLE_X, TITLE1_POS_Y, TITLE_STYLE "Equity", 0, 10,
                             TITLE_TEXT_COLOR, Center);
    title->setBackground(TITLE_BACKGROUND_COLOR, 0);
    title->setSize(TITLE_WIDTH, TITLE_HEIGHT);

    title = addText(TITLE_X, TITLE2_POS_Y, TITLE_STYLE "Drawdown pct", 0, 10,
                    TITLE_TEXT_COLOR, Center);
    title->setBackground(TITLE_BACKGROUND_COLOR, 0);
    title->setSize(TITLE_WIDTH, TITLE_HEIGHT);

    title = addText(TITLE_X, TITLE22_POS_Y, TITLE_STYLE "Drawdown value", 0, 10,
                    TITLE_TEXT_COLOR, Center);
    title->setBackground(TITLE_BACKGROUND_COLOR, 0);
    title->setSize(TITLE_WIDTH, TITLE_HEIGHT);

    title = addText(TITLE_X, TITLE3_POS_Y, TITLE_STYLE "Drawdown duration", 0,
                    10, TITLE_TEXT_COLOR, Center);
    title->setBackground(TITLE_BACKGROUND_COLOR, 0);
    title->setSize(TITLE_WIDTH, TITLE_HEIGHT);

    addChart(0, CHART1_POS_Y, &c1);
    addChart(0, CHART2_POS_Y, &c21);
    addChart(0, CHART22_POS_Y, &c22);
    addChart(0, CHART3_POS_Y, &c3);

    // output the chart
    makeChart((fileNameBase + ".png").c_str());
  }
};

class PriceChart : public FinanceChart {
 public:
  PriceChart()
      : FinanceChart(600)

  {
    // Create a finance chart demo containing 100 days of data
    int noOfDays = 1000;

    // To compute moving averages starting from the first day, we need to get
    // extra data points before the first day
    int extraDays = 30;

    // In this exammple, we use a random number generator utility to simulate
    // the data. We set up the random table to create 6 cols x (noOfDays +
    // extraDays) rows, using 9 as the seed.
    RanTable* rantable = new RanTable(9, 6, noOfDays + extraDays);

    // Set the 1st col to be the timeStamp, starting from Sep 4, 2002, with each
    // row representing one day, and counting week days only (jump over Sat and
    // Sun)
    rantable->setDateCol(0, chartTime(2002, 9, 4), 86400, true);

    // Set the 2nd, 3rd, 4th and 5th columns to be high, low, open and close
    // data. The open value starts from 100, and the daily change is random from
    // -5 to 5.
    rantable->setHLOCCols(1, 100, -5, 5);

    // Set the 6th column as the vol data from 5 to 25 million
    rantable->setCol(5, 50000000, 250000000);

    // Now we read the data from the table into arrays
    DoubleArray timeStamps = rantable->getCol(0);
    DoubleArray highData = rantable->getCol(1);
    DoubleArray lowData = rantable->getCol(2);
    DoubleArray openData = rantable->getCol(3);
    DoubleArray closeData = rantable->getCol(4);
    DoubleArray volData = rantable->getCol(5);

    // Create a FinanceChart object of width 600 pixels

    // Add a title to the chart
    addTitle("Finance Chart Demonstration");

    // Set the data into the finance chart object
    setData(timeStamps, highData, lowData, openData, closeData, volData,
            extraDays);

    // Add the main chart with 210 pixels in height
    addMainChart(210);

    // Add a 5 period simple moving average to the main chart, using brown color
    addSimpleMovingAvg(5, 0x663300);

    // Add a 20 period simple moving average to the main chart, using purple
    // color
    addSimpleMovingAvg(20, 0x9900ff);

    // Add an HLOC symbols to the main chart, using green/red for up/down days
    addHLOC(0x008000, 0xcc0000);

    // Add 20 days bollinger band to the main chart, using light blue (9999ff)
    // as the border and semi-transparent blue (c06666ff) as the fill color
    addBollingerBand(20, 2, 0x9999ff, 0xC06666FF);

    // Add a 70 pixels volume bars sub-chart to the bottom of the main chart,
    // using green/red/grey for up/down/flat days
    addVolBars(70, 0x99ff99, 0xff9999, 0x808080);

    // Append a 14-days RSI indicator chart (70 pixels height) after the main
    // chart. The main RSI line is purple (800080). Set threshold region to +/-
    // 20 (that is, RSI = 50 +/- 25). The upper/lower threshold regions will be
    // filled with red (ff0000)/blue (0000ff).
    addRSI(70, 14, 0x800080, 20, 0xff0000, 0x0000ff);

    // Append a 12-days momentum indicator chart (70 pixels height) using blue
    // (0000ff) color.
    addMomentum(70, 12, 0x0000ff);

    // output the chart
    makeChart("c:\\finance.png");

    // free up resources
    delete rantable;
  }
};

class Cfileplugins1App : public CWinApp {
 public:
  Cfileplugins1App();

  // Overrides
 public:
  virtual BOOL InitInstance();

  DECLARE_MESSAGE_MAP()
};

class FileSignalHandlerException : public SignalHandlerException {
 public:
  FileSignalHandlerException(const std::string& fileName)
      : SignalHandlerException(
            __super::name(),
            std::string("Could not open signals file for writing \"" +
                        fileName + "\"")) {}
};

// the hierarchy is a little messed up:
// The stats handler is a signal handler, but only because there is
// no basic stats handler, so this is just to use the plug-in events
//
// Now we have the actual signal handler also part of this stat handler, so
// the file signal handler will be derived from statshandler to use the basic
// signal methods, and the file stats handler will be derived from the file
// signal handler
//
// they have in common then a few things:
// they are all plug-ins
// they require processing of the end event
// the stats plugin will do the position sizing for positions as well
// as for signal
class FileSignalsHandler : public StatsHandler {
 private:
  std::string _htmlFileName;
  std::string _csvFileName;
  // this is the file containing the description for multi-page presentation
  // of the html file
  std::string _descFileName;
  std::ofstream _htmlOfs;
  std::ofstream _csvOfs;
  std::ofstream _descOfs;
  mutable Mutex _mx;
  bool _empty;
  size_t _linesPerPage;
  unsigned int _disabledCount;
  // if true, it will generate output, no output otherwise
  bool _outputSignals;

  SignalVector _signals;

  void htmlHeader() {
    assert(_descOfs);
    //    _htmlOfs << "<table class=\"list_table\">" ) << std::endl;
    _descOfs << "header="
             << "<tr class=\"h\"><td colspan=\"2\" class=\"h\">Type</td> <td "
                "class=\"h\">Symbol</td> <td class=\"h\">Signal date</td> <td "
                "class=\"h\">Type</td> <td class=\"h\">Shares</td> <td "
                "class=\"h\">Price</td> <td class=\"h\">Signal Name</td> <td "
                "class=\"h\">System Name</td> </tr>"
             << std::endl;
  }

  void htmlFooter() {
    assert(_descOfs);
    _descOfs << "end=" << count() << "," << _htmlOfs.tellp() << std::endl;
  }

  void csvFooter() {}

  void csvHeader() {
    assert(_csvOfs);
    _csvOfs << Signal::csvHeaderLine() << std::endl;
  }

  void csvLine(SignalPtr signal) {
    assert(signal);
    assert(_csvOfs);
    _csvOfs << signal->toCSVString() << std::endl;
  }

  // this is the number of signals excluding the disabled ones
  size_t count() const { return _signals.size() - _disabledCount; }

  void htmlLine(SignalPtr signal, size_t signalsCount) {
    assert(_htmlOfs);
    assert(signal);

    if ((signalsCount % _linesPerPage) == 0)
      _descOfs << "line=" << signalsCount << "," << _htmlOfs.tellp()
               << std::endl;
    _htmlOfs << "<tr class=\"" << (signalsCount % 2 ? "d0\"" : "d1\"") << ">";
    _htmlOfs << "<td class=\"c" << (signal->isShort() ? " sh" : " lg")
             << "\"></td>";
    _htmlOfs << "<td class=\"c" << (signal->isEntryPosition() ? " en" : " ex")
             << "\"></td>";
    // wrapping the singal symbol with * * so the php code can replace it with a
    // link to the chart
    _htmlOfs << "\t<td style='font-weight:bold' class=\"c\">*"
             << signal->symbol() << "*</td>";
    _htmlOfs << "\t<td class=\"c\">" << signal->time().date().to_simple_string()
             << "</td>";
    _htmlOfs << "\t<td class=\"c\">"
             << signal->signalTypeAsString(signal->type()) << "</td>";
    _htmlOfs << "\t<td class=\"c\">" << signal->shares() << "</td>";
    _htmlOfs << "\t<td class=\"c\">" << std::fixed << std::setprecision(2)
             << signal->price() << "</td>";
    _htmlOfs << "\t<td class=\"c\">" << signal->name() << "</td>";
    _htmlOfs << "\t<td class=\"c\">" << *signal->systemName() << "</td>";
    _htmlOfs << "</tr>" << std::endl;
  }

  void openFiles() {
    // we're here because the file names are not empty
    assert(!_htmlFileName.empty() && !_csvFileName.empty() &&
           !_descFileName.empty());

    _htmlOfs.open(_htmlFileName.c_str());

    assert(_htmlOfs);

    _csvOfs.open(_csvFileName.c_str());
    assert(_csvOfs);

    _descOfs.open(_descFileName.c_str());
    assert(_descOfs);
  }

  void commit() {
    if (_outputSignals) {
      openFiles();

      htmlHeader();
      csvHeader();

      unsigned int n = 0;
      for (SignalVector::size_type ix = 0; ix < _signals.size(); ++ix) {
        SignalPtr signal(_signals[ix]);
        if (signal->isEnabled()) {
          assert(signal);
          assert(signal->shares() > 0);
          htmlLine(signal, n++);
          csvLine(signal);
        }
      }

      htmlFooter();
      csvFooter();
    }
  }

  /**
   * Method when a signal of any type is triggered by a trading system
   * associated with this signal handler
   *
   * The default implementation ignores the signal
   *
   * @param _signal Reference to a Signal object, which contains the information
   * about the triggered signal
   */
  virtual void signal(SignalPtr _signal) throw(SignalHandlerException) {
    Lock lock(_mx);

    assert(_signal);

    _signals.push_back(_signal);
  }

 protected:
  FileSignalsHandler(const Info& info, const std::vector<std::string>& strings)
      : StatsHandler(info), _disabledCount(0) {
    // html signals
    _htmlFileName = strings[3];
    // csv signals
    _csvFileName = strings[4];
    // signal description (?)
    _descFileName = strings[5];
    // signals per html page
    istringstream lpp(strings[6]);
    lpp >> _linesPerPage;

    // either all files are set, or none of them
    assert(_htmlFileName.empty() && _csvFileName.empty() &&
               _descFileName.empty() ||
           !_htmlFileName.empty() && !_csvFileName.empty() &&
               !_descFileName.empty());

    // we can test just one, because all the other files will be the same
    _outputSignals = !_htmlFileName.empty();
  }

  SignalVector& getSignals() { return _signals; }

  void sessionEnd() {
    signalsSizing();
    commit();
  }

 private:
  void signalSizing(SignalPtr signal) {
    assert(signal);
    unsigned int newShares;
    if (signal->isEntryPosition()) {
      if (signal->applySignalSizing()) {
        const EquityCurve& ec = equityCurve();
        const Equity* eq = ec.getEquity(signal->time().date());

        // not sure yet if that's true, we'll see at runtime...
        assert(eq != 0);

        const PositionSizingParams* ps =
            sessionInfo().runtimeParams()->positionSizing();
        assert(ps != 0);

        const BarsPtr data = sessionInfo().getData(signal->symbol());
        // we got a signal, so there must be data
        assert(data);
        assert(data->size() > 0);

        Bars bars(dynamic_cast<const BarsAbstr*>(data.get()));

        // the signal should be on the last available bar
        assert(signal->time() == bars.time(data->size() - 1));

        // if the signal has price, use that for sizing, otherwise, use the
        // current close
        double price =
            signal->hasPrice() ? signal->price() : bars.close(data->size() - 1);
        double volume = bars.volume(data->size() - 1);
        assert(price > 0);

        // for signals, we don't care about the max number of positions - it
        // will be sorted out when the orders are filled

        // now adjust the signal size according to pos sizing params

        switch (ps->posSizeType()) {
          case PositionSizingParams::PosSizeType::system:
            // no adjustment, using the position size set by the system
            newShares = signal->shares();
            break;
          case PositionSizingParams::PosSizeType::shares:
            // setting the new number of shares
            newShares = (unsigned int)ps->posSize();
            break;
          case PositionSizingParams::PosSizeType::size:
            newShares = (unsigned int)round(ps->posSize() / price);
            break;
          case PositionSizingParams::PosSizeType::pctEquity:
            newShares = (unsigned int)round(
                (eq->getAll().getTotal() * ps->posSize() / 100) / price);
            break;
          case PositionSizingParams::PosSizeType::pctCash:
            newShares = (unsigned int)round(
                (eq->getAll().getCash() * ps->posSize() / 100) / price);
            break;
          default:
            assert(false);
        }

        switch (ps->posSizeLimitType()) {
          case PositionSizingParams::PosSizeLimitType::none:
            break;
          case PositionSizingParams::PosSizeLimitType::pctVolume:
            newShares = min(newShares, (unsigned int)round(ps->posSizeLimit() /
                                                           100 * volume));
            break;
          case PositionSizingParams::PosSizeLimitType::limit:
            if (price * newShares > ps->posSizeLimit())
              newShares = (size_t)(ps->posSizeLimit() / price);
            break;
          default:
            assert(false);
            break;
        }

        signal->setShares(newShares);
      }
    } else {
      // all exit signal are for the same number of shares as the position they
      // are supposed to exit unless the position is disabled
      Position pos = signal->position();
      if (pos.isEnabled()) {
        newShares = signal->position().getShares();
        signal->setShares(newShares);
      } else {
        signal->disable();
        ++_disabledCount;
      }
    }
  }

  void signalsSizing() {
    for (SignalVector::size_type ix = 0; ix < _signals.size(); ++ix) {
      SignalPtr signal = _signals[ix];
      signalSizing(signal);
    }
  }
};

class FileStatsHandler : public FileSignalsHandler {
 private:
  std::string _statsCSV;
  std::string _statsHTML;
  std::string _eqCurveBase;

 public:
  FileStatsHandler(const std::vector<std::string>& strings)
      : FileSignalsHandler(statsInfo, strings) {
    // stats csv
    _statsCSV = strings[0];
    // stats htm
    _statsHTML = strings[1];
    // eqcurve
    _eqCurveBase = strings[2];
  }

  ~FileStatsHandler() {}

  void toHTML() {
    LOG(log_debug, "1");
    std::ofstream ofs(_statsHTML.c_str());
    LOG(log_debug, "2");

    if (ofs) toFormat(StatsToHTML(ofs));
  }

  void toCSV() {
    std::ofstream ofs(_statsCSV.c_str());
    if (ofs) toFormat(StatsToCSV(ofs));
  }

  void eqCurveToChart() {
    /*
    Set ChartDirector license code here
    */
    //    Chart::setLicenseCode("XXXX-XXXX-XXXX-XXXX-XXXX-XXXX");

    const EquityCurve& ec(__super::equityCurve());

    if (ec.size() > 0) {
      const EquityCurve& bhec(__super::bhEquityCurve());

      const DrawdownCurve& totalDC(__super::totalDrawdownCurve());
      const DrawdownCurve& longDC(__super::longDrawdownCurve());
      const DrawdownCurve& shortDC(__super::shortDrawdownCurve());
      const DrawdownCurve& bhDC(__super::bhDrawdownCurve());

      XLabels labels(ec);

      AllChart ac(ec, bhec, totalDC, longDC, shortDC, bhDC, labels,
                  _eqCurveBase);
    }
  }

  void eqCurveToCSV() const {
    const EquityCurve& ec(__super::equityCurve());
    const EquityCurve& bhec(__super::bhEquityCurve());

    const DrawdownCurve& totalDC(__super::totalDrawdownCurve());
    const DrawdownCurve& longDC(__super::longDrawdownCurve());
    const DrawdownCurve& shortDC(__super::shortDrawdownCurve());
    const DrawdownCurve& bhDC(__super::bhDrawdownCurve());

    XLabels labels(ec);

    if (ec.getSize() > 0) {
      std::ofstream os((_eqCurveBase + ".csv").c_str());

      os << "Date,Total,Long,Short,Cash,Buy & Hold,Total dd,Long dd,Short "
            "dd,Buy & Hold dd,Total dd pct,Long dd pct,Short dd pct,B&H dd "
            "pct,Total dd days,Long dd days,Short dd days,B&H dd days"
         << std::endl;

      const double* total = ec.getTotal();
      const double* sh = ec.getShort();
      const double* lg = ec.getLong();
      const double* cash = ec.getCash();
      const double* bh = bhec.getTotal();

      for (size_t i = 0; i < ec.getSize(); i++) {
        os << labels[i] << "," << (total != 0 ? total[i] : 0) << ","
           << (lg != 0 ? lg[i] : 0) << "," << (sh != 0 ? sh[i] : 0) << ","
           << (cash != 0 ? cash[i] : 0) << "," << (bh != 0 ? bh[i] : 0) << ","
           <<

            (totalDC.getDDArray() != 0 ? totalDC.getDDArray()[i] : 0) << ","
           << (longDC.getDDArray() != 0 ? longDC.getDDArray()[i] : 0) << ","
           << (shortDC.getDDArray() != 0 ? shortDC.getDDArray()[i] : 0) << ","
           << (bhDC.getDDArray() != 0 ? bhDC.getDDArray()[i] : 0) << "," <<

            (totalDC.getDDPercentArray() != 0 ? totalDC.getDDPercentArray()[i]
                                              : 0)
           << "%,"
           << (longDC.getDDPercentArray() != 0 ? longDC.getDDPercentArray()[i]
                                               : 0)
           << "%,"
           << (shortDC.getDDPercentArray() != 0 ? shortDC.getDDPercentArray()[i]
                                                : 0)
           << "%,"
           << (bhDC.getDDPercentArray() != 0 ? bhDC.getDDPercentArray()[i] : 0)
           << "%," <<

            (totalDC.getBarsArray() != 0 ? totalDC.getBarsArray()[i] : 0) << ","
           << (longDC.getBarsArray() != 0 ? longDC.getBarsArray()[i] : 0) << ","
           << (shortDC.getBarsArray() != 0 ? shortDC.getBarsArray()[i] : 0)
           << "," << (bhDC.getBarsArray() != 0 ? bhDC.getBarsArray()[i] : 0)
           << "," << std::endl;
      }
    }
  }

  void eqCurveToHTML() {}
  void toFormat(const StatsToFormat& format) const;

  void sessionEnded(PositionsContainer& positions) {
    LOG(log_debug, "1");

    RuntimeStats& rts = *sessionInfo().runtimeStats();
    std::cout << "Calculating equity date range" << std::endl;
    rts.setMessage("Calculating equity date range");
    rts.setStatus(RuntimeStatus::RUNNING);
    EquityDateRange edr(__super::sessionInfo());
    rts.step(getDateRangeStep());

    if (sessionInfo().runtimeParams()->statsEnabled() ||
        sessionInfo().runtimeParams()->equityCurveEnabled() ||
        sessionInfo().runtimeParams()->tradesEnabled())

    // the eq curve calculation is necessary if we have to calc stats or trades
    // (we have to apply pos sizing), so we have to do that if the stats are
    // enabled
    {
      std::cout << "Calculating equity curve" << std::endl;
      __super::calcEqCurve(edr, positions);
    }

    // only do transformations if the eq curve is enabled
    if (!_eqCurveBase.empty()) {
      LOG(log_debug, "before Eq curve to chart");
      eqCurveToChart();
      LOG(log_debug, "before Eq curve to CSV");
      eqCurveToCSV();
      LOG(log_debug, "before Eq curve to html");
      eqCurveToHTML();
      LOG(log_debug, "Eq curve ended");
      //			PriceChart pc;
    }

    if (sessionInfo().runtimeParams()->statsEnabled()) {
      LOG(log_debug, "Calculating stats");
      __super::calcStats(edr, positions);
      __super::calcScore();
    }

    if (!_statsHTML.empty() || !_statsCSV.empty()) {
      // only calculate stats if necessary

      LOG(log_debug, "saving stats as html");
      toHTML();
      LOG(log_debug, "saving stats as csv");
      toCSV();
      LOG(log_debug, "done with stats");
    }

    __super::sessionEnd();
  }
};

class FileStatsPlugin : public SignalHandlerPlugin {
 public:
  FileStatsPlugin()
      : SignalHandlerPlugin(
            Info("5B776120-3459-4527-8722-8530B4EDBBD8", "", "")) {}

  virtual InfoPtr first() const throw(PluginException) {
    return InfoPtr(new Info(statsInfo));
  }

  virtual InfoPtr next() const throw(PluginException) { return InfoPtr(); }

  virtual ManagedPtr<SignalHandler> get(
      const UniqueId& id, const std::vector<std::string>* createStrings =
                              0) throw(PluginException) {
    LOG(log_info, "getting the signal handler " << id.toString());

    if (createStrings == 0)
      throw PluginException(
          "File stats plugin",
          "The createStrings parameter must not be null for this plugin");

    if (id == statsInfo.id()) {
      LOG(log_info, "making the signal handler "
                        << id.toString()
                        << ", create string: " << (*createStrings)[0]);
      return new FileStatsHandler(*createStrings);
    } else
      return 0;
  }

  virtual bool canCreate() const { return false; }

  virtual ManagedPtr<SignalHandler> create(
      const std::vector<std::string>* createStrings =
          0) throw(PluginException) {
    return 0;
  }

  virtual bool canEdit(const UniqueId& id) const { return false; }

  virtual ManagedPtr<SignalHandler> edit(const UniqueId& id) throw(
      PluginException) {
    return 0;
  }

  virtual bool canRemove(const UniqueId& id) const { return false; }

  virtual void remove(const UniqueId& id) throw(PluginException) {}

  virtual bool hasWindow(const UniqueId& id) const { return false; }
};

class FileDataSourceFormat1ForWeb : public FileDataSourceFormat1 {
 private:
  // this is to show a "sanitized" error message in a web environment
  virtual void fileNotFoundErrorHandler(const std::string& symbol,
                                        const std::string& fileName) const {
    std::ostringstream o;

    o << "No data for symbol \"" << symbol << "\"";
    throw DataSourceException(OPENING_BARS_FILE_ERROR, o.str(), name());
  }

 public:
  FileDataSourceFormat1ForWeb(const std::string& createString, bool flatData,
                              ErrorHandlingMode errorHandlingMode)
      : FileDataSourceFormat1(dataSourceInfoFormat1NewId, createString, "csv",
                              flatData, errorHandlingMode) {}
};

class FileDataSourceFormat3ForWeb : public FileDataSourceFormat3 {
 private:
  // this is to show a "sanitized" error message in a web environment
  virtual void fileNotFoundErrorHandler(const std::string& symbol,
                                        const std::string& fileName) const {
    std::ostringstream o;

    o << "No data for symbol \"" << symbol << "\"";
    throw DataSourceException(OPENING_BARS_FILE_ERROR, o.str(), name());
  }

 public:
  FileDataSourceFormat3ForWeb(const std::string& createString, bool flatData,
                              ErrorHandlingMode errorHandlingMode)
      : FileDataSourceFormat3(dataSourceInfoFormat3NewId, createString, "csv",
                              flatData, errorHandlingMode) {}
};

class FileDataSourcePlugin : public DataSourcePlugin {
 private:
  std::vector<ManagedPtr<Info> > _configs;
  mutable std::vector<ManagedPtr<Info> >::const_iterator _i;

 public:
  FileDataSourcePlugin()
      : DataSourcePlugin(Info("C44EB64E-42A6-48ed-8C6C-3604C5B468DA", "", "")) {
    _configs.push_back(new Info(dataSourceInfoFormat1));
    _configs.push_back(new Info(dataSourceInfoFormat3));
  }

  virtual InfoPtr first() const throw(PluginException) {
    _i = _configs.begin();
    return InfoPtr(new Info(**_i));
  }

  virtual InfoPtr next() const throw(PluginException) {
    ++_i;
    return _i == _configs.end() ? InfoPtr() : InfoPtr(new Info(**_i));
  }

  virtual ManagedPtr<DataSource> get(
      const UniqueId& id, const std::vector<std::string>* createStrings =
                              0) throw(PluginException) {
    // TODO: make sure createString != 0, if it is throw exception
    if (createStrings == 0)
      throw PluginException(
          "File data source plugin 1",
          "The createStrings parameter must not be null for this plugin");

    ErrorHandlingMode mode(fatal);

    if ((*createStrings).size() > 1) {
      // second element of vector is the error handling mode
      if ((*createStrings)[1] == ERROR_HANDLING_MODE_FATAL)
        mode = fatal;
      else if ((*createStrings)[1] == ERROR_HANDLING_MODE_WARNING)
        mode = warning;
      else if ((*createStrings)[1] == ERROR_HANDLING_MODE_IGNORE)
        mode = ErrorHandlingMode::ignore;
      else
        throw PluginException("File data source plugin 1",
                              "Unknown error hanlding mode string");
    }

    // passing false, indicating that the data files are in various
    // subdirectories, not all in one dir (flat)
    if (id == dataSourceInfoFormat1.id())
      return new FileDataSourceFormat1ForWeb((*createStrings)[0], false, mode);
    else if (id == dataSourceInfoFormat3.id())
      return new FileDataSourceFormat3ForWeb((*createStrings)[0], false, mode);
    else
      return 0;
  }

  virtual bool canCreate() const { return false; }

  virtual ManagedPtr<DataSource> create(
      const std::vector<std::string>* createStrings =
          0) throw(PluginException) {
    return 0;
  }

  virtual bool canEdit(const UniqueId& id) const { return false; }

  virtual ManagedPtr<DataSource> edit(const UniqueId& id) throw(
      PluginException) {
    return 0;
  }

  virtual bool canRemove(const UniqueId& id) const { return false; }

  virtual void remove(const UniqueId& id) throw(PluginException) {}

  virtual bool hasWindow(const UniqueId& id) const { return false; }
};

class FileSymbolsSourcePlugin : public SymbolsSourcePlugin {
 public:
  FileSymbolsSourcePlugin()
      : SymbolsSourcePlugin(
            Info("6267C82E-EF65-4b89-AC47-2B25CAEE70B0", "", "")) {}

  virtual InfoPtr first() const throw(PluginException) {
    return InfoPtr(new Info(symbolsSourceInfo));
  }

  virtual InfoPtr next() const throw(PluginException) { return InfoPtr(); }

  virtual ManagedPtr<SymbolsSource> get(
      const UniqueId& id, const std::vector<std::string>* createStrings =
                              0) throw(PluginException) {
    if (createStrings == 0)
      throw PluginException(
          "File symbols source plugin 1",
          "The createStrings parameter must not be null for this plugin");

    // TODO: make sure createString != 0, if it is throw exception
    if (id == symbolsSourceInfo.id())
      return new FileSymbolsSource(symbolsSourceInfo, (*createStrings)[0]);
    else
      return 0;
  }

  virtual bool canCreate() const { return false; }

  virtual ManagedPtr<SymbolsSource> create(
      const std::vector<std::string>* createStrings =
          0) throw(PluginException) {
    return 0;
  }

  virtual bool canEdit(const UniqueId& id) const { return false; }

  virtual ManagedPtr<SymbolsSource> edit(const UniqueId& id) throw(
      PluginException) {
    return 0;
  }

  virtual bool canRemove(const UniqueId& id) const { return false; }

  virtual void remove(const UniqueId& id) throw(PluginException) {}

  virtual bool hasWindow(const UniqueId& id) const { return false; }
};
