local http = require("simplehttp")
local iconv = require("iconv")

-- Algorithm from https://github.com/wanghuafeng/common_utils/blob/master/rong_tools/sogou_cloud_words.py

function rc(x)
    local start = 0
    for i = 1, #x do
        start = start ~ string.byte(x, i)
    end
    return string.char(start)
end

function serial_keys(keys)
    local token = "\0\5\0\0\0\0\1"
    local total_len = #token + #keys + 3
    local data = string.char(total_len) .. token .. string.char(#keys) .. keys
    return data .. rc(data)
end

function key_from_serial(data)
    local token = "\0\5\0\0\0\0\1"
    local total_len = string.byte(data, 1)
    local key_len = total_len - #token - 3
    return string.sub(data, -key_len - 1, -2)
end

function open_sogou(keys, durtot, version)
    durtot = durtot or 0
    version = version or "3.7"
    local url = string.format(
        "http://shouji.sogou.com/web_ime/mobile.php?durtot=%d&h=000000000000000&r=store_mf_wandoujia&v=%s", durtot,
        version)
    local data = serial_keys(keys)
    return http.request(url, data)
end

function parse_result(result)
    local words = {}

    if string.byte(result, 1) + 2 ~= #result then
        log.error("[sougou] invalid size, expected", string.byte(result, 1) + 2, "got", #result)
        return words
    end

    local num_words = string.unpack("<H", string.sub(result, 0x12 + 1, 0x12 + 2))
    if num_words == 0 or num_words > 32 then
        log.warning("[sougou] strange words num", num_words)
    end

    local pos = 0x14 -- data packet starts at 0x14

    for i = 1, num_words do
        local str_len = string.unpack("<H", string.sub(result, pos + 1, pos + 2))
        if str_len == 0 or str_len > 0xFF then
            log.error("[sougou] Invalid string length")
        end
        pos = pos + 2

        if str_len == 0 then
            -- Skip empty string
        else
            local word = string.sub(result, pos + 1, pos + str_len)
            local cd, err = iconv.new("utf-8", "utf-16le")
            if not cd then
                log.error(string.format("[sougou] word %s can't convert to utf-8: %s", word, err))
            else
                word, err = cd:iconv(word)
                if not word then
                    log.error(string.format("[sougou] word can't convert to utf-8: %s", err))
                end
            end
            table.insert(words, word)
        end
        pos = pos + str_len

        -- unknown part, like 0x12c, 0x12b, etc.
        str_len = string.unpack("<H", string.sub(result, pos + 1, pos + 2))
        pos = pos + str_len + 2

        -- unknown part, like 0x01, 0x02, etc.
        str_len = string.unpack("<H", string.sub(result, pos + 1, pos + 2))
        pos = pos + str_len + 2 + 1
    end

    if pos ~= #result then
        log.warning("[sougou] buffer not exhausted!")
    end

    return words
end

function get_cloud_words(keys)
    local resp, code = open_sogou(keys)
    if code ~= 200 then
        log.error(string.format("[sougou] invalid response for input <%s>, status code: %d", keys, code))
        return {}
    end

    return parse_result(resp)
end

local function translator(input, seg)
    local list = get_cloud_words(input)
    local yielded_candidates = 0
    local max_candidates = 5
    for i, v in ipairs(list) do
        if yielded_candidates >= max_candidates then
            break
        end

        local c = Candidate("simple", seg.start, seg._end, v, "(搜狗云拼音)")
        c.quality = 2
        yield(c)

        yielded_candidates = yielded_candidates + 1
    end
end

return translator
