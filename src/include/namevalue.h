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

//#pragma message( "including namevalue.h" )

#pragma warning(disable : 4800 4275 4251 4244 4003)

#include <map>
#include "strings.h"
#include "sharedptr.h"

#include <boost/shared_ptr.hpp>

namespace tradery {
typedef std::pair<std::string, std::string> NameValueBase;

class NameValue : public NameValueBase {
 public:
  NameValue() {}

  template <typename T>
  NameValue(const std::string& name, const T& value)
      : NameValueBase(name, std::string() << value) {
    assert(!name.empty());
  }

  /*	NameValue( const std::wstring& name, const std::wstring& value )
          : std::pair< std::wstring, std::wstring >( name, value )
          {
                  assert( !name.empty() );
          }

          NameValue( const std::wstring& name, double value )
          {
                  assert( !name.empty() );
                  __super::first = name;
                  std::wostringstream os;
                  os << value;
                  __super::second = os.str();

          }
  */
  const std::string& name() const { return __super::first; }
  const std::string& value() const { return __super::second; }

  operator bool() const { return !NameValueBase::first.empty(); }

  tradery::StringPtr tradery::NameValue::toString() const {
    tradery::StringPtr str(new std::string());
    (*str) += name() + "=" + value();
    return str;
  }
};

typedef boost::shared_ptr<NameValue> NameValuePtr;

typedef std::multimap<std::string, NameValuePtr> NameValueMapBase;

class MISC_API NameValueMap : public NameValueMapBase {
 private:
  typedef std::vector<NameValueMapBase::const_iterator> NameValueVector;
  std::vector<NameValuePtr> m_vector;

 public:
  template <typename T>
  void set(const std::wstring& name, const T& value) {
    set(NameValue(name, value));
  }
  void set(const NameValue& nameValue);
  void add(const NameValue& nameValue);
  NameValue operator[](unsigned int n) const;
  void push_back(const tradery::NameValue& nv);
  template <typename T>
  void add(const std::string& name, const T& value) {
    add(NameValue(name, value));
  }
  void add(const std::string& name, const std::string& value);

  // if php style is true, if there are multiple values with the same name
  // they will be represented as "name[]=v1&name[]=v2&...". This allows a php
  // script to automatically generate an array in the receiving variable
  tradery::StringPtr toString(bool phpStyle = true) const;
};

}  // namespace tradery
