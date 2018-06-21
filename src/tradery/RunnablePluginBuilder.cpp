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
#include "Process.h"
#include "RunnablePluginBuilder.h"
#include "BuildErrorsParser.h"
#include "SessionContext.h"

void build_path(string& path, const std::vector<std::string> paths,
                const std::string& type) {
  for_each(paths.begin(), paths.end(),
           [&](const std::string& p) { path += type + "\\\"" + p + "\\\" "; });
}

RunnablePluginBuilder::RunnablePluginBuilder(
    SessionContextPtr context, bool& _cancel) throw(RunProcessException)
    : _exitCode(0) {
  /*    std::cout << _T( "**** In RunnablePluginBuilder constructor****" ) <<
  std::endl; std::cout << cmdLine.getCmdLineString() << std::endl << std::endl;
  */
  std::ostringstream _cmdLine;

  // a local txt errors file. This is just for trace purposes, to see the
  // actuall compiler errors a sanitized html file will be generated
  std::string errorsFile =
      addFSlash(context->getSessionConfig()->getSessionPath()) + "errs.txt";

  string libpath;
  string includepath;

  std::string sessionPath = context->getSessionConfig()->getSessionPath();

  build_path(libpath, context->getConfig()->libPath(), "/LIBPATH:");
  build_path(includepath, context->getConfig()->includePaths(), "/I ");
  build_path(includepath, std::vector<std::string>{removeFSlash(sessionPath)},
             "/I ");

  _cmdLine << "/f \"" << addFSlash(context->getConfig()->projectPath())
           << "makefile.mak\" "
           << "INCLUDEPATH=\"" << includepath << "\" "
           << "LIBPATH=\"" << libpath << "\" "
           << "OUTDIR=\"" << removeFSlash(sessionPath) << "\" "
           << "INTDIR=\""
           << (addFSlash(context->getConfig()->outputPath()) + "common")
           << "\" "
           << "PROJDIR=\"" << context->getConfig()->projectPath() << "\" "
           << "BUILDERRORSFILE=\"" << errorsFile << "\" "
           << "TOOLSPATH=\"" << context->getConfig()->toolsPath() << "\" "
           << (context->getConfig()->debug() ? "CFG=Debug " : "")
      //<< _T( " /X \"c:\\build_errors.txt\"" )
      ;

  LOG1(log_debug, context->getSessionConfig()->getSessionId(),
       "make cmd line:\n"
           << _cmdLine.str());

  Environment env(*context->getConfig()->getEnvironment());

  env.add("TEMP", context->getConfig()->outputPath());

  LOG1(log_debug, context->getSessionConfig()->getSessionId(),
       "environment:\n"
           << env.toString());

  const ProcessResult pr(process(
      context, _cancel,
      addFSlash(context->getConfig()->toolsPath()) + "nmake.exe",
      _cmdLine.str(),
      std::auto_ptr<std::string>(
          new std::string(addFSlash(context->getConfig()->outputPath())))
          .get(),
      env));

  _exitCode = pr.exitCode();
  LOG1(log_debug, context->getSessionConfig()->getSessionId(),
       "[RunnablePluginBuilder constr] - exit code: " << _exitCode);

  ifstream ifs(errorsFile.c_str());

  if (ifs) {
    LOG1(log_debug, context->getSessionConfig()->getSessionId(),
         "reading the parsed errors");
    BuildErrorsParser parser(ifs, context);

    ifs.close();

    ofstream ofs(context->getSessionConfig()->getErrorsFile().c_str());
    if (ofs) {
      LOG1(log_debug, context->getSessionConfig()->getSessionId(),
           "writing the parsed errors");
      parser.write(ofs, true, 5);
    } else {
      // could not open the errors file for writing
    }
  }
}
