local nk = require("moonnuklear")

local next <const> = input.get_event("menu_next")
local prev <const> = input.get_event("menu_prev")
local increase <const> = input.get_event("menu_increase")
local decrease <const> = input.get_event("menu_decrease")
local choose <const> = input.get_event("menu_choose")
local escape <const> = input.get_event("escape")

local function check_event_click(event)
  return input.check_event(event, input.state_click | input.state_double_click | input.state_long_click)
end

local function check_event_long_press(event)
  return input.timed_check_event(event, input.state_press | input.state_double_press, 0, constants.size_max) or
    input.timed_check_event(event, input.state_long_press | input.state_double_press, constants.one_second / 2.0, constants.one_second / 15.0)
end

local window_flags <const> = nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_BACKGROUND

local next_window_table <const> = {
  "continue_game",       "Continue", -- так то мы это берем из локализации
  "new_game_window",     "New game",
  "worlds_window",       "Worlds",
  "load_game_window",    "Load game",
  "multiplayer_window",  "Multiplayer",
  "options_window",      "Options",
  "achievements_window", "Achievements",
  "credits_window",      "Credits",
  "quit_game_window",    "Quit game"
}

local entries_size <const> = #next_window_table / 2

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

local create_menu_layout = function(ctx, b)
  -- размер шрифта? наверное нужно взять текущий
  local bounds = fix_struct(b)
  local menu_count = entries_size
  local font_height <const> = ctx:font():height()
  local offset_y <const> = 3
  local offset_x <const> = 10
  local menu_height <const> = menu_count * offset_y * 2.0 + menu_count * font_height
  local half_menu_height <const> = menu_height / 2.0
  local y <const> = bounds.h/2.0 - half_menu_height;
  local label_height <const> = offset_y + font_height + offset_y
  local menu_width <const> = bounds.w * 0.25
  local x <const> = bounds.w/2.0 - menu_width/2.0

  local offset_to_menu <const> = y + label_height*0-offset_y
  local logo = {0, offset_to_menu/2.0-50, bounds.w, offset_to_menu}

  local entries = {}
  for i=1,#next_window_table,2 do
    local index = (i+1)/2
    entries[#entries+1] = {x, y + label_height*index-offset_y, menu_width, font_height}
  end

  -- нужны ли изображения? у меня их все равно сейчас нет
  return {
    logo = logo,
    entries = entries
  }
end

local main_menu_focus = 0
local main_menu_next = "main_menu_window"

-- почему
-- val 4.0
-- val 0.0
-- val 1272.0
-- val 724.0

function main_menu_window(ctx, interface, data)
  local dx, dy = ctx:mouse_delta()
  local mouse_movement = not (dx == 0.0 and dy == 0.0)
  local fbw, fbh = input.get_framebuffer_size()

  local function check_focus(ctx, current_focus, current_next)
    local menu_label_bounds = nk.widget_bounds(ctx)
    local is_hover = ctx:is_mouse_hovering_rect(menu_label_bounds)
    if is_hover and mouse_movement and main_menu_focus ~= current_focus then main_menu_focus = current_focus end
    if ctx:is_mouse_click_in_rect(nk.BUTTON_LEFT, menu_label_bounds) then
      main_menu_next = current_next
      print("choose " .. main_menu_next)
    end
    return is_hover
  end

  local hover_one = false
  if nk.window_begin(ctx, "main_menu_window", {0, 0, fbw, fbh}, window_flags) then
    nk.layout_space_begin(ctx, nk.STATIC, fbh, 10);
    nk.layout_space_push(ctx, {0, 0, fbw, fbh})
    local bounds = nk.layout_space_bounds(ctx)
    local menu_layout = create_menu_layout(ctx, bounds)

    nk.layout_space_push(ctx, menu_layout.logo)
    --nk.layout_space_push(ctx, {0, -7, 1280, 5})
    nk.label(ctx, "Apate's quest", nk.TEXT_ALIGN_CENTERED, {255, 0, 0, 255})

    --print(#menu_layout.entries)
    --print(#next_window_table)
    assert(#menu_layout.entries == #next_window_table / 2)
    for i=1,#next_window_table,2 do
      local index = (i+1)/2
      nk.layout_space_push(ctx, menu_layout.entries[index])
      nk.label(ctx, next_window_table[i+1], nk.TEXT_ALIGN_LEFT, main_menu_focus == index and {255, 255, 0, 255} or {255, 0, 0, 255})
      hover_one = hover_one or check_focus(ctx, index, next_window_table[i])
    end

    nk.layout_space_end(ctx)
  end
  nk.window_end(ctx)

  --if not hover_one then main_menu_focus = 0

  if check_event_long_press(next) then
    main_menu_focus = (main_menu_focus + 1) % (entries_size + 1)
    main_menu_focus = main_menu_focus == 0 and 1 or main_menu_focus
    -- print("next focus " .. next_window_table[main_menu_focus])
  end

  if check_event_long_press(prev) then
    main_menu_focus = main_menu_focus - 1
    main_menu_focus = main_menu_focus < 1 and entries_size or main_menu_focus
    -- print("previous focus " .. next_window_table[main_menu_focus])
  end

  if check_event_click(choose) and main_menu_focus ~= 0 then
    local index = 1+(main_menu_focus-1)*2
    main_menu_next = next_window_table[index]
    print("choose " .. main_menu_next)
  end

  -- по хорошему нужно и эскейп обработать, а в интерфейсе хранить указатель на дефолтное окно
  -- if check_event_click(escape) or main_menu_next ~= "main_menu_window" then
  --   interface:open_window(main_menu_next, data)
  --   main_menu_next = "main_menu_window"
  --   main_menu_focus = 1
  --   return true -- сообщает что мы хотим разрушить окно
  -- end

  return false
end
