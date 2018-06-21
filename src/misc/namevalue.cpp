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
#include <namevalue.h>

///////////////////////////////////////////////////
///
/// NameValueVector methods

using namespace tradery;

typedef tradery::NameValueMapBase Base;
typedef std::pair<Base::const_iterator, Base::const_iterator> BaseItPair;

MISC_API void NameValueMap::set(const NameValue& nameValue) {
  assert(!nameValue.name().empty());
  Base::size_type n = Base::count(nameValue.name());

  if (n == 0) {
    add(nameValue);
  } else if (n == 1) {
    Base::iterator i = Base::find(nameValue.name());
    assert(i != Base::end());

    *(i->second) = nameValue;
  } else {
    // can't set if there are multiple values with the same name for now
    assert(false);
  }
}

MISC_API void NameValueMap::add(const NameValue& nameValue) {
  add(nameValue.name(), nameValue.value());
}

MISC_API tradery::NameValue tradery::NameValueMap::operator[](
    unsigned int n) const {
  if (size() > 0 && n < size()) {
    NameValuePtr nameValue(m_vector[n]);

    return Base::count(nameValue->name()) > 1
               ? NameValue(nameValue->name() + "[]", nameValue->value())
               : *nameValue;
  } else
    return NameValue();
}

MISC_API void tradery::NameValueMap::push_back(const tradery::NameValue& nv) {
  add(nv);
}

MISC_API void NameValueMap::add(const std::string& name,
                                const std::string& value) {
  assert(!name.empty());

  NameValuePtr nameValue(new NameValue(name, value));

  m_vector.push_back(nameValue);
  __super::insert(Base::value_type(name, nameValue));
}

MISC_API tradery::StringPtr tradery::NameValueMap::toString(
    bool phpStyle) const {
  tradery::StringPtr str(new std::string());

  for (Base::const_iterator i = Base::begin(); i != Base::end();) {
    std::string name((*i).first);
    BaseItPair its = Base::equal_range(name);

    if (Base::count(name) > 1 && phpStyle) {
      for (i = its.first; i != its.second; ++i) {
        if (!str->empty()) (*str) += "&";

        (*str) += name + "[]=" + i->second->value();
      }
    } else {
      if (!str->empty()) (*str) += "&";

      (*str) += name + "=" + i->second->value();
      ++i;
    }
  }

  return str;
}

/*
typedef std::vector< NameValue > NameValueVectorBase;
MISC_API void tradery::NameValueVector::add( const std::wstring& name, const
std::wstring& value )
{
        assert( !name.empty() );

        NameValueVectorBase::push_back( NameValue( name, value ) );
}

MISC_API tradery::StringPtr NameValueVector::toString() const
{
        tradery::StringPtr str( new std::wstring() );

        for( __super::size_type n = 0; n < __super::size(); ++n )
        {
                if( n > 0 )
                        (*str) += "&";

                (*str) += *__super::operator[]( n ).toString();
        }

        return str;
}
*/
