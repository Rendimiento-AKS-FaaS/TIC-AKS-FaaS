#ifndef MEDIA_MICROSERVICES_CASTINFOHANDLER_H
#define MEDIA_MICROSERVICES_CASTINFOHANDLER_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include <bson/bson.h>
#include <mongoc.h>
#include <nlohmann/json.hpp>

#include "../../gen-cpp/CastInfoService.h"
#include "../logger.h"
#include "../tracing.h"

namespace media_service {
using json = nlohmann::json;

class CastInfoHandler : public CastInfoServiceIf {
 public:
  explicit CastInfoHandler(mongoc_client_pool_t *mongodb_client_pool);
  ~CastInfoHandler() override = default;

  void ReadCastInfo(std::vector<CastInfo> &, int64_t,
      const std::vector<int64_t> &,
      const std::map<std::string, std::string> &) override;

 private:
  mongoc_client_pool_t *_mongodb_client_pool;
};

CastInfoHandler::CastInfoHandler(mongoc_client_pool_t *mongodb_client_pool) {
  _mongodb_client_pool = mongodb_client_pool;
}

void CastInfoHandler::ReadCastInfo(
    std::vector<CastInfo> &_return,
    int64_t req_id,
    const std::vector<int64_t> &cast_info_ids,
    const std::map<std::string, std::string> &carrier) {
  TextMapReader reader(carrier);
  std::map<std::string, std::string> writer_text_map;
  TextMapWriter writer(writer_text_map);
  auto parent_span = opentracing::Tracer::Global()->Extract(reader);
  auto span = opentracing::Tracer::Global()->StartSpan(
      "ReadCastInfo",
      {opentracing::ChildOf(parent_span->get())});
  opentracing::Tracer::Global()->Inject(span->context(), writer);

  if (cast_info_ids.empty()) {
    return;
  }

  std::set<int64_t> requested_ids(cast_info_ids.begin(), cast_info_ids.end());
  std::map<int64_t, CastInfo> return_map;

  bson_t *query = bson_new();
  bson_t query_child;
  bson_t query_cast_info_id_list;
  const char *key;
  int idx = 0;
  char buf[16];
  BSON_APPEND_DOCUMENT_BEGIN(query, "cast_info_id", &query_child);
  BSON_APPEND_ARRAY_BEGIN(&query_child, "$in", &query_cast_info_id_list);
  for (auto item : requested_ids) {
    bson_uint32_to_string(idx, &key, buf, sizeof buf);
    BSON_APPEND_INT64(&query_cast_info_id_list, key, item);
    idx++;
  }
  bson_append_array_end(&query_child, &query_cast_info_id_list);
  bson_append_document_end(query, &query_child);

  mongoc_client_t *mongodb_client = mongoc_client_pool_pop(_mongodb_client_pool);
  if (!mongodb_client) {
    bson_destroy(query);
    ServiceException se;
    se.errorCode = ErrorCode::SE_MONGODB_ERROR;
    se.message = "Failed to pop a client from MongoDB pool";
    throw se;
  }

  auto collection = mongoc_client_get_collection(
      mongodb_client, "cast-info", "cast-info");
  if (!collection) {
    bson_destroy(query);
    mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);
    ServiceException se;
    se.errorCode = ErrorCode::SE_MONGODB_ERROR;
    se.message = "Failed to open cast-info collection";
    throw se;
  }

  mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
      collection, query, nullptr, nullptr);
  const bson_t *doc;
  while (mongoc_cursor_next(cursor, &doc)) {
    CastInfo new_cast_info;
    char *cast_info_json_char = bson_as_json(doc, nullptr);
    json cast_info_json = json::parse(cast_info_json_char);
    new_cast_info.cast_info_id = cast_info_json["cast_info_id"];
    new_cast_info.gender = cast_info_json["gender"];
    new_cast_info.name = cast_info_json["name"];
    new_cast_info.intro = cast_info_json["intro"];
    return_map.insert({new_cast_info.cast_info_id, new_cast_info});
    bson_free(cast_info_json_char);
  }

  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);

  for (auto cast_info_id : cast_info_ids) {
    auto it = return_map.find(cast_info_id);
    if (it != return_map.end()) {
      _return.emplace_back(it->second);
    }
  }

  span->Finish();
}

} // namespace media_service

#endif // MEDIA_MICROSERVICES_CASTINFOHANDLER_H
