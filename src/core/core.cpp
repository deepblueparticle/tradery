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

#include "stdafx.h"
#include "positions.h"
#include "system.h"
#include "datamanager.h"
#include "scheduler.h"
#include "errorsink.h"
#include "bars.h"
#include "ticks.h"

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
      // init TA-LIB
      TA_RetCode retCode;
      /*	      TA_InitializeParam param;
                    memset( &param, 0, sizeof( TA_InitializeParam ) );
                    param.logOutput = stdout;
                    retCode = TA_Initialize( &param );
      */
      retCode = TA_Initialize();

      if (retCode == TA_SUCCESS) {
        LOG(log_info, "TA-LIB correctly initialized.");
      } else {
        LOG(log_error, "Error initializing TA-LIB: " << retCode);
      }
    } break;
    case DLL_THREAD_ATTACH:
      // this seems to be necessary to be able to start the application in any
      // conditions if not here, strangely, the app doesn't start when in
      // certain directories
      //    Sleep( 500 );
      break;
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      TA_Shutdown();
#ifdef _DEBUG
      // std::cout << "TA-LIB shut down" << std::endl;
#endif  //_DEBUG
      break;
  }
  return TRUE;
}

PositionsManagerAbstr* PositionsManagerAbstr::create(
    PositionsContainer* posList, DateTime startTrades, DateTime endTrades,
    Slippage* slippage, Commission* commission) {
  return new PositionsManagerImpl(posList, startTrades, endTrades, slippage,
                                  commission);
}

Scheduler* Scheduler::create(RunEventHandler* runEventHandler) {
  return new SchedulerImpl(runEventHandler);
}

PositionsContainer* PositionsContainer::create() {
  return new PositionsContainerImpl();
}

PositionsContainer* create(const PositionsContainer* pc) {
  return new PositionsContainerImpl(pc);
}

DataManager* DataManager::create(unsigned int cacheSize) {
  return new DataManagerImpl(cacheSize);
}

BarsPtr tradery::createBars(const std::string& dataSourceName,
                            const std::string& symbol, BarsAbstr::Type type,
                            unsigned int resolution, DateTimeRangePtr range,
                            ErrorHandlingMode errorHandlingMode) {
  return BarsPtr(new BarsImpl(dataSourceName, symbol, type, resolution, range,
                              errorHandlingMode));
}

Ticks* tradery::createTicks(const std::string& dataSourceName,
                            const std::string& symbol, const Range* range) {
  return new TicksImpl(dataSourceName, symbol, range);
}

CORE_API ErrorEventSink* tradery::createBasicErrorEventSink() {
  return new ErrorEventSinkImpl();
}

ErrorEvent::ErrorEvent(Types type, Category category,
                       const std::string& systemName,
                       const std::string& message, const std::string& symbol)
    : _type(type),
      _systemName(systemName),
      _message(message),
      _symbol(symbol),
      _category(category) {}

const std::string ErrorEvent::toString() const {
  std::ostringstream o;
  o << "Error type: " << _type << " - " << _message << " - system: \""
    << _systemName << "\", symbol: \"" << _symbol << "\"";
  return o.str();
}

Session::Session(RunEventHandler* runEventHandler) throw(SymbolsInfoException)
    : _defScheduler(new SchedulerImpl(runEventHandler)) {}

Session::~Session() {}

void Session::addRunnable(Runnable* runnable, PositionsVector& pos,
                          ErrorEventSink* errorSink,
                          DataInfoIterator* dataInfoIterator,
                          SignalHandler* signalHandler,
                          RunnableRunInfoHandler* runnableRunInfoHandler,
                          Slippage* slippage, Commission* commission,
                          chart::ChartManager* chartManager,
                          const tradery::ExplicitTrades* explicitTrades) {
  _defScheduler->addRunnable(runnable, dataInfoIterator, pos, errorSink,
                             signalHandler, runnableRunInfoHandler, slippage,
                             commission, chartManager, explicitTrades);
}

void tradery::Session::addRunnable(
    Runnable* runnable, PositionsVector& pos, ErrorEventSink* errorSink,
    DataInfoIterator* dataInfoIterator,
    const std::vector<SignalHandler*>& signalHandlers,
    RunnableRunInfoHandler* runnableRunInfoHandler, Slippage* slippage,
    Commission* commission, chart::ChartManager* chartManager,
    const tradery::ExplicitTrades* explicitTrades) {
  _defScheduler->addRunnable(runnable, dataInfoIterator, pos, errorSink,
                             signalHandlers, runnableRunInfoHandler, slippage,
                             commission, chartManager, explicitTrades);
}

const char* Signal::signalTypeAsString(SignalType type) {
  switch (type) {
    case BUY_AT_MARKET:
      return "Buy at market";
      break;
    case BUY_AT_CLOSE:
      return "Buy at close";
      break;
    case BUY_AT_STOP:
      return "Buy at stop";
      break;
    case BUY_AT_LIMIT:
      return "Buy at limit";
      break;
    case SELL_AT_MARKET:
      return "Sell at market";
      break;
    case SELL_AT_CLOSE:
      return "Sell at close";
      break;
    case SELL_AT_STOP:
      return "Sell at stop";
      break;
    case SELL_AT_LIMIT:
      return "Sell at limit";
      break;
    case SHORT_AT_MARKET:
      return "Short at market";
      break;
    case SHORT_AT_CLOSE:
      return "Short at close";
      break;
    case SHORT_AT_STOP:
      return "Short at stop";
      break;
    case SHORT_AT_LIMIT:
      return "Short at limit";
      break;
    case COVER_AT_MARKET:
      return "Cover at market";
      break;
    case COVER_AT_CLOSE:
      return "Cover at close";
      break;
    case COVER_AT_STOP:
      return "Cover at stop";
      break;
    case COVER_AT_LIMIT:
      return "Cover at limit";
      break;
    default:
      assert(false);
      return 0;
      break;  // should not happen
  }
}

extern SeriesCache* _cache;

CORE_API void tradery::init(unsigned int cacheSize) {
  // the data cache is disabled for now
  _cache = new SeriesCache(100, false);
  _dataManager = new DataManagerImpl(cacheSize);
}

CORE_API void tradery::uninit() {
  delete _cache;
  delete _dataManager;
}

extern DataManager* _dataManager;

CORE_API void tradery::registerDataSource(DataSource* dataSource) {
  assert(_dataManager != 0);
  assert(dataSource != 0);
  _dataManager->addDataSource(dataSource);
}

CORE_API bool tradery::unregisterDataSource(DataSource* dataSource) {
  assert(_dataManager != 0);
  assert(dataSource != 0);
  return _dataManager->removeDataSource(dataSource);
}

CORE_API bool tradery::unregisterDataSource(const UniqueId& dataSourceId) {
  assert(_dataManager != 0);
  return _dataManager->removeDataSource(dataSourceId);
}

CORE_API void tradery::setDataCacheSize(unsigned int cacheSize) {
  _dataManager->setCacheSize(cacheSize);
}

CORE_API void Session::run(bool asynch, unsigned int threads, bool cpuAffinity,
                           DateTimeRangePtr range,
                           DateTime startTradesDateTime) {
  _defScheduler->run(asynch, threads, cpuAffinity, range, startTradesDateTime);
}
/**
 * Indicates the running scheduler running status.
 *
 * @return true if any system thread is running, false if no system threads are
 * running
 */
CORE_API bool Session::isRunning() const { return _defScheduler->isRunning(); }

/**
 * Indicates the running scheduler canceling status.
 *
 * @return true if any system thread is running, false if no system threads are
 * running
 */
CORE_API bool Session::isCanceling() const {
  return _defScheduler->isCanceling();
}

/**
 * Cancels the current run synchronously, i.e. the call will return only after
 * there are no more runnables running
 *
 * The cancel works by telling all threads to return after the current runnable
 * has completed its run, so depending on the system, the data etc, the cancel
 * process may not be instantaneous
 *
 * @see runAsync
 */
CORE_API void Session::cancelSync() const { _defScheduler->cancelSync(); }
/**
 * Cancels the current run asynchrnously, i.e. it returns immediately,
 * without waiting for the runnables stop.
 *
 * The cancel works by telling all threads to return after the current runnable
 * has completed its run, so depending on the system, the data etc, the cancel
 * process may not be instantaneous
 *
 * When using this call, the user will usually have to check the running status
 * using isRunning before performing some other action on the SchedulerI object
 *
 * @see isRunning
 */
CORE_API void Session::cancelAsync() const { _defScheduler->cancelAsync(); }

CORE_API void Session::resetRunnables() { _defScheduler->resetRunnables(); }

CORE_API const std::string Signal::csvHeaderLine() {
  return "Symbol,Signal date/time,Shares,Side,Type,Price,Name,System id, "
         "System name, Position id";
}

CORE_API const std::string Signal::toCSVString() const {
  std::ostringstream o;
  o << symbol() << ',' << time().to_simple_string() << ',' << shares() << ',';

  switch (type()) {
    case SignalType::BUY_AT_MARKET:
      o << "Buy,Market,";
      break;
    case SignalType::BUY_AT_STOP:
      o << "Buy,Stop,";
      break;
    case SignalType::BUY_AT_LIMIT:
      o << "Buy,Limit,";
      break;
    case SignalType::BUY_AT_CLOSE:
      o << "Buy,Close,";
      break;
    case SignalType::SELL_AT_MARKET:
      o << "Sell,Market,";
      break;
    case SignalType::SELL_AT_STOP:
      o << "Sell,Stop,";
      break;
    case SignalType::SELL_AT_LIMIT:
      o << "Sell,Limit,";
      break;
    case SignalType::SELL_AT_CLOSE:
      o << "Sell,Close,";
      break;
    case SignalType::SHORT_AT_MARKET:
      o << "Short,Market,";
      break;
    case SignalType::SHORT_AT_STOP:
      o << "Short,Stop,";
      break;
    case SignalType::SHORT_AT_LIMIT:
      o << "Short,Limit,";
      break;
    case SignalType::SHORT_AT_CLOSE:
      o << "Short,Close,";
      break;
    case SignalType::COVER_AT_MARKET:
      o << "Cover,Market,";
      break;
    case SignalType::COVER_AT_STOP:
      o << "Cover,Stop,";
      break;
    case SignalType::COVER_AT_LIMIT:
      o << "Cover,Limit,";
      break;
    case SignalType::COVER_AT_CLOSE:
      o << "Cover,Close,";
      break;
    default:
      assert(false);
  }
  o << price() << ',' << name();
  o << ',' << *systemId() << ',' << *systemName() << ","
    << (position() ? position().getId() : 0);

  return o.str();
}

CORE_API DataRequester* tradery::getDataRequester() { return _dataManager; }

// this has to be created here because this is where it will get deleted
// if this is created in the fileplugin dll, it crashes at deletion
// most likely because the 2 dlls have different options, and also possibly
// because of creation and deletion in and from different heaps.
CORE_API DataLocationInfoPtr tradery::makeDataFileLocationInfo(
    const std::string& fileName, __int64 startPos, __int64 count) {
  return DataLocationInfoPtr(
      new DataFileLocationInfo(fileName, startPos, count));
}
