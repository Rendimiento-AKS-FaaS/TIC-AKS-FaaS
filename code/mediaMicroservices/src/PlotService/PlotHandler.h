#ifndef MEDIA_MICROSERVICES_PLOTHANDLER_H
#define MEDIA_MICROSERVICES_PLOTHANDLER_H

#include <map>
#include <string>

#include <bson/bson.h>
#include <mongoc.h>

#include "../../gen-cpp/PlotService.h"
#include "../logger.h"
#include "../tracing.h"

namespace media_service {

class PlotHandler : public PlotServiceIf {
 public:
  explicit PlotHandler(mongoc_client_pool_t *mongodb_client_pool);
  ~PlotHandler() override = default;

  void ReadPlot(std::string &_return, int64_t, int64_t,
      const std::map<std::string, std::string> &) override;

 private:
  mongoc_client_pool_t *_mongodb_client_pool;
};

PlotHandler::PlotHandler(mongoc_client_pool_t *mongodb_client_pool) {
  _mongodb_client_pool = mongodb_client_pool;
}

void PlotHandler::ReadPlot(
    std::string &_return,
    int64_t req_id,
    int64_t plot_id,
    const std::map<std::string, std::string> &carrier) {
  TextMapReader reader(carrier);
  std::map<std::string, std::string> writer_text_map;
  TextMapWriter writer(writer_text_map);
  auto parent_span = opentracing::Tracer::Global()->Extract(reader);
  auto span = opentracing::Tracer::Global()->StartSpan(
      "ReadPlot",
      {opentracing::ChildOf(parent_span->get())});
  opentracing::Tracer::Global()->Inject(span->context(), writer);

  mongoc_client_t *mongodb_client = mongoc_client_pool_pop(_mongodb_client_pool);
  if (!mongodb_client) {
    ServiceException se;
    se.errorCode = ErrorCode::SE_MONGODB_ERROR;
    se.message = "Failed to pop a client from MongoDB pool";
    throw se;
  }

  auto collection = mongoc_client_get_collection(mongodb_client, "plot", "plot");
  if (!collection) {
    mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);
    ServiceException se;
    se.errorCode = ErrorCode::SE_MONGODB_ERROR;
    se.message = "Failed to open plot collection";
    throw se;
  }

  bson_t *query = bson_new();
  BSON_APPEND_INT64(query, "plot_id", plot_id);
  mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
      collection, query, nullptr, nullptr);
  const bson_t *doc;
  bool found = mongoc_cursor_next(cursor, &doc);

  if (!found) {
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);
    ServiceException se;
    se.errorCode = ErrorCode::SE_THRIFT_HANDLER_ERROR;
    se.message = "Plot_id " + std::to_string(plot_id) + " was not found";
    throw se;
  }

  bson_iter_t iter;
  if (!bson_iter_init_find(&iter, doc, "plot")) {
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);
    ServiceException se;
    se.errorCode = ErrorCode::SE_THRIFT_HANDLER_ERROR;
    se.message = "Plot document missing plot field";
    throw se;
  }

  auto plot_value = bson_iter_utf8(&iter, nullptr);
  _return = std::string(plot_value);

  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);
  span->Finish();
}

} // namespace media_service

#endif // MEDIA_MICROSERVICES_PLOTHANDLER_H
