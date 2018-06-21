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

using namespace tradery;

class Symbols {
 private:
  StrListPtr _symbols;

  bool _uniqueSymbolsPopulated;
  UniqueSymbolsSetPtr _uniqueSymbols;
  const bool _removeDuplicateSymbols;
  const SymbolTransformer* _st;

  mutable tradery::Mutex _mx;

 protected:
  Symbols(bool removeDuplicateSymbols, const SymbolTransformer* st = 0)
      : _removeDuplicateSymbols(removeDuplicateSymbols),
        _st(st),
        _symbols(new StrList()),
        _uniqueSymbols(new UniqueSymbolsSet()),
        _uniqueSymbolsPopulated(false) {}

  void parse(std::istream& is) {
    std::string str;

    do {
      std::getline(is, str);
      if (!str.empty()) parseLine(str);
    } while (!is.eof());
  }

  template <typename Container>
  void add(Container container) {
    for (Container::iterator i = container.begin(); i != container.end(); ++i) {
      // add the symbol only if the symbol didn't exist already in the set, and
      // if the _removeDuplicateSymbols flag is set
      if (!_removeDuplicateSymbols || _uniqueSymbols->add(*i))
        _symbols->push_back((_st == 0 ? *i : (*_st)(*i)));
    }
  }

  void parseLine(const std::string& str) {
    // TODO: allow for quoted strings in order to support spaces in names.
    if (str.at(0) == '#' ||
        str.length() > 1 && str.at(0) == '/' && str.at(1) == '/')
      return;

    Tokenizer tokens(str, ",; \t");

    add(tokens);
  }

 public:
  StrListPtr get() { return _symbols; }
  UniqueSymbolsSetPtr getUniqueSymbolsSet() {
    if (!_removeDuplicateSymbols && !_uniqueSymbolsPopulated) {
      for (StrList::const_iterator i = _symbols->begin(); i != _symbols->end();
           ++i)
        _uniqueSymbols->add(*i);

      _uniqueSymbolsPopulated = true;
    }
    return _uniqueSymbols;
  }
};

/**
 * Loads a lost of symbols from a text file
 *
 * Symbols separators are: ",; \t" and new line
 */
class FileSymbolsList : public Symbols {
 private:
  bool _removeDuplicateSymbols;

 public:
  /**
   * Constructor that takes as arguments the file name and a flag indicating
   * whether to eliminate duplicate symbols
   *
   * @param fileName file name
   * @param removeDuplicateSymbols
   *                 Indicates whether to eliminate duplicate symbols
   *
   * @exception FileSymbolsParserException
   */
  FileSymbolsList(
      const std::string& fileName, bool removeDuplicateSymbols,
      const SymbolTransformer* st = 0) throw(FileSymbolsParserException)
      : Symbols(removeDuplicateSymbols, st) {
    (*this)(fileName);
  }

  /**
   * Constructor that takes as argument a vector of filenames, and flag
   * indicating whether to remove duplicate symbols
   *
   * @param fileNames vector of file names
   * @param removeDuplicateSymbols
   *                  eliminate duplicate symbols if true
   *
   * @exception FileSymbolsParserException
   */
  FileSymbolsList(const StrVector& fileNames, bool removeDuplicateSymbols,
                  const SymbolTransformer* st = 0,
                  const tradery::StrList* extraSymbols =
                      0) throw(FileSymbolsParserException)
      : Symbols(removeDuplicateSymbols, st) {
    for (std::vector<std::string>::const_iterator i = fileNames.begin();
         i != fileNames.end(); i++)
      (*this)(*i);

    if (extraSymbols != 0) __super::add(*extraSymbols);
  }

  bool operator()(const std::string& fileName) {
    if (!fileName.empty()) {
      std::ifstream file(fileName.c_str());
      if (!file)
        throw FileSymbolsParserException(
            std::string("Could not open symbols file \"") << fileName << "\"");
      __super::parse(file);
      return true;
    } else {
      throw FileSymbolsParserException(
          std::string("Symbols file name is empty"));
    }
  }
};

void deleteSymbols(const StrList* list) { delete list; }

StrListPtr tradery::getSymbols(
    const std::string& fileName, bool removeDuplicateSymbols,
    const SymbolTransformer* st) throw(FileSymbolsParserException) {
  // the reason I'm using a deleter function is so the deletion happens in the
  // same module that created the list, or memory errors will occur.
  return FileSymbolsList(fileName, removeDuplicateSymbols, st).get();
}

StrListPtr tradery::getSymbols(
    const StrVector& fileNames, bool removeDuplicateSymbols,
    const SymbolTransformer* st,
    const tradery::StrList* extraSymbols) throw(FileSymbolsParserException) {
  // the reason I'm using a deleter function is so the deletion happens in the
  // same module that created the list, or memory errors will occur.
  return FileSymbolsList(fileNames, removeDuplicateSymbols, st, extraSymbols)
      .get();
  ;
}

UniqueSymbolsSetPtr tradery::getUniqueSymbols(const StrVector& fileNames) throw(
    FileSymbolsParserException) {
  return FileSymbolsList(fileNames, true).getUniqueSymbolsSet();
}

UniqueSymbolsSetPtr tradery::getUniqueSymbols(const std::string& file) {
  StrVector files;

  files.push_back(file);

  return getUniqueSymbols(files);
}

UniqueSymbolsSetPtr tradery::getUniqueSymbols(
    const std::string file1,
    const std::string& file2) throw(FileSymbolsParserException) {
  StrVector files;

  files.push_back(file1);
  files.push_back(file2);

  return tradery::getUniqueSymbols(files);
}
