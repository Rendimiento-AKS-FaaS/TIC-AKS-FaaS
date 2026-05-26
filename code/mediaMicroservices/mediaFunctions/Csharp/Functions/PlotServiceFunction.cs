using System.Net;
using Microsoft.Azure.Functions.Worker;
using Microsoft.Azure.Functions.Worker.Http;
using Microsoft.Extensions.Logging;
using MongoDB.Driver;
using MongoDB.Bson;
using MediaFunctions.Helpers;

namespace MediaFunctions.Functions
{
    public class PlotServiceFunction
    {
        private readonly ILogger _logger;
        private readonly IMongoCollection<BsonDocument> _plotCollection;

        public PlotServiceFunction(ILoggerFactory loggerFactory)
        {
            _logger = loggerFactory.CreateLogger<PlotServiceFunction>();
            
            var connectionString = Environment.GetEnvironmentVariable("MongoDBConnectionString") ?? "mongodb://localhost:27017";
            _plotCollection = DbClients.GetMongoClient(connectionString)
                                       .GetDatabase("plot")
                                       .GetCollection<BsonDocument>("plot");
        }

        [Function("ReadPlot")]
        public async Task<HttpResponseData> ReadPlot(
            [HttpTrigger(AuthorizationLevel.Function, "get")] HttpRequestData req)
        {
            if (!long.TryParse(req.Query["plot_id"], out long plotId)) return req.CreateResponse(HttpStatusCode.BadRequest);

            // Direct MongoDB query (no Memcached)
            var filter = Builders<BsonDocument>.Filter.Eq("plot_id", plotId);
            var doc = await _plotCollection.Find(filter).FirstOrDefaultAsync();

            if (doc != null && doc.Contains("plot"))
            {
                string plotText = doc["plot"].AsString;
                var response = req.CreateResponse(HttpStatusCode.OK);
                await response.WriteStringAsync(plotText);
                return response;
            }

            return req.CreateResponse(HttpStatusCode.NotFound);
        }
    }
}
