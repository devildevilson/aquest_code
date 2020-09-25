local nk = require("moonnuklear")

local function init_nk_context(raw_context)
  --print(raw_context)
  --print(type(raw_context))
  return nk.init_from_ptr(raw_context)
end

local function init_nk_font(raw_font)
  --print(raw_font)
  --print(type(raw_font))
  return nk.font_from_ptr(raw_font)
end

return init_nk_context, init_nk_font
