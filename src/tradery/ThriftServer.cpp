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
#include "ThriftServer.h"
#include "SourceGenerator.h"
#include "configuration.h"
#include "ProcessingThreads.h"
#include "runtime_stats_impl.h"
#include "SessionConfig.h"
#include "SessionContext.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;
using namespace boost::filesystem;

class TraderyHandler : virtual public tradery_thrift_api::TraderyIf {
 public:
  TraderyHandler() {}

 private:
  void ensureDir(const std::string& dir) {
    if (exists(dir)) {
      remove_all(dir);
    }

    create_directories(dir);
  }

  bool ensureGeneratedDefinesFile(const std::string& sessionId,
                                  const std::string& fileName) {
    const unsigned __int64 MAX_TRY_COUNT = 1000;
    const unsigned int WAIT_DURATION = 10;  // ms

    // TODO
    // for some reason, the second run fails to get the file immediately, and
    // this wait loop will give it some time to "settle" or whatever is
    // happening there, and hopefully see the file at some point. After that one
    // case, everthing works fine with all other files To investigate further
    // and hopefully remove this hack.
    __int64 count = 0;
    while (!fileExists(fileName) && count < MAX_TRY_COUNT) {
      Sleep(WAIT_DURATION);
      ++count;
    }

    if (count < MAX_TRY_COUNT) {
      LOG1(log_info, sessionId,
           "generated defines file exists: " << fileName << ", count: " << count
                                             << ", " << count * WAIT_DURATION
                                             << "ms");
      return true;
    } else {
      LOG1(log_error, sessionId,
           "could not read generated defines file: "
               << fileName << ", count: " << count << ", "
               << count * WAIT_DURATION << "ms");
      return false;
    }
  }

 public:
  void startSession(tradery_thrift_api::ID& sessionId,
                    const tradery_thrift_api::SessionParams& sessionParams) {
    // Your implementation goes here

    ConfigurationPtr config(::getConfig());

    SessionConfigPtr sessionConfig(
        boost::make_shared<SessionConfig>(config->outputPath()));
    sessionId = sessionConfig->getSessionId();

    LOG1(log_info, sessionConfig->getSessionId(),
         "start session: " << sessionParams);
    const std::string& sessionDir = sessionConfig->getSessionPath();
    ensureDir(sessionDir);

    ThriftSystems systems(sessionParams.systems);
    SourceGenerator gen(systems);

    LOG1(log_info, sessionConfig->getSessionId(),
         "saving generated defines file: "
             << sessionConfig->getGeneratedDefinesFile());
    std::ofstream of(sessionConfig->getGeneratedDefinesFile(), ios::binary);
    of << *gen.generate();
    of.flush();
    of.close();

    if (!ensureGeneratedDefinesFile(sessionConfig->getSessionId(),
                                    sessionConfig->getGeneratedDefinesFile())) {
      std::string message("Could not read generated headers file: ");
      message += sessionConfig->getGeneratedDefinesFile();
      throw std::exception(message.c_str());
    }

    for (ThriftSystems::const_iterator i = systems.begin(); i != systems.end();
         ++i) {
      std::string systemFileName = sessionDir + i->getUUID() + ".h";
      LOG1(log_info, sessionConfig->getSessionId(),
           "saving generated system file: " << systemFileName);
      std::ofstream of(systemFileName, ios::binary);
      of << i->getCode();
      of.flush();
      of.close();

      sessionConfig->addRunnableId(i->getUUID());
    }

    std::ofstream symbolsFile(sessionConfig->getSymbolsFileName());
    std::copy(sessionParams.symbols.begin(), sessionParams.symbols.end(),
              std::ostream_iterator<std::string>(symbolsFile, " "));

    if (sessionParams.generateCharts) {
      // todo: see if we want to have a limit to # of charts, in Tradery 1.0
      // there was,
      // but it was set to -1 (unlimited).

      // this serializez the symbols into the symmchartfile
      std::ofstream scf(sessionConfig->getSymbolsToChartFile());
      std::copy(sessionParams.symbols.begin(), sessionParams.symbols.end(),
                std::ostream_iterator<std::string>(scf, " "));
      ensureDir(sessionConfig->getChartRootPath());
    }

    // todo: in the php version, the equity curve file
    // is appended an int at the end, incremented for each run
    // not sure why it's there for, as the files are deleted each time
    // a new session is started. For now using a straight name,
    // without an int

    // todo: apparently the trading engine only receive the file name
    // without extension, and will generate csv and png files
    // verify. (In the php code there is also the equity curve url).

    ProcessingThreads::run(boost::make_shared<SessionContext>(
        boost::make_shared<tradery_thrift_api::SessionParams>(sessionParams),
        sessionConfig, config));
  }

  bool heartbeat(const tradery_thrift_api::ID& sessionId) { return true; }

  void cancelSession(const tradery_thrift_api::ID& sessionId) {
    LOG1(log_info, sessionId, "");
  }

  virtual void getRuntimeStats(tradery_thrift_api::RuntimeStats& _return,
                               const tradery_thrift_api::ID& sessionId) {
    ConfigurationPtr config(::getConfig());

    SessionConfigPtr sessionConfig(
        boost::make_shared<SessionConfig>(sessionId, config->outputPath()));

    std::string runtimeStatsFile = sessionConfig->getRuntimeStatsFile();
    if (fileExists(runtimeStatsFile)) {
      //			LOG1(log_info, sessionId, "stats file name: " <<
      // runtimeStatsFile);
      std::ifstream rsf(runtimeStatsFile);
      std::stringstream rs_json;
      rs_json << rsf.rdbuf();

      std::string s(rs_json.str());
      //			LOG1(log_debug, sessionId, "runtime stats file
      // content: " << s);
      nlohmann::json j = nlohmann::json::parse(s);
      _return = RuntimeStatsImpl(j);

      //			LOG1(log_info, sessionId, "RuntimeStatsImpl: "
      //<< rs.toString());
    } else {
      LOG1(log_info, sessionId, "runtime stats file unavailable");
      _return = RuntimeStatsImpl();
    }
  }
};

void ThriftServer::run(ThreadContext* context) {
  int port = ::getConfig()->getThriftPort();

  LOG(log_info, "Starting thrift server thread on port: " << port);
  boost::shared_ptr<TraderyHandler> handler(new TraderyHandler());
  boost::shared_ptr<TProcessor> processor(
      new tradery_thrift_api::TraderyProcessor(handler));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(
      new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(
      new TBinaryProtocolFactory());

  server = boost::shared_ptr<TSimpleServer>(new TSimpleServer(
      processor, serverTransport, transportFactory, protocolFactory));

  server->serve();
}

void ThriftServer::start() { __super::start(); }

void ThriftServer::stop() {
  tradery::Lock lock(mutex);

  if (server) {
    server->stop();
  }
}

ThriftServer::ThriftServer() : Thread("Thrift Server Thread") {}

ThriftServer::~ThriftServer() {}
