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

#include "ThriftSystem.h"
#include "resource.h"

#define SYSTEM_CLASS_PREFIX std::string("System_")

ThriftSystem::ThriftSystem(const tradery_thrift_api::System& system)
    : tradery_thrift_api::System(system),
      className(SYSTEM_CLASS_PREFIX +
                boost::replace_all_copy(id.toString(), "-", "_")) {}

ThriftSystem::ThriftSystem(const ThriftSystem& system)
    : tradery_thrift_api::System(system),
      id(system.getUUID()),
      className(system.className) {}

const StrPtr ThriftSystem::generateClass() const {
  static const TextResource systemTemplate(IDR_SYSTEM);

  StrPtr code = boost::make_shared<std::string>();

  *code = systemTemplate;

  std::string zz = MACRO(SYSTEM_UUID);

  boost::replace_all(*code, MACRO(SYSTEM_UUID), getUUID());
  boost::replace_all(*code, MACRO(SYSTEM_NAME), getName());
  boost::replace_all(*code, MACRO(SYSTEM_DESCRIPTION), getDescription());
  boost::replace_all(*code, MACRO(SYSTEM_CLASS_NAME), getClassName());
  boost::replace_all(*code, MACRO(SYSTEM_DB_ID), getDbId());

  return code;
}

std::string ThriftSystem::generateInsert() const {
  std::stringstream ss;
  ss << "\tinsert< " + getClassName() + " >();\\" << std::endl;
  return ss.str();
}

ThriftSystems::ThriftSystems(
    const std::vector<tradery_thrift_api::System>& systems) {
  std::transform(systems.begin(), systems.end(), std::back_inserter(*this),
                 [](const tradery_thrift_api::System& system) {
                   ThriftSystem s(system);
                   return s;
                 });
}
