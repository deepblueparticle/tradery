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

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be
                                            // explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN  // Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the
// ones specified below. Refer to MSDN for the latest info on corresponding
// values for different platforms.
#ifndef WINVER  // Allow use of features specific to Windows 95 and Windows NT 4
                // or later.
#define WINVER \
  _WIN32_WINNT_WINXP  // Change this to the appropriate value to target Windows
                      // 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT  // Allow use of features specific to Windows NT 4 or
                      // later.
#define _WIN32_WINNT \
  _WIN32_WINNT_WINXP  // Change this to the appropriate value to target Windows
                      // 2000 or later.
#endif

#ifndef _WIN32_WINDOWS  // Allow use of features specific to Windows 98 or
                        // later.
#define _WIN32_WINDOWS \
  _WIN32_WINNT_WINXP  // Change this to the appropriate value to target Windows
                      // Me or later.
#endif

#include <afx.h>
#include <afxwin.h>  // MFC core and standard components

// TODO: reference additional headers your program requires here

#include <boost\static_assert.hpp>
#include <boost\type_traits.hpp>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <tradery.h>

#include <misc.h>
#include <strings.h>
#include <simlibplugin.h>
#include <common.h>
