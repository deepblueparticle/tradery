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

#include <iostream>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be
                                            // explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN  // Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>    // MFC core and standard components
#include <afxext.h>    // MFC extensions
#include <afxdtctl.h>  // MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>  // MFC support for Windows Common Controls
#endif               // _AFX_NO_AFXCMN_SUPPORT

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be
                                            // explicit

#include <atlbase.h>

#include <sstream>
#include <string>
#include <stdio.h>
#include <process.h>
#include <tchar.h>
#include <userenv.h>
#include <winsvc.h>

// TODO: reference additional headers your program requires here
#include <vector>
#include <queue>
#include <ctime>
#include <cassert>
#include <iomanip>

#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <boost\static_assert.hpp>
#include <boost\type_traits.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/filter_view.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/vector.hpp>

#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/static_assert.hpp>

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
#include <boost/algorithm/string.hpp>
#include <boost\regex.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>
#include <boost/type_index.hpp>

#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include <tradery.h>

#include <nlohmann\json.hpp>

#include <misc.h>
#include <miscwin.h>
#include <miscfile.h>
#include <thread.h>
#include <core.h>
#include <common.h>
#include <versionno.h>
#include <stats.h>
#include <log.h>
#include <resourcewrapper.h>

#include <plugin.h>

#include <Tradery.h>
#include "..\plugin\plugin.h"
#include "..\plugin\plugintree.h"

#include "session.h"

#include <ZipArchive.h>
