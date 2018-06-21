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
#include <plugin.h>
#include "plugin.h"
#include <log.h>

void PluginExplorer::initIgnoreModulesSet(
    std::set<std::string>& ignoreModulesSet) {
  ignoreModulesSet.insert(tradery::to_lower_case("simlib.dll"));
  ignoreModulesSet.insert(tradery::to_lower_case("misc.dll"));
  ignoreModulesSet.insert(tradery::to_lower_case("miscwin.dll"));
  ignoreModulesSet.insert(tradery::to_lower_case("datasource.dll"));
  ignoreModulesSet.insert(tradery::to_lower_case("chartdir60.dll"));
  ignoreModulesSet.insert(tradery::to_lower_case("libcrypto-1_1.dll"));
  ignoreModulesSet.insert(tradery::to_lower_case("libssl-1_1.dll"));
}

bool PluginExplorer::ignoreModule(const std::string& _fileName) {
  static std::set<std::string> ignoreModules;

  initIgnoreModulesSet(ignoreModules);

  const std::string fileName(tradery::to_lower_case(_fileName));

  return ignoreModules.find(fileName) != ignoreModules.end();
}

void PluginExplorer::explore(const std::string& p, const std::string& ext,
                             bool recursive,
                             PluginLoadingStatusHandler* loadingStatusHandler,
                             std::vector<InfoPtr>& duplicates) {
  std::string path = p.length() == 0 ? "." : p;

  LOG(log_info, std::string("exploring path: ") + path);
  {
    std::string fileName(tradery::addFSlash(path) + "*.*");

    CFileFind finder;

    BOOL bWorking = finder.FindFile(s2ws(fileName.c_str()).c_str());
    while (bWorking) {
      bWorking = finder.FindNextFile();

      // skip . and .. files; otherwise, we'd
      // recur infinitely!
      if (finder.IsDots()) continue;

      // if it's a directory, recursively search it
      if (finder.IsDirectory() && recursive)
        explore(ws2s(finder.GetFilePath().GetString()), ext, recursive,
                loadingStatusHandler, duplicates);
    }
    finder.Close();
  }
  {
    std::string fileName(tradery::addFSlash(path));

    if (ext.length() == 0)
      fileName += "*.*";
    else if (fileName.at(0) == '.')
      fileName += std::string("*") + ext;
    else
      fileName += std::string("*.") + ext;

    CFileFind finder;

    BOOL bWorking = finder.FindFile(s2ws(fileName).c_str());
    while (bWorking) {
      bWorking = finder.FindNextFile();

      // skip . and .. files; otherwise, we'd
      // recur infinitely!
      if (finder.IsDots()) continue;

      if (finder.IsDirectory() ||
          ignoreModule(ws2s(finder.GetFileName().GetString())))
        continue;
      else {
        std::string filePath(ws2s(finder.GetFilePath().GetString()));
        LOG_PLUGIN(log_info, std::string("found file: ") + filePath);

        process(filePath, loadingStatusHandler, duplicates);
      }
    }
    finder.Close();
  }
}
