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

#include "wchart.h"
#include <tokenizer.h>
#include <explicittrades.h>
#include "RunSystemContext.h"
#include "SessionContext.h"

// format:
/*

// comment line
# comment line

symbol, date,
buy/sell/short(sellshort,sell_short)/cover/sell_all(sellall)/cover_all(coverall)/exit_all(exitall),
market/limit/stop/close/, [shares],[price]


*/

class WebDocument : public DocumentBase {
 private:
  const std::string _name;
  const UniqueIdPtr _defDataSource;
  const UniqueIdPtr _defSymbolsSource;
  const UniqueIdPtr _defStatsHandler;
  const UniqueIdPtr _defSlippage;
  const UniqueIdPtr _defCommission;
  UniqueIdVector _runnables;
  mutable size_t _runnablesIterator;
  RuntimeParamsImpl _runtimeParams;
  std::vector<std::string> _symbolsSourceStrings;
  std::vector<std::string> _dataSourceStrings;
  std::vector<std::string> _statsHandlerStrings;
  std::vector<std::string> _slippageStrings;
  std::vector<std::string> _commissionStrings;
  typedef std::map<UniqueId, ExplicitTradesPtr> ExplicitTradesMap;
  ExplicitTradesMap _explicitTrades;

  const std::string _sessionPath;
  PluginTree _sessionPluginTree;

  ChartManagerPtr _chartManager;
  const UniqueId _sessionId;

 public:
  WebDocument(SessionContextPtr context) try
      : _runnablesIterator(0),
        _runnables(context->getSessionConfig()->getRunnables()),
        _sessionPath(context->getSessionConfig()->getSessionPath()),
        _defDataSource(new UniqueId(context->getConfig()->dataSource())),
        _defSymbolsSource(new UniqueId(context->getConfig()->symbolsSource())),
        _defStatsHandler(
            context->getConfig()->hasStatsHandler()
                ? new UniqueId(context->getConfig()->statsHandler())
                : 0),
        _defSlippage(context->getSessionParams()->slippage == 0
                         ? 0
                         : new UniqueId(context->getConfig()->defSlippageId())),
        _defCommission(
            context->getSessionParams()->commission == 0
                ? 0
                : new UniqueId(context->getConfig()->defCommissionId())),
        _chartManager(new WebChartManager(
            "", context->getSessionConfig()->getSymbolsToChartFile(),
            context->getSessionConfig()->getChartRootPath(),
            context->getSessionConfig()->getChartDescriptionFile(),
            context->getSessionConfig()->getRunnables().size() >
                1 /*multi system has reduced charts*/)),
        _sessionId(context->getSessionConfig()->getSessionId()) {
    try {
      _sessionPluginTree.explore(context->getSessionConfig()->getSessionPath(),
                                 context->getConfig()->getPluginExt(), false,
                                 0);
      LOG(log_debug, "Run system after explore");

      DateTime _from(context->getSessionParams()->range.startDate);
      DateTime _to(context->getSessionParams()->range.endDate);
      DateTime _startTradesDateTime(
          context->getSessionParams()->startTradesDate);
      _runtimeParams.setPositionSizingParams(
          context->getSessionParams()->positionSizing);
      _runtimeParams.setChartsEnabled(
          context->getSessionParams()->generateCharts);
      _runtimeParams.setStatsEnabled(
          context->getSessionParams()->generateStats);
      _runtimeParams.setEquityEnabled(
          context->getSessionParams()->generateEquityCurve);
      _runtimeParams.setTradesEnabled(
          context->getSessionParams()->generateTrades);
      _runtimeParams.setThreads(context->getConfig()->getThreads());
      _runtimeParams.setThreadAlgorithm(context->getConfig()->getThreadAlg());

      try {
        _runtimeParams.setRange(boost::make_shared<DateTimeRange>(_from, _to));
      } catch (const DateTimeRangeException& e) {
        std::string message;
        message
            << "Invalid date/time range - \"From\" must occur before \"To\": "
            << _from.to_simple_string() << " - " << _to.to_simple_string();
        LOG(log_error, message);
        throw DocumentException(message);
      }

      //      std::cout << "in WebDocument 3" << std::endl;
      //      COUT << std::endl <<
      //      "_runtimeParams.fromRange().to_simple_string() << " - " <<
      //      _runtimeParams.toRange().to_simple_string() << std::endl;

      _statsHandlerStrings.push_back(
          context->getSessionConfig()->getStatsCSVFile());
      _statsHandlerStrings.push_back(
          context->getSessionConfig()->getStatsFile());
      _statsHandlerStrings.push_back(
          context->getSessionConfig()->getEquityCurveFile());
      _statsHandlerStrings.push_back(
          context->getSessionConfig()->getSignalsFile());
      _statsHandlerStrings.push_back(
          context->getSessionConfig()->getSignalsCSVFile());
      _statsHandlerStrings.push_back(
          context->getSessionConfig()->getSignalsDescriptionFile());

      std::ostringstream os;
      os << context->getConfig()->getLinesPerPage();
      _statsHandlerStrings.push_back(os.str());

      _symbolsSourceStrings.push_back(
          context->getSessionConfig()->getSymbolsFileName());
      _dataSourceStrings.push_back(context->getConfig()->dataSourcePath());
      _dataSourceStrings.push_back(errorHandlingModeAsString(
          (tradery::ErrorHandlingMode)context->getSessionParams()
              ->dataErrorHandling));

      std::ostringstream o;
      o << context->getSessionParams()->slippage;
      _slippageStrings.push_back(o.str());
      o.str("");
      o << context->getSessionParams()->commission;
      _commissionStrings.push_back(o.str());

      // creating explicit trades
      // each runnable has its own explicit trades file
      // to allow for multi-system sessions with different explicit trades per
      // system the name of the file is implicit: the runnable id + the
      // extension
      for (UniqueIdVector::const_iterator i =
               context->getSessionConfig()->getRunnables().begin();
           i != context->getSessionConfig()->getRunnables().end(); ++i) {
        const UniqueId id = *i;

        std::string explicitTradesFile =
            _sessionPath + "\\" + id.toString() + "." +
            context->getConfig()->explicitTradesExt();

        if (fileExists(explicitTradesFile))
          _explicitTrades.insert(ExplicitTradesMap::value_type(
              id,
              ExplicitTradesPtr(new FileExplicitTrades(explicitTradesFile))));
      }
      LOG(log_info, "exiting constructor");
    } catch (const DateException& e) {
      LOG(log_error, "DateException: " << e.message());
      throw DocumentException(e.message());
    }
  } catch (const ExplicitTradesException& e) {
    LOG(log_error, "TriggersException");
    throw DocumentException(e.message());
  }

  virtual const ExplicitTrades* getExplicitTrades(const UniqueId& id) const {
    ExplicitTradesMap::const_iterator i = _explicitTrades.find(id);

    return i != _explicitTrades.end() ? i->second.get() : 0;
  }

  virtual const std::string name() { return _name; }

  virtual const UniqueId* getNextRunnableId() const {
    return _runnablesIterator < _runnables.size()
               ? &_runnables[_runnablesIterator++]
               : 0;
  }

  virtual bool hasDefaultCommission() const throw(DocumentException) {
    return _defCommission.get() != 0;
  }
  virtual const UniqueId* getDefaultDataSourceId() const
      throw(DocumentException) {
    return _defDataSource.get();
  }

  virtual const UniqueId* getDefaultCommissionId() const
      throw(DocumentException) {
    return _defCommission.get();
  }
  virtual bool hasDefaultDataSource() const throw(DocumentException) {
    return _defDataSource.get() != 0;
  }
  virtual bool hasDefaultSignalHandler() const throw(DocumentException) {
    return _defStatsHandler.get() != 0;
  }
  virtual const UniqueId* getDefaultSignalHandlerId() const
      throw(DocumentException) {
    return _defStatsHandler.get();
  }
  virtual RuntimeParamsImpl& getRuntimeParams() { return _runtimeParams; }
  virtual bool hasDefaultSlippage() const throw(DocumentException) {
    return _defSlippage.get() != 0;
  }
  virtual const UniqueId* getFirstRunnableId() const {
    _runnablesIterator = 0;
    return getNextRunnableId();
  }
  virtual const UniqueId* getDefaultSlippageId() const
      throw(DocumentException) {
    return _defSlippage.get();
  }
  virtual bool hasDefaultSymbolsSource() const throw(DocumentException) {
    return _defSymbolsSource.get() != 0;
  }
  virtual size_t runnablesCount() const { return _runnables.size(); }
  virtual const UniqueId* getDefaultSymbolsSourceId() const
      throw(DocumentException) {
    return _defSymbolsSource.get();
  }

  virtual const std::vector<std::string>* defaultSymbolsSourceStrings() const
      throw(DocumentException) {
    return &_symbolsSourceStrings;
  }

  virtual const std::vector<std::string>* defaultDataSourceStrings() const
      throw(DocumentException) {
    return &_dataSourceStrings;
  }

  virtual const std::vector<std::string>* defaultSignalHandlerStrings() const
      throw(DocumentException) {
    return &_statsHandlerStrings;
  }

  virtual const std::vector<std::string>* defaultSlippageStrings() const
      throw(DocumentException) {
    return &_slippageStrings;
  }

  virtual const std::vector<std::string>* defaultCommissionStrings() const
      throw(DocumentException) {
    return &_commissionStrings;
  }

  virtual const std::vector<std::string>* getRunnablesStrings() const
      throw(DocumentException) {
    return 0;
  }

  virtual const PositionSizingParams* positionSizingParams() const {
    return _runtimeParams.positionSizing();
  }

  virtual ChartManager* chartManager() {
    assert(_chartManager.get());
    return _chartManager.get();
  }

  virtual const PluginTree& getSessionPluginTree() const {
    return _sessionPluginTree;
  }
  virtual PluginTree& getSessionPluginTree() { return _sessionPluginTree; }
  virtual const UniqueId& getSessionId() const { return _sessionId; }
};