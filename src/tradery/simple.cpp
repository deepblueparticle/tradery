// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 1993-1997  Microsoft Corporation.  All Rights Reserved.
//
//  MODULE:   simple.c
//
//  PURPOSE:  Implements the body of the service.
//            The default behavior is to open a
//            named pipe, \\.\pipe\simple, and read
//            from it.  It the modifies the data and
//            writes it back to the pipe.
//
//  FUNCTIONS:
//            ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv);
//            ServiceStop( );
//
//  COMMENTS: The functions implemented in simple.c are
//            prototyped in service.h
//
//
//  AUTHOR: Craig Link - Microsoft Developer Support
//

#include "stdafx.h"
#include "tradery.h"
#include "service.h"
#include "Configuration.h"
#include "processingthread.h"
#include "ProcessingThreads.h"
#include "pipes.h"
#include "ThriftServer.h"

void run(const std::wstring& cmdLineString);
int mainLoop(SECURITY_ATTRIBUTES*);

ThriftServer thriftServer;

class ServicePipes : public Pipes {
 public:
  ServicePipes() : Pipes(_T( "pipes" ), 10) {}

  //  serverProcessing
};

// this event is signalled when the
// service should end
//
HANDLE hServerStopEvent = NULL;

//
//  FUNCTION: ServiceStart
//
//  PURPOSE: Actual code of the service
//           that does the work.
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    The default behavior is to open a
//    named pipe, \\.\pipe\simple, and read
//    from it.  It the modifies the data and
//    writes it back to the pipe.  The service
//    stops when hServerStopEvent is signalled
//
VOID ServiceStart(DWORD dwArgc, LPTSTR* lpszArgv) {
  HANDLE hPipe = INVALID_HANDLE_VALUE;
  PSECURITY_DESCRIPTOR pSD = NULL;
  SECURITY_ATTRIBUTES sa;

  ///////////////////////////////////////////////////
  //
  // Service initialization
  //

  LOG(log_info, "Starting service");
  // report the status to the service control manager.
  //
  if (!ReportStatusToSCMgr(SERVICE_START_PENDING,  // service state
                           NO_ERROR,               // exit code
                           3000))                  // wait hint
    goto cleanup;
  LOG(log_info, "Creating event");

  // create the event object. The control handler function signals
  // this event when it receives the "stop" control code.
  //
  hServerStopEvent = CreateEvent(NULL,   // no security attributes
                                 TRUE,   // manual reset event
                                 FALSE,  // not-signalled
                                 NULL);  // no name

  if (hServerStopEvent == NULL) goto cleanup;

  // report the status to the service control manager.
  //
  if (!ReportStatusToSCMgr(SERVICE_START_PENDING,  // service state
                           NO_ERROR,               // exit code
                           3000))                  // wait hint
    goto cleanup;

  thriftServer.start();

  // create a security descriptor that allows anyone to write to
  //  the pipe...
  //
  pSD = (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);

  if (pSD == NULL) goto cleanup;

  LOG(log_info, "Initializing security descriptor");

  if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    goto cleanup;

  LOG(log_info, "Setting security descriptor");
  // add a NULL disc. ACL to the security descriptor.
  //
  if (!SetSecurityDescriptorDacl(pSD, TRUE, (PACL)NULL, FALSE)) goto cleanup;

  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = pSD;
  sa.bInheritHandle = TRUE;

  // report the status to the service control manager.
  //
  if (!ReportStatusToSCMgr(SERVICE_START_PENDING,  // service state
                           NO_ERROR,               // exit code
                           3000))                  // wait hint
    goto cleanup;

  // report the status to the service control manager.
  //
  if (!ReportStatusToSCMgr(SERVICE_RUNNING,  // service state
                           NO_ERROR,         // exit code
                           0))               // wait hint
    goto cleanup;

  LOG(log_info, "Starting main loop");
  mainLoop(&sa);

cleanup:
  if (hServerStopEvent) CloseHandle(hServerStopEvent);

  if (pSD) free(pSD);
}

//
//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    If a ServiceStop procedure is going to
//    take longer than 3 seconds to execute,
//    it should spawn a thread to execute the
//    stop code, and return.  Otherwise, the
//    ServiceControlManager will believe that
//    the service has stopped responding.
//
VOID ServiceStop() {
  if (hServerStopEvent) SetEvent(hServerStopEvent);

  thriftServer.stop();
}

#define CONNECTING_STATE 0
#define READING_STATE 1
#define WRITING_STATE 2
#define INSTANCES 10
#define PIPE_TIMEOUT 5000
#define BUFSIZE 8192

typedef struct {
  OVERLAPPED oOverlap;
  HANDLE hPipeInst;
  TCHAR chRequest[BUFSIZE];
  DWORD cbRead;
  TCHAR chReply[BUFSIZE];
  DWORD cbToWrite;
  DWORD dwState;
  BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;

VOID DisconnectAndReconnect(DWORD);
BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);
VOID GetAnswerToRequest(LPPIPEINST);

PIPEINST Pipe[INSTANCES];
HANDLE hEvents[INSTANCES + 1];

const std::string SERVICE_MAIN_LOOP("Service main loop");

int mainLoop(SECURITY_ATTRIBUTES* sa) {
  LOG(log_info, SERVICE_MAIN_LOOP << " - starting main loop");

  hEvents[INSTANCES] = hServerStopEvent;
  DWORD i, dwWait, cbRet, dwErr;
  BOOL fSuccess;
  //   LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

  // The initial loop creates several instances of a named pipe
  // along with an event object for each instance.  An
  // overlapped ConnectNamedPipe operation is started for
  // each instance.

  LOG(log_info, SERVICE_MAIN_LOOP << " - creating pipes");

  for (i = 0; i < INSTANCES; i++) {
    // Create an event object for this instance.

    hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (hEvents[i] == NULL) {
      LOG(log_error, SERVICE_MAIN_LOOP << " - createEvent failed with error: "
                                       << GetLastError());
      return 0;
    }

    Pipe[i].oOverlap.hEvent = hEvents[i];

    Pipe[i].hPipeInst = CreateNamedPipe(
        lpszPipeName.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, INSTANCES,
        BUFSIZE * sizeof(TCHAR), BUFSIZE * sizeof(TCHAR), PIPE_TIMEOUT, sa);

    if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE) {
      LOG(log_error, SERVICE_MAIN_LOOP
                         << " - createNamedPipe failed with error: "
                         << GetLastError());
      return 0;
    }

    // Call the subroutine to connect to the new client

    Pipe[i].fPendingIO =
        ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);

    Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
  }

  LOG(log_info, SERVICE_MAIN_LOOP << " - pipes created");

  while (1) {
    // Wait for the event object to be signaled, indicating
    // completion of an overlapped read, write, or
    // connect operation.

    LOG(log_info, SERVICE_MAIN_LOOP << "Waiting for an event to be signalled");
    dwWait = WaitForMultipleObjects(INSTANCES + 1, hEvents, FALSE, INFINITE);

    // dwWait shows which pipe completed the operation.

    i = dwWait - WAIT_OBJECT_0;  // determines which pipe
    // the service event is signaled, the service is closing
    if (i < 0 || i > (INSTANCES)) {
      LOG(log_error, SERVICE_MAIN_LOOP
                         << " - index of signalled object out of range: " << i);
      return 0;
    }

    LOG(log_info, SERVICE_MAIN_LOOP << " - event index " << i << " signalled");

    if (i == INSTANCES) break;

    // Get the result if the operation was pending.

    if (Pipe[i].fPendingIO) {
      fSuccess = GetOverlappedResult(Pipe[i].hPipeInst, &Pipe[i].oOverlap,
                                     &cbRet, FALSE);

      switch (Pipe[i].dwState) {
          // Pending connect operation
        case CONNECTING_STATE:
          if (!fSuccess) {
            LOG(log_error, SERVICE_MAIN_LOOP
                               << " - in CONNECTING_STATE, GetOverlappedResult "
                                  "failed with error "
                               << GetLastError());
            return 0;
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
          LOG(log_error, SERVICE_MAIN_LOOP << "Invalid pipe state");
          return 0;
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

        LOG(log_error,
            "An error occurred during read; disconnect from the client. Last "
            "error: "
                << dwErr);
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

        LOG(log_error,
            "An error occurred during write; disconnect from the client");
        // An error occurred; disconnect from the client.

        DisconnectAndReconnect(i);
        break;

      default: {
        LOG(log_error, "Invalid pipe state");
        return 0;
      }
    }
  }

  LOG(log_debug, "exited main loop");

  return 0;
}

// DisconnectAndReconnect(DWORD)
// This function is called when an error occurs or when the client
// closes its handle to the pipe. Disconnect from this client, then
// call ConnectNamedPipe to wait for another client to connect.

VOID DisconnectAndReconnect(DWORD i) {
  // Disconnect the pipe instance.

  if (!DisconnectNamedPipe(Pipe[i].hPipeInst)) {
    printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
  }

  // Call a subroutine to connect to the new client.

  Pipe[i].fPendingIO = ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);

  Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
}

// ConnectToNewClient(HANDLE, LPOVERLAPPED)
// This function is called to start an overlapped connect operation.
// It returns TRUE if an operation is pending or FALSE if the
// connection has been completed.

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) {
  BOOL fConnected, fPendingIO = FALSE;

  // Start an overlapped connection for this pipe instance.
  fConnected = ConnectNamedPipe(hPipe, lpo);

  // Overlapped ConnectNamedPipe should return zero.
  if (fConnected) {
    LOG(log_error, SERVICE_MAIN_LOOP << " connectNamedPipe failed with error"
                                     << GetLastError());
    return 0;
  } else {
    switch (GetLastError()) {
        // The overlapped connection in progress.
      case ERROR_IO_PENDING:
        fPendingIO = TRUE;
        break;

        // Client is already connected, so signal an event.

      case ERROR_PIPE_CONNECTED:
        if (SetEvent(lpo->hEvent)) break;

        // If an error occurs during the connect operation...
      default:
        LOG(log_error, SERVICE_MAIN_LOOP
                           << " - connectNamedPipe failed with error"
                           << GetLastError());
        return 0;
    }
  }
  return fPendingIO;
}

VOID GetAnswerToRequest(LPPIPEINST pipe) {
  const std::string cmdLineString(ws2s(pipe->chRequest));

  SessionContextPtr context(boost::make_shared<SessionContext>(cmdLineString));

  ProcessingThreads::run(context);

  lstrcpy(pipe->chReply, TEXT("Default answer from server"));
  pipe->cbToWrite = (lstrlen(pipe->chReply) + 1) * sizeof(TCHAR);
}

Mutex ProcessingThread::InstanceCounter::_m;
unsigned int ProcessingThread::InstanceCounter::_instances = 0;
