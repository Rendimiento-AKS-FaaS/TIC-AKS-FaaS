using System;
using System.Net;
using Microsoft.Azure.Functions.Worker;
using Microsoft.Azure.Functions.Worker.Http;
using Microsoft.Extensions.Logging;
using MongoDB.Driver;
using MongoDB.Bson;
using MediaFunctions.Models;
using MediaFunctions.Helpers;
using System.Text.Json;
using System.Net.Http.Json;

namespace MediaFunctions.Functions
{
    public class ReviewFunctions
    {
        private readonly ILogger _logger;
        private readonly IMongoCollection<Review> _reviewCollection;
        private readonly IMongoCollection<BsonDocument> _movieReviewCollection;
        private readonly HttpClient _httpClient;

        public ReviewFunctions(ILoggerFactory loggerFactory, IHttpClientFactory httpClientFactory)
        {
            _logger = loggerFactory.CreateLogger<ReviewFunctions>();
            _httpClient = httpClientFactory.CreateClient();

            var mongoConn = Environment.GetEnvironmentVariable("MongoDBConnectionString") ?? "mongodb://localhost:27017";
            var client = DbClients.GetMongoClient(mongoConn);
            
            _reviewCollection = client.GetDatabase("review").GetCollection<Review>("review");
            _movieReviewCollection = client.GetDatabase("movie-review").GetCollection<BsonDocument>("movie-review");
        }

        // --- ReviewStorageService: ReadReviews ---
        [Function("ReadReviews")]
        public async Task<HttpResponseData> ReadReviews(
            [HttpTrigger(AuthorizationLevel.Function, "post", Route = "api/ReadReviews")] HttpRequestData req)
        {
            List<long>? reviewIds = null;
            try
            {
                var reqObj = await req.ReadFromJsonAsync<ReadReviewsRequest>();
                reviewIds = reqObj?.ReviewIds;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to read ReadReviewsRequest from req");
            }

            _logger.LogInformation("ReadReviews received reviewIds count: {count}", reviewIds?.Count ?? 0);

            if (reviewIds == null || reviewIds.Count == 0)
            {
                var emptyResponse = req.CreateResponse(HttpStatusCode.OK);
                await emptyResponse.WriteAsJsonAsync(new List<Review>());
                return emptyResponse;
            }

            // Direct MongoDB query (no Memcached)
            var filter = Builders<Review>.Filter.In(r => r.ReviewId, reviewIds);
            var found = await _reviewCollection.Find(filter).ToListAsync();

            // Order results exactly as requested in reviewIds
            var resultsMap = found.ToDictionary(r => r.ReviewId);
            var orderedReviews = reviewIds
                .Where(id => resultsMap.ContainsKey(id))
                .Select(id => resultsMap[id])
                .ToList();

            var response = req.CreateResponse(HttpStatusCode.OK);
            await response.WriteAsJsonAsync(orderedReviews);
            return response;
        }

        // --- MovieReviewService: ReadMovieReviews ---
        [Function("ReadMovieReviews")]
        public async Task<HttpResponseData> ReadMovieReviews(
            [HttpTrigger(AuthorizationLevel.Function, "get", Route = "api/reading/review")] HttpRequestData req)
        {
            string movieId = req.Query["movie_id"] ?? string.Empty;
            if (string.IsNullOrEmpty(movieId)) return req.CreateResponse(HttpStatusCode.BadRequest);
            int.TryParse(req.Query["start"], out int start);
            int.TryParse(req.Query["stop"], out int stop);

            if (stop <= start || start < 0)
            {
                var emptyResponse = req.CreateResponse(HttpStatusCode.OK);
                await emptyResponse.WriteAsJsonAsync(new List<Review>());
                return emptyResponse;
            }

            // 1. Direct MongoDB query for movie's reviews (no Redis)
            var filter = Builders<BsonDocument>.Filter.Eq("movie_id", movieId);
            var doc = await _movieReviewCollection.Find(filter).FirstOrDefaultAsync();

            var reviewIds = new List<long>();
            if (doc != null && doc.Contains("reviews") && doc["reviews"].IsBsonArray)
            {
                var reviewsArray = doc["reviews"].AsBsonArray;
                var reviewEntries = new List<(long Timestamp, long ReviewId)>();

                foreach (var r in reviewsArray)
                {
                    if (r.IsBsonDocument)
                    {
                        var rDoc = r.AsBsonDocument;
                        if (rDoc.Contains("review_id") && rDoc.Contains("timestamp"))
                        {
                            reviewEntries.Add((
                                rDoc["timestamp"].AsInt64,
                                rDoc["review_id"].AsInt64
                            ));
                        }
                    }
                }

                // Sort by timestamp in descending order (matching C++ MovieReviewHandler.h)
                var sortedEntries = reviewEntries.OrderByDescending(e => e.Timestamp).ToList();

                // Slice the list from start to stop (matching C++: idx = start; idx < stop)
                for (int idx = start; idx < stop && idx < sortedEntries.Count; ++idx)
                {
                    reviewIds.Add(sortedEntries[idx].ReviewId);
                }
            }

            _logger.LogInformation("ReadMovieReviews: movie_id {movieId} found reviewIds count: {count}", movieId, reviewIds.Count);

            if (reviewIds.Count == 0)
            {
                var emptyResponse = req.CreateResponse(HttpStatusCode.OK);
                await emptyResponse.WriteAsJsonAsync(new List<Review>());
                return emptyResponse;
            }

            // 2. Fetch the actual content directly from MongoDB
            var revFilter = Builders<Review>.Filter.In(r => r.ReviewId, reviewIds);
            var foundReviews = await _reviewCollection.Find(revFilter).ToListAsync();

            // Order results exactly as requested in reviewIds
            var resultsMap = foundReviews.ToDictionary(r => r.ReviewId);
            var orderedReviews = reviewIds
                .Where(id => resultsMap.ContainsKey(id))
                .Select(id => resultsMap[id])
                .ToList();

            var response = req.CreateResponse(HttpStatusCode.OK);
            await response.WriteAsJsonAsync(orderedReviews);
            return response;
        }
    }
}
