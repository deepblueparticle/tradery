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

#include "StdAfx.h"
#include ".\pipes.h"

#define CONNECTING_STATE 0
#define READING_STATE 1
#define WRITING_STATE 2
#define PIPE_TIMEOUT 5000

Pipes::Pipes(const std::wstring& name, unsigned long count)
    : Pipe(new PIPEINST[count]),
      hEvents(new HANDLE[count + 1]),
      _name(name),
      _count(count) {
  hEvents[count] = CreateEvent(NULL,   // no security attributes
                               TRUE,   // manual reset event
                               FALSE,  // not-signalled
                               NULL);  // no name

  if (hEvents[count] == NULL) throw PipesException();

  DWORD i;

  PSECURITY_DESCRIPTOR pSD = NULL;
  SECURITY_ATTRIBUTES sa;

  pSD = (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);

  if (pSD == NULL) throw PipesException();

  if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    throw PipesException();

  // add a NULL disc. ACL to the security descriptor.
  //
  if (!SetSecurityDescriptorDacl(pSD, TRUE, (PACL)NULL, FALSE))
    throw PipesException();

  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = pSD;
  sa.bInheritHandle = TRUE;

  for (i = 0; i < _count; i++) {
    // Create an event object for this instance.

    hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (hEvents[i] == NULL) {
      std::wostringstream o;
      o << _T( "CreateEvent failed with error: ") << GetLastError();
      //      _log->writeFatalLine( SERVICE_MAIN_LOOP, o.str() );
      throw PipesException();
    }

    Pipe[i].oOverlap.hEvent = hEvents[i];

    std::wstring base(_T("\\\\.\\pipe\\" ));

    Pipe[i].hPipeInst = CreateNamedPipe(
        (base + _name).c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, _count,
        BUFSIZE * sizeof(TCHAR), BUFSIZE * sizeof(TCHAR), PIPE_TIMEOUT, &sa);

    if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE) {
      std::wostringstream o;

      o << _T( "CreateNamedPipe failed with error: ") << GetLastError();
      //      _log->writeFatalLine( SERVICE_MAIN_LOOP, o.str() );
      throw PipesException();
    }

    // Call the subroutine to connect to the new client

    Pipe[i].fPendingIO =
        ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);

    Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
  }
}

Pipes::~Pipes(void) {
  delete[](Pipe);
  delete[](hEvents);

  if (hEvents[_count]) CloseHandle(hEvents[_count]);
}

void Pipes::runServer() const {
  DWORD i, dwWait, cbRet, dwErr;
  BOOL fSuccess;

  while (1) {
    // Wait for the event object to be signaled, indicating
    // completion of an overlapped read, write, or
    // connect operation.

    //    _log->writeInfoLine( SERVICE_MAIN_LOOP, _T( "Waiting for an event to
    //    be signalled" ) );
    dwWait = WaitForMultipleObjects(_count + 1, hEvents, FALSE, INFINITE);

    // dwWait shows which pipe completed the operation.

    i = dwWait - WAIT_OBJECT_0;  // determines which pipe

    assert(i >= 0 && i <= _count);

    // the stop event is signaled, exiting
    if (i == _count) break;

    // Get the result if the operation was pending.

    if (Pipe[i].fPendingIO) {
      fSuccess = GetOverlappedResult(Pipe[i].hPipeInst, &Pipe[i].oOverlap,
                                     &cbRet, FALSE);

      switch (Pipe[i].dwState) {
        // Pending connect operation
        case CONNECTING_STATE:
          if (!fSuccess) {
            std::wostringstream o;

            o << _T( "In CONNECTING_STATE, GetOverlappedResult failed with error " )
              << GetLastError();
            //          _log->writeFatalLine( SERVICE_MAIN_LOOP, o.str() );
            throw PipesException();
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
          if (!fSuccess || cbRet != Pipe[i].cbToWrite) {
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

      case READING_STATE:
        fSuccess = ReadFile(Pipe[i].hPipeInst, Pipe[i].chRequest,
                            BUFSIZE * sizeof(TCHAR), &Pipe[i].cbRead,
                            &Pipe[i].oOverlap);

        // The read operation completed successfully.

        if (fSuccess && Pipe[i].cbRead != 0) {
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
        break;

        // WRITING_STATE:
        // The request was successfully read from the client.
        // Get the reply data and write it to the client.

      case WRITING_STATE:
        GetAnswerToRequest(&Pipe[i]);

        fSuccess = WriteFile(Pipe[i].hPipeInst, Pipe[i].chReply,
                             Pipe[i].cbToWrite, &cbRet, &Pipe[i].oOverlap);

        // The write operation completed successfully.

        if (fSuccess && cbRet == Pipe[i].cbToWrite) {
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

const std::wstring Pipes::sendRequest(const std::wstring& request) {
  return _T( "" );
}

void Pipes::DisconnectAndReconnect(DWORD i) const {
  // Disconnect the pipe instance.

  if (!DisconnectNamedPipe(Pipe[i].hPipeInst)) {
    printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
  }

  // Call a subroutine to connect to the new client.

  Pipe[i].fPendingIO = ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);

  Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
}

bool Pipes::ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) const {
  bool fPendingIO = false;

  // Start an overlapped connection for this pipe instance.

  // Overlapped ConnectNamedPipe should return zero.
  if (ConnectNamedPipe(hPipe, lpo) != 0) {
    std::wostringstream o;

    o << _T( "ConnectNamedPipe failed with error" ) << GetLastError();
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

        o << _T( "ConnectNamedPipe failed with error" ) << GetLastError();
        //        _log->writeFatalLine( SERVICE_MAIN_LOOP, o.str() );
        return false;
      }
    }
  }
  return fPendingIO;
}

void Pipes::GetAnswerToRequest(LPPIPEINST) const {}

void Pipes::stop() {
  if (hEvents[_count]) SetEvent(hEvents[_count]);
}
