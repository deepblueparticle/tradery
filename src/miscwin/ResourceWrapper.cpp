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
#include <resourcewrapper.h>

ResourceWrapper::ResourceWrapper(const std::string& type, int id) {
  HRSRC res =
      ::FindResource(NULL, MAKEINTRESOURCE(id), tradery::s2ws(type).c_str());
  size = ::SizeofResource(NULL, res);
  hRes = ::LoadResource(NULL, res);
  this->data = ::LockResource(hRes);
}

ResourceWrapper::~ResourceWrapper() {
  if (hRes != NULL) {
    ::UnlockResource(hRes);
    ::FreeResource(hRes);
  }
}
