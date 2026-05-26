request = function()
  local movie_id = "1"
  local path = url .. "/wrk2-api/page/read?movie_id=" .. movie_id ..
      "&review_start=0&review_stop=10"
  return wrk.format("GET", path)
end
