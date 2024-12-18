local json = require("json")
local http = require("simplehttp")
http.TIMEOUT = 1.5

local function make_url(input, num)
    return 'https://inputtools.google.com/request?text=' ..
        input .. '&itc=zh-t-i0-pinyin&num=' .. num .. '&cp=0&cs=1&ie=utf-8&oe=utf-8'
end

local function translator(input, seg)
    local url = make_url(input, 5)
    local reply = http.request(url)
    local _, j = pcall(json.decode, reply)
    if j[1] == 'SUCCESS' and j[2] and j[2][1] then
        for i, v in ipairs(j[2][1][2]) do
            local matched_length = j[2][1][4].matched_length
            if matched_length ~= nil then
                matched_length = matched_length[i]
            else
                matched_length = string.len(j[2][1][1])
            end
            local annotation = j[2][1][4].annotation[i]
            local c = Candidate("simple", seg.start, seg.start + matched_length, v, "(谷歌云拼音)")
            c.quality = 2
            if string.gsub(annotation, " ", "") == string.sub(input, 1, matched_length) then
                c.preedit = annotation
            end
            yield(c)
        end
    end
end

return translator
