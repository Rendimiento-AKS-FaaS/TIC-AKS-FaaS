#ifndef MEDIA_MICROSERVICES_SRC_MOVIEINFOSERVICE_MOVIEINFOHANDLER_H_
#define MEDIA_MICROSERVICES_SRC_MOVIEINFOSERVICE_MOVIEINFOHANDLER_H_

#include <map>
#include <string>
#include <vector>

#include <bson/bson.h>
#include <mongoc.h>
#include <nlohmann/json.hpp>

#include "../../gen-cpp/MovieInfoService.h"
#include "../logger.h"
#include "../tracing.h"

namespace media_service {
using json = nlohmann::json;

class MovieInfoHandler : public MovieInfoServiceIf {
 public:
  explicit MovieInfoHandler(mongoc_client_pool_t *mongodb_client_pool);
  ~MovieInfoHandler() override = default;

  void ReadMovieInfo(MovieInfo &_return, int64_t, const std::string &,
      const std::map<std::string, std::string> &) override;

 private:
  mongoc_client_pool_t *_mongodb_client_pool;
};

MovieInfoHandler::MovieInfoHandler(mongoc_client_pool_t *mongodb_client_pool) {
  _mongodb_client_pool = mongodb_client_pool;
}

void MovieInfoHandler::ReadMovieInfo(
    MovieInfo &_return,
    int64_t req_id,
    const std::string &movie_id,
    const std::map<std::string, std::string> &carrier) {
  TextMapReader reader(carrier);
  std::map<std::string, std::string> writer_text_map;
  TextMapWriter writer(writer_text_map);
  auto parent_span = opentracing::Tracer::Global()->Extract(reader);
  auto span = opentracing::Tracer::Global()->StartSpan(
      "ReadMovieInfo",
      {opentracing::ChildOf(parent_span->get())});
  opentracing::Tracer::Global()->Inject(span->context(), writer);

  mongoc_client_t *mongodb_client = mongoc_client_pool_pop(_mongodb_client_pool);
  if (!mongodb_client) {
    ServiceException se;
    se.errorCode = ErrorCode::SE_MONGODB_ERROR;
    se.message = "Failed to pop a client from MongoDB pool";
    throw se;
  }

  auto collection = mongoc_client_get_collection(
      mongodb_client, "movie-info", "movie-info");
  if (!collection) {
    mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);
    ServiceException se;
    se.errorCode = ErrorCode::SE_MONGODB_ERROR;
    se.message = "Failed to open movie-info collection";
    throw se;
  }

  bson_t *query = bson_new();
  BSON_APPEND_UTF8(query, "movie_id", movie_id.c_str());
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
    se.message = "Movie_id " + movie_id + " was not found";
    throw se;
  }

  char *movie_info_json_char = bson_as_json(doc, nullptr);
  json movie_info_json = json::parse(movie_info_json_char);
  _return.movie_id = movie_info_json["movie_id"];
  _return.title = movie_info_json["title"];
  _return.avg_rating = movie_info_json["avg_rating"];
  _return.num_rating = movie_info_json["num_rating"];
  _return.plot_id = movie_info_json["plot_id"];
  for (auto &item : movie_info_json["photo_ids"]) {
    _return.photo_ids.emplace_back(item);
  }
  for (auto &item : movie_info_json["video_ids"]) {
    _return.video_ids.emplace_back(item);
  }
  for (auto &item : movie_info_json["thumbnail_ids"]) {
    _return.thumbnail_ids.emplace_back(item);
  }
  for (auto &item : movie_info_json["casts"]) {
    Cast new_cast;
    new_cast.cast_id = item["cast_id"];
    new_cast.cast_info_id = item["cast_info_id"];
    new_cast.character = item["character"];
    _return.casts.emplace_back(new_cast);
  }

  bson_free(movie_info_json_char);
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push(_mongodb_client_pool, mongodb_client);
  span->Finish();
}

} // namespace media_service

#endif // MEDIA_MICROSERVICES_SRC_MOVIEINFOSERVICE_MOVIEINFOHANDLER_H_
