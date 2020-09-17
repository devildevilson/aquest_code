local nk = require("moonnuklear")

local window_flags = nk.WINDOW_BORDER|nk.WINDOW_MOVABLE|nk.WINDOW_CLOSABLE

local GUI = { op = 'easy', property = 20 }

local function set_op(op)
   if op == GUI.op then return end
   GUI.op = op
   print("op is now '"..GUI.op.."'")
end

local function set_property(property)
   if property == GUI.property then return end
   GUI.property = property
   print("property is now "..GUI.property.."")
end

-- короч moonnuklear создает свой тип для хранения контекста
-- и невозможно напрямую передать контекст
function test_interface(ctx, interface, data)
  assert(ctx ~= nil)
  --nk.style_set_font(ctx, interface)
  if nk.window_begin(ctx, "Show", {50, 50, 220, 220}, window_flags) then
    nk.layout_row_dynamic(ctx, 30, 1)
    nk.label(ctx, "Hello from lua", nk.TEXT_LEFT)
    nk.layout_row_static(ctx, 30, 80, 1)
    if nk.button(ctx, nil, "button") then
       print("button pressed")
    end

    nk.layout_row_dynamic(ctx, 30, 2)
    if nk.option(ctx, "easy", GUI.op == 'easy') then set_op('easy') end
    if nk.option(ctx, "hard", GUI.op == 'hard') then set_op('hard') end

    nk.layout_row_dynamic(ctx, 25, 1)
    set_property(nk.property(ctx, "Compression:", 0, GUI.property, 100, 10, 1))
  end
  nk.window_end(ctx)
  return false
end
