namespace cpp media_service
namespace lua media_service

struct Review {
    1: i64 review_id;
    2: i64 user_id;
    3: i64 req_id;
    4: string text;
    5: string movie_id;
    6: i32 rating;
    7: i64 timestamp;
}

enum ErrorCode {
  SE_THRIFT_CONNPOOL_TIMEOUT,
  SE_THRIFT_CONN_ERROR,
  SE_UNAUTHORIZED,
  SE_MONGODB_ERROR,
  SE_THRIFT_HANDLER_ERROR
}

struct CastInfo {
  1: i64 cast_info_id
  2: string name
  3: bool gender
  4: string intro
}

struct Cast {
  1: i32 cast_id
  2: string character
  3: i64 cast_info_id
}

struct MovieInfo {
  1: string movie_id,
  2: string title,
  3: list<Cast> casts
  4: i64 plot_id
  5: list<string> thumbnail_ids
  6: list<string> photo_ids
  7: list<string> video_ids
  8: double avg_rating
  9: i32 num_rating
}

struct Page {
  1: MovieInfo movie_info
  2: list<Review> reviews
  3: list<CastInfo> cast_infos
  4: string plot
}

exception ServiceException {
  1: ErrorCode errorCode;
  2: string message;
}

service ReviewStorageService {
  list<Review> ReadReviews(
      1: i64 req_id,
      2: list<i64> review_ids,
      3: map<string, string> carrier
  ) throws (1: ServiceException se)
}

service MovieReviewService {
  list<Review> ReadMovieReviews(
      1: i64 req_id,
      2: string movie_id,
      3: i32 start,
      4: i32 stop,
      5: map<string, string> carrier
  ) throws (1: ServiceException se)
}

service CastInfoService {
  list<CastInfo> ReadCastInfo(
      1: i64 req_id,
      2: list<i64> cast_ids,
      3: map<string, string> carrier
  ) throws (1: ServiceException se)
}

service PlotService {
  string ReadPlot(
      1: i64 req_id,
      2: i64 plot_id,
      3: map<string, string> carrier
  ) throws (1: ServiceException se)
}

service MovieInfoService {
  MovieInfo ReadMovieInfo(
      1: i64 req_id,
      2: string movie_id,
      3: map<string, string> carrier
  ) throws (1: ServiceException se)
}

service PageService {
  Page ReadPage(
    1: i64 req_id,
    2: string movie_id,
    3: i32 review_start,
    4: i32 review_stop,
    5: map<string, string> carrier
  ) throws (1: ServiceException se)
}
