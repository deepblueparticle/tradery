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

class HInstanceException {
 private:
  DWORD _lastError;
  const std::string _path;

 public:
  HInstanceException(const std::string& path)
      : _lastError(::GetLastError()), _path(path) {}

  DWORD getLastError() const { return _lastError; }
  const std::string& path() const { return _path; }
};

class HInstanceMethodException : public HInstanceException {
 private:
  const std::string _name;

 public:
  HInstanceMethodException(const std::string& path, const std::string& name)
      : _name(name), HInstanceException(path) {}

  const std::string& name() const { return _name; }
};

// a library instance class that deals correctly with loading/freeing the
// library
class HInstance {
 private:
  const std::string _path;
  HINSTANCE _hInstance;

 protected:
  HInstance(const std::string& fileName) throw(HInstanceException);
  ~HInstance();

  bool valid() const { return _hInstance != 0; }

  FARPROC getProcAddress(const std::string& procName) const
      throw(HInstanceMethodException);

 private:
  HINSTANCE init();

 public:
  const std::string& path() const { return _path; }
};