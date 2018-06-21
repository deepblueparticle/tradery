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

#include <boost/regex.hpp>
//#include "alertqueue.h"
#include <boost/make_shared.hpp>
#include <thread.h>
#include <traderyconnection.h>
#include <stringformat.h>

typedef tradery::ManagedPtr<Json::Value> JsonValuePtr;
typedef boost::shared_ptr<tradery::StrVector> StrVectorPtr;

BASIC_EXCEPTION(TraderySessionException)

class TraderySessionStatsEvent : public tradery::NameValueMap {
  //	OBJ_COUNTER( TraderySessionStats )
 private:
  bool m_dataAvailable;
  std::wstring m_dateRange;
  std::wstring m_duration;
  unsigned int m_symbolsProcessed;
  unsigned int m_symbolsProcessedWithErrors;
  unsigned int m_totalSymbols;
  unsigned int m_rawTrades;
  unsigned int m_processedTrades;
  unsigned int m_rawSignals;
  unsigned int m_processedSignals;
  double m_percentage;
  std::wstring m_status;

 private:
  void init();

 public:
  TraderySessionStatsEvent();

  TraderySessionStatsEvent(const Json::Value& stats);

  void reset() {
    __super::erase(__super::begin(), __super::end());
    init();
  }

  bool dataAvailable() const { return m_dataAvailable; }

  double percentage() const { return m_percentage; }

  const std::wstring& status() const { return m_status; }
};

typedef tradery::ManagedPtr<TraderySessionStatsEvent> TraderySessionStatsPtr;

class TraderySessionErrorEvent {
 private:
  std::wstring m_error;

 public:
  TraderySessionErrorEvent(const std::wstring& error) : m_error(error) {}

  TraderySessionErrorEvent() {}

  TraderySessionErrorEvent& operator=(const TraderySessionErrorEvent& error) {
    if (this != &error) {
      m_error = error.error();
    }

    return *this;
  }

  const std::wstring& error() const { return m_error; }
};

typedef tradery::ManagedPtr<TraderySessionErrorEvent>
    TraderySessionErrorEventPtr;

class TraderySessionEventListener {
  OBJ_COUNTER(TraderySessionEventListener)
 public:
  virtual ~TraderySessionEventListener() {}

  virtual void startOfSession(const std::wstring& diag = std::wstring()) = 0;
  virtual void statsEvent(TraderySessionStatsPtr stats,
                          const std::wstring& diag = std::wstring()) = 0;
  virtual void endOfsession(unsigned __int64 timeStamp,
                            const std::wstring& diag = std::wstring()) = 0;
  virtual void sessionCanceled(const std::wstring& diag = std::wstring()) = 0;
  virtual void errorEvent(TraderySessionErrorEventPtr error,
                          const std::wstring& diag = std::wstring()) = 0;
  virtual void sessionFailed(const std::wstring& diag = std::wstring()) = 0;
  virtual void retryingSession(size_t n, const std::wstring message,
                               const std::wstring& diag = std::wstring()) = 0;
};

class NullTraderySessionEventListener : public TraderySessionEventListener {
 public:
  virtual void startOfSession(const std::wstring& diag = std::wstring()) {}
  virtual void statsEvent(TraderySessionStatsPtr stats,
                          const std::wstring& diag = std::wstring()) {}
  virtual void endOfsession(unsigned __int64 timeStamp,
                            const std::wstring& diag = std::wstring()) {}
  virtual void sessionCanceled(const std::wstring& diag = std::wstring()) {}
  virtual void errorEvent(TraderySessionErrorEventPtr error,
                          const std::wstring& diag = std::wstring()) {}
  virtual void sessionFailed(const std::wstring& diag = std::wstring()) {}
  virtual void retryingSession(size_t n, const std::wstring message,
                               const std::wstring& diag = std::wstring()) {}
};

class TraderySessionEventListenerDelegator
    : public TraderySessionEventListener {
  typedef std::vector<TraderySessionEventListener*>
      TraderySessionEventListenerVector;

  TraderySessionEventListenerVector m_listeners;
  mutable tradery::Mutex m_mx;

 public:
  void addListener(TraderySessionEventListener* listener) {
    tradery::Lock lock(m_mx);

    assert(listener != 0);

    m_listeners.push_back(listener);
  }

  virtual void statsEvent(TraderySessionStatsPtr stats,
                          const std::wstring& diag = std::wstring()) {
    tradery::Lock lock(m_mx);

    for (TraderySessionEventListenerVector::iterator i = m_listeners.begin();
         i != m_listeners.end(); ++i)
      (*i)->statsEvent(stats, diag);
  }

  virtual void endOfsession(unsigned __int64 timeStamp,
                            const std::wstring& diag = std::wstring()) {
    tradery::Lock lock(m_mx);

    for (TraderySessionEventListenerVector::iterator i = m_listeners.begin();
         i != m_listeners.end(); ++i)
      (*i)->endOfsession(timeStamp, diag);
  }

  virtual void sessionCanceled(const std::wstring& diag = std::wstring()) {
    tradery::Lock lock(m_mx);

    for (TraderySessionEventListenerVector::iterator i = m_listeners.begin();
         i != m_listeners.end(); ++i)
      (*i)->sessionCanceled(diag);
  }

  virtual void errorEvent(TraderySessionErrorEventPtr error,
                          const std::wstring& diag = std::wstring()) {
    tradery::Lock lock(m_mx);

    for (TraderySessionEventListenerVector::iterator i = m_listeners.begin();
         i != m_listeners.end(); ++i)
      (*i)->errorEvent(error, diag);
  }

  virtual void sessionFailed(const std::wstring& diag = std::wstring()) {
    tradery::Lock lock(m_mx);

    for (TraderySessionEventListenerVector::iterator i = m_listeners.begin();
         i != m_listeners.end(); ++i)
      (*i)->sessionFailed(diag);
  }

  virtual void startOfSession(const std::wstring& diag = std::wstring()) {
    tradery::Lock lock(m_mx);

    for (TraderySessionEventListenerVector::iterator i = m_listeners.begin();
         i != m_listeners.end(); ++i)
      (*i)->startOfSession(diag);
  }

  virtual void retryingSession(size_t n, const std::wstring message,
                               const std::wstring& diag = std::wstring()) {
    tradery::Lock lock(m_mx);

    for (TraderySessionEventListenerVector::iterator i = m_listeners.begin();
         i != m_listeners.end(); ++i)
      (*i)->retryingSession(n, diag);
  }
};

typedef tradery::ManagedPtr<TraderySessionEventListener>
    TraderySessionStatsListenerPtr;

class TraderySession : public tradery::Thread {
  OBJ_COUNTER(TraderySession)

 private:
  TraderySessionEventListenerDelegator _listeners;
  TraderyAuthToken* m_authToken;
  bool m_canceling;
  const std::wstring m_name;

  bool m_result;

 public:
  TraderySession(const std::wstring& name, TraderyAuthToken* authToken,
                 TraderySessionEventListener& listener,
                 TraderySessionEventListener& listener1)
      : tradery::Thread(tradery::format("TraderySession %1% thread", name)),
        m_authToken(authToken),
        m_name(name),
        m_result(false) {
    assert(authToken != 0);
    _listeners.addListener(&listener);
    _listeners.addListener(&listener1);
  }

  TraderySession(const std::wstring& name)
      : m_name(name),
        tradery::Thread(tradery::format("TraderySession %1% thread", name)) {}

  ~TraderySession() { stop(); }

  void start();
  void stop();

  bool active() { return __super::isThreadActive(); }
  const std::wstring& name() const { return m_name; }

  bool success() const { return m_result; }

  void setFailed() { m_result = false; }
  void setSuccess() { m_result = true; }

 private:
  // this gets the currently active signals (from the db)
  void cancelSession(TraderyConnection& traderyConnection);
  TraderySessionStatsPtr getRuntimeStats(TraderyConnection& traderyConnection);
  class HeartBeatResult {
   private:
    bool m_result;
    int m_timeStamp;

   public:
    HeartBeatResult(bool result, int timeStamp)
        : m_result(result), m_timeStamp(timeStamp) {}

    operator bool() const { return m_result; }
    int timeStamp() const { return m_timeStamp; }
  };

  HeartBeatResult heartBeat(TraderyConnection& traderyConnection);
  // this gets the signals generated by a session, immediately after the session

  virtual void run(ThreadContext*) {}

 protected:
  virtual tradery::StringPtr getTraderyBalancesAndPositionsJsonString(
      JsonValuePtr ta) {
    return tradery::StringPtr();
  }
  TraderySessionEventListener& listeners() { return _listeners; }
  TraderyAuthToken* authToken() { return m_authToken; }
  JsonValuePtr getActiveEntrySignals(TraderyConnection& traderyConnection,
                                     const std::wstring& id);
  void maintainRun(TraderyConnection& traderyConnection);
  JsonValuePtr getSignals(TraderyConnection& traderyConnection,
                          const std::wstring& id);
  JsonValuePtr getStats(TraderyConnection& traderyConnection);
};

typedef tradery::ManagedPtr<TraderySession> TraderySessionPtr;
