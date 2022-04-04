local nk = require("moonnuklear")

local function bool_to_number(value) return value and 1 or 0 end

local path_colors = { utils.make_color(0.0, 0.7, 0.0, 0.5), utils.make_color(0.7, 0.0, 0.0, 0.5) }

local movement_was_pressed_on_map = false
local movement_was_pressed = false
local movement_last_xpos, movement_last_ypos = 0.0, 0.0
local map_move = input.get_event("map_move")
local function camera_movement(time, interface_hovered)
  local xpos, ypos = input.get_cursor_pos()
  local delta_xpos, delta_ypos = movement_last_xpos - xpos, movement_last_ypos - ypos
  movement_last_xpos = xpos
  movement_last_ypos = ypos

  local changed = not (movement_last_xpos == xpos and movement_last_ypos == ypos)

  local zoom = camera.zoom()
  local sens = utils.get_settings().game.camera_movement * (1.0 + zoom * 0.5)
  local x_sens = utils.get_settings().game.camera_movement_x
  local y_sens = utils.get_settings().game.camera_movement_y
  local x_move = sens * x_sens * constants.to_seconds(time) * delta_xpos
  local y_move = sens * y_sens * constants.to_seconds(time) * delta_ypos

  local is_currently_pressed = input.is_event_pressed(map_move)
  if is_currently_pressed and not movement_was_pressed then movement_was_pressed_on_map = not interface_hovered end

  -- тут мы должны подать delta в функцию перемещения камеры,
  -- мне бы хотелось ограничить эту дельту, чтобы нельзя было контролировать сенсу из луа
  -- а только через настройки, как это сделать? нужно придумать значение от 0 до 1
  -- такое чтобы оно было достаточно мало по сравнению с перемещением мыши
  -- брать часть от размера окна? это по идее не сильно поможет оградить от изменения сенсы
  -- точнее ограничит перемещение размерами окна + мне нужно еще как то передать время кадра
  -- время кадра тоже может помочь изменять сенсу, можно таймер туда передавать просто
  if not changed and is_currently_pressed and movement_was_pressed_on_map then camera.move(x_move, y_move) end

  movement_was_pressed = is_currently_pressed
  if not is_currently_pressed then movement_was_pressed_on_map = false end

  return is_currently_pressed
end

local selection_was_pressed = false
local selection_last_xpos, selection_last_ypos = -1.0, -1.0
local activate_click = input.get_event("activate_click")
local function map_selection(nk_ctx, selection_name)
  local xpos, ypos = input.get_cursor_pos()
  local current_pressed = input.is_event_pressed(activate_click)

  if not selection_was_pressed and not current_pressed then return false end

  if not selection_was_pressed and current_pressed then
    selection_last_xpos = xpos
    selection_last_ypos = ypos
    selection_was_pressed = current_pressed

    return current_pressed
  end

  if selection_was_pressed and not current_pressed then
    selection_last_xpos, selection_last_ypos = -1.0, -1.0
    selection_was_pressed = current_pressed

    selection.copy()

    local p = selection.primary
    print("p.count " .. p.count)
    for i = 1, p.count do
      print(p.array[i])
    end
    return current_pressed
  end

  local screen_minx = math.min(selection_last_xpos, math.max(xpos, 0.0))
  local screen_miny = math.min(selection_last_ypos, math.max(ypos, 0.0))
  local screen_maxx = math.max(selection_last_xpos, math.max(xpos, 0.0))
  local screen_maxy = math.max(selection_last_ypos, math.max(ypos, 0.0))
  local sizex, sizey = screen_maxx - screen_minx, screen_maxy - screen_miny
  local min_sizex, min_sizey = math.max(sizex, 1.0), math.max(sizey, 1.0)
  local final_screen_maxx, final_screen_maxy = screen_minx + min_sizex, screen_miny + min_sizey
  utils.set_selection_box(screen_minx, screen_miny, final_screen_maxx, final_screen_maxy)

  if nk.window_begin(nk_ctx, selection_name, {screen_minx, screen_miny, min_sizex, min_sizey}, nk.WINDOW_BORDER) then end -- luacheck: ignore
  nk.window_end(nk_ctx)

  return current_pressed
end

local control_click = input.get_event("control_click")
local action_was_pressed = false
local end_tile = -1
local new_tile = false
local click_count = 0
local function map_action(casted_tile_index)
  local current_pressed = input.is_event_pressed(control_click)
  local is_clicked = action_was_pressed and not current_pressed
  local is_not_pressed = not action_was_pressed and not current_pressed
  action_was_pressed = current_pressed

  -- тут пока не понятно как сделать, интерфейс у армий должен включать поиск пути
  -- обходим селектор, выбираем армии, тыкаем идти

  local p = selection.primary
  for i = 1, p.count do
    local obj = p.array[i]
    if core.type(obj) == "army" then core.highlight_unit_path(obj, path_colors) end
  end

  if is_not_pressed then return false end

  new_tile = end_tile ~= casted_tile_index
  end_tile = casted_tile_index

  local character = nil

  if p.count ~= 0 then
    if new_tile then
      local tile = core.get_tile(end_tile)
      if tile.height > 0.5 or tile.height < 0.0 then return current_pressed end

      print("finding path to " .. end_tile)
      for i = 1, p.count do
        local obj = p.array[i]
        if core.type(obj) == "army" then obj:find_path(end_tile) end
      end
      click_count = 0
    end

    click_count = click_count + bool_to_number(is_clicked)
    if click_count == 2 then
      print("advance army")
      for i = 1, p.count do
        local obj = p.array[i]
        if core.type(obj) == "army" then obj:advance() end
      end
    end
  else
    if is_clicked and core.is_tile_index_valid(casted_tile_index) then
      local province = core.get_tile_province(casted_tile_index)
      if province ~= nil then
        local title = province.title
        local faction = title.owner
        local char = faction.leader
        character = char
      end
    end
  end

  return current_pressed, character
end

local function classic_strategic_camera_movement(time)
  local fbw, fbh = input.get_framebuffer_size()
  local xpos, ypos = input.get_cursor_pos()
  local cursor_pos = {xpos / fbw, ypos / fbh} -- ???
  local movement_border = {0.01, 0.01}

  local square1 = cursor_pos[1] <  0.5 and cursor_pos[2]  < 0.5
  local square2 = cursor_pos[1] >= 0.5 and cursor_pos[2]  < 0.5
  local square3 = cursor_pos[1] <  0.5 and cursor_pos[2] >= 0.5
  local square4 = cursor_pos[1] >= 0.5 and cursor_pos[2] >= 0.5

  local axis_plus = not square1 and 0         or (    cursor_pos[1] <     cursor_pos[2] and 0 or 1)
        axis_plus = not square2 and axis_plus or (1.0-cursor_pos[1] <     cursor_pos[2] and 2 or 1)
        axis_plus = not square3 and axis_plus or (    cursor_pos[1] < 1.0-cursor_pos[2] and 0 or 3)
        axis_plus = not square4 and axis_plus or (1.0-cursor_pos[1] < 1.0-cursor_pos[2] and 2 or 3)

  local axis = axis_plus % 2 + 1

  if cursor_pos[axis] > movement_border[axis] and cursor_pos[axis] < 1.0-movement_border[axis] then return end

  -- сенсу нужно сильно увеличить, и нужно ограничить угол подъема камеры
  local zoom = camera.zoom()
  local sens = utils.get_settings().game.camera_movement * (1.0 + zoom * 0.5)
  local x_sens = utils.get_settings().game.camera_movement_x
  local y_sens = utils.get_settings().game.camera_movement_y
  local x_move = sens * x_sens * constants.to_seconds(time) * ((-1.0)*bool_to_number(axis_plus == 0) + bool_to_number(axis_plus == 2))
  local y_move = sens * y_sens * constants.to_seconds(time) * ((-1.0)*bool_to_number(axis_plus == 1) + bool_to_number(axis_plus == 3))

  camera.move(x_move, y_move)
end

local function mouse_movement(nk_ctx, game_ctx, time, interface_hovered, selection_name)
  -- возможность блокировки мыши внутри окна игры - это то что явно должно быть скрыто от моддера
  -- по идее мы легко можем отделить одно от другого, но функция должна учитывать фокус окна
  -- причем желательно зафорсить это поведение

  if interface_hovered or not camera.valid() then return end

  local pressed1 = camera_movement(time, interface_hovered)
  local pressed2 = map_selection(nk_ctx, selection_name)
  local pressed3, character = map_action(game_ctx.tile_index)

  if pressed1 or pressed2 or pressed3 then return character end

  classic_strategic_camera_movement(time)
  return character
end

return mouse_movement
