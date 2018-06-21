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

class ErrorEventSinkImpl : public ErrorEventSink {
 private:
  std::queue<ErrorEvent> _errors;
  mutable Mutex _mutex;

 public:
  ErrorEventSinkImpl() {}

  virtual ~ErrorEventSinkImpl() {}
  virtual void push(const ErrorEvent& event) {
    Lock lock(_mutex);

    _errors.push(event);
  }

  virtual void pop() {
    Lock lock(_mutex);

    _errors.pop();
  }

  virtual bool empty() const {
    Lock lock(_mutex);

    return _errors.empty();
  }

  virtual size_t size() const {
    Lock lock(_mutex);

    return _errors.size();
  }

  virtual const ErrorEvent* front() const {
    Lock lock(_mutex);

    if (empty())
      return 0;
    else
      return &_errors.front();
  }
};