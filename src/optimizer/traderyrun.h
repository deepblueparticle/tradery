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

#include <differential_evolution.hpp>
#include <de_types.hpp>

class TraderyRunException : public std::exception {
 public:
  TraderyRunException(const std::wstring& message)
      : std::exception(message.c_str()) {}
};

class TraderyRun {
  TraderyAuthTokenPtr m_authToken;
  ParametersPtr m_parameters;
  VariablesNamesPtr m_variablesNames;
  TraderySessionEventListener& m_listener;
  tradery::StringPtr m_symbolsList;
  SystemIdsPtr m_systemIds;
  OptionsPtr m_options;
  PositionSizingPtr m_positionSizing;
  const std::wstring m_name;
  tradery::Date m_startTradesDate;

  NullTraderySessionEventListener m_nullListener;

 public:
  TraderyRun(const std::wstring& name, TraderyCredentialsPtr credentials,
             ParametersPtr parameters, VariablesNamesPtr vn,
             TraderySessionEventListener& listener,
             tradery::StringPtr symbolsList, SystemIdsPtr systemIds,
             OptionsPtr options, PositionSizingPtr positionSizing,
             tradery::Date startTradesDate)
      : m_parameters(parameters),
        m_variablesNames(vn),
        m_listener(listener),
        m_authToken(boost::make_shared<TraderyAuthToken>(*credentials)),
        m_symbolsList(symbolsList),
        m_systemIds(systemIds),
        m_options(options),
        m_positionSizing(positionSizing),
        m_name(name),
        m_startTradesDate(startTradesDate) {
    // varables names is allowed to be null, in which case there are no
    // variables and default values are used by the system
    assert(credentials);
    assert(parameters);
    assert(symbolsList);
    assert(systemIds);
    assert(options);
  }

  typedef boost::tuple<PerformanceStatsPtr, ExternalVarsPtr> RunInfo;
  RunInfo run(de::DVectorPtr vars) {
    try {
      //			std::cout << std::endl << _T( "Setting variables
      // for " ) << m_name;

      ExternalVarsPtr ev(
          boost::make_shared<ExternalVars>(m_variablesNames, vars));

      OptimizerTraderySession ts(m_name, m_authToken.get(), m_listener,
                                 m_nullListener);

      ts.start(m_systemIds, m_symbolsList, m_parameters, m_positionSizing, ev,
               m_options, m_startTradesDate);

      ts.waitForThread();

      if (ts.success())
        return RunInfo(ts.performanceStats(), ev);
      else {
        m_listener.errorEvent(new TraderySessionErrorEvent("Session failed"),
                              m_name);
        throw TraderyRunException("Tradery session failed");
      }
    } catch (const ExternalVarsException&) {
      m_listener.errorEvent(
          new TraderySessionErrorEvent("External vars exception"), m_name);
      throw TraderyRunException("External vars error");
    } catch (const StatsException&) {
      m_listener.errorEvent(new TraderySessionErrorEvent("Stats exception"),
                            m_name);
      throw TraderyRunException("Stats error");
    }
  }

  const std::wstring& name() const { return m_name; }
};
