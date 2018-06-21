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

#include "runsystemcontext.h"
#include "ConfigurationData.h"

namespace po = boost::program_options;

class ConfigurationException : public exception {
 private:
  bool _error;

 public:
  ConfigurationException(const std::string& message, bool error)
      : exception(message.c_str()), _error(error) {}

  bool isError() const { return _error; }
};
// cmd line:
// simapp -dpath:D:\dev\sim\test\data\quotes -sympath: -sym:symbols.txt th:5

class Configuration : public RunSystemContext, private ConfigurationData {
 private:
  void init(const std::string& cmdLine, bool validate);

 private:
  void help(const po::options_description& desc) throw(ConfigurationException) {
    std::ostringstream o;

    o << "Cmd line: " << std::endl
      << "******" << std::endl
      << _inputCommandLine << "******" << std::endl
      << std::endl;
    throw ConfigurationException(o.str(), false);
  }

  void removeModuleName();

 public:
  Configuration(bool validate = true);
  Configuration(const std::string& cmdLine, bool validate = true);
  Configuration(const char* cmdLine, bool validate = true);
  Configuration(const Configuration& config) : ConfigurationData(config) {}

  const PositionSizingParams& positionSizingParams() const {
    return _posSizingParams;
  }

  const UniqueIdVector& getRunnables() const { return _runnables; }
  const std::string& getPluginPath() const { return _pluginPath; }
  const std::string& getPluginExt() const { return _pluginExt; }
  bool overridePluginPath() const { return _overridePluginPath; }
  const std::string& applicationPath() const { return _applicationPath; }
  const vector<std::string>& includePaths() const { return _includePaths; }
  const vector<std::string>& libPath() const { return _libPath; }
  const std::string& outputPath() const { return _outputPath; }
  const std::string& projectPath() const { return _projectPath; }
  const std::string& toolsPath() const { return _toolsPath; }
  const std::string& symbolsSource() const { return _symbolsSource; }
  const std::string& dataSource() const { return _dataSource; }
  const std::string& symbolsSourceFile() const { return _symbolsSourceFile; }
  const std::string& dataSourcePath() const { return _dataSourcePath; }
  bool debug() const { return _debug; }
  const std::string& signalsFile() const { return _signalsFile; }
  const std::string& tradesFile() const { return _tradesFile; }
  const std::string& statsFile() const { return _statsFile; }
  const std::string& outputFile() const { return _outputFile; }
  bool runSimulator() const { return _runSimulator; }
  const std::string& errorsFile() const { return _errorsFile; }
  const std::string& fromDateTime() const { return _fromDateTime; }
  const std::string& toDateTime() const { return _toDateTime; }
  const std::string& statsHandler() const { return _statsHandler; }
  const std::string& statsCSVFile() const { return _statsCSVFile; }
  const std::string& signalsCSVFile() const { return _signalsCSVFile; }
  const std::string& tradesCSVFile() const { return _tradesCSVFile; }
  const std::string& equityCurveFile() const { return _equityCurveFile; }
  unsigned long timeout() const { return _timeout; }

  bool uniqueId() const { return _uniqueId; }
  bool installService() const { return _installService; }
  bool removeService() const { return _removeService; }
  bool debugService() const { return _debugService; }
  bool startService() const { return _startService; }
  const std::string& zipFile() const { return _zipFile; }
  bool hasZipFile() const { return !_zipFile.empty(); }

  const std::string& userName() const { return _userName; }
  const std::string& password() const { return _password; }
  const std::string& logFile() const { return _logFile; }
  bool hasLogFile() const { return !_logFile.empty(); }

  bool asyncRun() const { return _asyncRun; }
  bool hasEndRunSignalFile() const { return !_endRunSignalFile.empty(); }
  const std::string& endRunSignalFile() const { return _endRunSignalFile; }
  const std::string& heartBeatFile() const { return _heartBeatFile; }
  const std::string& reverseHeartBeatFile() const {
    return _reverseHeartBeatFile;
  }
  const std::string& cancelFile() const { return _cancelFile; }

  size_t symbolTimeout() const { return _symbolTimeout; }
  size_t reverseHeartBeatPeriod() const { return _reverseHeartBeatPeriod; }
  size_t heartBeatTimeout() const { return _heartBeatTimeout; }
  size_t cacheSize() const { return _cacheSize; }
  double defCommissionValue() const { return _defCommissionValue; }
  double defSlippageValue() const { return _defSlippageValue; }
  const std::string& defSlippageId() const { return _defSlippageId; }
  const std::string& defCommissionId() const { return _defCommissionId; }
  const std::string& runtimeStatsFile() const { return _runtimeStatsFile; }
  size_t maxLines() const { return _maxLines; }
  size_t maxTotalBarCount() const { return _maxTotalBarCount; }
  const std::string& sessionPath() const { return _sessionPath; }
  const std::string& symbolsToChartFile() const { return _symbolsToChartFile; }
  const std::string& chartDescriptionFile() const {
    return _chartDescriptionFile;
  }
  const std::string& chartRootPath() const { return _chartRootPath; }
  const std::string& osPath() const { return _osPath; }
  const std::string& getSystemNameKeyword() const { return _systemNameKeyword; }
  const std::string& getClassNameKeyword() const { return _classNameKeyword; }
  const std::string& getTradesDescriptionFile() const {
    return _tradesDescriptionFile;
  }
  const std::string& getSignalsDescriptionFile() const {
    return _signalsDescriptionFile;
  }
  size_t getLinesPerPage() const { return _linesPerPage; }
  unsigned long getCPUCount() const { return _cpuCount; }
  unsigned long getThreads() const { return _threads; }
  ThreadAlgorithm getThreadAlg() const { return _threadAlg; }
  virtual void setThreads(unsigned long threads) { _threads = threads; }
  virtual void setRunSimulator(bool run = true) { _runSimulator = true; }
  void setSessionPath(const std::string& sessionPath) {
    _sessionPath = sessionPath;
  }
  void addRunnableId(const UniqueId& id) { _runnables.push_back(id); }

  const std::string& externalTriggersFile() const { return _extTriggersFile; }
  const std::string& explicitTradesExt() const { return _explicitTradesExt; }
  ErrorHandlingMode dataErrorHandlingMode() const {
    return _dataErrorHandlingMode;
  }

  const std::string& startTradesDateTime() const {
    return _startTradesDateTime;
  }

  const std::string& envPath() const { return _envPath; }
  const std::string& envInclude() const { return _envInclude; }
  const std::string& envLib() const { return _envLib; }

  tradery::EnvironmentPtr getEnvironment() const;

  bool enableRunAsUser() const { return _enableRunAsUser; }

  unsigned int getThriftPort() const { return _thriftPort; }

  bool hasUserName() const { return !_userName.empty(); }
  bool hasPassword() const { return !_password.empty(); }
  bool hasRunnables() const { return !_runnables.empty(); }
  bool hasInclude() const { return !_includePaths.empty(); }
  bool hasLib() const { return !_libPath.empty(); }
  bool hasToolsPath() const { return !_toolsPath.empty(); }
  bool hasSymbolsSourceId() const { return !_symbolsSource.empty(); }
  bool hasSymbolsSourceFile() const { return !_symbolsSource.empty(); }
  bool hasDataSourcePath() const { return !_dataSourcePath.empty(); }
  bool hasDataSourceId() const { return !_dataSource.empty(); }
  bool hasProjectPath() const { return !_projectPath.empty(); }
  bool hasOutputPath() const { return !_outputPath.empty(); }
  bool hasOsPath() const { return !_osPath.empty(); }
  bool hasSessionPath() const { return !_sessionPath.empty(); }
  bool hasExplicitTradesExt() const { return !_explicitTradesExt.empty(); }
  bool hasStatsHandler() const { return !_statsHandler.empty(); }
  bool hasTradesFile() const { return !_tradesFile.empty(); }
  bool hasEquityCurveFile() const { return !_equityCurveFile.empty(); }
  bool hasStatsFile() const { return !_statsFile.empty(); }
  bool hasChartDescriptionFile() const {
    return !_chartDescriptionFile.empty();
  }

  void setTradesDescriptionFile(const std::string& tradesDescriptionFile) {
    _tradesDescriptionFile = tradesDescriptionFile;
  }

  void setTradesFile(const std::string& tradesFile) {
    _tradesFile = tradesFile;
  }

  void setErrorsFile(const std::string& errorsFile) {
    _errorsFile = errorsFile;
  }

  void setRuntimeStatsFile(const std::string& runtimeStatsFile) {
    _runtimeStatsFile = runtimeStatsFile;
  }

  void setTradesCSVFile(const std::string& tradesCSVFile) {
    _tradesCSVFile = tradesCSVFile;
  }

  void setSymbolsToChartFile(const std::string& symbolsToChartFile) {
    _symbolsToChartFile = symbolsToChartFile;
  }

  void setChartRootPath(const std::string& chartRootPath) {
    _chartRootPath = chartRootPath;
  }

  void setChartDescription(const std::string& chartDescriptionFile) {
    _chartDescriptionFile = chartDescriptionFile;
  }

  void setSignalsFile(const std::string& signalsFile) {
    _signalsFile = signalsFile;
  }
  void setSignalsCSVFile(const std::string& signalsCSVFile) {
    _signalsCSVFile = signalsCSVFile;
  }

  void setSignalsDescriptionFile(const std::string& signalsDescriptionFile) {
    _signalsDescriptionFile = signalsDescriptionFile;
  }

  void setStatsFile(const std::string& statsFile) { _statsFile = statsFile; }

  void setStatsCSVFile(const std::string& statsCSVFile) {
    _statsCSVFile = statsCSVFile;
  }

  void setOutputFile(const std::string& outputFile) {
    _outputFile = outputFile;
  }

  void setEquityCurveFile(const std::string& equityCurveFile) {
    _equityCurveFile = equityCurveFile;
  }

  void setSymbolsFileName(const std::string& symbolsFileName) {
    _symbolsSourceFile = symbolsFileName;
  }

  void setStartDate(const DateTime startDate) {
    _fromDateTime = startDate.date().toString(DateFormat::us);
  }

  void setEndDate(const DateTime endDate) {
    _toDateTime = endDate.date().toString(DateFormat::us);
  }

  void setDataErrorHandlingMode(
      tradery_thrift_api::DataErrorHandling::type dataErrorHandlingMode) {
    switch (dataErrorHandlingMode) {
      case tradery_thrift_api::DataErrorHandling::type::FATAL:
        _dataErrorHandlingMode = ErrorHandlingMode::fatal;
        break;
      case tradery_thrift_api::DataErrorHandling::type::WARNING:
        _dataErrorHandlingMode = ErrorHandlingMode::warning;
        break;
      case tradery_thrift_api::DataErrorHandling::type::NONE:
        _dataErrorHandlingMode = ErrorHandlingMode::ignore;
        break;
      default:
        throw ConfigurationException("Invalid data error handling value", true);
    }
  }

  void setSlippage(double slippage) { __super::_defSlippageValue = slippage; }

  void setCommission(double commission) {
    __super::_defCommissionValue = commission;
  }

  class PositionSizingParamsWrapper : public PositionSizingParamsImpl {
   public:
    PositionSizingParamsWrapper(
        const tradery_thrift_api::PositionSizing& positionSizing)
        : PositionSizingParamsImpl(
              positionSizing.initialCapital, positionSizing.maxOpenPositions,
              toPosSizeType(positionSizing.positionSizeType),
              positionSizing.positionSize,
              toPosSizeLimitType(positionSizing.positionSizeLimitType),
              positionSizing.positionSizeLimit) {}

   private:
    PosSizeLimitType toPosSizeLimitType(
        tradery_thrift_api::PositionSizeLimitType::type posSizeLimitType) {
      switch (posSizeLimitType) {
        case tradery_thrift_api::PositionSizeLimitType::type::NONE:
          return none;
        case tradery_thrift_api::PositionSizeLimitType::type::PCT_VOL:
          return pctVolume;
        case tradery_thrift_api::PositionSizeLimitType::type::VALUE:
          return limit;
        default:
          throw ConfigurationException("Invalid position size limit type",
                                       true);
      }
    }

    PosSizeType toPosSizeType(
        tradery_thrift_api::PositionSizeType::type posSizeType) {
      switch (posSizeType) {
        case tradery_thrift_api::PositionSizeType::type::SYSTEM:
          return system;
        case tradery_thrift_api::PositionSizeType::type::SHARES:
          return shares;
        case tradery_thrift_api::PositionSizeType::type::VALUE:
          return size;
        case tradery_thrift_api::PositionSizeType::type::PCT_EQUITY:
          return pctEquity;
        case tradery_thrift_api::PositionSizeType::type::PCT_CASH:
          return pctCash;
        default:
          throw ConfigurationException("Invalid position size type", true);
      }
    }
  };

  void setPositionSizingParams(
      const tradery_thrift_api::PositionSizing& positionSizing) {
    __super::_posSizingParams = PositionSizingParamsWrapper(positionSizing);
  }

  const std::string getCmdLineString() const {
    std::ostringstream cmdLine;

    for (size_t n = 0; n < _args.size(); n++) {
      bool b = _args[n][0] == TCHAR('-');
      cmdLine << (!b ? "\"" : "") << _args[n] << (!b ? "\"" : " ");
    }

    return cmdLine.str();
  }

  void set(const tradery_thrift_api::SessionParams& sessionContext);
  void validate() const;
};

typedef boost::shared_ptr<Configuration> ConfigurationPtr;
