using System.Net;
using Microsoft.Azure.Functions.Worker;
using Microsoft.Azure.Functions.Worker.Http;
using Microsoft.Extensions.Logging;
using MongoDB.Driver;
using MediaFunctions.Models;
using MediaFunctions.Helpers;

namespace MediaFunctions.Functions
{
    public class CastInfoFunction
    {
        private readonly ILogger _logger;
        private readonly IMongoCollection<CastInfo> _castInfoCollection;

        public CastInfoFunction(ILoggerFactory loggerFactory)
        {
            _logger = loggerFactory.CreateLogger<CastInfoFunction>();
            
            var connectionString = Environment.GetEnvironmentVariable("MongoDBConnectionString") ?? "mongodb://localhost:27017";
            _castInfoCollection = DbClients.GetMongoClient(connectionString)
                                           .GetDatabase("cast-info")
                                           .GetCollection<CastInfo>("cast-info");
        }

        [Function("ReadCastInfo")]
        public async Task<HttpResponseData> ReadCastInfo(
            [HttpTrigger(AuthorizationLevel.Function, "post")] HttpRequestData req)
        {
            List<long>? castIds = null;
            try
            {
                var reqObj = await req.ReadFromJsonAsync<ReadCastInfoRequest>();
                castIds = reqObj?.CastIds;
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Failed to read ReadCastInfoRequest from req");
            }

            _logger.LogInformation("ReadCastInfo received castIds count: {count}", castIds?.Count ?? 0);

            if (castIds == null || castIds.Count == 0)
            {
                var emptyResponse = req.CreateResponse(HttpStatusCode.OK);
                await emptyResponse.WriteAsJsonAsync(new List<CastInfo>());
                return emptyResponse;
            }

            // Direct MongoDB query for all requested castIds
            var filter = Builders<CastInfo>.Filter.In(c => c.CastInfoId, castIds);
            var found = await _castInfoCollection.Find(filter).ToListAsync();

            var response = req.CreateResponse(HttpStatusCode.OK);
            await response.WriteAsJsonAsync(found);
            return response;
        }
    }
}
