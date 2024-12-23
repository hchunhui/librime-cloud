local json = require("json")
local http = require("simplehttp")
http.TIMEOUT = 0.5

local function make_url(input, bg, ed)
   return 'https://olime.baidu.com/py?input=' .. input ..
      '&inputtype=py&bg='.. bg .. '&ed='.. ed ..
      '&result=hanzi&resultcoding=utf-8&ch_en=0&clientinfo=web&version=1'
end

local function translator(input, seg, env)
   local config = env.engine.schema.config
   local config_list= config:get_list("translator/preedit_format")
   local delimiter = config:get_string("speller/delimiter"):sub(1, 1) -- 获取首个字符为分隔符，常见为空格

   local projection = Projection(config_list)
   local p_str = projection:apply(input, true)

   local url = make_url(p_str, 0, 5)
   local reply = http.request(url)
   local _, j = pcall(json.decode, reply)
   if j.status == "T" and j.result and j.result[1] then
      for ii, vv in ipairs(j.result) do
         for i, v in ipairs(vv) do
            local hanzi = v[1]
            local pylen = v[2]
            local py = v[3].pinyin
            local c = Candidate("simple", seg.start, seg.start + pylen, hanzi, "(百度云拼音)")
            c.quality = 2
            c.preedit = string.gsub(py, "'", delimiter) -- 避免编码区显示为原始输入字符串
            yield(c)
         end
      end
   end
end

return translator
