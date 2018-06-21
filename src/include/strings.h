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

#include <string>
#include <algorithm>
#include <tchar.h>
#include <sstream>
#include <codecvt>

namespace tradery {

/**\defgroup String String related types
 * Definition of string related types
 * @{
 */

inline std::wstring s2ws(const std::string& str) {
  typedef std::codecvt_utf8<TCHAR> convert_typeX;
  std::wstring_convert<convert_typeX, TCHAR> converterX;

  return converterX.from_bytes(str);
}

inline std::string ws2s(const std::wstring& wstr) {
  typedef std::codecvt_utf8<wchar_t> convert_typeX;
  std::wstring_convert<convert_typeX, wchar_t> converterX;

  return converterX.to_bytes(wstr);
}

// transforms s in lower case
inline const std::string to_upper_case(const std::string& str) {
  std::string s(str);

  std::transform(s.begin(), s.end(), s.begin(), toupper);
  return s;
}

inline const std::string to_lower_case(const std::string& str) {
  std::string s(str);
  std::transform(s.begin(), s.end(), s.begin(), tolower);
  return s;
}

const std::string whiteSpaces(" \t\n\r");

// removes leading and trailing white space
inline const std::string trim(const std::string& str) {
  std::wstring::size_type start = str.find_first_not_of(whiteSpaces);
  std::wstring::size_type end = str.find_last_not_of(whiteSpaces);

  if (start == std::wstring::npos)
    // empty string
    return "";
  else if (end == std::wstring::npos)
    return str.substr(start);
  else
    return str.substr(start, end - start + 1);
}

inline bool isBlanc(const std::string& str) {
  return str.find_first_not_of(whiteSpaces) == std::wstring::npos;
}

inline const std::string addExtension(const std::string& fileName,
                                      const std::string& ext) {
  std::string result(fileName);
  if (!ext.empty()) result += std::string(ext[0] == '.' ? "" : ".") + ext;

  return result;
}

inline const std::string addFSlash(const std::string& str) {
  if (str.empty() || *str.rbegin() == '\\' || *str.rbegin() == '/')
    return str;
  else
    return str + '\\';
}

inline const std::string removeFSlash(const std::string& str) {
  if (str.empty() || *str.rbegin() != '\\')
    return str;
  else
    return str.substr(0, str.length() - 1);
}

inline const std::wstring addFSlash(const std::wstring& str) {
  return tradery::s2ws(addFSlash(tradery::ws2s(str)));
}

inline const std::string quote(const std::string& str) {
  return "\"" + str + "\"";
}

template <typename T>
std::string& operator<<(std::string& str, const T& t) {
  std::ostringstream os;
  os << t;
  str += os.str();
  return str;
}

template <typename T>
std::wstring& operator<<(std::wstring& str, const T& t) {
  std::wostringstream os;
  os << t;
  str += os.str();
  return str;
}

inline const std::string unescape(const std::string& str) {
  std::string unescaped;
  bool escape = false;

  for (unsigned int n = 0; n < str.length(); n++) {
    char c = str[n];

    switch (c) {
      case '\\':
        if (escape) {
          unescaped += c;
          escape = false;
        } else
          escape = true;
        break;
      case '*':
        if (escape) {
          unescaped += ' ';
          escape = false;
        } else
          unescaped += c;
        break;
      case 'n':
        if (escape) {
          unescaped += '\n';
          escape = false;
        } else
          unescaped += c;
        break;
      case 't':
        if (escape) {
          unescaped += '\t';
          escape = false;
        } else
          unescaped += c;
        break;
      case 'r':
        if (escape) {
          unescaped += '\r';
          escape = false;
        } else
          unescaped += c;
        break;
      default:
        unescaped += c;
        escape = false;
        break;
    }
  }
  return unescaped;
}

inline const std::string unescape(const char* str) {
  return unescape(std::string(str));
}

inline const std::string escape(const std::string& str) {
  std::string escaped;

  for (unsigned int n = 0; n < str.length(); n++) {
    switch (str[n]) {
      case '\\':
        escaped += "\\\\";
        break;
      case ' ':
        escaped += "\\*";
        break;
      case '\n':
        escaped += "\\n";
        break;
      case '\t':
        escaped += "\\t";
        break;
      case '\r':
        escaped += "\\r";
        break;
      default:
        escaped += str[n];
        break;
    }
  }
  return escaped;
}

// end string related classes
//@}
}  // namespace tradery