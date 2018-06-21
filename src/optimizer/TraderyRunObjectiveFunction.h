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

#include "traderyrun.h"

class TraderyRunDEObjectiveFunction : public TraderyRun {
 private:
  size_t m_statToOptimize;
  StatsValue::ValueType m_statGroupToOptimize;

 public:
  TraderyRunDEObjectiveFunction(
      const std::wstring& name, TraderyCredentialsPtr credentials,
      ParametersPtr parameters, VariablesNamesPtr vn,
      TraderySessionEventListener& listener, tradery::StringPtr symbolsList,
      SystemIdsPtr systemIds, OptionsPtr options,
      PositionSizingPtr positionSizing, size_t statToOptimize,
      StatsValue::ValueType statGroupToOptimize, tradery::Date startTradesDate)
      : TraderyRun(name, credentials, parameters, vn, listener, symbolsList,
                   systemIds, options, positionSizing, startTradesDate),
        m_statToOptimize(statToOptimize),
        m_statGroupToOptimize(statGroupToOptimize) {}

  virtual double operator()(de::DVectorPtr vars) {
    try {
      TraderyRun::RunInfo runInfo(TraderyRun::run(vars));

      PerformanceStatsPtr stats(runInfo.get<0>());
      ExternalVarsPtr ev(runInfo.get<1>());
      assert(stats);
      assert(ev);

      double value(
          stats->getStatValue(m_statToOptimize, m_statGroupToOptimize));
      std::cout << std::endl
                << "[" << TraderyRun::name() << "] " << value
                << ", vars: " << (*ev->toString());
      return value;

    } catch (const TraderyRunException& e) {
      throw de::objective_function_exception(e.what());
    }
  }
};

typedef boost::shared_ptr<TraderyRunDEObjectiveFunction>
    TraderyRunDEObjectiveFunctionPtr;

class TraderyRunDEObjectiveFunctionFactory
    : public de::objective_function_factory<TraderyRunDEObjectiveFunction> {
 private:
  TraderyCredentialsPtr m_credentials;
  ParametersPtr m_parameters;
  VariablesNamesPtr m_variablesNames;
  TraderySessionEventListener& m_listener;
  tradery::StringPtr m_symbolsList;
  SystemIdsPtr m_systemIds;
  OptionsPtr m_options;
  PositionSizingPtr m_positionSizing;
  tradery::Date m_startTradesDate;
  size_t m_statToOptimize;
  StatsValue::ValueType m_statGroupToOptimize;
  std::wstring m_diagName;

 public:
  TraderyRunDEObjectiveFunctionFactory(
      const std::wstring& diagName, TraderyCredentialsPtr credentials,
      ParametersPtr parameters, VariablesNamesPtr vn,
      TraderySessionEventListener& listener, tradery::StringPtr symbolsList,
      SystemIdsPtr systemIds, OptionsPtr options,
      PositionSizingPtr positionSizing, size_t statToOptimize,
      StatsValue::ValueType statGroupToOptimize, tradery::Date startTradesDate)
      : m_credentials(credentials),
        m_parameters(parameters),
        m_variablesNames(vn),
        m_listener(listener),
        m_symbolsList(symbolsList),
        m_systemIds(systemIds),
        m_options(options),
        m_positionSizing(positionSizing),
        m_startTradesDate(startTradesDate),
        m_statToOptimize(statToOptimize),
        m_statGroupToOptimize(statGroupToOptimize),
        m_diagName(diagName) {
    assert(credentials);
  }

  TraderyRunDEObjectiveFunctionPtr make() {
    // will create a new token for each instance (for parallel runs the token
    // must be distinct)
    return TraderyRunDEObjectiveFunctionPtr(new TraderyRunDEObjectiveFunction(
        m_diagName, boost::make_shared<TraderyAuthToken>(*m_credentials),
        m_parameters, m_variablesNames, m_listener, m_symbolsList, m_systemIds,
        m_options, m_positionSizing, m_statToOptimize, m_statGroupToOptimize,
        m_startTradesDate));
  }
};

typedef boost::shared_ptr<TraderyRunDEObjectiveFunctionFactory>
    TraderyRunDEObjectiveFunctionFactoryPtr;
