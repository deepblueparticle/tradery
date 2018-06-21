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
#include "Configuration.h"
#include "service.h"
#include <params.h>
#include <tokenizer.h>
#include "tradery.h"
#include "defaults.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LPCSTR HELP = "help,?";
#define _TEST

// low case
LPCSTR DATASOURCEPATH = "datasourcepath,a";
LPCSTR SESSIONPATH = "sessionpath,b";
LPCSTR CONFIGFILEPATH = "configfilepath,c";
LPCSTR DATASOURCEID = "datasourceid,d";
LPCSTR TRADES_FILE = "tradesfile,e";
LPCSTR SIGNALS_FILE = "signalsfile,f";
LPCSTR DBG = "debug,g";
LPCSTR STATSHANDLERID = "statshandlerid,h";
LPCSTR INCLUDE = "include,i";
LPCSTR OUTPUTPATH = "outputpath,j";
LPCSTR INSTALLSERVICE = "installservice,k";
LPCSTR LIB = "lib,l";
LPCSTR TIMEOUT = "timeout,m";
LPCSTR SESSIONID = "sessionid,n";
LPCSTR SYMBOLSSOURCEID = "symbolssourceid,o";
LPCSTR PLUGIN_PATH = "pluginpath,p";
LPCSTR UNIQUE_ID = "uniqueid,q";
LPCSTR PROJECT_PATH = "projectpath,r";
LPCSTR STATS_FILE = "statsfile,s";
LPCSTR TOOLSPATH = "toolspath,t";
LPCSTR RUNNABLE = "runnable,u";
LPCSTR RUNSIMULATOR = "runsimulator,v";
LPCSTR REMOVESERVICE = "removeservice,w";
LPCSTR PLUGIN_EXT = "pluginext,x";
LPCSTR SYMBOLSSOURCEFILE = "symbolssourcefile,y";
LPCSTR ERRORSFILE = "errorsfile,z";

// caps
LPCSTR ASYNCHRONOUS_RUN = "asyncrun,A";
LPCSTR SYMBOL_TIMEOUT = "symboltimeout,B";
LPCSTR CANCELFILE = "cancelfile,C";
LPCSTR HEARTBEAT_TIMEOUT = "heartbeattimeout,D";
LPCSTR TRADES_CSV_FILE = "tradescsvfile,E";
LPCSTR SIGNALS_CSV_FILE = "signalscsvfile,F";
LPCSTR TRADES_DESCRIPTION_FILE = "tradesdescfile";
LPCSTR SIGNALS_DESCRIPTION_FILE = "signalsdescfile";
LPCSTR LINES_PER_PAGE = "linesperpage";
LPCSTR DEBUGSERVICE = "debugservice,G";
LPCSTR HEARTBEATFILE = "heartbeatfile,H";
LPCSTR ZIPFILE = "zipfile,I";
LPCSTR REVERSE_HEARTBEAT_PERIOD = "reverseheartbeatperiod,J";
LPCSTR RUNTIME_STATS_FILE = "runtimestatsfile,K";
LPCSTR LOGFILE = "logfile,L";
LPCSTR DEFCOMMISSIONVALUE = "defcommissionvalue,M";
LPCSTR ENDRUNSIGNALFILE = "endrunsignalfile,N";
LPCSTR OUTPUT_FILE = "outputfile,O";
LPCSTR LOCAL_ACCOUNT_PASSWORD = "password,P";
LPCSTR DEFSLIPPAGEVALUE = "defslippagevalue,Q";
LPCSTR FROM_DATETIME = "fromdatetime,R";
LPCSTR STATS_CSV_FILE = "statscsvfile,S";
LPCSTR TO_DATETIME = "todatetime,T";
LPCSTR LOCAL_ACCOUNT_USERNAME = "username,U";
LPCSTR REVERSEHEARTBEATFILE = "reverseheartbeatfile,V";
LPCSTR DEFSLIPPAGEID = "defslippageid,W";
LPCSTR DEFCOMMISSIONID = "defcommissionid,X";
LPCSTR CACHESIZE = "cachesize,Y";
LPCSTR STARTSERVICE = "startservice,Z";

// no one character shortcut options
LPCSTR MAX_LINES = "maxlines";
LPCSTR MAX_TOTAL_BAR_COUNT = "maxtotalbarcount";
LPCSTR FLAT_DATA = "flatdata";
LPCSTR EQUITY_CURVE_FILE = "equitycurvefile";
LPCSTR SYMBOLS_TO_CHART_FILE = "symchartfile";
LPCSTR CHART_DESCRIPTION_FILE = "chartdescriptionfile";
LPCSTR CHART_ROOT_PATH = "chartrootpath";

// position sizing options
LPCSTR INITIAL_CAPITAL = "initialcapital";
LPCSTR MAX_OPEN_POS = "maxopenpos";
LPCSTR POS_SIZE_TYPE = "possizetype";
LPCSTR POS_SIZE = "possize";
LPCSTR POS_SIZE_LIMIT_TYPE = "possizelimittype";
LPCSTR POS_SIZE_LIMIT = "possizelimit";
LPCSTR OS_PATH = "ospath";
LPCSTR SYSTEM_NAME_KEYWORD = "sysnamekword";
LPCSTR CLASS_NAME_KEYWORD = "classnamekword";

LPCSTR CPU_COUNT = "cpucount";
LPCSTR THREADS = "threads";
LPCSTR THREAD_ALG = "threadalg";

LPCSTR EXT_TRIGGERS_FILE = "exttriggersfile";

LPCSTR EXPLICIT_TRADES_EXT = "explicittradesext";
LPCSTR DATA_ERROR_HANDLING_MODE = "dataerrorhandling";

LPCSTR START_TRADES_DATE = "starttradesdate";

LPCSTR ENV_PATH = "envpath";
LPCSTR ENV_INCLUDE = "envinclude";
LPCSTR ENV_LIB = "envlib";

LPCSTR ENABLE_RUN_AS_USER = "enablerunasuser";

LPCSTR THRIFT_PORT = "thriftport";

LPCSTR CONFIG_FILE = "configfile";

void Configuration::removeModuleName() {
  // GetCommandLine also returns the full application path so remove it
  CFileFind fileFinder;

  TCHAR fileName[MAX_PATH + 1];

  GetModuleFileName(0, fileName, MAX_PATH);
  if (fileFinder.FindFile(fileName)) {
    fileFinder.FindNextFile();
    _applicationPath = ws2s(fileFinder.GetRoot().GetString());
  }

  for (vector<std::string>::iterator i = _args.begin(); i != _args.end(); i++) {
    // remove the arg with the tool name in it if any
    std::wstring::size_type index = to_lower_case(*i).find(
        to_lower_case(ws2s(fileFinder.GetFileName().GetString())));

    if (index != std::wstring::npos) {
      _args.erase(i);
      break;
    }
  }
}

std::string parse_config_file(const std::string& configFile) {
  ifstream ifs(configFile.c_str());
  if (!ifs)
    throw ConfigurationException(
        std::string("Could not open the configuration file ") << configFile,
        true);

  stringstream ss;

  ss << ifs.rdbuf();
  std::string sep("\n\r");
  Tokenizer tok(ss.str(), sep);

  std::string s;

  for (Tokenizer::const_iterator i = tok.begin(); i != tok.end(); ++i) {
    std::string token(tradery::trim(*i));

    if (!token.empty() && token[0] != '#') {
      std::wstring::size_type n = token.find_first_of(TCHAR('='));

      if (n != std::string::npos) {
        token[n] = ' ';
      }
      s += "--" + token + " ";
    }
  }

  return s;
}

class ConfigVariablesMap : public po::variables_map {
 public:
  bool contains(const std::string& name) const {
    return __super::count(name) > 0;
  }
};

void Configuration::init(const std::string& cmdLine, bool validate) {
  //	DebugBreak();
  _inputCommandLine = cmdLine;
  //	std::cout << _inputCommandLine << std::endl;
  //  std::cout << _T( "*************" ) << std::endl << _inputCommandLine <<
  //  _T( "*************" ) << std::endl; ofs << "in cmd line - cmdline string:
  //  " << cmdLine << std::endl;
  po::options_description cmd;
  po::options_description visible("Allowed options");

  try {
    // Declare a group of options that will be
    // allowed only on command line
    po::options_description desc("Tradery options");
    desc.add_options()(HELP, "shows this help message")(
        THRIFT_PORT,
        po::value<unsigned int>()->default_value(DEFAULT_THRIFT_PORT),
        "thrift server port")(SESSIONID, po::value<std::string>(),
                              "the session unique id")(
        SYMBOLSSOURCEFILE, po::value<std::string>(), "the symbols file")(
        UNIQUE_ID, "generates and unique id on the command line")(
        RUNSIMULATOR,
        "run just the simulation part. If this option is not on the command "
        "line, then build first then call itself with this option")(
        ERRORSFILE, po::value<std::string>()->default_value(""),
        "the file containing the build errors, if any, to be reported to the "
        "user")(
        PLUGIN_PATH, po::value<std::string>()->default_value(""),
        "the path for the built-in plug-ins (datasource, symbolssource etc)")(
        PLUGIN_EXT, po::value<std::string>()->default_value(""),
        "sets the plugin file extesion")(DATASOURCEID, po::value<std::string>(),
                                         "the data source for the session")(
        SYMBOLSSOURCEID, po::value<std::string>(),
        "the symbols source for the session")(
        STATSHANDLERID, po::value<std::string>(),
        "the stats handler for the session - will calculate statistics and "
        "save them to a file")(
        TOOLSPATH, po::value<std::string>()->default_value(""),
        "sets the path for MS tools, such as nmake, cl etc")(
        LIB, po::value<std::vector<std::string> >()->composing(),
        "sets path to the simlib lib files")(
        INCLUDE, po::value<vector<std::string> >()->composing(),
        "the path to the simlib include files")(
        OUTPUTPATH, po::value<std::string>(),
        "the runnable plug-in project path")
        // we'll be expecting 2 includes, one for the simlib includes, the other
        // one for the files generated at runtime
        (DATASOURCEPATH, po::value<std::string>(), "the datasource path")(
            CONFIGFILEPATH, po::value<std::string>(), "the config file path")(
            DBG, "makes the debug version")(
            SIGNALS_FILE, po::value<std::string>()->default_value(""),
            "the file that will contain the trading signals generated during "
            "the run")(TRADES_FILE, po::value<std::string>()->default_value(""),
                       "the file that will contain the simulated historical "
                       "trades generated during the run")(
            STATS_FILE, po::value<std::string>()->default_value(""),
            "the file that will contain the back-test statistics for all the "
            "trades generated during the run")(
            OUTPUT_FILE, po::value<std::string>()->default_value(""),
            "the file that will contain the formatted text output by the "
            "system")(FROM_DATETIME,
                      po::value<std::string>()->default_value(""),
                      "time range starting date/time in iso format. If absent, "
                      "it will mean Negative Infinity")(
            TO_DATETIME, po::value<std::string>()->default_value(""),
            "time range ending date/time in iso format. If absent, it will "
            "mean Positive Infinity")(
            SIGNALS_CSV_FILE, po::value<std::string>()->default_value(""),
            "file containing signals in csv format")(
            TRADES_CSV_FILE, po::value<std::string>()->default_value(""),
            "file containing trades in csv format")(
            STATS_CSV_FILE, po::value<std::string>()->default_value(""),
            "file containing stats in csv format")(
            EQUITY_CURVE_FILE, po::value<std::string>()->default_value(""),
            "the base name for files containing equity curve data: csv, htm, "
            "jpg")(TIMEOUT, po::value<unsigned long>(),
                   "timeout in miliseconds, after which the run stops")(
            INSTALLSERVICE, "installs the service")(
            REMOVESERVICE, "removes (uninstalls) the service")(
            DEBUGSERVICE, "starts the service in debug mode (with a cosole)")(
            STARTSERVICE,
            "called by the service manager when the service is about to be "
            "started")(
            LOCAL_ACCOUNT_PASSWORD, po::value<std::string>(),
            "password for the local account in which the simulation will run")(
            LOCAL_ACCOUNT_USERNAME, po::value<std::string>(),
            "user name for the local account in which the simulation will run")(
            PROJECT_PATH, po::value<std::string>(),
            "path to the project that will build the plug-ins")(
            ZIPFILE, po::value<std::string>(),
            "zip file containing the result of the run (all cvs files")(
            ENDRUNSIGNALFILE, po::value<std::string>()->default_value(""),
            "if this is present on the command line, the end of a run will be "
            "signaled by creating this file - used in case of an asynchronous "
            "run")(ASYNCHRONOUS_RUN,
                   "when present on the command line, it indicates that the "
                   "run will be asynchronous - the call will return "
                   "immediately, while the processing is still going on")(
            HEARTBEATFILE, po::value<std::string>(),
            "file that will signal to the running process that the client is "
            "still waiting for the result. If the file doesn't exist for a "
            "specified amount of time, the process will be terminated")(
            REVERSEHEARTBEATFILE, po::value<std::string>(),
            "file generated with a certain period, that indicates that the "
            "process is still running. If the file is not generated for a "
            "speicified amount of time, the client can assume that the server "
            "process is not running any more ")(
            CANCELFILE, po::value<std::string>(),
            "file that will be created as a result of the client requesting "
            "that the server cancels the processing")(
            SYMBOL_TIMEOUT,
            po::value<unsigned __int64>()->default_value(
                DEFAULT_SYMBOL_TIMEOUT),
            "symbol timeout in seconds - if a symbol processing takes longer "
            "than this amount, the processing will be canceled")(
            HEARTBEAT_TIMEOUT,
            po::value<unsigned __int64>()->default_value(
                DEFAULT_HEARTBEAT_TIMEOUT),
            "heartbeat timeout in seconds - used to keep the server processing "
            "a session alive, if a heartbeat signal is not received within "
            "this number of seconds, the processing is canceled")(
            REVERSE_HEARTBEAT_PERIOD,
            po::value<unsigned __int64>()->default_value(
                DEFAULT_REVERSE_HEARTBEAT_PERIOD),
            "the period of the hearbeat signal generated during processing, it "
            "will be used to keep the client alive")(
            INITIAL_CAPITAL,
            po::value<double>()->default_value(DEFAULT_INITIAL_CAPITAL),
            "the intial capital value used in simulation")(
            RUNTIME_STATS_FILE, po::value<std::string>()->default_value(""),
            "file that will contain runtime stats such elapsed time, number of "
            "errors, of trades etc")(
            CACHESIZE,
            po::value<unsigned __int64>()->default_value(DEFAULT_CACHE_SIZE),
            "the internal simlib cache max number of items, currently used "
            "only because symbol info is needed to calculate stats")(
            DEFSLIPPAGEVALUE,
            po::value<double>()->default_value(DEFAULT_SLIPPAGE_VALUE),
            "the default slippage value")(
            DEFCOMMISSIONVALUE,
            po::value<double>()->default_value(DEFAULT_COMMISION_VALUE),
            "the default commission value")(
            DEFSLIPPAGEID, po::value<std::string>()->default_value(""),
            "the default slippage plugin config id")(
            DEFCOMMISSIONID, po::value<std::string>()->default_value(""),
            "the default commission plugin config id")(
            FLAT_DATA, po::value<bool>()->default_value(false), "")(
            MAX_LINES,
            po::value<unsigned __int64>()->default_value(
                DEFAULT_MAX_LINES_PER_FILE),
            "max number of lines in file such as trades, etc. If this file "
            "exceeded, show a message")(
            MAX_TOTAL_BAR_COUNT,
            po::value<unsigned __int64>()->default_value(
                DEFAULT_MAX_BARS_PER_SESSION),
            "max total number of bars per session - used to limit the the "
            "usage to a max number of total bars, this may translate in a lot "
            "of symbols, few bars, or few symbols, many bars")(
            SESSIONPATH, po::value<std::string>(),
            "path where all the session activity happens - it is normally the "
            "output path + the session id")(
            MAX_OPEN_POS,
            po::value<unsigned __int64>()->default_value(
                DEFAULT_MAX_OPEN_POSITIONS),
            "the maximum position open at any one time during simulation")(
            SYMBOLS_TO_CHART_FILE, po::value<std::string>()->default_value(""),
            "The file containing a list of symbols for which the trading "
            "engine will generate charting data. The trading engine will "
            "ignore the charting statements when running on any other symbol")(
            CHART_DESCRIPTION_FILE, po::value<std::string>()->default_value(""),
            "The file containing the description of all charting info "
            "generated during the run. Will be used by the php script to "
            "generate the actual charts")(
            CHART_ROOT_PATH, po::value<std::string>()->default_value(""),
            "The directory where all the chart info will be stored - "
            "description files, data, images")(
            EXT_TRIGGERS_FILE, po::value<std::string>()->default_value(""),
            "if this option is present, this will be interpreted as a file "
            "containing a list of triggers that the simulator will use to "
            "generate trades")(
            POS_SIZE_TYPE,
            po::value<unsigned long>()->default_value(
                (unsigned long)DEFAULT_POS_SIZE_TYPE),
            "position size type: 0-4 (system, shares, value, pct eq, pct cash")(
            POS_SIZE, po::value<double>(), "position size actual value")(
            POS_SIZE_LIMIT_TYPE,
            po::value<unsigned long>()->default_value(
                DEFAULT_POS_SIZE_LIMIT_TYPE),
            "position size type: 0-2 (none, pct volume, value")(
            POS_SIZE_LIMIT, po::value<double>(), "the limit value")(
            OS_PATH, po::value<std::string>(), "The OS path")(
            SYSTEM_NAME_KEYWORD, po::value<std::string>(),
            "The system name keyword")(CLASS_NAME_KEYWORD,
                                       po::value<std::string>(),
                                       "The class name keyword")(
            TRADES_DESCRIPTION_FILE,
            po::value<std::string>()->default_value(""),
            "The file where the trades description will be stored")(
            SIGNALS_DESCRIPTION_FILE,
            po::value<std::string>()->default_value(""),
            "The file where the signals description will be stored")(
            LINES_PER_PAGE,
            po::value<size_t>()->default_value(DEFAULT_LINES_PER_PAGE),
            "The number of lines per page - used to store info about line "
            "positions in the file for faster access")(
            CPU_COUNT,
            po::value<unsigned long>()->default_value(DEFAULT_CPU_COUNT),
            "number of cpus")(
            THREADS,
            po::value<unsigned long>()->default_value(DEFAULT_THREAD_COUNT),
            "number of threads")(THREAD_ALG,
                                 po::value<unsigned long>()->default_value(
                                     DEFAULT_THREADING_ALGORITHM),
                                 "threading algorithm")  // default is one
                                                         // system will be run
                                                         // on multiple threads
                                                         // if possible
        (EXPLICIT_TRADES_EXT, po::value<std::string>(),
         "Explicit trades files extension")(
            DATA_ERROR_HANDLING_MODE,
            po::value<unsigned int>()->default_value(
                DEFAULT_DATA_ERROR_HANDLING_MODE),
            "Data error handling mode - 0: fatal, 1: warning, 2: ignore")(
            START_TRADES_DATE, po::value<std::string>()->default_value(""),
            "cutoff date for trades (no trades before this date, used for "
            "automated trading)")(LOGFILE,
                                  po::value<std::string>()->default_value(""),
                                  "log file name")(
            ENV_PATH, po::value<std::string>()->default_value(""),
            "environment variable PATH")(
            ENV_INCLUDE, po::value<std::string>()->default_value(""),
            "environment variable include")(
            ENV_LIB, po::value<std::string>()->default_value(""),
            "enviroment variable lib")(
            ENABLE_RUN_AS_USER,
            po::value<bool>()->default_value(DEFAULT_RUN_AS_USER),
            "enable/disable running as user")(
            CONFIG_FILE, po::value<std::vector<std::string> >(),
            "configuration file(s) name")(
            RUNNABLE, po::value<std::vector<std::string> >(), "runnable");

    po::positional_options_description p;
    p.add("runnable", -1);

    _args = tradery::cmdLineSplitter(cmdLine);

    // remove the executable name
    removeModuleName();

    ConfigVariablesMap vm;
    po::store(po::command_line_parser(_args).options(desc).positional(p).run(),
              vm);
    po::notify(vm);

    if (vm.contains(CONFIG_FILE)) {
      std::vector<std::string> configFiles(
          vm[CONFIG_FILE].as<std::vector<std::string> >());

      for (std::vector<std::string>::size_type n = 0; n < configFiles.size();
           ++n) {
        std::string s(parse_config_file(configFiles[n].c_str()));

        std::vector<std::string> configFileArgs = cmdLineSplitter(s);
        po::store(po::command_line_parser(configFileArgs)
                      .options(desc)
                      .positional(p)
                      .run(),
                  vm);
      }
    }

    // start logging immediately
    if (vm.contains("logfile")) _logFile = vm["logfile"].as<std::string>();

    hasLogFile() ? Log::setLogToFile(logFile()) : Log::setLogToConsole();

    LOG(log_debug, "input command line: " << cmdLine);

    LOG(log_debug, "args");
    for (size_t i = 0; i < _args.size(); ++i)
      LOG(log_debug, "args[" << i << "] = " << _args[i]);
    // handle standalone command line options

    if (vm.contains("help")) help(visible);

    if (vm.contains("uniqueid")) {
      _uniqueId = true;
      return;
    } else if (vm.contains("installservice")) {
      LOG(log_debug, "Installing service");
      _installService = true;
      return;
    }

    else if (vm.contains("removeservice")) {
      LOG(log_debug, "Removing service");
      _removeService = true;
      return;
    }

    else if (vm.contains("debugservice")) {
      LOG(log_debug, "Debugging service");
      _debugService = true;
      return;
    }

    else if (vm.contains("startservice")) {
      LOG(log_debug, "Starting service");
      _startService = true;
      return;
    }

    LOG(log_debug, "Processing command line: " << cmdLine);

    _fromDateTime = vm["fromdatetime"].as<std::string>();
    _toDateTime = vm["todatetime"].as<std::string>();

    _startTradesDateTime = vm[START_TRADES_DATE].as<std::string>();

    LOG(log_debug, "from - to: " << _fromDateTime << "-" << _toDateTime);

    if (_fromDateTime > _toDateTime) {
      // error - do something
    }

    _runSimulator = vm.contains("runsimulator");

    if (vm.contains("username")) _userName = vm["username"].as<std::string>();

    LOG(log_debug, "reading the password");
    if (vm.contains("password")) _password = vm["password"].as<std::string>();

    LOG(log_debug, "reading the runnables");
    if (vm.contains("runnable")) {
      _runnables = vm["runnable"].as<vector<std::string> >();
      LOG(log_debug, "runnables count: " << vm.contains("runnable"));
    }

    if (vm.contains("include"))
      _includePaths = vm["include"].as<vector<std::string> >();

    if (vm.contains("lib")) _libPath = vm["lib"].as<vector<std::string> >();

    if (vm.contains("toolspath"))
      _toolsPath = vm["toolspath"].as<std::string>();

    if (vm.contains("projectpath"))
      _projectPath = vm["projectpath"].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading outputpath");
#endif
    if (vm.contains("outputpath"))
      _outputPath = vm["outputpath"].as<std::string>();

#ifdef _TEST
//        std::cout << "cmd line 8" << std::endl;
#endif

      /*
      #ifdef _TEST
          std::cout << "cmd line 9" << std::endl;
      #endif
          if( vm.contains( "sessionid" )  )
            _sessionId = vm[ "sessionid" ].as< std::string >();
      */

#ifdef _TEST
    LOG(log_debug, "reading symbolssourceid");
#endif
    if (vm.contains("symbolssourceid"))
      _symbolsSource = vm["symbolssourceid"].as<std::string>();

    if (vm.contains("statshandlerid"))
      _statsHandler = vm["statshandlerid"].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading symbolssourcefile");
#endif

    if (vm.contains("symbolssourcefile"))
      _symbolsSourceFile = vm["symbolssourcefile"].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading datasourcepath");
#endif
    if (vm.contains("datasourcepath"))
      _dataSourcePath = vm["datasourcepath"].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading datasourceid");
#endif
    if (vm.contains("datasourceid"))
      _dataSource = vm["datasourceid"].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading pluginpath");
#endif
    if (vm.contains("pluginpath")) {
      _pluginPath = vm["pluginpath"].as<std::string>();
      _overridePluginPath = true;
    } else
      _overridePluginPath = false;

    _errorsFile = vm["errorsfile"].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading pluginext");
#endif
    if (vm.contains("pluginext")) {
      _pluginExt = vm["pluginext"].as<std::string>();
      _overridePluginExt = true;
    } else
      _overridePluginExt = false;
#ifdef _TEST
    LOG(log_debug, "reading debug");
#endif
    _debug = vm.contains("debug");

#ifdef _TEST
    LOG(log_debug, "reading signalsfile");
#endif
    _signalsFile = vm["signalsfile"].as<std::string>();
#ifdef _TEST
    LOG(log_debug, "reading tradesfile");
#endif

    _tradesFile = vm["tradesfile"].as<std::string>();
#ifdef _TEST
    LOG(log_debug, "reading statsfile");
#endif

    _statsFile = vm["statsfile"].as<std::string>();
#ifdef _TEST
    LOG(log_debug, "reading outputfile");
#endif

    _outputFile = vm["outputfile"].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading tradescsvfile");
#endif
    _tradesCSVFile = vm["tradescsvfile"].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading signalscsvfile");
#endif
    _signalsCSVFile = vm["signalscsvfile"].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading statscsvfile");
#endif
    _statsCSVFile = vm["statscsvfile"].as<std::string>();

    _equityCurveFile = vm[EQUITY_CURVE_FILE].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading timeout");
#endif
    if (vm.contains("timeout")) _timeout = vm["timeout"].as<unsigned long>();

#ifdef _TEST
    LOG(log_debug, "reading zipfile");
#endif
    if (vm.contains("zipfile")) _zipFile = vm["zipfile"].as<std::string>();

#ifdef _TEST
    LOG(log_debug, "reading statscsvfile");
#endif

#ifdef _TEST
    LOG(log_debug, "reading heartbeatfile");
#endif

    _thriftPort = vm["thriftport"].as<unsigned int>();
    if (vm.contains("heartbeatfile"))
      _heartBeatFile = vm["heartbeatfile"].as<std::string>();

    if (vm.contains("reverseheartbeatfile"))
      _reverseHeartBeatFile = vm["reverseheartbeatfile"].as<std::string>();

    if (vm.contains("cancelfile"))
      _cancelFile = vm["cancelfile"].as<std::string>();

    _symbolTimeout = vm["symboltimeout"].as<unsigned __int64>();
    _reverseHeartBeatPeriod =
        vm["reverseheartbeatperiod"].as<unsigned __int64>();
    _heartBeatTimeout = vm["heartbeattimeout"].as<unsigned __int64>();

    _endRunSignalFile = vm["endrunsignalfile"].as<std::string>();

    _asyncRun = vm.contains("asyncrun");

    if (vm.contains(INITIAL_CAPITAL))
      _posSizingParams.setInitialCapital(vm[INITIAL_CAPITAL].as<double>());
    if (vm.contains(MAX_OPEN_POS))
      _posSizingParams.setMaxOpenPos(vm[MAX_OPEN_POS].as<unsigned __int64>());
    if (vm.contains(POS_SIZE_TYPE))
      _posSizingParams.setPosSizeType(
          (PositionSizingParams::PosSizeType)vm[POS_SIZE_TYPE]
              .as<unsigned long>());
    if (vm.contains(POS_SIZE))
      _posSizingParams.setPosSize(vm[POS_SIZE].as<double>());
    if (vm.contains(POS_SIZE_LIMIT_TYPE))
      _posSizingParams.setPosSizeLimitType(
          (PositionSizingParams::PosSizeLimitType)vm[POS_SIZE_LIMIT_TYPE]
              .as<unsigned long>());
    if (vm.contains(POS_SIZE_LIMIT))
      _posSizingParams.setPosSizeLimit(vm[POS_SIZE_LIMIT].as<double>());

    _runtimeStatsFile = vm["runtimestatsfile"].as<std::string>();

    _cacheSize = vm["cachesize"].as<unsigned __int64>();
    _defSlippageValue = vm["defslippagevalue"].as<double>();
    //    std::cout << "in cmd line, slippage value: " << _defSlippageValue;
    _defCommissionValue = vm["defcommissionvalue"].as<double>();
    _defCommissionId = vm["defcommissionid"].as<std::string>();
    _defSlippageId = vm["defslippageid"].as<std::string>();
    _maxLines = vm["maxlines"].as<unsigned __int64>();
    _maxTotalBarCount = vm[MAX_TOTAL_BAR_COUNT].as<unsigned __int64>();
    _symbolsToChartFile = vm[SYMBOLS_TO_CHART_FILE].as<std::string>();
    _chartRootPath = vm[CHART_ROOT_PATH].as<std::string>();
    _chartDescriptionFile = vm[CHART_DESCRIPTION_FILE].as<std::string>();
    _extTriggersFile = vm[EXT_TRIGGERS_FILE].as<std::string>();
    _systemNameKeyword = vm[SYSTEM_NAME_KEYWORD].as<std::string>();
    _classNameKeyword = vm[CLASS_NAME_KEYWORD].as<std::string>();
    _tradesDescriptionFile = vm[TRADES_DESCRIPTION_FILE].as<std::string>();
    _signalsDescriptionFile = vm[SIGNALS_DESCRIPTION_FILE].as<std::string>();
    _linesPerPage = vm[LINES_PER_PAGE].as<size_t>();
    _cpuCount = vm[CPU_COUNT].as<unsigned long>();
    _threads = vm[THREADS].as<unsigned long>();
    _threadAlg = ThreadAlgorithm(vm[THREAD_ALG].as<unsigned long>());
    if (vm.contains(EXPLICIT_TRADES_EXT))
      _explicitTradesExt = vm[EXPLICIT_TRADES_EXT].as<std::string>();

    _dataErrorHandlingMode =
        (ErrorHandlingMode)vm[DATA_ERROR_HANDLING_MODE].as<unsigned int>();

    if (vm.contains(OS_PATH)) _osPath = vm[OS_PATH].as<std::string>();

    if (vm.contains("sessionpath"))
      _sessionPath = vm["sessionpath"].as<std::string>();

    _envPath = vm[ENV_PATH].as<std::string>();

    TCHAR path[8000] = {0};
    GetEnvironmentVariable(_T( "path" ), path, 8000);

    boost::replace_all(_envPath, "%path%", ws2s(path));
    _envInclude = vm[ENV_INCLUDE].as<std::string>();
    _envLib = vm[ENV_LIB].as<std::string>();

    _enableRunAsUser = vm[ENABLE_RUN_AS_USER].as<bool>();

    //    _flatData = vm[ "flatdata" ].as< std::wstring >();
    LOG(log_debug, "cmd line processing done");
#ifdef _TEST
    LOG(log_debug, "cmd line end");

#endif
    if (validate) this->validate();
  } catch (exception& e) {
    LOG(log_error, "exception: " << e.what());
    //    help( visible );
  }
}

Configuration::Configuration(const std::string& cmdLine, bool validate) {
  init(cmdLine, validate);
}

Configuration::Configuration(bool validate) {
  init(ws2s(::GetCommandLine()), validate);
}

Configuration::Configuration(const char* cmdLine, bool validate) {
  init(std::string(cmdLine), validate);
}

void Configuration::set(
    const tradery_thrift_api::SessionParams& sessionContext) {
  if (sessionContext.__isset.explicitTradesExt) {
    _explicitTradesExt = sessionContext.explicitTradesExt;
  }
}

tradery::EnvironmentPtr Configuration::getEnvironment() const {
  tradery::StrVector envVars;

  envVars.push_back(std::string("SystemRoot=") << osPath());
  envVars.push_back(std::string("path= ") + envPath());
  envVars.push_back(std::string("include=") + envInclude());
  envVars.push_back(std::string("lib=") + envLib());

  tradery::EnvironmentPtr env(new tradery::Environment(envVars));

  return env;
}

void Configuration::validate() const {
  if (!hasUserName()) {
    throw ConfigurationException("missing username argument", true);
    ;
  }

  if (!hasPassword()) {
    throw ConfigurationException("missing password argument", true);
  }

  if (!hasRunnables()) {
    throw ConfigurationException("missing runnables argument", true);
  }

  if (!hasInclude()) {
    throw ConfigurationException("missing include argument", true);
  }

  if (!hasLib()) {
    throw ConfigurationException("missing lib argument", true);
  }

  if (!hasToolsPath()) {
    throw ConfigurationException("missing toolspath argument", true);
  }

  if (!hasProjectPath()) {
    throw ConfigurationException("missing projectpath argument", true);
  }

  if (!hasOutputPath()) {
    throw ConfigurationException("missing outputpath argument", true);
  }

  if (!hasSymbolsSourceId()) {
    throw ConfigurationException("missing symbolssourceid argument", true);
  }

  if (!hasStatsHandler()) {
    throw ConfigurationException("missing statshandlerid argument", true);
  }

  if (!hasSymbolsSourceFile()) {
    throw ConfigurationException("missing symbolssourcefile argument", true);
  }

  if (!hasDataSourcePath()) {
    throw ConfigurationException("missing datasourcepath argument", true);
  }

  if (!hasDataSourceId()) {
    throw ConfigurationException("missing datasourceid argument", true);
  }

  if (!hasOsPath()) {
    throw ConfigurationException("missing ospath argument", true);
  }

  if (!hasSessionPath()) {
    throw ConfigurationException("missing sessionpath argument", true);
  }

  if (!hasExplicitTradesExt()) {
    throw ConfigurationException("missing explicit trades extension", true);
  }
}
