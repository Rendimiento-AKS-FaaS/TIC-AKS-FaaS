using System;
using System.Net;
using Microsoft.Azure.Functions.Worker;
using Microsoft.Azure.Functions.Worker.Http;
using Microsoft.Extensions.Logging;
using MongoDB.Driver;
using MongoDB.Bson;
using MediaFunctions.Models;
using MediaFunctions.Helpers;
using System.Linq;

namespace MediaFunctions.Functions
{
    public class PageServiceFunction
    {
        private readonly ILogger _logger;
        private readonly IMongoCollection<MovieInfo> _movieInfoCollection;
        private readonly IMongoCollection<BsonDocument> _movieReviewCollection;
        private readonly IMongoCollection<Review> _reviewCollection;
        private readonly IMongoCollection<CastInfo> _castInfoCollection;
        private readonly IMongoCollection<BsonDocument> _plotCollection;

        public PageServiceFunction(ILoggerFactory loggerFactory)
        {
            _logger = loggerFactory.CreateLogger<PageServiceFunction>();
            
            var connectionString = Environment.GetEnvironmentVariable("MongoDBConnectionString") ?? "mongodb://localhost:27017";
            var client = DbClients.GetMongoClient(connectionString);
            
            _movieInfoCollection = client.GetDatabase("movie-info").GetCollection<MovieInfo>("movie-info");
            _movieReviewCollection = client.GetDatabase("movie-review").GetCollection<BsonDocument>("movie-review");
            _reviewCollection = client.GetDatabase("review").GetCollection<Review>("review");
            _castInfoCollection = client.GetDatabase("cast-info").GetCollection<CastInfo>("cast-info");
            _plotCollection = client.GetDatabase("plot").GetCollection<BsonDocument>("plot");
        }

        [Function("ReadPage")]
        public async Task<HttpResponseData> ReadPage(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", Route = "api/reading/page")] HttpRequestData req)
        {
            return await ReadPageInternal(req);
        }

        [Function("ReadPageWrk2")]
        public async Task<HttpResponseData> ReadPageWrk2(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", Route = "wrk2-api/page/read")] HttpRequestData req)
        {
            return await ReadPageInternal(req);
        }

        private async Task<HttpResponseData> ReadPageInternal(HttpRequestData req)
        {
            string movieId = req.Query["movie_id"];
            int.TryParse(req.Query["review_start"], out int reviewStart);
            int.TryParse(req.Query["review_stop"], out int reviewStop);

            if (string.IsNullOrEmpty(movieId)) return req.CreateResponse(HttpStatusCode.BadRequest);

            try
            {
                // 1. Fetch MovieInfo directly
                var movieInfo = await _movieInfoCollection.Find(m => m.MovieId == movieId).FirstOrDefaultAsync();
                if (movieInfo == null)
                {
                    var notFoundResponse = req.CreateResponse(HttpStatusCode.NotFound);
                    await notFoundResponse.WriteStringAsync($"Movie with id {movieId} not found.");
                    return notFoundResponse;
                }

                // 2. Fetch Plot, CastInfo, and Reviews in parallel
                
                // Plot Task
                var plotTask = Task.Run(async () =>
                {
                    var plotDoc = await _plotCollection.Find(Builders<BsonDocument>.Filter.Eq("plot_id", movieInfo.PlotId)).FirstOrDefaultAsync();
                    return plotDoc != null && plotDoc.Contains("plot") ? plotDoc["plot"].AsString : string.Empty;
                });

                // CastInfo Task
                var castTask = Task.Run(async () =>
                {
                    var castInfoIds = movieInfo.Casts?.Select(c => c.CastInfoId).ToList() ?? new List<long>();
                    if (castInfoIds.Count == 0) return new List<CastInfo>();
                    
                    var filter = Builders<CastInfo>.Filter.In(c => c.CastInfoId, castInfoIds);
                    return await _castInfoCollection.Find(filter).ToListAsync();
                });

                // Reviews Task (MovieReview sorting & slicing + ReviewStorage lookup)
                var reviewsTask = Task.Run(async () =>
                {
                    if (reviewStop <= reviewStart || reviewStart < 0) return new List<Review>();

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

                        var sortedEntries = reviewEntries.OrderByDescending(e => e.Timestamp).ToList();
                        for (int idx = reviewStart; idx < reviewStop && idx < sortedEntries.Count; ++idx)
                        {
                            reviewIds.Add(sortedEntries[idx].ReviewId);
                        }
                    }

                    if (reviewIds.Count == 0) return new List<Review>();

                    var revFilter = Builders<Review>.Filter.In(r => r.ReviewId, reviewIds);
                    var foundReviews = await _reviewCollection.Find(revFilter).ToListAsync();

                    var resultsMap = foundReviews.ToDictionary(r => r.ReviewId);
                    return reviewIds
                        .Where(id => resultsMap.ContainsKey(id))
                        .Select(id => resultsMap[id])
                        .ToList();
                });

                // Wait for parallel tasks
                await Task.WhenAll(plotTask, castTask, reviewsTask);

                var plot = await plotTask;
                var castInfos = await castTask;
                var reviews = await reviewsTask;

                var pageResponse = new Page(movieInfo, reviews, castInfos, plot);

                var response = req.CreateResponse(HttpStatusCode.OK);
                await response.WriteAsJsonAsync(pageResponse);
                return response;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error orchestrating ReadPage for movie_id: {movieId}", movieId);
                var errorResponse = req.CreateResponse(HttpStatusCode.InternalServerError);
                await errorResponse.WriteStringAsync("Error processing request. Check logs.");
                return errorResponse;
            }
        }
    }
}
