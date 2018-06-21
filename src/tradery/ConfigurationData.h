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

class ConfigurationData {
 protected:
  virtual ~ConfigurationData() {}
  std::string _inputCommandLine;
  UniqueIdVector _runnables;

  std::string _pluginPath;
  std::string _pluginExt;
  bool _overridePluginPath = false;
  bool _overridePluginExt;
  vector<std::string> _includePaths;
  vector<std::string> _libPath;
  std::string _outputPath;
  std::string _projectPath;
  std::string _toolsPath;
  std::string _symbolsSource;
  std::string _statsHandler;
  std::string _dataSource;
  std::string _symbolsSourceFile;
  std::string _dataSourcePath;
  bool _debug = false;
  std::string _signalsFile;
  std::string _tradesFile;
  std::string _statsFile;
  std::string _outputFile;

  std::string _tradesCSVFile;
  std::string _signalsCSVFile;
  std::string _statsCSVFile;
  // this is the base file name, various instances take
  // various file extensions: csv, htm, jpg
  std::string _equityCurveFile;

  std::string _userName;
  std::string _password;

  unsigned long _timeout;
  std::string _zipFile;

  bool _runSimulator = false;
  std::string _errorsFile;
  std::string _toDateTime;
  std::string _fromDateTime;
  std::string _startTradesDateTime;
  std::string _logFile;
  std::string _endRunSignalFile;
  bool _asyncRun;

  std::string _heartBeatFile;
  std::string _reverseHeartBeatFile;
  std::string _cancelFile;
  size_t _symbolTimeout;
  size_t _reverseHeartBeatPeriod;
  size_t _heartBeatTimeout;

  unsigned int _thriftPort;

  PositionSizingParamsImpl _posSizingParams;

  size_t _cacheSize;

  double _defSlippageValue;
  double _defCommissionValue;

  std::string _defSlippageId;
  std::string _defCommissionId;

  std::string _runtimeStatsFile;

  // all the args on the command line without the app name
  std::vector<std::string> _args;

  std::string _applicationPath;

  bool _uniqueId = false;
  bool _installService = false;
  bool _removeService = false;
  bool _debugService = false;
  bool _startService = false;
  size_t _maxLines;
  size_t _maxTotalBarCount;
  std::string _sessionPath;
  std::string _symbolsToChartFile;
  std::string _chartDescriptionFile;
  std::string _chartRootPath;

  std::string _extTriggersFile;
  std::string _systemNameKeyword;
  std::string _classNameKeyword;
  std::string _osPath;
  std::string _tradesDescriptionFile;
  std::string _signalsDescriptionFile;
  size_t _linesPerPage;
  unsigned long _cpuCount;
  unsigned long _threads;
  ThreadAlgorithm _threadAlg;
  std::string _explicitTradesExt;
  ErrorHandlingMode _dataErrorHandlingMode;

  std::string _envPath;
  std::string _envInclude;
  std::string _envLib;
  bool _enableRunAsUser;
};
