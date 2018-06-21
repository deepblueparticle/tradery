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

class RunSystemContext {
 public:
  virtual ~RunSystemContext() {}

  virtual const UniqueIdVector& getRunnables() const = 0;
  virtual const std::string& getPluginPath() const = 0;
  virtual const std::string& getPluginExt() const = 0;
  virtual bool overridePluginPath() const = 0;
  virtual const std::string& applicationPath() const = 0;
  virtual const vector<std::string>& includePaths() const = 0;
  virtual const vector<std::string>& libPath() const = 0;
  virtual const std::string& outputPath() const = 0;
  virtual const std::string& projectPath() const = 0;
  virtual const std::string& toolsPath() const = 0;
  virtual const std::string& symbolsSource() const = 0;
  virtual const std::string& dataSource() const = 0;
  virtual const std::string& symbolsSourceFile() const = 0;
  virtual const std::string& dataSourcePath() const = 0;
  virtual bool debug() const = 0;
  virtual const std::string& signalsFile() const = 0;
  virtual const std::string& tradesFile() const = 0;
  virtual const std::string& statsFile() const = 0;
  virtual const std::string& outputFile() const = 0;
  virtual bool runSimulator() const = 0;
  virtual const std::string& errorsFile() const = 0;
  virtual const std::string& fromDateTime() const = 0;
  virtual const std::string& toDateTime() const = 0;
  virtual const std::string& statsHandler() const = 0;
  virtual const std::string& statsCSVFile() const = 0;
  virtual const std::string& signalsCSVFile() const = 0;
  virtual const std::string& tradesCSVFile() const = 0;
  virtual const std::string& equityCurveFile() const = 0;
  virtual unsigned long timeout() const = 0;
  virtual const std::string& zipFile() const = 0;
  virtual bool hasZipFile() const = 0;
  virtual size_t symbolTimeout() const = 0;
  virtual const PositionSizingParams& positionSizingParams() const = 0;
  virtual const std::string& runtimeStatsFile() const = 0;
  virtual size_t cacheSize() const = 0;
  virtual double defSlippageValue() const = 0;
  virtual double defCommissionValue() const = 0;
  virtual const std::string& defCommissionId() const = 0;
  virtual const std::string& defSlippageId() const = 0;
  virtual size_t maxLines() const = 0;
  virtual size_t maxTotalBarCount() const = 0;
  virtual bool debugService() const = 0;
  virtual const std::string& sessionPath() const = 0;
  virtual const std::string& symbolsToChartFile() const = 0;
  virtual const std::string& chartRootPath() const = 0;
  virtual const std::string& chartDescriptionFile() const = 0;
  virtual const std::string& externalTriggersFile() const = 0;
  virtual const std::string& heartBeatFile() const = 0;
  virtual size_t heartBeatTimeout() const = 0;
  virtual const std::string& cancelFile() const = 0;
  virtual const std::string& reverseHeartBeatFile() const = 0;
  virtual size_t reverseHeartBeatPeriod() const = 0;
  virtual const std::string& osPath() const = 0;
  virtual const std::string& getSystemNameKeyword() const = 0;
  virtual const std::string& getClassNameKeyword() const = 0;
  virtual const std::string& getTradesDescriptionFile() const = 0;
  virtual const std::string& getSignalsDescriptionFile() const = 0;
  virtual size_t getLinesPerPage() const = 0;
  virtual unsigned long getCPUCount() const = 0;
  virtual unsigned long getThreads() const = 0;
  virtual void setThreads(unsigned long threads) = 0;
  virtual ThreadAlgorithm getThreadAlg() const = 0;
  virtual void setRunSimulator(bool run = true) = 0;
  virtual const std::string& explicitTradesExt() const = 0;
  virtual ErrorHandlingMode dataErrorHandlingMode() const = 0;
  virtual const std::string& startTradesDateTime() const = 0;
  virtual bool hasStatsHandler() const = 0;
  virtual bool hasTradesFile() const = 0;
  virtual bool hasEquityCurveFile() const = 0;
  virtual bool hasStatsFile() const = 0;
  virtual bool hasChartDescriptionFile() const = 0;
};