local nk = require("moonnuklear")

function init_nk_context(raw_context)
  --print(raw_context)
  --print(type(raw_context))
  return nk.init_from_ptr(raw_context)
end

function init_nk_font(raw_font)
  --print(raw_font)
  --print(type(raw_font))
  return nk.font_from_ptr(raw_font)
end
