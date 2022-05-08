-- короче нужно переписать каждое окно в отдельный скрипт

local nk = require("moonnuklear")

local next = input.get_event("menu_next")
local prev = input.get_event("menu_prev")
--local increase = input.get_event("menu_increase")
--local decrease = input.get_event("menu_decrease")
local choose = input.get_event("menu_choose")
local escape = input.get_event("escape")

local function check_event_click(event)
  return input.check_event(event, input.state_click | input.state_double_click | input.state_long_click)
end

local function check_event_press(event)
  return input.timed_check_event(event, input.state_press | input.state_double_press, 0, constants.size_max) or
    input.timed_check_event(event, input.state_long_press | input.state_double_press, constants.one_second / 2.0, constants.one_second / 15.0)
end

local window_flags = nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_BACKGROUND

local function fix_struct(bounds)
  --print(type(bounds))
  assert(bounds[4] ~= nil)
  return {
    x = bounds[1],
    y = bounds[2],
    w = bounds[3],
    h = bounds[4]
  }
end

local create_menu_layout = function(ctx, b, entries_table)
  -- размер шрифта? наверное нужно взять текущий
  local bounds = fix_struct(b)
  local menu_count = #entries_table / 2
  local font_height = ctx:font():height()
  local offset_y = 3
  local offset_x = 10
  local menu_height = menu_count * offset_y * 2.0 + menu_count * font_height
  local half_menu_height = menu_height / 2.0
  local y = bounds.h/2.0 - half_menu_height;
  local label_height = offset_y + font_height + offset_y
  local menu_width = bounds.w * 0.25
  local x = bounds.w/2.0 - menu_width/2.0

  local offset_to_menu = y + label_height*0-offset_y
  local logo = {0, offset_to_menu/2.0-50, bounds.w, offset_to_menu}

  local entries = {}
  for i=1,#entries_table,2 do
    local index = (i+1)/2
    entries[#entries+1] = {x, y + label_height*index-offset_y, menu_width, font_height}
  end

  -- нужны ли изображения? у меня их все равно сейчас нет
  return {
    logo = logo,
    entries = entries
  }
end

local menu_entries = {
  "test123", "Video modes",
  "test123", "current mode",
  "test123", "Fullscreen",
  --"test123", "Projection",
  "test123", "Back"
}

local current_graphics_focus = 0
local current_video_mode = 0

local function graphics_options_window(ctx, menu_stack)
  --local abs = math.abs;
  --local dx, dy = ctx:mouse_delta()
  --local mouse_movement = not (abs(dx) <= constants.epsilon and abs(dy) <= constants.epsilon)
  local fbw, fbh = input.get_framebuffer_size()
  -- как получать настроечки?
  local video_count = utils.settings.graphics:video_modes_count()
  local entries_size = #menu_entries / 2
  -- нужно придумать как сделать управление

  if nk.window_begin(ctx, "graphics_options", {0, 0, fbw, fbh}, window_flags) then
    nk.layout_space_begin(ctx, nk.STATIC, fbh, 10);
    nk.layout_space_push(ctx, {0, 0, fbw, fbh})
    local bounds = nk.layout_space_bounds(ctx)
    local menu_layout = create_menu_layout(ctx, bounds, menu_entries)

    nk.layout_space_push(ctx, menu_layout.logo)
    nk.label(ctx, "Apate's quest", nk.TEXT_ALIGN_CENTERED, {255, 0, 0, 255})

    nk.layout_space_push(ctx, menu_layout.entries[1])
    if nk.combo_begin(ctx, nil, "Video modes", {300, 300}) then
      for i=0,video_count-1 do
        local width, height, refresh_rate = utils.settings.graphics:get_video_mode(i)
        local text = ""
        if utils.settings.graphics.fullscreen then text = tostring(width) .. " " .. tostring(height) .. " " .. tostring(refresh_rate)
        else text = tostring(width) .. " " .. tostring(height) end
        --local selected = nk.combo_item(ctx, nil, text, nk.TEXT_ALIGN_LEFT)
        nk.layout_row_dynamic(ctx, ctx:font():height(), 1)
        local selected = nk.combo_item(ctx, nil, text, nk.TEXT_ALIGN_LEFT)
        if selected then
          utils.settings.graphics.width = width
          utils.settings.graphics.height = height
          utils.settings.graphics.video_mode = i
        end
      end
      nk.combo_end(ctx)
    end

    nk.layout_space_push(ctx, menu_layout.entries[2])
    if utils.settings.graphics.fullscreen then
      local width, height, refresh_rate = utils.settings.graphics:get_video_mode(utils.settings.graphics.video_mode)
      local text = tostring(width) .. " " .. tostring(height) .. " " .. tostring(refresh_rate)
      nk.label(ctx, "Current video mode: " .. text, nk.TEXT_ALIGN_LEFT)
    else
      local text = tostring(utils.settings.graphics.width) .. " " .. tostring(utils.settings.graphics.height)
      nk.label(ctx, "Current resolution: " .. text, nk.TEXT_ALIGN_LEFT)
    end

    nk.layout_space_push(ctx, menu_layout.entries[3])
    utils.settings.graphics.fullscreen = nk.option(ctx, "Fullscreen", utils.settings.graphics.fullscreen) -- , changed
    -- nk.layout_space_push(ctx, menu_layout.entries[4])
    -- settings.graphics.projection, abc123 = nk.option(ctx, "Projection", settings.graphics.projection) -- , changed

    nk.layout_space_push(ctx, menu_layout.entries[4])
    nk.label(ctx, "Back", nk.TEXT_ALIGN_LEFT)
    --local bounds = nk.widget_bounds(ctx)
    if ctx:is_mouse_click_in_rect(nk.BUTTON_LEFT, bounds) then
      menu_stack:pop_left()
      current_graphics_focus = 0
    end

    nk.layout_space_end(ctx)
  end
  nk.window_end(ctx)

  utils.settings.graphics:apply()

  if check_event_press(next) then
    current_graphics_focus = (current_graphics_focus + 1) % (entries_size + 1)
    current_graphics_focus = current_graphics_focus == 0 and 1 or current_graphics_focus
    -- print("next focus " .. entries_table[main_menu_focus])
  end

  if check_event_press(prev) then
    current_graphics_focus = current_graphics_focus - 1
    current_graphics_focus = current_graphics_focus < 1 and entries_size or current_graphics_focus
    -- print("previous focus " .. entries_table[main_menu_focus])
  end

  if check_event_click(escape) then
    menu_stack:pop_right()
  end

  if current_graphics_focus ~= 0 and check_event_click(choose) then
    local index = 1+(current_graphics_focus-1)*2
    if current_graphics_focus == 3 then utils.settings.fullscreen = not utils.settings.fullscreen end
    if current_graphics_focus == 4 then utils.settings.projection = not utils.settings.projection end
    if current_graphics_focus == 5 then menu_stack:pop_right() end
    print("choose " .. current_graphics_focus)
  end
end

local next_window_table = {
  "graphics_window", "Graphics options",
  --"game_window",     "Game options",
  --"keys_window",     "Keys options",
  "main_menu",       "Back to main menu"
}

local current_window_focus = 0
local current_window = "options_menu"

local function options_menu_window(ctx, _, _, _, menu_stack) -- game_ctx, timer, local_table
  local abs = math.abs
  local dx, dy = ctx:mouse_delta()
  local mouse_movement = not (abs(dx) <= constants.epsilon and abs(dy) <= constants.epsilon)
  local fbw, fbh = input.get_framebuffer_size()
  local entries_size = #next_window_table / 2

  local hover_one = false

  local function check_focus(_, current_focus, current_next, bounds)
    local is_hover = ctx:is_mouse_hovering_rect(bounds)
    if is_hover and mouse_movement and current_window_focus ~= current_focus then current_window_focus = current_focus end
    if ctx:is_mouse_click_in_rect(nk.BUTTON_LEFT, bounds) then
      current_window = current_next
      print("choose " .. current_window)
    end
    return is_hover
  end

  if nk.window_begin(ctx, "options_menu", {0, 0, fbw, fbh}, window_flags) then
    nk.layout_space_begin(ctx, nk.STATIC, fbh, 10);
    nk.layout_space_push(ctx, {0, 0, fbw, fbh})
    local bounds = nk.layout_space_bounds(ctx)
    local menu_layout = create_menu_layout(ctx, bounds, next_window_table)

    nk.layout_space_push(ctx, menu_layout.logo)
    nk.label(ctx, "Apate's quest", nk.TEXT_ALIGN_CENTERED, {255, 0, 0, 255})

    assert(#menu_layout.entries == #next_window_table / 2)
    for i=1,#next_window_table,2 do
      local index = (i+1)/2
      nk.layout_space_push(ctx, menu_layout.entries[index])
      nk.label(ctx, next_window_table[i+1], nk.TEXT_ALIGN_LEFT, current_window_focus == index and {255, 255, 0, 255} or {255, 0, 0, 255})
      hover_one = hover_one or check_focus(ctx, index, next_window_table[i], menu_layout.entries[index])
    end

    nk.layout_space_end(ctx)
  end
  nk.window_end(ctx)

  if check_event_press(next) then
    current_window_focus = (current_window_focus + 1) % (entries_size + 1)
    current_window_focus = current_window_focus == 0 and 1 or current_window_focus
    -- print("next focus " .. entries_table[main_menu_focus])
    -- print("current_window_focus " .. current_window_focus)
  end

  if check_event_press(prev) then
    current_window_focus = current_window_focus - 1
    current_window_focus = current_window_focus < 1 and entries_size or current_window_focus
    -- print("previous focus " .. entries_table[main_menu_focus])
    -- print("current_window_focus " .. current_window_focus)
  end

  if current_window_focus ~= 0 and check_event_click(choose) then
    local index = 1+(current_window_focus-1)*2
    current_window = next_window_table[index]
    print("choose " .. current_window)
  end

  if current_window == "main_menu" or check_event_click(escape) then
    menu_stack:pop_right()
    current_window = "options_menu"
    current_window_focus = 0
  end

  if current_window ~= "options_menu" then
    assert(current_window == "graphics_window")
    menu_stack:push_right(graphics_options_window)
    current_window = "options_menu"
    current_window_focus = 0
  end
end

return options_menu_window
