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

using std::vector;

/**
 * Reads a list of symbols from a text file.
 * Symbols can be separeted by new line, space, tab, "," or ";".
 */

// TODO: make this thread safe in order to allow multiple systems to use the
// same iterator (when running the same system in multiple threads, and still
// want to run each symbol
// only once

class FileSymbol : public tradery::Symbol {
 private:
  const std::string& _path;

 public:
  FileSymbol(const std::string& symbol, const std::string& path)
      : Symbol(symbol), _path(path) {}

  const std::string& path() const { return _path; }
};

class FileSymbolsSource : public SymbolsSource {
 private:
  const std::string _fileName;

 public:
  FileSymbolsSource(const Info& info,
                    const std::string& fileName) throw(SymbolsSourceException)
      : _fileName(fileName), SymbolsSource(info) {
    try {
      StrListPtr s = getSymbols(fileName);
      for (StrList::const_iterator i = s->begin(); i != s->end(); i++)
        push_back((*i));
    } catch (const FileSymbolsParserException& e) {
      throw SymbolsSourceException(e.message());
    }
  }

  const std::string& fileName() const { return _fileName; }
  SymbolConstPtr makeSymbol(const_iterator& i) const {
    return SymbolConstPtr(new FileSymbol(*i, fileName()));
  }
};
