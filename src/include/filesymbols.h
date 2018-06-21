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

///
/// @cond
///
namespace tradery {
///
/// @endcond
///

/**
 * Template class - represents a class (generally a collection) to which we can
 * add elements
 */
template <class T>
class Addable {
 public:
  virtual ~Addable() {}

  virtual void add(const T& t) = 0;
};

class FileSymbolsParserException {
 private:
  const std::string _message;

 public:
  FileSymbolsParserException(const std::string& message) : _message(message) {}

  const std::string& message() const { return _message; }
};

class SymbolTransformer {
 public:
  virtual ~SymbolTransformer() {}

  virtual std::string operator()(const std::string& symbol) const = 0;
};

typedef std::set<std::string> UniqueSymbolsSetBase;

class UniqueSymbolsSet : public UniqueSymbolsSetBase {
 private:
  tradery::Mutex _mx;

 public:
  bool add(const std::string& symbol) {
    return __super::insert(tradery::to_lower_case(symbol)).second;
  }

  bool hasSymbol(const std::string& symbol) const {
    return __super::find(tradery::to_lower_case(symbol)) != __super::end();
  }

  const UniqueSymbolsSet& operator+=(const UniqueSymbolsSet& symbols) {
    __super::insert(symbols.begin(), symbols.end());

    return *this;
  }
};

typedef tradery::ManagedPtr<UniqueSymbolsSet> UniqueSymbolsSetPtr;

StrListPtr getSymbols(
    const std::string& fileName, bool removeDuplcateSymbols = false,
    const SymbolTransformer* st = 0) throw(FileSymbolsParserException);
StrListPtr getSymbols(
    const StrVector& fileNames, bool removeDuplcateSymbols = false,
    const SymbolTransformer* st = 0,
    const tradery::StrList* extraSymbols = 0) throw(FileSymbolsParserException);

UniqueSymbolsSetPtr getUniqueSymbols(const StrVector& fileNames) throw(
    FileSymbolsParserException);
UniqueSymbolsSetPtr getUniqueSymbols(const std::string& file) throw(
    FileSymbolsParserException);
UniqueSymbolsSetPtr getUniqueSymbols(
    const std::string file1,
    const std::string& file2) throw(FileSymbolsParserException);

}  // namespace tradery