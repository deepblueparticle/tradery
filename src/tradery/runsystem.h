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

#include "service.h"
#include "SessionContext.h"

#define TRACE_RUNSYSTEM
//#define ofs std::cout

enum CoreErrorCode {
  success,
  build_error,
  system_run_error,
  process_run_error,
  cmd_line_error,
  config_error,
  mfc_init_error,
  unknown_error
};

class RunSystemException {
 private:
  CoreErrorCode _errorCode;
  const std::string _message;

 public:
  RunSystemException(CoreErrorCode errorCode, const std::string& message)
      : _errorCode(errorCode), _message(message) {}

  CoreErrorCode errorCode() const { return _errorCode; }
  const std::string& message() const { return _message; }
};

class RunSystem {
 private:
  SessionContextPtr _context;

 public:
  RunSystem(SessionContextPtr context);

  void run();

  ~RunSystem() {}
};
