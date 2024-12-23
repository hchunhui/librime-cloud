local function make(trig_key, trig_translator)
   local flag = false
   local last_script_text = ""
   local function processor(key, env)
      local kAccepted = 1
      local kNoop = 2
      local engine = env.engine
      local context = engine.context
      if key:repr() == trig_key then
	 if context:is_composing() then
      last_script_text = context:get_script_text()
      last_script_text = string.gsub(last_script_text, " ", "'")
      flag = true
	    context:refresh_non_confirmed_composition()
	    return kAccepted
	 end
      end

      return kNoop
   end

   local function translator(input, seg, env)
      if flag then
	 flag = false
    trig_translator(last_script_text, seg, env)
      end
   end

   return { processor = processor, translator = translator }
end

return make
