local http = require("simplehttp")
http.TIMEOUT = 1.5 -- set timeout in secs

-- simple form
--[[
    response, code = http.request(url [, body])
]]
-- GET
local r, c = http.request("https://www.baidu.com")
assert(c == 200)
print(r)
-- POST
local r, c = http.request("https://postman-echo.com/post", "test")
assert(c == 200)
print(r)

-- advanced form
--[[
    response, code, headers = http.request{
        url = string,
        [method = string,]
        [headers = header-table,]
        [data = string,]
    }
]]
local r, c, h = http.request{url = "https://www.baidu.com"}
assert(c == 200)
for k, v in pairs(h) do
    print(k, v)
end

local r, c, h = http.request{
    url = "https://postman-echo.com/post",
    method = "POST",
    headers = {
        ["foo"] = "42",
        ["bar"] = "baz",
    },
    data = "test",
}
assert(c == 200)
