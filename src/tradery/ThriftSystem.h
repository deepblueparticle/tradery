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

#include <Tradery.h>

#define MACRO(x) "$(" #x ")"

typedef boost::shared_ptr<std::string> StrPtr;

class ThriftSystem : protected tradery_thrift_api::System {
 private:
  const UniqueId id;
  const std::string className;

 public:
  ThriftSystem(const tradery_thrift_api::System& system);
  ThriftSystem(const ThriftSystem& system);

  const std::string& getDescription() const { return __super::description; }
  const std::string& getName() const { return __super::name; }
  const std::string& getClassName() const { return className; }
  std::string getUUID() const { return id.toString(); }
  const std::string& getCode() const { return __super::code; }
  const std::string& getDbId() const { return __super::dbId; }

  const StrPtr generateClass() const;
  std::string generateInsert() const;
};

class ThriftSystems : public std::vector<ThriftSystem> {
 public:
  ThriftSystems(const std::vector<tradery_thrift_api::System>& systems);
};