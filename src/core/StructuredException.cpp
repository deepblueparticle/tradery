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

#include "structuredexception.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace std;

using namespace tradery;

static void my_translator(unsigned code, EXCEPTION_POINTERS *info) {
  switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:
      throw AccessViolationExceptionImpl(*info);
      break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      _clearfp();
      throw DivideByZeroExceptionImpl(*info);
      break;
    default:
      throw StructuredException(UNKNOWN_STRUCTURED_ERROR, *info);
      break;
  }
}

StructuredException::StructuredException(ErrorCode code,
                                         EXCEPTION_POINTERS const &info) throw()
    : CoreException(code) {
  EXCEPTION_RECORD const &exception = *(info.ExceptionRecord);
  address_ = exception.ExceptionAddress;
  // code_ = exception.ExceptionCode;
}

void StructuredException::install() throw() {
  int _cw;
  _cw = _controlfp(0, 0);

  // Set the exception masks OFF, turn exceptions on.
  int cwx = _cw & ~(EM_ZERODIVIDE);

  // Set the control word.
  _controlfp(cwx, MCW_EM);

  _set_se_translator(my_translator);
}

std::string StructuredException::what() const throw() {
  return "Unspecified Structured Exception";
}

void const *StructuredException::where() const throw() { return address_; }

//
//  AccessViolation::
//
AccessViolationExceptionImpl::AccessViolationExceptionImpl(
    EXCEPTION_POINTERS const &info) throw()
    : StructuredException(ACCESS_VIOLATION_ERROR, info) {}

std::string AccessViolationExceptionImpl::what() const throw() {
  return "Access violation exception";
}

//
//  DivideByZero::
//
DivideByZeroExceptionImpl::DivideByZeroExceptionImpl(
    EXCEPTION_POINTERS const &info) throw()
    : StructuredException(DIVIDE_BY_ZERO_ERROR, info) {}

std::string DivideByZeroExceptionImpl::what() const throw() {
  return "Divide by zero exception";
}
