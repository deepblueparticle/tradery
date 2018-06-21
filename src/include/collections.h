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

class ArrayIndexNotFoundException : public CoreException {
 public:
  ArrayIndexNotFoundException(size_t index)
      : CoreException(ARRAY_INDEX_NOT_FOUND_EXCEPTION,
                      std::string("Index not found in array: ") << index) {}
};

template <class Value>
class ARRAY {
 private:
  std::vector<Value> _v;

 public:
  size_t count() const { return _v.size(); }
  size_t size() const { return _v.size(); }
  void add(const Value& v) { _v.push_back(v); }
  Value& operator[](size_t index) {
    if (index < _v.size())
      return _v[index];
    else
      throw ArrayIndexNotFoundException(index);
  }

  bool empty() const { return _v.empty(); }
  void clear() { _v.clear(); }
};

class DictionaryKeyNotFoundException : public CoreException {
 public:
  DictionaryKeyNotFoundException()
      : CoreException(DICTIONARY_KEY_NOT_FOUND_EXCEPTION,
                      "Key not found in dictionary collection") {}
};

#define Array(Value) ARRAY<Value>

template <class Key, class Value>
class DICTIONARY {
 private:
  typedef std::map<Key, Value> Map;

  mutable Array(Key) _keys;
  Map _m;
  mutable bool _clean;

 public:
  DICTIONARY() : _clean(true) {}

  bool add(const Key& key, const Value& value) {
    std::pair<Map::iterator, bool> i = _m.insert(Map::value_type(key, value));
    if (i.second) _clean = false;
    return i.second;
  }

  Value& operator[](const Key& key) {
    if (has(key))
      return _m[key];
    else
      throw DictionaryKeyNotFoundException();
  }

  bool has(const Key& key) { return _m.find(key) != _m.end(); }

  Array(Key) getKeys() const {
    if (!_clean) {
      _keys.clear();
      for (Map::const_iterator i = _m.begin(); i != _m.end(); i++) {
        _keys.add((*i).first);
      }
      _clean = true;
    }

    return _keys;
  }
};

#define Dictionary(Key, Value) DICTIONARY<Key, Value>
