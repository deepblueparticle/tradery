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
#include <pipe.h>
#include <boost/shared_array.hpp>
#include <strings.h>

#define BUFSIZE 5

enum { CONNECTING_STATE, READING_STATE, WRITING_STATE };

#define PIPE_TIMEOUT 5000

class PipeServerImpl : public PipeServer {
 private:
  struct PIPEINST {
    OVERLAPPED oOverlap;
    HANDLE hPipeInst;
    TCHAR chRequest[BUFSIZE];
    DWORD cbRead;
    DWORD dwState;
    BOOL fPendingIO;

    std::wstring response;

    PIPEINST() : hPipeInst(0), cbRead(0), dwState(0), fPendingIO(false) {
      memset(&oOverlap, 0, sizeof(OVERLAPPED));
      memset(&chRequest, 0, BUFSIZE * sizeof(TCHAR));
    }

    ~PIPEINST() { CloseHandle(hPipeInst); }
  };

  typedef PIPEINST* LPPIPEINST;

 private:
  const std::wstring& _name;
  const unsigned long _count;

  boost::shared_array<PIPEINST> Pipe;
  boost::shared_array<HANDLE> hEvents;

  PipeDataHandler* m_dataHandler;

 public:
  ~PipeServerImpl() {
    for (unsigned int n = 0; n < _count; ++n) CloseHandle(hEvents.get()[n]);
  }

  PipeServerImpl(const std::wstring& name, unsigned long count,
                 PipeDataHandler* dataHandler)
      : Pipe(new PIPEINST[count]),
        hEvents(new HANDLE[count + 1]),
        _name(name),
        _count(count),
        m_dataHandler(dataHandler) {
    assert(dataHandler != 0);

    hEvents[count] = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (hEvents[count] == NULL) {
      std::string excMessage;

      excMessage
          << "PipeServer constructor - could not create event. Last error "
          << GetLastError();
      throw PipeException(excMessage);
    }

    boost::shared_ptr<SECURITY_DESCRIPTOR> pSD(new SECURITY_DESCRIPTOR());
    SECURITY_ATTRIBUTES sa;

    assert(pSD);

    if (!InitializeSecurityDescriptor(pSD.get(),
                                      SECURITY_DESCRIPTOR_REVISION)) {
      std::string excMessage;

      excMessage << "PipeServer constructor - could not initialize security "
                    "descriptor. Last error "
                 << GetLastError();
      throw PipeException(excMessage);
    }

    // add a NULL disc. ACL to the security descriptor.
    //
    if (!SetSecurityDescriptorDacl(pSD.get(), TRUE, (PACL)NULL, FALSE)) {
      std::string excMessage;

      excMessage << "PipeServer constructor - could not set security "
                    "descriptor dacl. Last error "
                 << GetLastError();
      throw PipeException(excMessage);
    }

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = pSD.get();
    sa.bInheritHandle = TRUE;

    for (unsigned int i = 0; i < _count; i++) {
      // Create an event object for this instance.

      hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);

      if (hEvents[i] == NULL) {
        std::ostringstream o;
        o << "CreateEvent failed with error: " << GetLastError();
        //      _log->writeFatalLine( SERVICE_MAIN_LOOP, o.str() );
        throw PipeException(o.str());
      }

      Pipe[i].oOverlap.hEvent = hEvents[i];

      std::wstring base(_T("\\\\.\\pipe\\" ));

      Pipe[i].hPipeInst = CreateNamedPipe(
          (base + _name).c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
          PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, _count,
          BUFSIZE * sizeof(TCHAR), BUFSIZE * sizeof(TCHAR), PIPE_TIMEOUT, &sa);

      if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE) {
        std::ostringstream o;

        o << "CreateNamedPipe failed with error: " << GetLastError();
        //      _log->writeFatalLine( SERVICE_MAIN_LOOP, o.str() );
        throw PipeException(o.str());
      }

      // Connect to the new client
      Pipe[i].fPendingIO =
          ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);
      Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
    }
  }

  void runServer() {
    DWORD i, dwWait, cbRet, dwErr;
    BOOL fSuccess;

    while (1) {
      // Wait for the event object to be signaled, indicating
      // completion of an overlapped read, write, or
      // connect operation.

      //    _log->writeInfoLine( SERVICE_MAIN_LOOP, _T( "Waiting for an event to
      //    be signalled" ) );
      dwWait =
          WaitForMultipleObjects(_count + 1, hEvents.get(), FALSE, INFINITE);

      // dwWait shows which pipe completed the operation.

      i = dwWait - WAIT_OBJECT_0;  // determines which pipe

      assert(i >= 0 && i <= _count);

      // the stop event is signaled, exiting
      if (i == _count) {
        LOG(log_debug, "stop event signalled, exiting");

        break;
      }

      LOG(log_debug, "pipe handle set, index " << i);

      // Get the result if the operation was pending.

      if (Pipe[i].fPendingIO) {
        fSuccess = GetOverlappedResult(Pipe[i].hPipeInst, &Pipe[i].oOverlap,
                                       &cbRet, FALSE);

        DWORD lastError = GetLastError();

        switch (Pipe[i].dwState) {
          // Pending connect operation
          case CONNECTING_STATE:
            if (!fSuccess) {
              std::ostringstream o;

              o << "In CONNECTING_STATE, GetOverlappedResult failed with error "
                << GetLastError();
              //          _log->writeFatalLine( SERVICE_MAIN_LOOP, o.str() );
              LOG(log_debug, o.str());

              throw PipeException(o.str());
            }
            Pipe[i].dwState = READING_STATE;
            break;

            // Pending read operation
          case READING_STATE:
            if (!fSuccess || cbRet == 0) {
              DisconnectAndReconnect(i);
              continue;
            }
            Pipe[i].dwState = WRITING_STATE;
            break;

            // Pending write operation
          case WRITING_STATE:
            if (!fSuccess || cbRet != Pipe[i].response.length()) {
              DisconnectAndReconnect(i);
              continue;
            }
            Pipe[i].dwState = READING_STATE;
            break;

          default: {
            // Invalid pipe state
            assert(false);
          }
        }
      }

      // The pipe state determines which operation to do next.

      switch (Pipe[i].dwState) {
          // READING_STATE:
          // The pipe instance is connected to the client
          // and is ready to read a request from the client.

        case READING_STATE: {
          std::wstring data;
          while (true) {
            fSuccess = ReadFile(Pipe[i].hPipeInst, Pipe[i].chRequest,
                                BUFSIZE * sizeof(TCHAR), &Pipe[i].cbRead,
                                &Pipe[i].oOverlap);
            dwErr = GetLastError();

            if (!fSuccess) {
              if (dwErr == ERROR_MORE_DATA)
                data +=
                    std::wstring(Pipe[i].chRequest, BUFSIZE * sizeof(TCHAR));
              else
                break;
            } else {
              data += std::wstring(Pipe[i].chRequest, Pipe[i].cbRead);
              break;
            }
          }

          if (fSuccess && Pipe[i].cbRead != 0) {
            tradery::StringPtr response(
                m_dataHandler->data(tradery::ws2s(data)));
            assert(response);

            Pipe[i].response = tradery::s2ws(*response);
            Pipe[i].fPendingIO = FALSE;
            Pipe[i].dwState = WRITING_STATE;
            continue;
          }

          // The read operation is still pending.

          dwErr = GetLastError();
          if (!fSuccess && (dwErr == ERROR_IO_PENDING)) {
            Pipe[i].fPendingIO = TRUE;
            continue;
          }

          // An error occurred; disconnect from the client.

          DisconnectAndReconnect(i);
        } break;

          // WRITING_STATE:
          // The request was successfully read from the client.
          // Get the reply data and write it to the client.

        case WRITING_STATE:
          fSuccess =
              WriteFile(Pipe[i].hPipeInst, Pipe[i].response.c_str(),
                        Pipe[i].response.length(), &cbRet, &Pipe[i].oOverlap);

          // The write operation completed successfully.

          if (fSuccess && cbRet == Pipe[i].response.length()) {
            Pipe[i].fPendingIO = FALSE;
            Pipe[i].dwState = READING_STATE;
            continue;
          }

          // The write operation is still pending.

          dwErr = GetLastError();
          if (!fSuccess && (dwErr == ERROR_IO_PENDING)) {
            Pipe[i].fPendingIO = TRUE;
            continue;
          }

          // An error occurred; disconnect from the client.

          DisconnectAndReconnect(i);
          break;

        default: {
          // Invalid pipe state
          assert(false);
        }
      }
    }
  }

  void DisconnectAndReconnect(DWORD i) const {
    // Disconnect the pipe instance.

    if (!DisconnectNamedPipe(Pipe[i].hPipeInst)) {
      printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
    }

    // Call a subroutine to connect to the new client.

    Pipe[i].fPendingIO =
        ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);

    Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
  }

  bool ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) const {
    bool fPendingIO = false;

    // Start an overlapped connection for this pipe instance.

    // Overlapped ConnectNamedPipe should return zero.
    if (ConnectNamedPipe(hPipe, lpo) != 0) {
      std::wostringstream o;

      o << "ConnectNamedPipe failed with error" << GetLastError();
      //    _log->writeFatalLine( SERVICE_MAIN_LOOP, o.str() );
      return false;
    } else {
      switch (GetLastError()) {
        // The overlapped connection in progress.
        case ERROR_IO_PENDING:
          fPendingIO = true;
          break;

          // Client is already connected, so signal an event.

        case ERROR_PIPE_CONNECTED:
          if (SetEvent(lpo->hEvent)) break;

          // If an error occurs during the connect operation...
        default: {
          std::wostringstream o;

          o << "ConnectNamedPipe failed with error" << GetLastError();
          //        _log->writeFatalLine( SERVICE_MAIN_LOOP, o.str() );
          return false;
        }
      }
    }
    return fPendingIO;
  }

  virtual void run() { runServer(); }

  virtual void stop() {
    if (hEvents[_count]) SetEvent(hEvents[_count]);
  }
};

PipeServer* PipeServer::make(const std::wstring& name, unsigned long count,
                             PipeDataHandler* dataHandler) {
  return new PipeServerImpl(name, count, dataHandler);
}
