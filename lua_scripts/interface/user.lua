local nk = require("moonnuklear")

local worlds_window_func  = require("apates_quest.scripts.interface.worlds")
local options_window_func = require("apates_quest.scripts.interface.settings_menu")

local menu_table = {
  worlds_window = worlds_window_func,
  options_window = options_window_func
}

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

local next_window_table = {
  -- "continue_game",       "Continue", -- так то мы это берем из локализации
  -- "new_game_window",     "New game",
  "worlds_window",       "Worlds",
  -- "load_game_window",    "Load game",
  -- "multiplayer_window",  "Multiplayer",
  "options_window",      "Options",
  -- "achievements_window", "Achievements",
  -- "credits_window",      "Credits",
  "quit_game_window",    "Quit game"
}

--local entries_size = #next_window_table / 2

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
  --local menu_count = entries_size
  local menu_count = #entries_table / 2
  --local font_height = ctx:font():height()
  --local f1 = get_font(0)
  --print(ctx)
  --print(f1)
  --nk.style_set_font(ctx, f1)
  --print(ctx:font())
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

local main_menu_focus = 0
local main_menu_next = "main_menu"

-- почему
-- val 4.0
-- val 0.0
-- val 1272.0
-- val 724.0

local function core_main_menu(ctx, game_ctx, timer, local_table, menu_stack, entries_table)
  local abs = math.abs
  local dx, dy = ctx:mouse_delta()
  local mouse_movement = not (abs(dx) <= constants.epsilon and abs(dy) <= constants.epsilon)
  local fbw, fbh = input.get_framebuffer_size()
  local entries_size = #next_window_table / 2

  -- сюда по идее должно приходить menu
  -- там мы можем определить например menu:quit_game()

  local function check_focus(_, current_focus, current_next)
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
  if nk.window_begin(ctx, "main_menu_window", {0, 0, 100, 100}, window_flags) then
    nk.layout_space_begin(ctx, nk.STATIC, fbh, 10);
    nk.layout_space_push(ctx, {0, 0, fbw, fbh})
    local bounds = nk.layout_space_bounds(ctx)
    local menu_layout = create_menu_layout(ctx, bounds, entries_table)

    nk.layout_space_push(ctx, menu_layout.logo)
    --nk.layout_space_push(ctx, {0, -7, 1280, 5})
    nk.label(ctx, "Apate's quest", nk.TEXT_ALIGN_CENTERED, {255, 0, 0, 255})

    --print(#menu_layout.entries)
    --print(#entries_table)
    assert(#menu_layout.entries == #entries_table / 2)
    for i=1,#entries_table,2 do
      local index = (i+1)/2
      nk.layout_space_push(ctx, menu_layout.entries[index])
      nk.label(ctx, entries_table[i+1], nk.TEXT_ALIGN_LEFT, main_menu_focus == index and {255, 255, 0, 255} or {255, 0, 0, 255})
      hover_one = hover_one or check_focus(ctx, index, entries_table[i])
    end

    nk.layout_space_end(ctx)
  end
  nk.window_end(ctx)

  --if not hover_one then main_menu_focus = 0

  if check_event_press(next) then
    main_menu_focus = (main_menu_focus + 1) % (entries_size + 1)
    main_menu_focus = main_menu_focus == 0 and 1 or main_menu_focus
    -- print("next focus " .. entries_table[main_menu_focus])
  end

  if check_event_press(prev) then
    main_menu_focus = main_menu_focus - 1
    main_menu_focus = main_menu_focus < 1 and entries_size or main_menu_focus
    -- print("previous focus " .. entries_table[main_menu_focus])
  end

  if main_menu_focus ~= 0 and check_event_click(choose) then
    local index = 1+(main_menu_focus-1)*2
    main_menu_next = entries_table[index]
    print("choose " .. main_menu_next)
  end

  if check_event_click(escape) then
    menu_stack:pop_right()
    return
  end

  if main_menu_next == "back_to_main_menu" then game_ctx:main_menu()
  elseif main_menu_next == "quit_game_window" then game_ctx:quit_game()
  elseif main_menu_next ~= "main_menu" then
    -- как теперь добавляем меню? раньше я добавлял строку, а теперь наверное нужно добавлять фукнкцию
    -- я думаю, нужно ли таблицу создавать для разных нужд?
    -- чистить таблицу после использования чтобы немного памяти сэкономить
    -- хотя все равно, тут должна быть таблица функций меню
    print(main_menu_next)
    local abc = menu_table[main_menu_next]
    assert(type(abc) == "function")
    menu_stack:push_right(abc)
    main_menu_next = "main_menu"
    main_menu_focus = 0
  end
end

local function main_menu_window(ctx, game_ctx, timer, local_table, menu_stack)
  core_main_menu(ctx, game_ctx, timer, local_table, menu_stack, next_window_table)
end

local map_window_table = {
  "back_to_main_menu",   "Back to main menu",
  "quit_game_window",    "Quit game"
}

local function main_menu_map(ctx, game_ctx, timer, local_table, menu_stack)
  core_main_menu(ctx, game_ctx, timer, local_table, menu_stack, map_window_table)
end

return {
  core_menu = main_menu_window,
  map_menu = main_menu_map,
}
