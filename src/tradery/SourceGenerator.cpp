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
#include "SourceGenerator.h"
#include <resourcewrapper.h>
#include "Resource.h"
#include "ThriftSystem.h"

/*
sample file

#pragma once
using namespace tradery;


<<< HEADER >>>

//*********************************************
#undef SYSTEM_ID
#define SYSTEM_ID "9259a413-071b-9e23-0bad-c98562716758"


#pragma message( "#systemName=SMA System with Pullback" )
#pragma message( "#className=System_9259a413_071b_9e23_0bad_c98562716758" )
class System_9259a413_071b_9e23_0bad_c98562716758 : public BarSystem<
System_9259a413_071b_9e23_0bad_c98562716758 >
{
public: System_9259a413_071b_9e23_0bad_c98562716758(const std::vector< t_string
>* params = 0) : BarSystem< System_9259a413_071b_9e23_0bad_c98562716758
>(Info("9259a413-071b-9e23-0bad-c98562716758", _TT("SMA System with Pullback"),
_TT("Inspired by OscillatorPullBack by .toe in Wealthlab.")), "385")
{}
private:
#include <9259a413-071b-9e23-0bad-c98562716758.h>
};

#define PLUGIN_INIT_METHOD \
virtual void init() \
{ \
                insert< System_9259a413_071b_9e23_0bad_c98562716758 >(); \
}

<< < FOOTER >> >
*/

SourceGenerator::SourceGenerator(const ThriftSystems& systems)
    : systems(systems) {}

SourceGenerator::~SourceGenerator() {}

boost::shared_ptr<std::string> SourceGenerator::generate() {
  static const TextResource header(IDR_HEADER);
  LOG(log_info, "----------------- header:\n" << header);
  static const TextResource footer(IDR_FOOTER);
  LOG(log_info, "----------------- footer:\n" << footer);
  static const TextResource init(IDR_INIT);
  LOG(log_info, "----------------- init:\n" << init);

  boost::shared_ptr<std::string> code = boost::make_shared<std::string>();
  std::string inserts;

  *code += header;

  //	std::cout << "-----------------" << std::endl << *code;

  for (ThriftSystems::const_iterator i = systems.begin(); i != systems.end();
       ++i) {
    *code += *(i->generateClass());
    inserts += i->generateInsert();
  }
  // std::cout << "-----------------" << std::endl << *code;

  *code += boost::replace_all_copy(std::string(init), MACRO(INSERTS), inserts);
  // std::cout << "-----------------" << std::endl << *code;
  *code += footer;
  // std::cout << "-----------------" << std::endl << *code;

  return code;
}
