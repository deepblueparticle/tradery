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
#include "miscwinapp.h"
#include <curl/curl.h>
#include <thread.h>
#include <shlobj.h>
#include <miscfile.h>
#include <io.h>
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().

#include <boost/shared_array.hpp>
#include <wininet.h>
#include <Processthreadsapi.h>
#include <WinBase.h>

using namespace tradery;

BEGIN_MESSAGE_MAP(CMiscWinApp, CWinApp)
END_MESSAGE_MAP()

// CFilePluginsApp construction

CMiscWinApp::CMiscWinApp() {
  // TODO: add construction code here,
  // Place all significant initialization in InitInstance
}

// The one and only CFilePluginsApp object

CMiscWinApp theApp;

// CFilePluginsApp initialization
BOOL CMiscWinApp::InitInstance() {
  CWinApp::InitInstance();
  curl_global_init(CURL_GLOBAL_ALL);

  return TRUE;
}

int CMiscWinApp::ExitInstance() {
  curl_global_cleanup();

  return CWinApp::ExitInstance();
}

MISCWIN_API const std::wstring tradery::makeFileName(const std::wstring& path,
                                                     const std::wstring& name,
                                                     const std::wstring& ext) {
  std::wstring fileName(path);

  // open the file with the data
  if (fileName.length() > 0) {
    if (fileName.at(fileName.size() - 1) != _TCHAR('\\') &&
        fileName.at(path.size() - 1) != _TCHAR('/'))
      fileName += _TCHAR('\\');
  }
  fileName += name;
  if (ext.at(0) != _TCHAR('.')) fileName += ('.');

  return fileName + ext;
}

MISCWIN_API const RunProcessResult tradery::runProcess(
    const std::string& processFileName, const std::string& cmdLine,
    bool waitForProcess, const std::string* startingDirectory,
    const Environment& env, unsigned long timeout) throw(RunProcessException) {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  LPCSTR startingDir = startingDirectory != 0 ? startingDirectory->c_str() : 0;

  // Run the autoupdate program.
  if (CreateProcess(
          s2ws(processFileName.empty() ? 0 : processFileName).c_str(),
          const_cast<LPTSTR>(s2ws(cmdLine).c_str()),  // Command line.
          NULL,                       // Process handle not inheritable.
          NULL,                       // Thread handle not inheritable.
          TRUE,                       // Set handle inheritance to FALSE.
          0,                          // No creation flags.
          0,                          // Use parent's environment block.
          s2ws(startingDir).c_str(),  // Use parent's starting directory.
          &si,                        // Pointer to STARTUPINFO structure.
          &pi)  // Pointer to PROCESS_INFORMATION structure.
  ) {
    DWORD exitCode(0);

    bool timedout(false);
    // Wait until child process exits.
    if (waitForProcess) {
      if (timeout == 0)
        WaitForSingleObject(pi.hProcess, INFINITE);
      else {
        if (WaitForSingleObject(pi.hProcess, timeout) == WAIT_TIMEOUT) {
          // if timeout, forcefully therminate the process
          TerminateProcess(pi.hProcess, -1);
          timedout = true;
        }
      }
    }
    GetExitCodeProcess(pi.hProcess, &exitCode);

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return RunProcessResult(timedout, exitCode);
  } else
    throw RunProcessException(processFileName, cmdLine, GetLastError());
}

bool SetPrivilege(HANDLE hToken, TCHAR const* privilege, bool enable) {
  TOKEN_PRIVILEGES tp;
  LUID luid;

  if (!LookupPrivilegeValue(NULL,
                            privilege,  // privilege to lookup
                            &luid       // receives LUID of privilege
                            )) {
    printf("LookupPrivilegeValue error: %u\n", GetLastError());
    return false;
  }

  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  if (enable)
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  else
    tp.Privileges[0].Attributes = 0;

  // Enable the privilege or disable all privileges.
  if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
                             (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL)) {
    printf("AdjustTokenPrivileges error: %u\n", GetLastError());
    return false;
  }

  return true;
}

MISCWIN_API const RunProcessResult tradery::runProcessAsUser(
    const std::string& userName, const std::string& domain,
    const std::string& password, const std::string& processFileName,
    const std::string& cmdLine, bool waitForProcess,
    const std::string* startingDirectory,
    unsigned long timeout) throw(RunProcessException) {
  // std::ofstream ofs( "c:\\runProcess.txt" );

  //  ofs << userName << ", " << password << std::endl;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  LPCSTR startingDir = startingDirectory != 0 ? startingDirectory->c_str() : 0;

  HANDLE token;

  /*
    // boosting privileges, not needed when called from a service
      HANDLE hCurrent = GetCurrentProcess();
      HANDLE currentToken;
      OpenProcessToken( hCurrent, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
    &currentToken ); LUID privilegeId1; LUID privilegeId2; SetPrivilege(
    currentToken, SE_ASSIGNPRIMARYTOKEN_NAME, true ); SetPrivilege(
    currentToken, SE_INCREASE_QUOTA_NAME, true );
  */

  if (LogonUser(s2ws(userName).c_str(),
                s2ws(domain.empty() ? 0 : domain.c_str()).c_str(),
                s2ws(password).c_str(), LOGON32_LOGON_NETWORK,
                LOGON32_PROVIDER_DEFAULT, &token)) {
    HANDLE newToken;
    if (ImpersonateLoggedOnUser(token)) {
      /*      LPVOID envBlock;
            bool b = CreateEnvironmentBlock( &envBlock, token, true );
            TCHAR profileDir[ 2000 ];
            DWORD size( 1990 );
            GetUserProfileDirectory( token, profileDir, &size );
            PROFILEINFO profileInfo;
            LoadUserProfile( token, &profileInfo );
      */
      if (DuplicateTokenEx(token, TOKEN_ALL_ACCESS, 0, SecurityIdentification,
                           TokenPrimary, &newToken)) {
        if (CreateProcessAsUser(
                newToken, s2ws(processFileName).c_str(),
                const_cast<LPTSTR>(s2ws(cmdLine).c_str()),  // Command line.
                NULL,                       // Process handle not inheritable.
                NULL,                       // Thread handle not inheritable.
                TRUE,                       // Set handle inheritance to FALSE.
                0,                          // No creation flags.
                0,                          // Use parent's environment block.
                s2ws(startingDir).c_str(),  // Use parent's starting directory.
                &si,                        // Pointer to STARTUPINFO structure.
                &pi)  // Pointer to PROCESS_INFORMATION structure.
        ) {
          bool timedout(false);
          // Wait until child process exits.
          if (waitForProcess) {
            if (timeout == 0)
              WaitForSingleObject(pi.hProcess, INFINITE);
            else {
              if (WaitForSingleObject(pi.hProcess, timeout) == WAIT_TIMEOUT) {
                // if timeout, forcefully therminate the process
                TerminateProcess(pi.hProcess, -1);
                timedout = true;
              }
            }
          }
          DWORD exitCode;
          GetExitCodeProcess(pi.hProcess, &exitCode);

          // Close process and thread handles.
          CloseHandle(pi.hProcess);
          CloseHandle(pi.hThread);

          return RunProcessResult(timedout, exitCode);
        } else
          LOG(log_error,
              "CreateProcessAsUser failed with error: " << GetLastError());
      } else
        LOG(log_error,
            "DuplicateTokenEx failed with error: " << GetLastError());
    } else
      LOG(log_error,
          "ImpersonateLoggedOnUser failed with error: " << GetLastError);
  }
  throw RunProcessException(processFileName, cmdLine, GetLastError());
}

MISCWIN_API void setCurrentThreadIdealProcessor(unsigned int processor) {
  /*	DWORD pa;
          DWORD sa;
          GetProcessAffinityMask( GetCurrentProcess(), &pa, &sa );
          COUT << pa << _T( ", " ) << sa << std::endl;
          */
  HANDLE hThread = GetCurrentThread();
  ::SetThreadIdealProcessor(hThread, processor);
  //	::SetThreadAffinityMask( GetCurrentThread(), 1 >> processor );
}

MISCWIN_API unsigned long getCurrentCPUNumber() {
  return GetCurrentProcessorNumber();
  // this api is not supported on XP
  //::GetCurrentProcessorNumber();
}

////////////////////////////////
// log related static variables
////////////////////////////////
Level tradery::Log::_globalLevel;
std::string tradery::Log::_logToFile;
bool tradery::Log::_logToDebugOutput = false;
bool tradery::Log::_logToConsole = false;
tradery::Mutex Log::_mutex;

LOG_API unsigned __int64 LogEntryExit::m_crtId(0);
LOG_API tradery::Mutex LogEntryExit::m_mx;

class ConsoleLog : public Log {
 public:
  ConsoleLog(Level level, const char* func) : Log(level, func) {}

  virtual std::ostream& getOstream() { return std::cout; }
};

class OutputDebugLog : public Log {
  tradery::t_dostream _dos;

 public:
  OutputDebugLog(Level level, const char* func) : Log(level, func) {}

  virtual std::ostream& getOstream() { return _dos; }
};

MISCWIN_API void tradery::Log::xfilelog(const std::string& fileName,
                                        Level level, char* function,
                                        const std::string& value) {
  tradery::Lock lock(_mutex);

  if (Log::checkLevel(level)) {
    std::string h = Log::header(level, function);

    try {
      FileLog fileLog(fileName, level, function);
      fileLog << h << value << std::endl;
    } catch (const LogException&) {
      // could not open log file - now what?
    }
  }
}

MISCWIN_API void tradery::Log::xlog(Level level, const char* function,
                                    const std::string& value) {
  tradery::Lock lock(_mutex);

  if (Log::checkLevel(level)) {
    std::string h = Log::header(level, function);

    if (Log::logToFile()) {
      try {
        FileLog fileLog(level, function);
        fileLog << h << value << std::endl;
      } catch (const LogException&) {
        // could not open log file - now what?
      }
    }

    if (Log::logToConsole()) {
      ConsoleLog consoleLog(level, function);
      consoleLog << h << value << std::endl;
      ;
    }

    if (Log::logToDebugOutput()) {
      OutputDebugLog odl(level, function);
      odl << h << value << std::endl;
      ;
    }
  }
}

MISCWIN_API void tradery::Log::xlog(Level level, const std::string& function,
                                    const std::string& value) {
  xlog(level, function.c_str(), value);
}

void tradery::Log::flipLevel() {
  if (isDebugLevel())
    setNormalLevel();
  else
    setDebugLevel();
}

MISCWIN_API void tradery::Log::setLevel(Level level) {
  Lock lock(_mutex);

  _globalLevel = level;
}

//**************************
// Thread code
//**************************

void ThreadBase::start(ThreadContext* context) {
  if (!_running) startx(context);
}

void ThreadBase::startSync(ThreadContext* context, unsigned long timeout) {
  if (!_running) startx(context, true, timeout);
}

void ThreadBase::stopSync() {
  if (_running) stopx();
}
void ThreadBase::stopAsync() {
  if (_running) stopy();
}
void ThreadBase::restart(ThreadContext* context) {
  stopSync();
  start(context);
}
bool ThreadBase::isRunning() const { return _running; }
bool ThreadBase::stopping() { return !_run; }

unsigned int ThreadBase::threadProc(void* p) {
  std::auto_ptr<Context> context(static_cast<Context*>(p));

  //	TSPRINT_DEBUG( _T( "in Thread::threadProc before run\n" ) );
  try {
    LOG(log_info, "Starting thread: \"" << context->diagString() << "\"");
    context->thread()->run(context->threadContext());
    context->thread()->_run = false;
    context->thread()->_running = false;
    LOG(log_info, "Exiting thread: \"" << context->diagString() << "\"");
  } catch (std::exception& e) {
    LOG(log_error,
        "Unhandled exception in thread: \"" << context->diagString() << "\"");
    LOG(log_error, "error: \"" << e.what() << "\"");
    context->thread()->_run = false;
    context->thread()->_running = false;
  } catch (...) {
    LOG(log_error, "Unhandled unknown exception in thread: \""
                       << context->diagString() << "\"");

    context->thread()->_run = false;
    context->thread()->_running = false;
  }
  return 0;
}

HANDLE ThreadBase::create(ThreadContext* context, unsigned int (*proc)(void*)) {
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;

  return (HANDLE)_beginthreadex(
      (void*)&sa, 0, (unsigned(__stdcall*)(void*))proc,
      new Context(this, context), 0, (unsigned int*)&_threadId);
}

void ThreadBase::startx(ThreadContext* context, bool sync,
                        unsigned long timeout) {
  if (!_running) {
    _run = true;
    _running = true;

    _hThread = create(context, threadProc);

    if (_hThread) {
      if (sync)
        WaitForSingleObject(_hThread, (timeout > 0 ? timeout : INFINITE));

      TSPRINT_DEBUG("in Thread::startx after beginthread\n");
      if (_priority) {
        BOOL b = ::SetThreadPriority(_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
        //        DWORD error = GetLastError();
      }
    } else {
      // error
    }
  }
}

void ThreadBase::stopx() {
  if (_hThread != 0) {
    _run = false;
    WaitForSingleObject(_hThread, INFINITE);
    _hThread = 0;
  }
}

void ThreadBase::stopy() {
  if (_hThread != 0) {
    _run = false;
    _hThread = 0;
  }
}

MISCWIN_API std::string tradery::getAppDataPath() {
  TCHAR path[MAX_PATH];

  if (SUCCEEDED(SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT,
                                path))) {
    return ws2s(path);
  } else
    return "";
}

MISCWIN_API std::string tradery::getDirectory(const std::string& root,
                                              const std::string& dir) {
  if (!root.empty()) {
    std::string path(addFSlash(root) + dir);
    if (tradery::DirectoryExists(path.c_str()) ||
        SUCCEEDED(CreateDirectory(s2ws(path).c_str(), 0)))
      return path;
    else
      return "";
  } else
    return "";
}

MISCWIN_API tradery::VersionPtr tradery::getFileVersion(
    const std::string& fileName) {
  DWORD handle;
  DWORD size(::GetFileVersionInfoSize(s2ws(fileName).c_str(), &handle));

  if (size > 0) {
    boost::shared_array<char> data(new char[size]);

    if (::GetFileVersionInfo(s2ws(fileName).c_str(), 0, size, data.get())) {
      VS_FIXEDFILEINFO* pVS = NULL;
      unsigned int len;

      if (VerQueryValue(data.get(), _T("\\"), (void**)&pVS, &len) &&
          (pVS != NULL)) {
        unsigned int n1, n2, n3, n4;
        n1 = pVS->dwProductVersionMS >> 16;
        n2 = pVS->dwProductVersionMS & 0xFFFF;
        n3 = pVS->dwProductVersionLS >> 16;
        n4 = pVS->dwProductVersionLS & 0xFFFF;

        return VersionPtr(new tradery::Version(n1, n2, n3, n4));
      } else
        return VersionPtr();
    } else
      return VersionPtr();

  } else
    return VersionPtr();
}

MISCWIN_API std::string tradery::getFileLanguage(const std::string& fileName) {
  return "en";
}

MISCWIN_API bool tradery::DirectoryExists(const char* dir) {
  assert(dir != 0 && strlen(dir) > 0);

  if (dir == 0 || strlen(dir) == 0) return false;

  if (_access(dir, 0) == 0) {
    struct _stat status;
    ::_stat(dir, &status);

    return status.st_mode & S_IFDIR;
  } else
    return false;
}

MISCWIN_API bool tradery::DirectoryExists(const std::string& dir) {
  return DirectoryExists(dir.c_str());
}

MISCWIN_API std::string tradery::getModulePath() {
  std::string dir;
  TCHAR fileName[MAX_PATH + 1];
  CFileFind fileFinder;

  GetModuleFileName(0, fileName, MAX_PATH);
  if (fileFinder.FindFile(fileName)) {
    fileFinder.FindNextFile();
    dir = ws2s(fileFinder.GetRoot().GetString());
    return dir;
  } else {
    assert(false);
    return "";
  }
}

MISCWIN_API bool tradery::fileExists(const char* filepath) {
  HANDLE hFile =
      CreateFile(s2ws(filepath).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile == INVALID_HANDLE_VALUE) return false;

  CloseHandle(hFile);
  return true;
}

MISCWIN_API DateTime tradery::getFileLastWriteTime(
    const std::string& fileName) throw(FileException) {
  HANDLE file =
      CreateFile(s2ws(fileName).c_str(), GENERIC_READ, FILE_SHARE_READ, 0,
                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (file == INVALID_HANDLE_VALUE)
    throw FileException();
  else {
    FILETIME creation;
    FILETIME lastAccess;
    FILETIME lastWrite;

    GetFileTime(file, &creation, &lastAccess, &lastWrite);

    CloseHandle(file);

    return toDateTime(lastWrite);
  }
}
MISCWIN_API std::string tradery::getFilePath(const std::string& file) {
  CFileFind ff;

  ff.FindFile(s2ws(file).c_str());
  ff.FindNextFile();

  return ws2s((LPCTSTR)ff.GetRoot());
}

extern "C" {
_CRTIMP int __cdecl _open_osfhandle(intptr_t, int);
}

// reserved windows file names
boost::regex expression(
    "(?:\\`|\\\\|/"
    ")(?:CON|PRN|AUX|NUL|(?:COM[1-9])|(?:LPT[1-9]))(?:\\'|\\\\|\\.|/)",
    boost::regex::perl | boost::regex::icase);

boost::match_flag_type flags = boost::match_default;

bool hasReservedFileName(const std::string& fileName) {
  // a reserved sring at the begining of string, or preceded by

  boost::match_results<const char*> what;

  return boost::regex_search(fileName.c_str(), what, expression, flags);
}

#define FILE_PREFIX std::string("\\\\?\\")

SpecialFileRead::SpecialFileRead(const std::string& fileName) : Base(fileName) {
  if (hasReservedFileName(fileName))
    createReadFile(FILE_PREFIX + fileName);
  else
    __super::setFStream(file_type_ptr(new std::ifstream(fileName.c_str())));
}

SpecialFileWrite::SpecialFileWrite(const std::string& fileName, bool append,
                                   bool binary)
    : Base(fileName) {
  if (hasReservedFileName(fileName))
    createWriteFile(FILE_PREFIX + fileName, append, binary);
  else {
    file_type_ptr f(new std::ofstream());

    int mode = (append ? std::ios_base::app : 0) |
               (binary ? std::ios_base::binary : 0);

    if (mode != 0)
      f->open(fileName.c_str(), mode);
    else
      f->open(fileName.c_str());

    f->precision(12);
    __super::setFStream(f);
  }
}

MISCWIN_API void tradery::SpecialFileRead::createReadFile(
    const std::string& fileName) {
  HANDLE file_handle =
      CreateFile(s2ws(fileName).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  __super::setHandle(file_handle);
  int file_descriptor = _open_osfhandle((intptr_t)file_handle, 0);

  setFileDescriptor(file_descriptor);
  FILE* file = _fdopen(file_descriptor, "r");

  __super::setFILE(file);

  __super::setFStream(file_type_ptr(new std::ifstream(file)));
}

MISCWIN_API void tradery::SpecialFileWrite::createWriteFile(
    const std::string& fileName, bool append, bool binary) {
  HANDLE file_handle = CreateFile(
      s2ws(fileName).c_str(), append ? FILE_APPEND_DATA : GENERIC_WRITE, 0,
      NULL, append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  __super::setHandle(file_handle);

  int file_descriptor = _open_osfhandle((intptr_t)file_handle, 0);

  setFileDescriptor(file_descriptor);
  FILE* file = _fdopen(file_descriptor, "w");

  __super::setFILE(file);

  file_type_ptr fs(new std::ofstream(file));

  __super::setFStream(fs);
}

MISCWIN_API std::wstring urlEncode(const std::wstring& url) {
  if (url.empty()) return std::wstring();

  TCHAR buff[1];
  DWORD buffSize(1);

  if (!InternetCanonicalizeUrl(url.c_str(), buff, &buffSize, 0))
    return std::wstring();
  else if (buffSize > 0) {
    boost::shared_array<TCHAR> buffer(new TCHAR[buffSize]);

    if (!InternetCanonicalizeUrl(url.c_str(), buffer.get(), &buffSize, 0))
      return std::wstring();
    else
      return std::wstring(buffer.get());

  } else
    return std::wstring();
}
