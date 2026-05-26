-- Inicializar soporte de números de 64 bits (Thrift) y corregir bug de endianness en doubles
require "liblualongnumber"
local TBinaryProtocol = require "TBinaryProtocol"
local libluabpack = require "libluabpack"
TBinaryProtocol.readDouble = function(self)
    local buff = self.trans:readAll(8)
    local val = libluabpack.bunpack('d', string.reverse(buff))
    return val
end
TBinaryProtocol.writeDouble = function(self, dub)
    local buff = libluabpack.bpack('d', dub)
    self.trans:write(string.reverse(buff))
end

local cjson = require "cjson"
local GenericObjectPool        = require "GenericObjectPool"
require "media_service_MovieReviewService"

ngx.header.content_type = "application/json; charset=utf-8"

local args = ngx.req.get_uri_args()

if not args.movie_id or args.movie_id == "" then
    ngx.status = 400
    ngx.say(cjson.encode({ error = "Missing required parameter: movie_id" }))
    return
end

local req_id      = tonumber(string.sub(ngx.var.request_id or "0000000000000000", 1, 15), 16) or 0
local carrier     = {}
local review_start = tonumber(args.start or 0)
local review_stop  = tonumber(args.stop  or 10)

local client = GenericObjectPool:connection(
    MovieReviewServiceClient,
    "movie-review-service.default.svc.cluster.local",
    9090
)

local ok, result = pcall(
    client.ReadMovieReviews,
    client,
    req_id,
    tostring(args.movie_id),
    review_start,
    review_stop,
    carrier
)

GenericObjectPool:returnConnection(client)

if not ok then
    ngx.status = 500
    local err_msg = (type(result) == "table" and result.message) and result.message or tostring(result)
    ngx.say(cjson.encode({ error = "MovieReviewService error: " .. err_msg }))
    return
end

-- Serializar resultado Thrift a tabla Lua plana
local function thrift_to_table(obj)
    local t_type = type(obj)
    if t_type == "userdata" or t_type == "cdata" then
        local str = tostring(obj)
        if string.find(str, "[UL]+$") then
            str = string.gsub(str, "[UL]+$", "")
        end
        return tonumber(str) or str
    elseif t_type == "function" or t_type == "thread" then
        return tostring(obj)
    elseif t_type ~= "table" then
        return obj
    end
    local t = {}
    for k, v in pairs(obj) do
        if type(k) == "number" or (type(k) == "string" and string.sub(k, 1, 2) ~= "__") then
            t[k] = thrift_to_table(v)
        end
    end
    return t
end

ngx.say(cjson.encode(thrift_to_table(result)))