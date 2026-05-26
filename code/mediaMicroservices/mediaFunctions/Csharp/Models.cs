using System.Text.Json.Serialization;
using MongoDB.Bson.Serialization.Attributes;

namespace MediaFunctions.Models
{
    [BsonIgnoreExtraElements]
    public record User(
        [property: JsonPropertyName("user_id"), BsonElement("user_id")] long UserId,
        [property: JsonPropertyName("first_name"), BsonElement("first_name")] string FirstName,
        [property: JsonPropertyName("last_name"), BsonElement("last_name")] string LastName,
        [property: JsonPropertyName("username"), BsonElement("username")] string Username,
        [property: JsonPropertyName("password"), BsonElement("password")] string Password,
        [property: JsonPropertyName("salt"), BsonElement("salt")] string Salt
    );

    [BsonIgnoreExtraElements]
    public record Review(
        [property: JsonPropertyName("review_id"), BsonElement("review_id")] long ReviewId,
        [property: JsonPropertyName("user_id"), BsonElement("user_id")] long UserId,
        [property: JsonPropertyName("req_id"), BsonElement("req_id")] long ReqId,
        [property: JsonPropertyName("text"), BsonElement("text")] string Text,
        [property: JsonPropertyName("movie_id"), BsonElement("movie_id")] string MovieId,
        [property: JsonPropertyName("rating"), BsonElement("rating")] int Rating,
        [property: JsonPropertyName("timestamp"), BsonElement("timestamp")] long Timestamp
    );

    [BsonIgnoreExtraElements]
    public record CastInfo(
        [property: JsonPropertyName("cast_info_id"), BsonElement("cast_info_id")] long CastInfoId,
        [property: JsonPropertyName("name"), BsonElement("name")] string Name,
        [property: JsonPropertyName("gender"), BsonElement("gender")] bool Gender,
        [property: JsonPropertyName("intro"), BsonElement("intro")] string Intro
    );

    public record Cast(
        [property: JsonPropertyName("cast_id"), BsonElement("cast_id")] int CastId,
        [property: JsonPropertyName("character"), BsonElement("character")] string Character,
        [property: JsonPropertyName("cast_info_id"), BsonElement("cast_info_id")] long CastInfoId
    );

    [BsonIgnoreExtraElements]
    public record MovieInfo(
        [property: JsonPropertyName("movie_id"), BsonElement("movie_id")] string MovieId,
        [property: JsonPropertyName("title"), BsonElement("title")] string Title,
        [property: JsonPropertyName("casts"), BsonElement("casts")] List<Cast> Casts,
        [property: JsonPropertyName("plot_id"), BsonElement("plot_id")] long PlotId,
        [property: JsonPropertyName("thumbnail_ids"), BsonElement("thumbnail_ids")] List<string> ThumbnailIds,
        [property: JsonPropertyName("photo_ids"), BsonElement("photo_ids")] List<string> PhotoIds,
        [property: JsonPropertyName("video_ids"), BsonElement("video_ids")] List<string> VideoIds,
        [property: JsonPropertyName("avg_rating"), BsonElement("avg_rating")] double AvgRating,
        [property: JsonPropertyName("num_rating"), BsonElement("num_rating")] int NumRating
    );

    public record Page(
        [property: JsonPropertyName("movie_info")] MovieInfo MovieInfo,
        [property: JsonPropertyName("reviews")] List<Review> Reviews,
        [property: JsonPropertyName("cast_infos")] List<CastInfo> CastInfos,
        [property: JsonPropertyName("plot")] string Plot
    );

    // --- Request Contracts for clean model binding ---
    public record ReadCastInfoRequest(
        [property: JsonPropertyName("cast_ids")] List<long> CastIds
    );

    public record ReadReviewsRequest(
        [property: JsonPropertyName("review_ids")] List<long> ReviewIds
    );

    public enum ErrorCode
    {
        SE_THRIFT_CONNPOOL_TIMEOUT,
        SE_THRIFT_CONN_ERROR,
        SE_UNAUTHORIZED,
        SE_MEMCACHED_ERROR,
        SE_MONGODB_ERROR,
        SE_REDIS_ERROR,
        SE_THRIFT_HANDLER_ERROR
    }

    public class ServiceException : Exception
    {
        public ErrorCode ErrorCode { get; set; }
        public ServiceException(ErrorCode errorCode, string message) : base(message)
        {
            ErrorCode = errorCode;
        }
    }
}
