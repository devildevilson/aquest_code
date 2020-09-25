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

local function check_event_press(event)
  return input.timed_check_event(event, input.state_press | input.state_double_press, 0, constants.size_max) or
    input.timed_check_event(event, input.state_long_press | input.state_double_press, constants.one_second / 2.0, constants.one_second / 15.0)
end

local window_flags <const> = nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_BACKGROUND

local next_window_table <const> = {
  -- "continue_game",       "Continue", -- так то мы это берем из локализации
  -- "new_game_window",     "New game",
  "worlds_window",       "Worlds",
  -- "load_game_window",    "Load game",
  -- "multiplayer_window",  "Multiplayer",
  -- "options_window",      "Options",
  -- "achievements_window", "Achievements",
  -- "credits_window",      "Credits",
  "quit_game_window",    "Quit game"
}

--local entries_size <const> = #next_window_table / 2

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

local function core_main_menu(ctx, menu, data, entries_table)
  local abs = math.abs;
  local dx, dy = ctx:mouse_delta()
  local mouse_movement = not (abs(dx) <= constants.epsilon and abs(dy) <= constants.epsilon)
  local fbw, fbh = input.get_framebuffer_size()
  local entries_size <const> = #next_window_table / 2

  assert(data == nil)

  -- сюда по идее должно приходить menu
  -- там мы можем определить например menu:quit_game()

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

  if main_menu_next == "quit_game_window" then menu:quit_game()
  elseif main_menu_next ~= "main_menu" then
    menu:push(main_menu_next)
    main_menu_next = "main_menu"
  end
  --if check_event_click(escape) then menu:escape() end -- по идее это не обязательно
  --if back then menu:escape() end

  -- по хорошему нужно и эскейп обработать, а в интерфейсе хранить указатель на дефолтное окно
  -- if check_event_click(escape) or main_menu_next ~= "main_menu_window" then
  --   main_menu_next = "main_menu_window"
  --   main_menu_focus = 1
  --   return true -- сообщает что мы хотим разрушить окно
  -- end

  --return false
end

function main_menu_window(ctx, menu, data)
  core_main_menu(ctx, menu, data, next_window_table)
end

local map_window_table <const> = {
  "back_to_main_menu",       "Back to main menu",
  "quit_game_window",    "Quit game"
}

function main_menu_map(ctx, menu, data)
  core_main_menu(ctx, menu, data, map_window_table)
end

local option_selected = -1

local mt_color = {
  __close = function(self, error)
    nk.style_set_color(self.ctx, self.style_field, self.old_color)
  end,
}

local mt_style_item = {
  __close = function(self, error)
    nk.style_set_style_item(self.ctx, self.style_field, self.old_style_item)
  end,
}

local function create_closable_color(ctx, style_field, color)
  local old_color = nk.style_get_color(ctx, style_field)
  nk.style_set_color(ctx, style_field, color)
  local table = { ctx = ctx, style_field = style_field, old_color = old_color, }
  setmetatable(table, mt_color)
  return table
end

local function create_closable_style_item(ctx, style_field, style_item)
  local old_style_item = nk.style_get_style_item(ctx, style_field)
  nk.style_set_style_item(ctx, style_field, style_item)
  local table = { ctx = ctx, style_field = style_field, old_style_item = old_style_item, }
  setmetatable(table, mt_style_item)
  return table
end

function worlds_window_func(ctx, menu, demiurge)
  local abs = math.abs;
  local min = math.min;
  local dx, dy <const> = ctx:mouse_delta()
  local mouse_movement <const> = not (abs(dx) <= constants.epsilon and abs(dy) <= constants.epsilon)
  local fbw, fbh <const> = input.get_framebuffer_size()

  assert(demiurge ~= nil)

  -- тут должен быть доступ к мирам на диске
  -- в demiurge должен придти класс с помощью которого мы можем просмотреть миры которые есть у нас на диске
  -- с помощью него же мы должны решить удалить ли нам этот мир или наоборот загрузить его
  -- я еще не сделал выход из меню, то есть когда мы приняли какое то решение, нужно выйти из меню и почистить
  -- ресурсы, решение? видимо если мы в demiurge выбрали какой то мир и нажали кнопку загрузить

  local sizex, sizey <const> = 500, 500
  local y <const> = fbh/2.0-sizey/2.0
  local selectable_size = ctx:font():height() + 15 --  + ctx:font():height() + 5
  local count <const> = demiurge:worlds_count()
  local group_bounds = {fbw/2.0-sizex/2.0, y, sizex, sizey}

  if nk.window_begin(ctx, "worlds_window", {0, 0, fbw, fbh}, window_flags) then
    -- должно быть небольшое окно с прокруткой
    --local window_bounds = nk.window_get_bounds(ctx)
    nk.layout_space_begin(ctx, nk.STATIC, fbh, 10)
    nk.layout_space_push(ctx, {fbw/2.0-sizex/2.0, y/2.0-ctx:font():height()/2.0, sizex, ctx:font():height()})
    nk.label(ctx, "Apate's quest", nk.TEXT_ALIGN_CENTERED)

    nk.layout_space_push(ctx, group_bounds)
    local bounds <const> = nk.layout_space_bounds(ctx)
    if nk.group_begin(ctx, "worlds_group", nk.WINDOW_BORDER) then
      -- не сработало
      --local window_background_color <close> = create_closable_color(ctx, 'window.background', {1,0,0,1})
      --local window_fixed_background_color <close> = create_closable_style_item(ctx, 'window.fixed_background', {1,0,0,1})
      --nk.style_set_style_item(ctx, 'window.fixed_background', {1,0,0,1})
      nk.layout_space_begin(ctx, nk.STATIC, fbh, 64)
      nk.layout_space_push(ctx, {0, 0, sizex * 0.4, selectable_size})
      --nk.layout_row_dynamic(ctx, selectable_size, 1)
      --nk.layout_row(ctx, nk.DYNAMIC, selectable_size, {0.4, 0.6})
      --local bounds = nk.widget_bounds(ctx)
      local selected, changed = nk.selectable(ctx, nil, "New world", nk.TEXT_LEFT, option_selected == 0) -- наверное мы можем поверх селектабл расположить надписи
      --nk.label(ctx, '', nk.TEXT_ALIGN_LEFT)
      if changed and selected then option_selected = 0 end
      if changed and not selected then option_selected = -1 end
      -- nk.layout_space_push(ctx, bounds) -- не понимаю как можно рисовать поверх
      -- nk.label(ctx, "123", nk.TEXT_ALIGN_RIGHT)
      --nk.layout_row_dynamic(ctx, 3, 1)
      --nk.spacing(ctx, 1)
      for i=1,count do
        -- тут видимо придется использовать не селектабл
        local startx = selectable_size * i
        local world_table = demiurge:world(i-1)
        --nk.layout_row_dynamic(ctx, selectable_size, 1)
        --nk.layout_row(ctx, nk.DYNAMIC, selectable_size, {0.4, 0.6})
        nk.layout_space_push(ctx, {0, startx, sizex * 0.4, selectable_size})
        local bounds = nk.widget_bounds(ctx)
        local selected, changed = nk.selectable(ctx, nil, world_table.world_name, nk.TEXT_LEFT, option_selected == i)
        nk.layout_space_push(ctx, {sizex * 0.4, startx, sizex - bounds[3], selectable_size})
        nk.label(ctx, world_table.settings, nk.TEXT_LEFT) -- можно хотябы так сделать
        --nk.layout_row_dynamic(ctx, 3, 1)
        --nk.spacing(ctx, 1)
        if changed and selected then option_selected = i end
        if changed and not selected then option_selected = -1 end
      end
      nk.group_end(ctx)
    end

    local first_window_offset_x, first_window_offset_y = nk.group_get_scroll(ctx, "worlds_group")

    local footer_count = 1
    if option_selected > -1 then
      footer_count = 3
    end

    local footer_starty = y+sizey+10
    local footer_startx = fbw/2.0-sizex/2.0
    local footer_height = min(fbh-footer_starty, 30)
    local button_sizex = sizex / 3 - 20

    nk.layout_space_push(ctx, {footer_startx+10, footer_starty, button_sizex, footer_height})
    if nk.button(ctx, nil, "Back") then
      menu:escape()
    end
    if option_selected > -1 then
      nk.layout_space_push(ctx, {footer_startx+10+button_sizex+10+10, footer_starty, button_sizex, footer_height})
      if nk.button(ctx, nil, "Remove") then
        if option_selected > 0 then print("remove " .. demiurge:world(option_selected-1).world_name) end
      end
      nk.layout_space_push(ctx, {footer_startx+10+button_sizex+10+10+button_sizex+10+10, footer_starty, button_sizex, footer_height})
      if nk.button(ctx, nil, option_selected == 0 and "Create" or "Load") then
        if option_selected == 0 then demiurge:create_new_world() else demiurge:choose_world(option_selected-1) end
      end
    end

    nk.layout_space_end(ctx)
  end
  nk.window_end(ctx)

  -- поверх нарисовать тоже невышло
  -- local window_background_color <close> = create_closable_color(ctx, 'window.background', {1,1,1,0})
  -- local window_fixed_background_color <close> = create_closable_style_item(ctx, 'window.fixed_background', {1,1,1,0})
  -- if nk.window_begin(ctx, "tmp", group_bounds, nk.WINDOW_NO_SCROLLBAR) then
  --   nk.layout_space_begin(ctx, nk.STATIC, group_bounds[4], 1)
  --   nk.layout_space_push(ctx, group_bounds)
  --   if nk.group_begin(ctx, "worlds_group_tmp", nk.WINDOW_BORDER) then
  --     nk.layout_row_dynamic(ctx, selectable_size, 1)
  --     --local bounds = nk.widget_bounds(ctx)
  --     -- local selected, changed = nk.selectable(ctx, nil, "New world", nk.TEXT_ALIGN_LEFT, option_selected == 0) -- наверное мы можем поверх селектабл расположить надписи
  --     -- if changed and selected then option_selected = 0 end
  --     -- if changed and not selected then option_selected = -1 end
  --     --nk.layout_space_push(ctx, bounds) -- не понимаю как можно рисовать поверх
  --     nk.label(ctx, "123", nk.TEXT_ALIGN_RIGHT)
  --     nk.layout_row_dynamic(ctx, 3, 1)
  --     nk.spacing(ctx, 1)
  --     for i=1,count do
  --       -- тут видимо придется использовать не селектабл
  --       local world_table = demiurge:world(i-1)
  --       nk.layout_row_dynamic(ctx, selectable_size, 1)
  --       --local selected, changed = nk.selectable(ctx, nil, world_table.world_name, nk.TEXT_ALIGN_LEFT, option_selected == i)
  --       --local bounds = nk.widget_bounds(ctx)
  --       --nk.layout_space_push(ctx, bounds)
  --       nk.label(ctx, "123", nk.TEXT_ALIGN_RIGHT)
  --       nk.layout_row_dynamic(ctx, 3, 1)
  --       nk.spacing(ctx, 1)
  --       --if changed and selected then option_selected = i end
  --       --if changed and not selected then option_selected = -1 end
  --     end
  --     nk.group_end(ctx)
  --   end
  --   nk.layout_space_end(ctx)
  -- end
  -- nk.window_end(ctx)

  if check_event_press(next) then
    option_selected = (option_selected + 1) % (count + 1)
    -- print("next focus " .. next_window_table[main_menu_focus])
  end

  if check_event_press(prev) then
    option_selected = option_selected - 1
    option_selected = option_selected < 0 and count or option_selected
    -- print("previous focus " .. next_window_table[main_menu_focus])
  end

  if option_selected ~= -1 and check_event_click(choose) then
    if option_selected == 0 then demiurge:create_new_world() else demiurge:choose_world(option_selected-1) end
    print("choose " .. option_selected)
  end
end
