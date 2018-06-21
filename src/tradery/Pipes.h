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

class PipesException {};

#define BUFSIZE 8192

class Pipes {
 private:
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

 private:
  const std::wstring& _name;
  const unsigned long _count;

  PIPEINST* Pipe;
  HANDLE* hEvents;

 public:
  Pipes(const std::wstring& name, unsigned long count);
  virtual ~Pipes(void);

  void runServer() const;
  virtual void serverProcessing(const std::wstring& input) = 0;
  const std::wstring sendRequest(const std::wstring& request);
  void stop();

 private:
  void DisconnectAndReconnect(DWORD) const;
  bool ConnectToNewClient(HANDLE, LPOVERLAPPED) const;
  void GetAnswerToRequest(LPPIPEINST) const;
};
