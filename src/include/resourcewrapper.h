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
#include "miscwin.h"
#include <string.h>

class MISCWIN_API ResourceWrapper {
 private:
  HGLOBAL hRes = NULL;
  void* data = NULL;
  unsigned int size = 0;

 public:
  ResourceWrapper(const std::string& type, int id);
  virtual ~ResourceWrapper();
  void* getData() const { return this->data; }

  unsigned int getSize() { return size; }
};

#define TEXT_TYPE "TEXT"

class MISCWIN_API TextResource : public ResourceWrapper {
  boost::scoped_array<char> data;

 public:
  TextResource(int id)
      : ResourceWrapper(TEXT_TYPE, id), data(new char[__super::getSize() + 1]) {
    strncpy_s(data.get(), getSize() + 1,
              reinterpret_cast<const char*>(getData()), getSize());
  }

  operator const char*() const { return data.get(); }

  const char* get() const { return data.get(); }
};
