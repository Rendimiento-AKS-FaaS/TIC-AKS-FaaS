#ifndef MEDIA_MICROSERVICES_REVIEWSTOREHANDLER_H
#define MEDIA_MICROSERVICES_REVIEWSTOREHANDLER_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include <bson/bson.h>
#include <mongoc.h>
#include <nlohmann/json.hpp>

#include "../../gen-cpp/ReviewStorageService.h"
#include "../logger.h"
#include "../tracing.h"

namespace media_service {
using json = nlohmann::json;

class ReviewStorageHandler : public ReviewStorageServiceIf {
 public:
  explicit ReviewStorageHandler(mongoc_client_pool_t *mongodb_client_pool);
  ~ReviewStorageHandler() override = default;

  void ReadReviews(std::vector<Review> &, int64_t,
      const std::vector<int64_t> &,
      const std::map<std::string, std::string> &) override;

 private:
  mongoc_client_pool_t *_mongodb_client_pool;
};

ReviewStorageHandler::ReviewStorageHandler(
    mongoc_client_pool_t *mongodb_client_pool) {
  _mongodb_client_pool = mongodb_client_pool;
}

void ReviewStorageHandler::ReadReviews(
    std::vector<Review> &_return,
    int64_t req_id,
    const std::vector<int64_t> &review_ids,
    const std::map<std::string, std::string> &carrier) {
  TextMapReader reader(carrier);
  std::map<std::string, std::string> writer_text_map;
  TextMapWriter writer(writer_text_map);
  auto parent_span = opentracing::Tracer::Global()->Extract(reader);
  auto span = opentracing::Tracer::Global()->StartSpan(
      "ReadReviews",
      {opentracing::ChildOf(parent_span->get())});
  opentracing::Tracer::Global()->Inject(span->context(), writer);

  if (review_ids.empty()) {
    return;
  }

  std::set<int64_t> requested_ids(review_ids.begin(), review_ids.end());
  if (requested_ids.size() != review_ids.size()) {
    ServiceException se;
    se.errorCode = ErrorCode::SE_THRIFT_HANDLER_ERROR;
    se.message = "review_ids are duplicated";
    throw se;
  }

  std::map<int64_t, Review> return_map;

  mongoc_client_t *mongodb_client = mongoc_client_pool_pop(_mongodb_client_pool);
  if (!mongodb_client) {
    ServiceException se;
    se.errorCode = ErrorCode::SE_MONGODB_ERROR;
    se.message = "Failed to pop a client from MongoDB pool";
    throw se;
  }

  auto collection = mongoc_client_get_collection(mongodb_client, "review", "review");
  if (!collection) {
    mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);
    ServiceException se;
    se.errorCode = ErrorCode::SE_MONGODB_ERROR;
    se.message = "Failed to open review collection";
    throw se;
  }

  bson_t *query = bson_new();
  bson_t query_child;
  bson_t query_review_id_list;
  const char *key;
  int idx = 0;
  char buf[16];
  BSON_APPEND_DOCUMENT_BEGIN(query, "review_id", &query_child);
  BSON_APPEND_ARRAY_BEGIN(&query_child, "$in", &query_review_id_list);
  for (auto item : requested_ids) {
    bson_uint32_to_string(idx, &key, buf, sizeof buf);
    BSON_APPEND_INT64(&query_review_id_list, key, item);
    idx++;
  }
  bson_append_array_end(&query_child, &query_review_id_list);
  bson_append_document_end(query, &query_child);

  mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
      collection, query, nullptr, nullptr);
  const bson_t *doc;
  while (mongoc_cursor_next(cursor, &doc)) {
    Review new_review;
    char *review_json_char = bson_as_json(doc, nullptr);
    json review_json = json::parse(review_json_char);
    new_review.req_id = review_json["req_id"];
    new_review.user_id = review_json["user_id"];
    new_review.movie_id = review_json["movie_id"];
    new_review.text = review_json["text"];
    new_review.rating = review_json["rating"];
    new_review.timestamp = review_json["timestamp"];
    new_review.review_id = review_json["review_id"];
    return_map.insert({new_review.review_id, new_review});
    bson_free(review_json_char);
  }

  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);

  for (auto review_id : review_ids) {
    auto it = return_map.find(review_id);
    if (it != return_map.end()) {
      _return.emplace_back(it->second);
    }
  }

  span->Finish();
}

} // namespace media_service

#endif // MEDIA_MICROSERVICES_REVIEWSTOREHANDLER_H
