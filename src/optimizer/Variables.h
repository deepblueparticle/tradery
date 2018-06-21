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

#include <boost/make_shared.hpp>
#include <de_types.hpp>
#include <de_constraints.hpp>
#include <tokenizer.h>
#include <stringformat.h>
#include <strings.h>
#include <boost/foreach.hpp>

// variables classes used in the command line
class VariableException : public std::exception {
 public:
  VariableException(const std::wstring& message)
      : std::exception(message.c_str()) {}
};

class Variable {
 private:
  std::wstring m_name;
  de::constraint_ptr m_constraint;

 public:
  // variable format: "name;type;min;max", ex: $profitTarget;real;0.5;20
  Variable(const std::wstring& str) {
    Tokenizer tokens(str, ";,");

    if (tokens.size() != 4)
      throw VariableException(
          tradery::format("wrong variable format in \"%1%\"", str));

    m_name = tradery::trim(tokens[0]);
    const std::wstring type(tradery::trim(tokens[1]));
    double _min(_tstof(tradery::trim(tokens[2]).c_str()));
    double _max(_tstof(tradery::trim(tokens[3]).c_str()));

    m_constraint = strToConstraint(type, _min, _max);
  }

  const std::wstring& name() const { return m_name; }
  de::constraint_ptr constraint() const { return m_constraint; }

 private:
  de::constraint_ptr strToConstraint(const std::wstring& type, double min,
                                     double max) {
    if (tradery::to_lower_case(type) == "real")
      return boost::make_shared<de::real_constraint>(min, max);
    else if (tradery::to_lower_case(type) == "int" ||
             tradery::to_lower_case(type) == "integer")
      return boost::make_shared<de::int_constraint>(min, max);
    else
      throw VariableException(
          tradery::format("unknown variable type \"%1%\"", type));
  }
};

typedef std::vector<Variable> VariablesBase;

class Variables : public VariablesBase {
 public:
  Variables() {}

  Variables(const std::vector<std::wstring>& strs) {
    BOOST_FOREACH (std::wstring str, strs) {
      __super::push_back(Variable(str));
    }
  }
};

typedef std::vector<std::wstring> VariablesNamesBase;

class VariablesNames : public VariablesNamesBase {
 public:
  VariablesNames(const Variables& variables) {
    BOOST_FOREACH (const Variable& variable, variables) {
      __super::push_back(variable.name());
    }
  }

  VariablesNames() {}
};

typedef boost::shared_ptr<VariablesNames> VariablesNamesPtr;

class ExternalVarsException {};

// class that pairs variables names used by Tradery and values
class ExternalVars {
 private:
  VariablesNamesPtr m_variablesNames;
  de::DVectorPtr m_values;

 public:
  ExternalVars(VariablesNamesPtr variablesNames, de::DVectorPtr values)
      : m_variablesNames(variablesNames), m_values(values) {
    assert(variablesNames);
    assert(values);
  }

  tradery::StringPtr toJsonString() const {
    if (m_values->size() >= m_variablesNames->size()) {
      Json::Value json;

      for (size_t i = 0; i < m_variablesNames->size(); ++i) {
        const std::wstring& name((*m_variablesNames)[i]);
        json[name] = (double)(*m_values)[i];
      }

      Json::StyledWriter writer;

      return tradery::StringPtr(new std::wstring(writer.write(json)));
    } else
      throw ExternalVarsException();
  }

  tradery::StringPtr toString() const {
    tradery::StringPtr str(new std::wstring());

    if (m_values->size() >= m_variablesNames->size()) {
      for (size_t i = 0; i < m_variablesNames->size(); ++i) {
        const std::wstring& name((*m_variablesNames)[i]);
        (*str) += tradery::format("%1%: %2%; ", name, (double)(*m_values)[i]);
      }

      return str;
    } else
      throw ExternalVarsException();
  }
};

typedef boost::shared_ptr<ExternalVars> ExternalVarsPtr;

#define VARS_COUNT 20
class VariablesConstraints : public de::constraints {
 public:
  VariablesConstraints(const Variables& vars, size_t size = VARS_COUNT)
      : constraints(size, -1e6, 1e6) {
    for (size_t i = 0; i < vars.size(); ++i) {
      if (i < size)
        __super::at(i) = vars[i].constraint();
      else
        __super::push_back(vars[i].constraint());
    }
  }
};
