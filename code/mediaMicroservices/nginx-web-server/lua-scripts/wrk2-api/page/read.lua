local _M = {}
local k8s_suffix = os.getenv("fqdn_suffix")
if (k8s_suffix == nil) then
  k8s_suffix = ""
end

local function _StrIsEmpty(s)
  return s == nil or s == ''
end

function _M.ReadPage()
  local bridge_tracer = require "opentracing_bridge_tracer"
  local GenericObjectPool = require "GenericObjectPool"
  local PageServiceClient = require "media_service_PageService"
  local cjson = require "cjson"
  local ngx = ngx

  local req_id = tonumber(string.sub(ngx.var.request_id, 0, 15), 16)
  local tracer = bridge_tracer.new_from_global()
  local parent_span_context = tracer:binary_extract(ngx.var.opentracing_binary_context)
  local span = tracer:start_span("ReadPage", {["references"] = {{"child_of", parent_span_context}}})
  local carrier = {}
  tracer:text_map_inject(span:context(), carrier)

  ngx.req.read_body()
  local args = ngx.req.get_uri_args()
  if _StrIsEmpty(args.movie_id) then
    args = ngx.req.get_post_args()
  end

  if (_StrIsEmpty(args.movie_id) or _StrIsEmpty(args.review_start) or _StrIsEmpty(args.review_stop)) then
    ngx.status = ngx.HTTP_BAD_REQUEST
    ngx.say("Incomplete arguments")
    ngx.log(ngx.ERR, "Incomplete arguments")
    ngx.exit(ngx.HTTP_BAD_REQUEST)
  end

  local client = GenericObjectPool:connection(PageServiceClient, "page-service.media-microservices.svc.cluster.local" ,9090)

  local status, page = pcall(client.ReadPage, client, req_id, tostring(args.movie_id), tonumber(args.review_start), tonumber(args.review_stop), carrier)

  GenericObjectPool:returnConnection(client)

  span:finish()

  if not status then
    ngx.status = ngx.HTTP_INTERNAL_SERVER_ERROR
    if type(page) == "table" and page.message then
      ngx.say("Service Exception: " .. tostring(page.message))
    else
      ngx.say("Error calling PageService: " .. tostring(page))
    end
    ngx.exit(ngx.HTTP_INTERNAL_SERVER_ERROR)
  else
    ngx.header.content_type = "application/json; charset=utf-8"
    ngx.say(cjson.encode(page))
  end
end

return _M
