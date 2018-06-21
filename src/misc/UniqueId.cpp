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

#include "StdAfx.h"
#include "misc.h"

using namespace ::boost::uuids;

uuid generate() {
  boost::uuids::random_generator gen;
  return gen();
}

namespace tradery {
UniqueId::UniqueId() { id = to_string(::generate()); }

UniqueId::UniqueId(const std::string& i) : id(to_upper_case(i)) {}

UniqueId::UniqueId(const char* i) throw(UniqueIdException) {
  uuid u;
  std::stringstream ss;

  ss << i;
  ss >> u;
  id = to_upper_case(to_string(u));
}

UniqueId::UniqueId(const TCHAR* id) throw(UniqueIdException) {
  (*this) = ws2s(id).c_str();
}

UniqueId::UniqueId(const UniqueId& id) : id(to_upper_case(id.id)) {}

UniqueId::operator const std::string() const { return id; }

UniqueId::operator const std::wstring() const { return s2ws(id); }

std::string UniqueId::toString() const { return id; }

bool UniqueId::operator<(const UniqueId& other) const { return id < other.id; }

bool UniqueId::operator==(const UniqueId& other) const {
  return id == other.id;
}

const UniqueId& UniqueId::operator=(const UniqueId& other) {
  if (this == &other) {
    return *this;
  }

  return *this;
}

UniqueId UniqueId::generate() { return UniqueId(); }
}  // namespace tradery