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

#include <Tradery.h>

#define DURATION "duration"
#define TOTAL_BAR_COUNT "totalBarCount"
#define STATUS "status"
#define TOTAL_SYMBOL_COUNT "symbolCount"
#define PROCESSED_SYMBOL_COUNT "processedSymbolCount"
#define SYMBOL_PROCESSED_WITH_ERRORS_COUNT "symbolsProcessedWithErrorsCount"
#define RAW_TRADE_COUNT "rawTradeCount"
#define PROCESSED_TRADE_COUNT "processedTradeCount"  // after position sizing
#define SIGNAL_COUNT "signalCount"
#define PROCESSED_SIGNAL_COUNT "processedSignalCount"  // after signal sizing
#define ERROR_COUNT "errorCount"
#define CURRENT_SYMBOL "currentSymbol"
#define PERCENTAGE_DONE "percentageDone"
#define SYSTEM_COUNT "systemCount"
#define MESSAGE "message"

class RuntimeStatsImpl : public RuntimeStats,
                         public tradery_thrift_api::RuntimeStats {
 private:
  // this is the number of increments that will make up 100%
  // used to show percentages between different processing elements
  unsigned __int64 _totalClicks;
  mutable Timer _timer;

  mutable Mutex _mutex;

  double _extraPct;

 public:
  RuntimeStatsImpl() : _extraPct(0) { setStatus(RuntimeStatus::READY); }

  RuntimeStatsImpl(const nlohmann::json& j) {
    __super::duration = j[DURATION].get<double>();
    __super::processedSymbolCount =
        j[PROCESSED_SYMBOL_COUNT].get<unsigned int>();
    __super::symbolProcessedWithErrorsCount =
        j[SYMBOL_PROCESSED_WITH_ERRORS_COUNT].get<unsigned int>();
    __super::totalSymbolCount = j[TOTAL_SYMBOL_COUNT].get<unsigned int>();
    __super::systemCount = j[SYSTEM_COUNT].get<unsigned int>();
    __super::rawTradeCount = j[RAW_TRADE_COUNT].get<unsigned int>();
    __super::processedTradeCount = j[PROCESSED_TRADE_COUNT].get<unsigned int>();
    __super::signalCount = j[SIGNAL_COUNT].get<unsigned int>();
    __super::processedSignalCount =
        j[PROCESSED_SIGNAL_COUNT].get<unsigned int>();
    __super::totalBarCount = j[TOTAL_BAR_COUNT].get<unsigned int>();
    __super::errorCount = j[ERROR_COUNT].get<unsigned int>();
    __super::percentageDone = j[PERCENTAGE_DONE].get<double>();
    __super::currentSymbol = j[CURRENT_SYMBOL].get<std::string>();
    __super::status =
        (tradery_thrift_api::RuntimeStatus::type)j[STATUS].get<unsigned int>();
    __super::message = j[MESSAGE].get<std::string>();
  }

  void setTotalSymbols(unsigned int totalSymbols) {
    Lock lock(_mutex);
    __super::totalSymbolCount = totalSymbols;
  }

  virtual void addPct(double pct) {
    Lock lock(_mutex);
    _extraPct += pct;
    assert(_extraPct < 100);
  }

  double getPercentage() const {
    Lock lock(_mutex);
    return __super::percentageDone;
  }

  virtual void step(double pct) {
    Lock lock(_mutex);
    percentageDone += pct;
  }

  void incSignals() {
    Lock lock(_mutex);
    __super::signalCount++;
  }
  void setRawTrades(unsigned int trades) {
    Lock lock(_mutex);
    __super::rawTradeCount = trades;
  }

  void setProcessedTrades(unsigned int trades) {
    Lock lock(_mutex);
    __super::processedTradeCount = trades;
  }

  void setProcessedSignals(unsigned int signals) {
    Lock lock(_mutex);
    __super::processedSignalCount = signals;
  }

  void incErrors() {
    Lock lock(_mutex);
    __super::errorCount++;
  }
  void incTotalRuns() {
    Lock lock(_mutex);
    __super::totalRuns++;
    __super::percentageDone +=
        (100.0 - _extraPct) / ((double)__super::totalSymbolCount);
    //    std::cout << "incTotalRuns - totalruns: " << _totalRuns << ", total
    //    symbols:" << _totalSymbols << ", crtpct: " << _crtPct << ", extraPct:
    //    " << _extraPct << std::endl;
  }
  void incErrorRuns() {
    Lock lock(_mutex);
    __super::errorCount++;
  }

  void incTotalBarCount(unsigned int barsCount) {
    Lock lock(_mutex);
    __super::totalBarCount += barsCount;
  }

  unsigned int getTotalBarCount() const {
    Lock lock(_mutex);
    return __super::totalBarCount;
  }

  virtual void setStatus(RuntimeStatus status) {
    switch (status) {
      case READY:
        __super::status = tradery_thrift_api::RuntimeStatus::READY;
        break;
      case RUNNING:
        __super::status = tradery_thrift_api::RuntimeStatus::RUNNING;
        break;
      case CANCELING:
        __super::status = tradery_thrift_api::RuntimeStatus::CANCELING;
        break;
      case ENDED:
        __super::status = tradery_thrift_api::RuntimeStatus::ENDED;
        break;
      case CANCELED:
        __super::status = tradery_thrift_api::RuntimeStatus::CANCELED;
        break;
      default:
        break;
    }
  }

  virtual void setMessage(const std::string& message) {
    __super::message = message;
  }

  void to_json(nlohmann::json& j) const {
    j = nlohmann::json{{DURATION, __super::duration},
                       {PROCESSED_SYMBOL_COUNT, __super::processedSymbolCount},
                       {SYMBOL_PROCESSED_WITH_ERRORS_COUNT,
                        __super::symbolProcessedWithErrorsCount},
                       {TOTAL_SYMBOL_COUNT, __super::totalSymbolCount},
                       {SYSTEM_COUNT, __super::systemCount},
                       {RAW_TRADE_COUNT, __super::rawTradeCount},
                       {PROCESSED_TRADE_COUNT, __super::processedTradeCount},
                       {SIGNAL_COUNT, __super::signalCount},
                       {PROCESSED_SIGNAL_COUNT, __super::processedSignalCount},
                       {TOTAL_BAR_COUNT, __super::totalBarCount},
                       {ERROR_COUNT, __super::errorCount},
                       {PERCENTAGE_DONE, __super::percentageDone},
                       {CURRENT_SYMBOL, __super::currentSymbol},
                       {STATUS, __super::status},
                       {MESSAGE, __super::message}};
  }

  std::string to_json() const {
    nlohmann::json j;
    to_json(j);
    return j.dump(4);
  }

 protected:
  void outputStats(std::ostream& os) const {
    Lock lock(_mutex);

    nlohmann::json j = *this;
    os << j.dump(4);
  }
};

inline void to_json(nlohmann::json& j, const ::RuntimeStats& rs) {
  rs.to_json(j);
}

class FileRuntimeStats : public RuntimeStatsImpl {
 private:
  const std::string _fileName;

 public:
  FileRuntimeStats(const std::string& fileName) : _fileName(fileName) {}

  void outputStats() const {
    if (_fileName.length() > 0) {
      std::ofstream outputStats(_fileName.c_str());

      if (outputStats) {
        //        std::cout << "wrote output stats" << std::endl;
        //        __super::outputStats( std::cout );
        __super::outputStats(outputStats);
      } else

      {
        LOG(log_error, "Could not open runtime stats file for writing");
        // todo could not open file for writing
      }

      /*
      TCHAR inbuf[ 8000 ];
      DWORD bytesRead;
      if( !CallNamedPipe( lpszPipeName.c_str(), const_cast< TCHAR* >(cmdLine
      .str().c_str() ), cmdLine.str().length() + 1, inbuf, sizeof(inbuf),
      &bytesRead, NMPWAIT_WAIT_FOREVER) )
      {
      // todo: handle errors
      }
      */
      // todo handle response from server
    }
  }
};
