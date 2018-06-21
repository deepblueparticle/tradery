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
#include "datasource.h"

using namespace tradery;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

class SymbolsIteratorImpl : public tradery::SymbolsIterator {
  friend SymbolsSource;

 private:
  const SymbolsSource::const_iterator _end;
  mutable Mutex _mutex;
  SymbolsSource::const_iterator _i;
  const SymbolsSource& _sls;

 private:
  SymbolsIteratorImpl(const SymbolsSource& sls)
      : _i(sls.begin()), _end(sls.end()), _sls(sls) {}

 public:
  /**
   * Gets the next symbol
   *
   * This method should be thread safe if it is inteded to be used from more
   * than one thread at a time.
   *
   * @return A pointer to a SymbolInfo object if next is available, or 0 if no
   * more symbols
   */
  virtual SymbolConstPtr getNext() {
    Lock lock(_mutex);
    return _i == _end ? SymbolConstPtr() : _sls.makeSymbol(_i++);
  }

  virtual void reset() {
    Lock lock(_mutex);
    _i = _sls.begin();
  }

  virtual SymbolConstPtr getFirst() {
    Lock lock(_mutex);
    reset();
    return getNext();
  }

  virtual SymbolConstPtr getCurrent() {
    Lock lock(_mutex);
    return _i == _end ? SymbolConstPtr() : _sls.makeSymbol(_i);
  }

  virtual bool hasMore() {
    Lock lock(_mutex);
    return _i != _end;
  }
};

tradery::SymbolsIterator* tradery::SymbolsSource::makeIterator() {
  SymbolsIterator* i = new SymbolsIteratorImpl(*this);
  _iterators.push_back(i);
  return i;
}

std::ostream& tradery::SymbolsSource::dump(std::ostream& os) const {
  std::ostringstream o;
  for (const_iterator i = begin(); i != end(); ++i) o << *i << ", ";
  o << std::endl;

  tradery::sprint(o, os);
  return os;
}
