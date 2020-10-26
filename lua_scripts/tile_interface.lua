local nk = require("moonnuklear")

-- тут к нам приходит индекс тайла
-- было бы неплохо здесь получить тип рендеринга карты
-- где расположить окно

local window_flags = nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_NO_INPUT

local function bool_to_number(value)
  return value and 1 or 0
end

function tile_window(ctx, tile_index)
  local mouse_x, mouse_y = ctx:mouse_pos()
  local fbw, fbh = input.get_framebuffer_size()
  local province = core.get_tile_province(tile_index)
  local height = core.get_tile_height(tile_index)
  local city = core.get_tile_city(tile_index)
  local strings_count = 2 + bool_to_number(province ~= nil) + bool_to_number(city ~= nil)
  local sizex, sizey = 300, strings_count * (ctx:font():height() + 5)
  -- нужно как то посчитать sizex
  --local x, y = fbw/2 - sizex/2, fbh/2 - sizey/2
  local x, y = mouse_x + 10, mouse_y + 10
  if x + sizex > fbw then
    local a = x + sizex - fbw
    x = x - a
  end

  if y + sizey > fbh then
    local a = y + sizey - fbh
    y = y - a
  end

  local width = ctx:font():width(ctx:font():height(), "Province title: ")
  local ratio1 = width / sizex
  --print(width, ratio1)

  if nk.window_begin(ctx, "tile_window", {x, y, sizex, sizey}, window_flags) then
    --nk.layout_row_dynamic(ctx, ctx:font():height(), 2)
    nk.layout_row(ctx, nk.DYNAMIC, ctx:font():height(), {ratio1, 1.0 - ratio1})
    nk.label(ctx, "Tile index: ", nk.TEXT_ALIGN_LEFT)
    nk.label(ctx, tile_index, nk.TEXT_ALIGN_LEFT, {0.0, 1.0, 0.0, 1.0})
    nk.label(ctx, "Tile height: ", nk.TEXT_ALIGN_LEFT)
    nk.label(ctx, height, nk.TEXT_ALIGN_LEFT, {0.0, 1.0, 0.0, 1.0})

    if province ~= nil then
      local title = province.title
      nk.label(ctx, "Province title: ", nk.TEXT_ALIGN_LEFT)
      nk.label(ctx, title.id, nk.TEXT_ALIGN_LEFT, {0.0, 1.0, 0.0, 1.0})
    end

    if city ~= nil then
      local title = city.title
      nk.label(ctx, "City title: ", nk.TEXT_ALIGN_LEFT)
      nk.label(ctx, title.id, nk.TEXT_ALIGN_LEFT, {0.0, 1.0, 0.0, 1.0})
    end
  end
  nk.window_end(ctx)
end
