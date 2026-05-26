using System.Net;
using Microsoft.Azure.Functions.Worker;
using Microsoft.Azure.Functions.Worker.Http;
using Microsoft.Extensions.Logging;
using MongoDB.Driver;
using MediaFunctions.Models;
using MediaFunctions.Helpers;

namespace MediaFunctions.Functions
{
    public class MovieInfoFunction
    {
        private readonly ILogger _logger;
        private readonly IMongoCollection<MovieInfo> _movieInfoCollection;

        public MovieInfoFunction(ILoggerFactory loggerFactory)
        {
            _logger = loggerFactory.CreateLogger<MovieInfoFunction>();
            
            var connectionString = Environment.GetEnvironmentVariable("MongoDBConnectionString") ?? "mongodb://localhost:27017";
            var client = DbClients.GetMongoClient(connectionString);
            var database = client.GetDatabase("movie-info");
            _movieInfoCollection = database.GetCollection<MovieInfo>("movie-info");
        }

        [Function("ReadMovieInfo")]
        public async Task<HttpResponseData> ReadMovieInfo(
            [HttpTrigger(AuthorizationLevel.Function, "get", "post", Route = "api/reading/movie")] HttpRequestData req)
        {
            _logger.LogInformation("C# HTTP trigger function processed ReadMovieInfo request.");

            string movieId = req.Query["movie_id"];

            if (string.IsNullOrEmpty(movieId))
            {
                var badResponse = req.CreateResponse(HttpStatusCode.BadRequest);
                await badResponse.WriteStringAsync("Please pass a movie_id on the query string.");
                return badResponse;
            }

            // Direct MongoDB query (no Memcached)
            var filter = Builders<MovieInfo>.Filter.Eq(m => m.MovieId, movieId);
            var movie = await _movieInfoCollection.Find(filter).FirstOrDefaultAsync();

            if (movie == null)
            {
                var errorResponse = req.CreateResponse(HttpStatusCode.NotFound);
                await errorResponse.WriteAsJsonAsync(new { error = $"Movie_id: {movieId} doesn't exist" });
                return errorResponse;
            }

            var successResponse = req.CreateResponse(HttpStatusCode.OK);
            await successResponse.WriteAsJsonAsync(movie);
            return successResponse;
        }
    }
}
