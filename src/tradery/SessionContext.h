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

#include "SessionConfig.h"
#include "Configuration.h"

class SessionContext {
 private:
  const boost::shared_ptr<tradery_thrift_api::SessionParams> _sessionParams;
  const SessionConfigPtr _sessionConfig;
  const ConfigurationPtr _config;

 public:
  SessionContext(
      boost::shared_ptr<tradery_thrift_api::SessionParams> sessionParams,
      SessionConfigPtr sessionConfig, const ConfigurationPtr config)
      : _sessionParams(sessionParams),
        _sessionConfig(sessionConfig),
        _config(config) {}

  SessionContext(const std::string& cmdLineString) {
    // TODO implement
  }

  const boost::shared_ptr<tradery_thrift_api::SessionParams> getSessionParams()
      const {
    return _sessionParams;
  }

  const SessionConfigPtr getSessionConfig() const { return _sessionConfig; }

  const ConfigurationPtr getConfig() const { return _config; }
};

typedef boost::shared_ptr<SessionContext> SessionContextPtr;
