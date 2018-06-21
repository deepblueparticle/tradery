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
#include <vector>
#include <tchar.h>
#include <assert.h>

#include <misc.h>
#include <exceptions.h>

/* @cond */
namespace tradery {
/* @endcond */

#ifdef MISCWIN_EXPORTS
#define MISCWIN_API __declspec(dllexport)
#else
#define MISCWIN_API __declspec(dllimport)
#endif

/* @cond */
// registry related APIs

#ifdef _UNICODE
#if !defined(UNICODE)
#define UNICODE
#endif
#endif

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

using namespace std;

#include <memory>
#include <shlobj.h>

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE 0x0040
#endif

/* @endcond */

///////////////////////////////////
// makes a file name from its components. Takes into account existing or non
// existing \ or / or . characters
MISCWIN_API const std::wstring makeFileName(const std::wstring& path,
                                            const std::wstring& name,
                                            const std::wstring& ext);
MISCWIN_API void autoUpdate();
MISCWIN_API void autoUpdateSilent(const std::wstring& author,
                                  const std::wstring& programName);

class RunProcessException {
 private:
  DWORD _errorCode;
  const std::string _processFileName;
  const std::string _cmdLine;

 public:
  RunProcessException(const std::string& processFileName,
                      const std::string& cmdLine, DWORD errorCode)
      : _errorCode(errorCode) {}

  const DWORD errorCode() const { return _errorCode; }

  const std::string& processFileName() const { return _processFileName; }
  const std::string& cmdLine() const { return _cmdLine; }

  const std::string message() const {
    std::ostringstream o;

    o << "Attempting to run \"" << _processFileName << "\" with command line "
      << _cmdLine << " has failed with error " << _errorCode;
    return o.str();
  }
};

class RunProcessResult {
 private:
  bool _timeout;
  DWORD _exitCode;

 public:
  RunProcessResult(bool timeout, DWORD exitCode)
      : _timeout(timeout), _exitCode(exitCode) {}

  bool timeout() const { return _timeout; }
  DWORD exitCode() const { return _exitCode; }
};

typedef const std::vector<std::pair<std::string, std::string> > StrPairVector;

class Environment {
 private:
  mutable bool _valid;
  mutable std::string _envStr;

  StrVector _envStrings;

 public:
  Environment() {}

  Environment(const Environment& env) {
    _envStrings = env._envStrings;
    _valid = env._valid;
    _envStr = env._envStr;
  }

  Environment(const StrPairVector& env) : _valid(false) {
    for (StrPairVector::const_iterator i = env.begin(); i != env.end(); i++)
      add((*i).first, (*i).second);
  }

  Environment(const StrVector& env) : _valid(false) { _envStrings = env; }

  void add(LPCSTR name, LPCSTR value) {
    add(std::string(name), std::string(value));
  }

  void add(const std::string& name, const std::string& value) {
    _valid = false;
    _envStrings.push_back(name + '=' + value);
  }

  operator LPVOID() const {
    return _envStr.empty() && _envStrings.empty() ? 0 : (LPVOID)get().data();
  }

  const std::string& get() const {
    if (!_valid) {
      for (StrVector::const_iterator i = _envStrings.begin();
           i != _envStrings.end(); i++)
        _envStr += *i + '\0';

      _envStr += '\0';
      _valid = true;
    }

    return _envStr;
  }

  const std::string toString() const {
    std::string str;
    for (StrVector::const_iterator i = _envStrings.begin();
         i != _envStrings.end(); i++)
      str += *i + "\n";

    return str;
  }
};

typedef tradery::ManagedPtr<Environment> EnvironmentPtr;

MISCWIN_API const RunProcessResult
runProcess(const std::string& processFileName, const std::string& cmdLine,
           bool waitForProcess, const std::string* startingDirectory = 0,
           const Environment& env = Environment(),
           unsigned long timeout = 0) throw(RunProcessException);
MISCWIN_API const RunProcessResult
runProcessAsUser(const std::string& userName, const std::string& domain,
                 const std::string& password,
                 const std::string& processFileName, const std::string& cmdLine,
                 bool waitForProcess, const std::string* startingDirectory = 0,
                 unsigned long timeout = 0) throw(RunProcessException);

// Sent to parent window when user clicked on the checkbox of an item:
// wParam: The item index in the list ctrl
// lParam: The mouse event type(WM_LBUTTONDOWN, WM_RBUTTONDOWN, etc) which
// generated this event. Note: This message is not sent when the checkbox states
// were altered programmatically
//       by calling "SetItem", it is only sent when the user "physically"
//       clicked the checkbox using mouse or joystick etc.
#define WM_ON_CHKBOX (WM_APP + 10000)

// Sent to parent window when a column of items were sorted
// wParam: The column index
// lParam: The sort method, either 0(descending) or 1(ascending)
#define WM_ITEM_SORTED (WM_APP + 10001)

// Sent to parent window when an item text editing was committed
// wParam: The item index
// lParam: The column index
#define WM_EDIT_COMMITTED (WM_APP + 10002)

// Checkbox styles.
#define RC_CHKBOX_NONE 0    // No checkbox displayed
#define RC_CHKBOX_NORMAL 1  // Normal, multiple check allowed
#define RC_CHKBOX_SINGLE 2  // Single check only
#define RC_CHKBOX_DISABLED \
  3  // Disabled, cannot be checked/unchecked by user input,
     // but can be by your code.

// Item state flags for selection, deletion, etc.
// Multiple flags can be combined together using the bit-or operator.
// Note: If RC_ITEM_ALL is set, all other flags are ignored
#define RC_ITEM_NONE 0x0000        // Void, indicates invalid items only
#define RC_ITEM_ALL 0x0001         // All items regardless of states
#define RC_ITEM_SELECTED 0x0002    // Selected items
#define RC_ITEM_UNSELECTED 0x0004  // Unselected items
#define RC_ITEM_CHECKED 0x0008     // Checked items
#define RC_ITEM_UNCHECKED 0x0010   // Unchecked items
#define RC_ITEM_FOCUSED 0x0020     // Focused item
#define RC_ITEM_UNFOCUSED 0x0040   // Unfocused items

// Item inverting types
#define RC_INVERT_SELECTION 0  // Invert item selection
#define RC_INVERT_CHECKMARK 1  // Invert item check mark

// Removes any custom color from item text and item backgroun
#define COLOR_INVALID 0xffffffff

//////////////////////////////////////////////////////////////////////////
// The CReportCtrl Class Definition
//////////////////////////////////////////////////////////////////////////

//******************
// debug ostream
#include <Windows.h>
#include <ostream>
#include <sstream>
#include <string>

template <class CharT, class TraitsT = std::char_traits<CharT> >
class basic_debugbuf : public std::basic_stringbuf<CharT, TraitsT> {
 public:
  virtual ~basic_debugbuf() { sync(); }

 protected:
  int sync() {
    output_debug_string(str().c_str());
    str(std::basic_string<CharT>());  // Clear the string buffer

    return 0;
  }

  void output_debug_string(const CharT* text) {}
};

template <>
void basic_debugbuf<char>::output_debug_string(const char* text) {
  ::OutputDebugStringA(text);
}

template <>
void basic_debugbuf<wchar_t>::output_debug_string(const wchar_t* text) {
  ::OutputDebugStringW(text);
}

template <class CharT, class TraitsT = std::char_traits<CharT> >
class basic_dostream : public std::basic_ostream<CharT, TraitsT> {
 public:
  basic_dostream()
      : std::basic_ostream<CharT, TraitsT>(
            new basic_debugbuf<CharT, TraitsT>()) {}
  ~basic_dostream() { delete rdbuf(); }
};

/*typedef basic_dostream<char>    dostream;
typedef basic_dostream<wchar_t> wdostream;
*/
typedef basic_dostream<char> t_dostream;

//////////////////////////////////////
/// Named mutex

class NamedMutex {
 private:
  HANDLE m_mutex;

  bool m_alreadyExists;

  DWORD m_lastError;

 public:
  NamedMutex(const std::wstring& name) : m_mutex(0) {
    m_mutex = CreateMutex(NULL, true, name.c_str());

    m_mutex != 0 ? m_lastError = GetLastError() : 0;
  }

  ~NamedMutex() {
    if (m_mutex != 0) CloseHandle(m_mutex);
  }

  bool alreadyExists() const { return m_lastError == ERROR_ALREADY_EXISTS; }
};

typedef tradery::ManagedPtr<NamedMutex> NamedMutexPtr;
}
