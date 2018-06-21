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

#pragma warning(disable : 4786)
#pragma warning(disable : 4251)

// Insert your headers here
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers

#ifdef _UNICODE
#if !defined UNICODE
#define UNICODE
#endif
#endif  //_UNICODE

#ifdef UNICODE
#if !defined _UNICODE
#define _UNICODE
#endif
#endif  // UNICODE

#include <windows.h>
#include <process.h>
#include <typeinfo>
#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <queue>
#include <assert.h>
#include <set>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <sys/timeb.h>
#include <iomanip>
#include <fcntl.h>
#include <io.h>
#include <functional>
#include <float.h>
#include <math.h>

// C includes
#include <assert.h>

#pragma warning(disable : 4786)
#pragma warning(disable : 4275)
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#pragma warning(default : 4275)

// TA-LIB includes
#include <ta_func.h>

#include <nlohmann/json.hpp>

#include <exception>
#include <core.h>
#include <common.h>
#include <misc.h>
#include <stats.h>

//{{AFX_INSERT_LOCATION}}
