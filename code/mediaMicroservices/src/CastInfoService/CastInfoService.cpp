#include <signal.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include "../utils.h"
#include "../utils_mongodb.h"
#include "CastInfoHandler.h"

using apache::thrift::protocol::TBinaryProtocolFactory;
using apache::thrift::server::TThreadedServer;
using apache::thrift::transport::TFramedTransportFactory;
using apache::thrift::transport::TServerSocket;
using json = nlohmann::json;
using namespace media_service;

void sigintHandler(int) {
  exit(EXIT_SUCCESS);
}

int main(int, char *[]) {
  signal(SIGINT, sigintHandler);
  init_logger();
  SetUpTracer("config/jaeger-config.yml", "cast-info-service");

  json config_json;
  if (load_service_config(&config_json) != 0) {
    exit(EXIT_FAILURE);
  }

  int port = config_json["cast-info-service"]["port"];
  mongoc_client_pool_t *mongodb_client_pool =
      init_mongodb_client_pool(config_json, "cast-info", MONGODB_POOL_MAX_SIZE);
  if (mongodb_client_pool == nullptr) {
    return EXIT_FAILURE;
  }

  TThreadedServer server(
      std::make_shared<CastInfoServiceProcessor>(
          std::make_shared<CastInfoHandler>(mongodb_client_pool)),
      std::make_shared<TServerSocket>("0.0.0.0", port),
      std::make_shared<TFramedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  std::cout << "Starting cast-info-service (read-only) ..." << std::endl;
  server.serve();
}
