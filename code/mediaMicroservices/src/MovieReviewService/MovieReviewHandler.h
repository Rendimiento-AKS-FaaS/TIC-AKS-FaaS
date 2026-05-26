#ifndef MEDIA_MICROSERVICES_MOVIEREVIEWHANDLER_H
#define MEDIA_MICROSERVICES_MOVIEREVIEWHANDLER_H

#include <algorithm>
#include <future>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <bson/bson.h>
#include <mongoc.h>

#include "../../gen-cpp/MovieReviewService.h"
#include "../../gen-cpp/ReviewStorageService.h"
#include "../ClientPool.h"
#include "../ThriftClient.h"
#include "../logger.h"
#include "../tracing.h"

namespace media_service {

class MovieReviewHandler : public MovieReviewServiceIf {
 public:
  MovieReviewHandler(
      mongoc_client_pool_t *mongodb_client_pool,
      ClientPool<ThriftClient<ReviewStorageServiceClient>> *review_client_pool);
  ~MovieReviewHandler() override = default;

  void ReadMovieReviews(std::vector<Review> &, int64_t, const std::string &,
      int32_t, int32_t,
      const std::map<std::string, std::string> &) override;

 private:
  mongoc_client_pool_t *_mongodb_client_pool;
  ClientPool<ThriftClient<ReviewStorageServiceClient>> *_review_client_pool;
};

MovieReviewHandler::MovieReviewHandler(
    mongoc_client_pool_t *mongodb_client_pool,
    ClientPool<ThriftClient<ReviewStorageServiceClient>> *review_client_pool) {
  _mongodb_client_pool = mongodb_client_pool;
  _review_client_pool = review_client_pool;
}

void MovieReviewHandler::ReadMovieReviews(
    std::vector<Review> &_return,
    int64_t req_id,
    const std::string &movie_id,
    int32_t start,
    int32_t stop,
    const std::map<std::string, std::string> &carrier) {
  TextMapReader reader(carrier);
  std::map<std::string, std::string> writer_text_map;
  TextMapWriter writer(writer_text_map);
  auto parent_span = opentracing::Tracer::Global()->Extract(reader);
  auto span = opentracing::Tracer::Global()->StartSpan(
      "ReadMovieReviews",
      {opentracing::ChildOf(parent_span->get())});
  opentracing::Tracer::Global()->Inject(span->context(), writer);

  if (stop <= start || start < 0) {
    span->Finish();
    return;
  }

  std::vector<std::pair<int64_t, int64_t>> review_entries;

  mongoc_client_t *mongodb_client = mongoc_client_pool_pop(_mongodb_client_pool);
  if (!mongodb_client) {
    ServiceException se;
    se.errorCode = ErrorCode::SE_MONGODB_ERROR;
    se.message = "Failed to pop a client from MongoDB pool";
    throw se;
  }

  auto collection = mongoc_client_get_collection(
      mongodb_client, "movie-review", "movie-review");
  if (!collection) {
    mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);
    ServiceException se;
    se.errorCode = ErrorCode::SE_MONGODB_ERROR;
    se.message = "Failed to open movie-review collection";
    throw se;
  }

  bson_t *query = BCON_NEW("movie_id", BCON_UTF8(movie_id.c_str()));
  mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(
      collection, query, nullptr, nullptr);
  const bson_t *doc;
  if (mongoc_cursor_next(cursor, &doc)) {
    bson_iter_t reviews_iter;
    if (bson_iter_init_find(&reviews_iter, doc, "reviews") &&
        BSON_ITER_HOLDS_ARRAY(&reviews_iter)) {
      bson_iter_t review_iter;
      bson_iter_recurse(&reviews_iter, &review_iter);
      while (bson_iter_next(&review_iter)) {
        if (!BSON_ITER_HOLDS_DOCUMENT(&review_iter)) {
          continue;
        }
        uint32_t doc_len = 0;
        const uint8_t *review_doc_buf = nullptr;
        bson_iter_document(&review_iter, &doc_len, &review_doc_buf);
        bson_t *review_doc = bson_new_from_data(review_doc_buf, doc_len);
        bson_iter_t review_id_iter;
        bson_iter_t timestamp_iter;
        if (bson_iter_init_find(&review_id_iter, review_doc, "review_id") &&
            BSON_ITER_HOLDS_INT64(&review_id_iter) &&
            bson_iter_init_find(&timestamp_iter, review_doc, "timestamp") &&
            BSON_ITER_HOLDS_INT64(&timestamp_iter)) {
          review_entries.emplace_back(
              bson_iter_int64(&timestamp_iter),
              bson_iter_int64(&review_id_iter));
        }
        bson_destroy(review_doc);
      }
    }
  }

  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);

  std::sort(review_entries.begin(), review_entries.end(),
      [](const std::pair<int64_t, int64_t> &lhs,
         const std::pair<int64_t, int64_t> &rhs) {
        return lhs.first > rhs.first;
      });

  std::vector<int64_t> review_ids;
  for (int32_t idx = start; idx < stop && idx < static_cast<int32_t>(review_entries.size()); ++idx) {
    review_ids.emplace_back(review_entries[idx].second);
  }

  if (review_ids.empty()) {
    span->Finish();
    return;
  }

  auto review_client_wrapper = _review_client_pool->Pop();
  if (!review_client_wrapper) {
    ServiceException se;
    se.errorCode = ErrorCode::SE_THRIFT_CONN_ERROR;
    se.message = "Failed to connect to review-storage-service";
    throw se;
  }

  auto review_client = review_client_wrapper->GetClient();
  try {
    review_client->ReadReviews(_return, req_id, review_ids, writer_text_map);
  } catch (...) {
    _review_client_pool->Push(review_client_wrapper);
    LOG(error) << "Failed to read reviews from review-storage-service";
    throw;
  }
  _review_client_pool->Push(review_client_wrapper);
  span->Finish();
}

} // namespace media_service

#endif // MEDIA_MICROSERVICES_MOVIEREVIEWHANDLER_H
