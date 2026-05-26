#include <signal.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include "../utils.h"
#include "../utils_mongodb.h"
#include "MovieReviewHandler.h"

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
  SetUpTracer("config/jaeger-config.yml", "movie-review-service");

  json config_json;
  if (load_service_config(&config_json) != 0) {
    exit(EXIT_FAILURE);
  }

  int port = config_json["movie-review-service"]["port"];
  int review_storage_port = config_json["review-storage-service"]["port"];
  std::string review_storage_addr = config_json["review-storage-service"]["addr"];

  mongoc_client_pool_t *mongodb_client_pool =
      init_mongodb_client_pool(config_json, "movie-review", MONGODB_POOL_MAX_SIZE);
  ClientPool<ThriftClient<ReviewStorageServiceClient>> review_storage_client_pool(
      "review-storage-client", review_storage_addr, review_storage_port, 0, 128, 1000);

  if (mongodb_client_pool == nullptr) {
    return EXIT_FAILURE;
  }

  TThreadedServer server(
      std::make_shared<MovieReviewServiceProcessor>(
          std::make_shared<MovieReviewHandler>(
              mongodb_client_pool, &review_storage_client_pool)),
      std::make_shared<TServerSocket>("0.0.0.0", port),
      std::make_shared<TFramedTransportFactory>(),
      std::make_shared<TBinaryProtocolFactory>());
  std::cout << "Starting movie-review-service (read-only) ..." << std::endl;
  server.serve();
}
