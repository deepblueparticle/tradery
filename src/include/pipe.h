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

BASIC_EXCEPTION(PipeException);

class PipeDataHandler {
 public:
  virtual ~PipeDataHandler() {}

  virtual tradery::StringPtr data(const std::string& str) = 0;
};

class PipeServer {
 public:
  virtual ~PipeServer() {}
  MISCWIN_API static PipeServer* make(const std::wstring& name,
                                      unsigned long count,
                                      PipeDataHandler* dataHandler);

  virtual void run() = 0;
  virtual void stop() = 0;
};

typedef tradery::ManagedPtr<PipeServer> PipeServerPtr;

class PipeClient {
 private:
  const std::string m_name;

 public:
  PipeClient(const std::string& name) : m_name(name) {}

  std::wstring send(const std::string& str) {
    std::wstring base(_T("\\\\.\\pipe\\" ));

    DWORD bytesRead;

    TCHAR inbuf[8000];

    DWORD lasterror(0);

    if (!CallNamedPipe((base + tradery::s2ws(m_name)).c_str(),
                       const_cast<TCHAR*>(tradery::s2ws(str).c_str()),
                       str.length(), inbuf, sizeof(inbuf), &bytesRead,
                       NMPWAIT_WAIT_FOREVER)) {
      lasterror = ::GetLastError();

      std::string excMessage;

      excMessage << "CallNamedPipe \"" << tradery::ws2s(base) + m_name
                 << "\" failed with last error " << GetLastError();
      throw PipeException(excMessage);
    } else
      return inbuf;
  }
};
