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

#include <winbase.h>
#include "strings.h"
#include "sharedptr.h"
#include "misc.h"

namespace tradery {

inline DateTime toDateTime(const SYSTEMTIME& st) {
  return DateTime(Date(st.wYear, st.wMonth, st.wDay),
                  TimeDuration(st.wHour, st.wMinute, st.wSecond, 0));
}

inline DateTime toDateTime(const FILETIME fileTime) {
  SYSTEMTIME st;
  FileTimeToSystemTime(&fileTime, &st);
  return toDateTime(st);
}

class FileException {};

MISCWIN_API tradery::DateTime getFileLastWriteTime(
    const std::string& fileName) throw(FileException);

MISCWIN_API bool fileExists(const char* filepath);

inline bool fileExists(const std::string& filepath) {
  return fileExists(filepath.c_str());
}

MISCWIN_API bool DirectoryExists(const char* dir);
MISCWIN_API bool DirectoryExists(const std::string& dir);

// returns the path without the file name
MISCWIN_API std::string getModulePath();

inline std::string getModuleFileName() {
  TCHAR fileName[MAX_PATH + 1] = {0};

  GetModuleFileName(0, fileName, MAX_PATH);

  return ws2s(fileName);
}

MISCWIN_API std::string getAppDataPath();

MISCWIN_API std::string getDirectory(const std::string& root,
                                     const std::string& dir);

typedef ManagedPtr<Version> VersionPtr;
MISCWIN_API VersionPtr getFileVersion(const std::string& fileName);
MISCWIN_API std::string getFileLanguage(const std::string& fileName);

MISCWIN_API std::string getFilePath(const std::string& file);

class SpecialFileException {};

typedef boost::shared_ptr<std::fstream> fstream_ptr;
template <typename file_type>
class SpecialFile {
 protected:
  typedef boost::shared_ptr<file_type> file_type_ptr;

 private:
  file_type_ptr _fs;
  FILE* _f;
  HANDLE _h;
  int _fd;
  const std::string _fileName;

 protected:
  SpecialFile(const std::string& fileName)
      : _f(0), _h(0), _fd(0), _fileName(fileName) {}

  void setHandle(HANDLE h) {
    if (h != INVALID_HANDLE_VALUE)
      _h = h;
    else {
      LOG(log_error, "last error: " << GetLastError());
      throw SpecialFileException();
    }
  }
  void setFILE(FILE* f) {
    if (f != 0)
      _f = f;
    else {
      LOG(log_error, "last error: " << GetLastError());
      throw SpecialFileException();
    }
  }

  void setFStream(file_type_ptr fs) {
    if (fs && *fs)
      _fs = fs;
    else {
      LOG(log_error, "last error: " << GetLastError());
      throw SpecialFileException();
    }
  }

  void setFileDescriptor(int fd) {
    if (fd >= 0)
      _fd = fd;
    else {
      LOG(log_error, "last error: " << GetLastError());
      throw SpecialFileException();
    }
  }

  virtual ~SpecialFile() {
    if (_fs && _fs->is_open()) _fs->close();

    if (_f) fclose(_f);
  }

 public:
  operator file_type&() {
    assert(this->operator bool());
    return *_fs;
  }

  operator bool() const { return _fs && *_fs; }
};

class MISCWIN_API SpecialFileRead : public SpecialFile<std::ifstream> {
 private:
  typedef SpecialFile<std::ifstream> Base;

 private:
  void createReadFile(const std::string& fileName);

 public:
  SpecialFileRead(const std::string& fileName);
};

class MISCWIN_API SpecialFileWrite : public SpecialFile<std::ofstream> {
 private:
  typedef SpecialFile<std::ofstream> Base;

 private:
  void createWriteFile(const std::string& fileName, bool append, bool binary);

 public:
  SpecialFileWrite(const std::string& fileName, bool append = false,
                   bool binary = false);
};

typedef boost::shared_ptr<SpecialFileRead> SpecialFileReadPtr;
typedef boost::shared_ptr<SpecialFileWrite> SpecialFileWritePtr;

}  // namespace tradery