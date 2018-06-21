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

#include "RunSystemContext.h"
#include "runsystem.h"
#include "wdoc.h"
#include <nlohmann\json.hpp>

#include "runtime_stats_impl.h"

class XErrorEventSink : public tradery::ErrorEventSink {
 private:
  typedef std::vector<ManagedPtr<ErrorEvent> > ErrorEventVector;
  ErrorEventVector _events;
  mutable Mutex _m;
  RuntimeStats& _errorsCounter;

 public:
  XErrorEventSink(RuntimeStats& errorsCounter)
      : _errorsCounter(errorsCounter) {}

  virtual void push(const ErrorEvent& event) {
    Lock lock(_m);
    _events.push_back(new ErrorEvent(event));
    _errorsCounter.incErrors();
    // std::cout << event.toString() << std::endl;
  }
  virtual void pop() {
    Lock lock(_m);
    if (!_events.empty()) _events.pop_back();
  }
  virtual const ErrorEvent* front() const {
    Lock lock(_m);
    return _events.empty() ? 0 : _events.back().get();
  }
  virtual bool empty() const {
    Lock lock(_m);
    return _events.empty();
  }
  virtual size_t size() const {
    Lock lock(_m);
    return _events.size();
  }

  void toHTML(std::ostream& os) {
    Lock lock(_m);
    os << "<table class=\"list_table\">" << std::endl;
    os << "<tr class='h'><td class='h'></td><td class='h'>Type</td><td "
          "class='h'>Symbol</td><td class='h'>System</td><td "
          "class='h'>Message</td></tr>"
       << std::endl;

    unsigned __int64 count = 0;

    for (ErrorEventVector::const_iterator i = _events.begin();
         i != _events.end(); i++, count++) {
      std::string typeClass;
      if ((*i)->category() == ErrorEvent::Category::error)
        typeClass = "errorLine";
      else if ((*i)->category() == ErrorEvent::Category::warning)
        typeClass = "warningLine";

      os << "<tr class=\"" << (count % 2 ? "d0 " : "d1 ") << "\">" << std::endl;
      os << "<td style='font-weight: bold' class='" << typeClass << "'>"
         << (*i)->categoryToString() << "</td>" << std::endl;
      os << "<td class='" << typeClass << "'>" << (*i)->typeToString()
         << "</td>" << std::endl;
      os << "<td class='" << typeClass << "'>" << (*i)->symbol() << "</td>"
         << std::endl;
      os << "<td class='" << typeClass << "'>" << (*i)->systemName() << "</td>"
         << std::endl;
      os << "<td class='" << typeClass << "'>" << (*i)->message() << "</td>"
         << std::endl;
      os << "</tr>" << std::endl;
    }

    os << "</table>" << std::endl;
  }
};

class XPositionHandlerException {
 private:
  const std::string _fileName;

 public:
  XPositionHandlerException(const std::string& fileName)
      : _fileName(fileName) {}

  const std::string& fileName() const { return _fileName; }
  const std::string message() const {
    return std::string("Could not open positions file for writing \"") +
           _fileName + "\"";
  }
};

// will update session real-time stats
class XSignalHandler : public SignalHandler {
 private:
  mutable Mutex _mx;
  RuntimeStats& _signalsCounter;

  SignalVector _signals;

 private:
 public:
  XSignalHandler(RuntimeStats& signalCounter)
      : SignalHandler(Info()), _signalsCounter(signalCounter) {}

  virtual void signal(SignalPtr _signal) throw(SignalHandlerException) {
    Lock lock(_mx);

    assert(_signal);

    _signalsCounter.incSignals();
    _signals.push_back(_signal);
  }

  unsigned int processedSignalsCount() {
    unsigned int count = 0;
    for (SignalVector::size_type n = 0; n < _signals.size(); ++n) {
      if (_signals[n]->isEnabled()) ++count;
    }
    return count;
  }
};

class XRunnableRunInfoHandler : public RunnableRunInfoHandler {
 private:
  mutable Mutex _m;
  mutable Timer _runTimer;
  RuntimeStats& _runsCounter;

  unsigned int _count;

 public:
  XRunnableRunInfoHandler(RuntimeStats& runsCounter)
      : _count(0), _runsCounter(runsCounter) {}

  // a runnable status event has been received
  virtual void status(const RunnableRunInfo& status) {
    Lock lock(_m);
    _runsCounter.incTotalRuns();
    if (status.errors()) _runsCounter.incErrorRuns();
    _count++;
    LOG(log_debug, (status.errors() ? "!" : "+")
                       << "[" << status.threadName() << ":"
                       << status.cpuNumber() << "] " << status.status()
                       << " on \"" << status.symbol() << "\"");
    _runTimer.restart();
    _runsCounter.incTotalBarCount(status.dataUnitCount());
    _runsCounter.setMessage(std::string("Running \"")
                            << status.status() << "\" on \"" << status.symbol()
                            << "\"");
    _runsCounter.setStatus(RuntimeStatus::RUNNING);
  }

  std::wstring countAsString() const {
    Lock lock(_m);
    std::wostringstream o;

    o << _count;

    return o.str();
  }

  bool timeout(unsigned __int64 runTimeout) const {
    Lock lock(_m);
    //    std::cout << "in symbol timeout test: "  << _runTimer.elapsed() << ",
    //    " << runTimeout << std::endl;
    return _runTimer.elapsed() > runTimeout;
  }

  bool exceededBarCount(unsigned __int64 maxTotalBarCount) const {
    Lock lock(_m);
    return maxTotalBarCount > 0 &&
           _runsCounter.getTotalBarCount() > maxTotalBarCount;
  }

  unsigned __int64 totalBarCount() { return _runsCounter.getTotalBarCount(); }
};

class XOutputSink : public OutputSink {
 private:
  ofstream _ofs;
  const std::string _fileName;

 public:
  XOutputSink(const std::string& fileName) : _fileName(fileName) {
    if (!_fileName.empty()) {
      _ofs.open(_fileName.c_str());
    }

    if (!_ofs) {
      // todo: error, throw exception
    }
  }
  virtual void print(const std::string& str) {
    if (_ofs) _ofs << str;
  }
  virtual void printLine(const std::string& str) {
    if (_ofs) _ofs << str << "\n";
  }

  virtual void clear() {}

  virtual void print(Control ctrl) {
    if (_ofs) {
      switch (ctrl) {
        case def:
        case black:
        case blue:
        case green:
        case red:
        case yellow:
        case purple:
        case gray:

        case bold:
        case not_bold:
        case italic:
        case not_italic:
        case underline:
        case not_underline:
        case reset:
          break;

        case tradery::endl:
          _ofs << "\n";
          break;
        default:
          break;
      }
    }
  }
};

class ErrorFile {
 public:
  ErrorFile(const std::string& fileName, const std::string& message,
            const std::string& caption) {
    std::ofstream errorsFile(fileName.c_str(), ios_base::out | ios_base::app);
    if (errorsFile) {
      LOG(log_debug, "dumping errors to error file");

      if (!caption.empty())
        errorsFile << "<h2>" << caption << "</h2>" << std::endl;

      errorsFile << message;
    } else
      LOG(log_debug,
          "error - can't open the errors file for writing: " << fileName);
  }
};

RunSystem::RunSystem(SessionContextPtr context) : _context(context) {}

void RunSystem::run() {
  try {
    LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
         "Start runsystem");
    std::wstring str;

    FileRuntimeStats runtimeStats(
        _context->getSessionConfig()->getRuntimeStatsFile());

    LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
         "slippage value: " << _context->getSessionParams()->slippage);
    LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
         "slippage id: " << _context->getConfig()->defSlippageId());

    //		LOG(log_debug, "max open pos: " <<
    //_context->getSessionParams()->maxOpenPositions);

    LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
         "stats file: " << _context->getSessionConfig()->getStatsFile());
    WebDocument document(_context);

    std::auto_ptr<XSignalHandler> sh(new XSignalHandler(runtimeStats));
    LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
         "in runsystem.h, signals file: "
             << _context->getSessionConfig()->getSignalsFile());

    XRunnableRunInfoHandler rih(runtimeStats);
    XOutputSink os(_context->getSessionConfig()->getOutputFile());
    PositionsVector pv;
    std::auto_ptr<XErrorEventSink> errsink(new XErrorEventSink(runtimeStats));
    LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
         "RunSystem::RunSystem - starting sessioin");
    TASession session(document, sh.get(), errsink.get(), &rih, &runtimeStats,
                      os);
    session.start(pv);

    try {
      // this has to be called after start, as the symbol source is loaded then,
      // otherwise we'll get an exception
      runtimeStats.setTotalSymbols(
          session.defSymbolsCount() *
          _context->getSessionConfig()->getRunnables().size());
    } catch (const SymbolsSourceNotAvailableException&) {
      LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
           "SymbolsSourceNotAvailableException!!!");
    }

    bool symbolTimedOut = false;
    bool maxTotalBarCountExceeded = false;
    Timer runtimeStatsTimer;
    // create the initial output stats file so the user doesn't see a blanc
    // screen
    runtimeStats.outputStats();

    while (session.isRunning()) {
      // check for timeout if haven't received the sessionEnded notification.
      // timeout in this context means a system that takes more than the
      // specified time on one symbol, and it's meant to prevent taking up too
      // much resources either accidentally, or in purpose sessionEnded signals
      // that all the systems have been run on all symbols, and now we may be
      // processing sessionEnded in various plugin config. The stats plugin
      // config may take a long time, so don't do timeout on it the reason there
      // is a timeout on symbols, is because the user code may contain infinite
      // loops, or other blocking elements and the session must have a way out
      // of this. when in plugins however, we control more or less things todo:
      // what if the user overrides the virtual sessionEnded for the system
      // plugin that is being run? Disable this!
      if (_context->getConfig()->symbolTimeout() > 0 &&
          rih.timeout(_context->getConfig()->symbolTimeout()) &&
          !session.sessionEndedReceived()) {
        // if timeout, just exit the loop and the process. This will crash the
        // session thread,
        LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
             "symbol timedout, in while loop");
        symbolTimedOut = true;
        break;
      }

      if (rih.exceededBarCount(_context->getConfig()->maxTotalBarCount())) {
        LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
             "Session exceeded max number of bars allowed: "
                 << _context->getConfig()->maxTotalBarCount() << " exceeded");
        maxTotalBarCountExceeded = true;
        // cancel session than continue the loop waiting for the session to end
        session.cancel();
      }
      // save new stats every 1 seconds
      if (runtimeStatsTimer.elapsed() > 1) {
        runtimeStats.setRawTrades(session.runTradesCount());
        runtimeStats.outputStats();
        runtimeStatsTimer.restart();
      }
      Sleep(50);
    }

    // this gets all the positions in one container. It does a copy of
    // positions, so not very efficient
    PositionsContainer* posp(pv.getAllPositions());
    assert(posp != 0);
    PositionsContainer& pos(*posp);
    // sort positions so they look good in the list
    pos.sortByEntryTime();
    // create the final output stats file, to show in the stats page
    // but first make sure we have the right number of trades, as they are not
    // event based, but they need to be requested
    runtimeStats.setRawTrades(session.runTradesCount());
    runtimeStats.setProcessedTrades(pos.enabledCount());
    runtimeStats.setProcessedSignals(sh->processedSignalsCount());
    runtimeStats.setMessage("Session complete");
    runtimeStats.setStatus(RuntimeStatus::ENDED);
    runtimeStats.outputStats();

    LOG1(log_info, _context->getSessionConfig()->getSessionId(),
         runtimeStats.to_json());

    if (!_context->getSessionConfig()->getTradesFile().empty()) {
      std::ofstream tradesFile(
          _context->getSessionConfig()->getTradesFile().c_str());

      if (!tradesFile) {
        OutputDebugString(
            s2ws("error - can't open the trades file for writing").c_str());
      } else {
        LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
             "Trades desc file: "
                 << _context->getSessionConfig()->getTradesDescriptionFile());
        std::ofstream tradesDescFile(
            _context->getSessionConfig()->getTradesDescriptionFile().c_str());
        if (tradesDescFile)
          PositionsContainerToHTML toHtml(
              pos, tradesFile, tradesDescFile,
              _context->getConfig()->getLinesPerPage());
        else
          LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
               "error - can't open the trades desc file for writing: "
                   << _context->getSessionConfig()->getTradesDescriptionFile());
      }
    }

    if (!_context->getSessionConfig()->getTradesCSVFile().empty()) {
      LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
           "Creating trades csv file: "
               << _context->getSessionConfig()->getTradesCSVFile());
      std::ofstream tradesCSVFile(
          _context->getSessionConfig()->getTradesCSVFile().c_str());

      if (!tradesCSVFile) {
        LOG1(log_error, _context->getSessionConfig()->getSessionId(),
             "error - can't open the trades CSV file for writing");
        OutputDebugString(
            s2ws("error - can't open the trades CSV file for writing").c_str());
      } else
        PositionsContainerToCSV toCSV(pos, tradesCSVFile);
    }

    if (!errsink->empty()) {
      LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
           "Creating errors file: "
               << _context->getSessionConfig()->getErrorsFile());
      std::fstream ef(_context->getSessionConfig()->getErrorsFile().c_str(),
                      ios_base::out | ios_base::app);
      if (!ef) {
        DWORD dw = GetLastError();
        LOG1(log_error, _context->getSessionConfig()->getSessionId(),
             "error " << dw << ", can't open the errors file for writing: "
                      << _context->getSessionConfig()->getErrorsFile());
      } else {
        LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
             "dumping errors to error file");
        errsink->toHTML(ef);
      }
    }

    if (symbolTimedOut) {
      std::fstream errorsFile(
          _context->getSessionConfig()->getErrorsFile().c_str(),
          ios_base::out | ios_base::app);
      if (!errorsFile) {
        LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
             "error - can't open the errors file for writing");
      } else {
        LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
             "writing symbol timedout to error file");
        errorsFile << "symbol processing timedout - processing canceled";
      }
    }

    if (maxTotalBarCountExceeded) {
      int n = 3;
      std::fstream errorsFile;
      for (errorsFile.open(
               _context->getSessionConfig()->getErrorsFile().c_str(),
               ios_base::out | ios_base::app);
           !errorsFile && n > 0;
           n--, errorsFile.open(
                    _context->getSessionConfig()->getErrorsFile().c_str(),
                    ios_base::out | ios_base::app))
        Sleep(10);
      if (!errorsFile) {
        LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
             "error - can't open the errors file for writing");
      } else {
        LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
             "Writing total number of bars exceeded to errors file");
        errorsFile << "Session exceeded maximum number of bars allowed: "
                   << _context->getConfig()->maxTotalBarCount() << ", "
                   << rih.totalBarCount() << std::endl;
      }
    }

    LOG1(log_debug, _context->getSessionConfig()->getSessionId(),
         "\n*****\n*** Processing ended with no errors ***\n*****\n");
  } catch (const DocumentException& e) {
    ErrorFile(_context->getSessionConfig()->getErrorsFile(), e.message(),
              "Session error");

    LOG1(log_error, _context->getSessionConfig()->getSessionId(),
         "DocumentException: " << e.message());
    throw RunSystemException(system_run_error, e.message());
  } catch (const PluginTreeException& e) {
    LOG1(log_error, _context->getSessionConfig()->getSessionId(),
         "PluginTreeException: " << e.message());
    throw RunSystemException(system_run_error, e.message());
  } catch (const RunProcessException& e) {
    LOG1(log_error, _context->getSessionConfig()->getSessionId(),
         "RunProcessException: " << e.message());
    throw RunSystemException(system_run_error, e.message());
  } catch (const SignalHandlerException& e) {
    LOG1(log_error, _context->getSessionConfig()->getSessionId(),
         "SignalHandlerException: " << e.message());
    throw RunSystemException(system_run_error, e.message());
  } catch (const XPositionHandlerException& e) {
    LOG1(log_error, _context->getSessionConfig()->getSessionId(),
         "XPositionHandlerException: " << e.message());
    throw RunSystemException(system_run_error, e.message());
  } catch (const TASessionException& e) {
    LOG1(log_error, _context->getSessionConfig()->getSessionId(),
         "TASessionException: " << e.message());
    throw RunSystemException(system_run_error, e.message());
  } catch (const CoreException& e) {
    LOG1(log_error, _context->getSessionConfig()->getSessionId(),
         "CoreException: " << e.message());
    throw RunSystemException(system_run_error, e.message());
  } catch (const std::exception& e) {
    ErrorFile(_context->getSessionConfig()->getErrorsFile(),
              "Internal server error", "Server error");
    LOG1(log_error, _context->getSessionConfig()->getSessionId(),
         "exception: " << e.what());
    throw RunSystemException(system_run_error, "Unknown error");
  } catch (...) {
    ErrorFile(_context->getSessionConfig()->getErrorsFile(),
              "Internal server error", "Server error");
    LOG1(log_error, _context->getSessionConfig()->getSessionId(),
         "Unknown exception");
    throw RunSystemException(system_run_error, "Unknown error");
  }
}
