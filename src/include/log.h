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

#include <sys/timeb.h>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "misc.h"

#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API __declspec(dllimport)
#endif

/**/

inline std::string hexstr(unsigned int n, unsigned int digits) {
  std::ostringstream os;
  os << "0x" << std::setw(digits) << std::setfill('0') << std::hex << n;
  return os.str();
}

inline std::string hexstr(HWND n, unsigned int digits) {
  return hexstr(reinterpret_cast<unsigned int>(n), digits);
}

inline std::string hexstr(const void* n, unsigned int digits) {
  return hexstr((unsigned int)n, digits);
}

#define HEXSTR(n, digits) hexstr(n, digits).c_str()
#define HEXSTR4(n) HEXSTR(n, 4)
#define HEXSTR8(n) HEXSTR(n, 8)
#define HEXSTR5(n) HEXSTR(n, 5)

/**/

enum Level { log_debug, log_info, log_error, log_assert, log_any };

namespace tradery {
class LogException {};

// this is the diagnostic logging

class LOG_API Log {
 private:
  static Level _globalLevel;
  Level _level;
  Level _thisLevel;

  static std::string _logToFile;
  static bool _logToDebugOutput;
  static bool _logToConsole;
  static tradery::Mutex _mutex;

 public:
  // level 0 - everything passes
  Log(Level level, const char* func) : _level(level) {}

  // start session
  Log() : _level(log_any) {}

  ~Log() {}

  static std::string levelToString(Level level) {
    switch (level) {
      case log_debug:
        return "debug ";
      case log_info:
        return "info  ";
      case log_error:
        return "error ";
      case log_assert:
        return "assert";
      case log_any:
        return "any   ";
      default:
        assert(false);
        return "";
    }
  }

  static bool checkLevel(Level level) {
    Lock lock(_mutex);
    return _globalLevel <= level;
  }

  static Level level() {
    Lock lock(_mutex);
    return _globalLevel;
  }

  static std::string header(Level level, const char* func) {
    assert(func != 0);
    std::ostringstream o;
    o << "[" << timeStamp(true) << " " << HEXSTR5(GetCurrentThreadId()) << ",";
    o << HEXSTR5(GetCurrentProcessId()) << "][" << levelToString(level) << "] ["
      << func << "] ";
    return o.str();
  }

  static void setLevel(Level level);

  static void setDebugLevel() {
    Lock lock(_mutex);
    _globalLevel = log_debug;
  }

  static void setNormalLevel() {
    Lock lock(_mutex);
    _globalLevel = log_info;
  }

  static bool isDebugLevel() {
    Lock lock(_mutex);

    return _globalLevel == log_debug;
  }

  static void flipLevel();

  static void setLogToFile(const std::string& file) {
    Lock lock(_mutex);
    _logToFile = file;
  }
  static void setLogToConsole(bool log = true) {
    Lock lock(_mutex);
    _logToConsole = log;
  }
  static void setLogToDebugOutput(bool log = true) {
    Lock lock(_mutex);
    _logToDebugOutput = log;
  }

  static bool logToFile() {
    Lock lock(_mutex);
    return !_logToFile.empty();
  }
  static const std::string& getLogFileName() {
    Lock lock(_mutex);
    return _logToFile;
  }
  static bool logToConsole() {
    Lock lock(_mutex);
    return _logToConsole;
  }
  static bool logToDebugOutput() {
    Lock lock(_mutex);
    return _logToDebugOutput;
  }

  static void xlog(Level level, const char* function, const std::string& value);
  static void xlog(Level level, const std::string& function,
                   const std::string& value);
  static void xfilelog(const std::string& fileName, Level level, char* function,
                       const std::string& value);

 protected:
  virtual std::ostream& getOstream() = 0;

 public:
  template <typename T>
  std::ostream& operator<<(const T& t) {
    std::ostream& os = getOstream();

    if (_globalLevel <= _level && os) os << t;

    return os;
  }
};

class FileLog : public Log {
 private:
  std::ofstream _ofs;

 public:
  FileLog(Level level, const char* func)
      : _ofs(getLogFileName().c_str(),
             std::ios_base::ate | std::ios_base::out | std::ios_base::app),
        Log(level, func) {
    if (!_ofs) {
      throw LogException();
    }
  }

  FileLog(char* fileName, Level level, const char* func)
      : _ofs(fileName,
             std::ios_base::ate | std::ios_base::out | std::ios_base::app),
        Log(level, func) {
    if (!_ofs) {
      throw LogException();
    }
  }

  FileLog(const std::string& fileName, Level level, const char* func)
      : _ofs(fileName.c_str(),
             std::ios_base::ate | std::ios_base::out | std::ios_base::app),
        Log(level, func) {
    if (!_ofs) {
      throw LogException();
    }
  }

  virtual std::ostream& getOstream() { return _ofs; }
};

}  // namespace tradery

using tradery::operator<<;
#define LOG(level, value) \
  tradery::Log::xlog(level, __FUNCTION__, std::string() << value);
#define LOG1(level, value1, value2) LOG(level, value1 << " - " << value2)
#define FILE_LOG(fileName, level, value) \
  tradery::Log::xfilelog(fileName, level, __FUNCTION__, std::string() << value);

class LogEntryExit {
 private:
  Level m_level;
  const std::string m_name;
  const std::string m_message;
  const unsigned __int64 m_id;

  LOG_API static unsigned __int64 m_crtId;

  LOG_API static tradery::Mutex m_mx;

  static unsigned __int64 getId() {
    tradery::Lock lock(m_mx);
    return m_crtId++;
  }

 public:
  LogEntryExit(Level level, char* name, const std::string& message)
      : m_level(level), m_name(name), m_message(message), m_id(getId()) {
    tradery::Log::xlog(m_level, const_cast<char*>(m_name.c_str()),
                       std::string()
                           << m_message << " - entry [" << m_id << "]");
  }

  ~LogEntryExit() {
    tradery::Log::xlog(m_level, const_cast<char*>(m_name.c_str()),
                       std::string()
                           << m_message << " - exit [" << m_id << "]");
  }
};

#define LOG_ENTRY_EXIT(level, message) \
  LogEntryExit ___logentryexit___(level, __FUNCTION__, message);
