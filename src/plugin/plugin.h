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

#include <hinstance.h>
#include <stringformat.h>

//#define ENABLE_LOG_PLUGIN

#if defined(ENABLE_LOG_PLUGIN)
#define LOG_PLUGIN(x, y) LOG(x, y)
#else
#define LOG_PLUGIN(x, y)
#endif

class PluginMethodException : public PluginException {
 private:
  const std::string _methodName;

 public:
  PluginMethodException(const std::string& PluginName,
                        const std::string& methodName)
      : _methodName(methodName),
        PluginException(PluginName, "Method exception") {}

  const std::string& getMethodName() const { return _methodName; }
};

class PluginInstanceException : public PluginException {
 public:
  PluginInstanceException(const std::string& pluginName)
      : PluginException(pluginName, "Plugin instance exception") {}
};

template <class T>
class PluginInstanceBase : protected HInstance {
 protected:
  typedef Plugin<T>* (*GET_PLUGIN)();
  Plugin<T>* _plugin;

 public:
  PluginInstanceBase(const std::string& path) : HInstance(path), _plugin(0) {}

  Plugin<T>* operator->() {
    assert(_plugin != 0);
    return _plugin;
  }

  Plugin<T>& operator*() {
    if (_plugin != 0)
      return *_plugin;
    else
      throw PluginInstanceException(path());
  }
};

/**
 * A generic Plugin instance
 * loads a specific plugin based on it's path (including file name)
 * calls the getPlugin method which each plugin must implement
 * if this method is not implemented, or the path doesn't point to a valid
 * module an exception is thrown
 */
template <class T>
class PluginInstance : public PluginInstanceBase<T> {
 public:
  // the complete path, including name and extension
  PluginInstance(const std::string& path) throw(PluginMethodException,
                                                PluginInstanceException) try
      : PluginInstanceBase
    <T>(path) {
      {
        try {
          try {
            LOG_PLUGIN(log_debug, "Candidate plugin: " << path);
            GET_PLUGIN getPlugin =
                reinterpret_cast<GET_PLUGIN>(getProcAddress(procName));
            LOG_PLUGIN(log_debug, "Plugin detected: " << path);
            _plugin = (*getPlugin)();
          } catch (const HInstanceMethodException& e) {
            LOG_PLUGIN(log_debug, "Likely not a plugin: "
                                      << e.path() << ", method \"" << e.name()
                                      << "\" was not found, last error: "
                                      << e.getLastError());
          }
        } catch (const HInstanceException& e) {
          LOG_PLUGIN(log_debug, "HInstanceException, path: "
                                    << e.path()
                                    << ", last error: " << e.getLastError());
          throw PluginException("", __super::path());
        } catch (const HInstanceMethodException& e) {
          LOG_PLUGIN(log_debug, "HInstanceMethodException caught");
          throw PluginMethodException(__super::path(), e.name());
        }
      }
    }
  catch (const HInstanceException& e) {
    throw PluginInstanceException(
        tradery::format("%1%, last error: %2%", path, e.getLastError()));
  }

  typedef void (*RELEASE_PLUGIN)();
  ~PluginInstance() {
    try {
      RELEASE_PLUGIN releasePlugin =
          reinterpret_cast<RELEASE_PLUGIN>(getProcAddress(releaseProcName));
      (*releasePlugin)();
    } catch (const HInstanceMethodException&) {
    }
  }

 private:
  static const std::string procName;
  static const std::string releaseProcName;
};

template <>
const std::string PluginInstance<SymbolsSource>::procName =
    "getSymbolsSourcePlugin";
template <>
const std::string PluginInstance<Runnable>::procName = "getRunnablePlugin";
template <>
const std::string PluginInstance<Slippage>::procName = "getSlippagePlugin";
template <>
const std::string PluginInstance<Commission>::procName = "getCommissionPlugin";
template <>
const std::string PluginInstance<DataSource>::procName = "getDataSourcePlugin";
template <>
const std::string PluginInstance<SignalHandler>::procName =
    "getSignalHandlerPlugin";

template <>
const std::string PluginInstance<SymbolsSource>::releaseProcName =
    "releaseSymbolsSourcePlugin";
template <>
const std::string PluginInstance<Runnable>::releaseProcName =
    "releaseRunnablePlugin";
template <>
const std::string PluginInstance<Slippage>::releaseProcName =
    "releaseSlippagePlugin";
template <>
const std::string PluginInstance<Commission>::releaseProcName =
    "releaseCommissionPlugin";
template <>
const std::string PluginInstance<DataSource>::releaseProcName =
    "releaseDataSourcePlugin";
template <>
const std::string PluginInstance<SignalHandler>::releaseProcName =
    "releaseSignalHandlerPlugin";
