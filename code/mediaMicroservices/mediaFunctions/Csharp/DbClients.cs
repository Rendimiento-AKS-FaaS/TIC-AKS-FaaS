using System;
using MongoDB.Driver;

namespace MediaFunctions.Helpers
{
    public static class DbClients
    {
        private static IMongoClient? _mongoClient;
        private static readonly object _lock = new object();

        public static IMongoClient GetMongoClient(string connectionString)
        {
            if (_mongoClient == null)
            {
                lock (_lock)
                {
                    if (_mongoClient == null)
                    {
                        var settings = MongoClientSettings.FromConnectionString(connectionString);
                        // Pool optimization for Azure Functions / local environments
                        settings.MaxConnectionPoolSize = 100;
                        settings.MinConnectionPoolSize = 5;
                        _mongoClient = new MongoClient(settings);
                    }
                }
            }
            return _mongoClient;
        }
    }
}
