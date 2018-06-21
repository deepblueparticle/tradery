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

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN  // Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the
// ones specified below. Refer to MSDN for the latest info on corresponding
// values for different platforms.
#ifndef WINVER  // Allow use of features specific to Windows 95 and Windows NT 4
                // or later.
#define WINVER \
  0x0500  // Change this to the appropriate value to target Windows 98 and
          // Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT  // Allow use of features specific to Windows NT 4 or
                      // later.
#define _WIN32_WINNT \
  0x0500  // Change this to the appropriate value to target Windows 98 and
          // Windows 2000 or later.
#endif

#ifndef _WIN32_WINDOWS  // Allow use of features specific to Windows 98 or
                        // later.
#define _WIN32_WINDOWS \
  0x0410  // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE  // Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE \
  0x0400  // Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be
                                            // explicit

// turns off MFC's hiding of some common and often safely ignored warning
// messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>   // MFC core and standard components
#include <afxext.h>   // MFC extensions
#include <afxdisp.h>  // MFC Automation classes

#include <afxdtctl.h>  // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>  // MFC support for Windows Common Controls
#endif               // _AFX_NO_AFXCMN_SUPPORT

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#define _USE_MATH_DEFINES 1
#include <math.h>

#include <strings.h>
#include <miscwin.h>
#include <thread.h>
#include <namevalue.h>
#include <datetime.h>
#include <miscfile.h>
#include <httprequest.h>
#include <log.h>
#include <objcounter.h>

#include <json/json.h>

#include <boost\program_options\cmdline.hpp>
#include <boost/program_options/environment_iterator.hpp>
#include <boost/program_options/eof_iterator.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/version.hpp>

// TODO: reference additional headers your program requires here
