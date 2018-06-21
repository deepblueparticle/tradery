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

/**
 * Base class for structured exceptions
 *
 * This class is used by specific exceptions that are not handled by the C++
 * exception system, mainly division by 0.
 *
 * This class is not relevant from an API standpoint, and it would have been
 * hidden normally, but because of certain implementation issues related to
 * structured exceptions, this was not possible. A user should only call methods
 * of its base class CoreExceptions.
 *
 * @see AccessViolationException
 * @see DivideByZeroException
 * @see CoreException
 */
class StructuredException : public CoreException {
 public:
  StructuredException(ErrorCode code, EXCEPTION_POINTERS const &) throw();
  static void install() throw();
  virtual std::string what() const throw();
  void const *where() const throw();

 private:
  void const *address_;
  // unsigned code_;
};

/**
 * Structured exception thrown in case of an access violation exception
 *
 * This exception is never thrown in simlib presently and it is here for future
 * use only
 *
 * @see CoreException
 * @see ErrorCode
 */
class AccessViolationExceptionImpl : public StructuredException,
                                     public AccessViolationException {
 public:
  AccessViolationExceptionImpl(EXCEPTION_POINTERS const &) throw();
  virtual std::string what() const throw();

  virtual std::string message() const { return what(); }
};

/**
 * Thrown in case of a divizion by zero error
 *
 * @see CoreException
 * @see ErrorCode
 */
class DivideByZeroExceptionImpl : public StructuredException,
                                  public DivideByZeroException {
 public:
  DivideByZeroExceptionImpl(EXCEPTION_POINTERS const &) throw();
  virtual std::string what() const throw();
  virtual std::string message() const { return what(); }
};
