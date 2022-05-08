local nk = require("moonnuklear")

local function check_event_click(event)
  return input.check_event(event, input.state_click | input.state_double_click | input.state_long_click)
end

local function check_event_press(event)
  return input.timed_check_event(event, input.state_press | input.state_double_press, 0, constants.size_max) or
    input.timed_check_event(event, input.state_long_press | input.state_double_press, constants.one_second / 2.0, constants.one_second / 15.0)
end

local next = input.get_event("menu_next")
local prev = input.get_event("menu_prev")
--local increase = input.get_event("menu_increase")
--local decrease = input.get_event("menu_decrease")
local choose = input.get_event("menu_choose")
local escape = input.get_event("escape")

local window_flags = nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_BACKGROUND

local worlds_data_container = nil
local option_selected = -1

local function worlds_window(ctx, _, _, _, menu_stack) -- game_ctx, timer, local_table
  if worlds_data_container == nil then worlds_data_container = utils.demiurge.new() end
  local exit = false

  --local abs = math.abs
  local min = math.min
  --local dx, dy = ctx:mouse_delta()
  --local mouse_movement = not (abs(dx) <= constants.epsilon and abs(dy) <= constants.epsilon)
  local fbw, fbh = input.get_framebuffer_size()

  assert(worlds_data_container ~= nil)

  -- тут должен быть доступ к мирам на диске
  -- в demiurge должен придти класс с помощью которого мы можем просмотреть миры которые есть у нас на диске
  -- с помощью него же мы должны решить удалить ли нам этот мир или наоборот загрузить его
  -- я еще не сделал выход из меню, то есть когда мы приняли какое то решение, нужно выйти из меню и почистить
  -- ресурсы, решение? видимо если мы в demiurge выбрали какой то мир и нажали кнопку загрузить

  local sizex, sizey = 500, 500
  local y = fbh/2.0-sizey/2.0
  local selectable_size = ctx:font():height() + 15 --  + ctx:font():height() + 5
  local count = worlds_data_container:worlds_count()
  local group_bounds = {fbw/2.0-sizex/2.0, y, sizex, sizey}

  if nk.window_begin(ctx, "worlds_window", {0, 0, fbw, fbh}, window_flags) then
    -- должно быть небольшое окно с прокруткой
    --local window_bounds = nk.window_get_bounds(ctx)
    nk.layout_space_begin(ctx, nk.STATIC, fbh, 10)
    nk.layout_space_push(ctx, {fbw/2.0-sizex/2.0, y/2.0-ctx:font():height()/2.0, sizex, ctx:font():height()})
    nk.label(ctx, "Apate's quest", nk.TEXT_ALIGN_CENTERED)

    nk.layout_space_push(ctx, group_bounds)
    --local bounds = nk.layout_space_bounds(ctx)
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
      do
        -- наверное мы можем поверх селектабл расположить надписи (чет не получилось)
        local selected, changed = nk.selectable(ctx, nil, "New world", nk.TEXT_LEFT, option_selected == 0)
        --nk.label(ctx, '', nk.TEXT_ALIGN_LEFT)
        if changed and selected then option_selected = 0 end
        if changed and not selected then option_selected = -1 end
      end
      -- nk.layout_space_push(ctx, bounds) -- не понимаю как можно рисовать поверх
      -- nk.label(ctx, "123", nk.TEXT_ALIGN_RIGHT)
      --nk.layout_row_dynamic(ctx, 3, 1)
      --nk.spacing(ctx, 1)
      for i = 1,count do
        -- тут видимо придется использовать не селектабл
        local startx = selectable_size * i
        local world_table = worlds_data_container:world(i)
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

    --local first_window_offset_x, first_window_offset_y = nk.group_get_scroll(ctx, "worlds_group")

    -- local footer_count = 1
    -- if option_selected > -1 then
    --   footer_count = 3
    -- end

    local footer_starty = y+sizey+10
    local footer_startx = fbw/2.0-sizex/2.0
    local footer_height = min(fbh-footer_starty, 30)
    local button_sizex = sizex / 3 - 20

    nk.layout_space_push(ctx, {footer_startx+10, footer_starty, button_sizex, footer_height})
    if nk.button(ctx, nil, "Back") then
      menu_stack:pop_right()
      exit = true
      option_selected = -1
    end
    if option_selected > -1 then
      nk.layout_space_push(ctx, {footer_startx+10+button_sizex+10+10, footer_starty, button_sizex, footer_height})
      if nk.button(ctx, nil, "Remove") then
        if option_selected > 0 then print("remove " .. worlds_data_container:world(option_selected).world_name) end
      end
      nk.layout_space_push(ctx, {footer_startx+10+button_sizex+10+10+button_sizex+10+10, footer_starty, button_sizex, footer_height})
      if nk.button(ctx, nil, option_selected == 0 and "Create" or "Load") then
        if option_selected == 0 then worlds_data_container:create_new_world()
        else worlds_data_container:choose_world(option_selected) end

        exit = true
        menu_stack:clear()
      end
    end

    nk.layout_space_end(ctx)
  end
  nk.window_end(ctx)

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
    if option_selected == 0 then worlds_data_container:create_new_world()
    else worlds_data_container:choose_world(option_selected) end

    exit = true
    menu_stack:clear()
    print("choose " .. option_selected)
  end

  if check_event_click(escape) then
    exit = true
    menu_stack:pop_right()
    print("escape")
  end

  if exit then worlds_data_container = nil end
end

return worlds_window
