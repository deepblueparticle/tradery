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
#include <hinstance.h>

HInstance::HInstance(const std::string& fileName) throw(HInstanceException)
    : _path(fileName), _hInstance(init()) {}

HInstance::~HInstance() {
  LOG(log_debug, "freeing library \"" << _path << "\"");
  if (_hInstance != 0) AfxFreeLibrary(_hInstance);
}

HINSTANCE HInstance::init() {
  std::wstring wpath(s2ws(_path));
  HINSTANCE h = AfxLoadLibrary(wpath.c_str());
  if (0 != h) {
    LOG(log_debug, "loaded library \"" << _path << "\"");
    return h;
  } else {
    LOG(log_error, "error loading library \""
                       << _path << "\", last error: " << GetLastError());
    throw HInstanceException(_path);
  }
}

FARPROC HInstance::getProcAddress(const std::string& procName) const
    throw(HInstanceMethodException) {
  assert(_hInstance != 0);
  if (procName == "releaseDataSourcePlugin") {
    int n = 0;
    ++n;
  }
  FARPROC proc = GetProcAddress(_hInstance, procName.c_str());

  if (proc != 0) {
    LOG(log_debug,
        "got proc \"" << procName << "\" address from \"" << _path << "\"");
    return proc;
  } else {
    LOG(log_error, "error getting proc \""
                       << procName << "\" address from \"" << _path
                       << "\", last error: " << GetLastError());
    throw HInstanceMethodException(path(), procName);
  }
}
