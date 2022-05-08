local nk = require("moonnuklear") -- это зарегестрированный Си модуль
--local fmt = require("fmt")        -- это зарегестрированный Си модуль
--local queue = require("apates_quest.scripts.queue")
local menu = require("apates_quest.scripts.interface.main_menu")
local gen_func1 = require("apates_quest.scripts.interface.gen_part1")
local gen_func2 = require("apates_quest.scripts.interface.gen_part2")
local gen_func3 = require("apates_quest.scripts.interface.gen_part3")

-- было бы неплохо чистить как то эти функции, так как это require
-- они хранятся во внутренней таблице, для того чтобы их почистить
-- из этой таблице, нужно наверное завести еще одну функцию
-- которая тупо пройдется по модулям и удалит их из таблицы
local progress_bar = require("apates_quest.scripts.interface.generator_progress")
local tooltip_func = require("apates_quest.scripts.interface.world_map_tooltip")
local char_panel = require("apates_quest.scripts.interface.character_view")
local mouse_movement = require("apates_quest.scripts.mouse_input")
local key_input = require("apates_quest.scripts.keyboard_input")

local escape = input.get_event("escape")

local function check_event_click(event)
  return input.check_event(event, input.state_click | input.state_double_click | input.state_long_click)
end

-- для того чтобы почистить модуль можно использовать что то вроде
--utils.clear_module("apates_quest.scripts.interface.generator_progress")
-- должна ли эта функция вызывать чистку мусора?

local generator_interfaces = nil

local tooltip_name = "tile_tooltip"
local screen_selection_box_name = "screen_selection_box"

local escape_clicked = false

local function main_menu_state_function(nk_ctx, game_ctx, timer, local_table)
  --keys_input() -- в главном меню поди не нужно
  if not menu.exist() then menu.init_menu(true) end
  -- тут мы должны вызвать функцию главного меню, где добавим в стек первое меню
  local ret = menu.core_menu(nk_ctx, game_ctx, timer, local_table)
  assert(ret == false)
  -- как менять состояния игры?
  -- game.generate_new_map() -- game.load_map(variable) -- game.load_safe_file(variable)
  -- эти функции по идее должны влиять на состояние
  -- тут нам нужно чистить стек в таком случае
end

local function world_map_generator_state_function(nk_ctx, _, _, local_table) -- game_ctx, timer
  -- тут мы выводим интерфейс генератора, получаем функции интерфейса как и раньше
  -- + у нас добавится информация в таблице
  local t = local_table.post_generation_table
  if t ~= nil then -- luacheck: ignore
    -- в этой таблице могут быть например мета информация о сгенерированых персонажах
  end

  local mg = local_table.map_generator
  -- стейт сменится на следующий кадр, а в этом кадре можно пока что сохранить информацию с генератора
  if mg.is_finished() then
    generator_interfaces = nil
    -- core.clear_module("apates_quest.scripts.interface.gen_part1")
    -- core.clear_module("apates_quest.scripts.interface.gen_part2")
    -- core.clear_module("apates_quest.scripts.interface.gen_part3")
    return
  end

  if generator_interfaces == nil then
    assert(require ~= nil)
    assert(type(require) == "function")
    --local gen_func1 = require("apates_quest.scripts.interface.gen_part1")
    --local gen_func2 = require("apates_quest.scripts.interface.gen_part2")
    --local gen_func3 = require("apates_quest.scripts.interface.gen_part3")
    generator_interfaces = {gen_func1, gen_func2, gen_func3}
  end

  assert(mg ~= nil)
  assert(generator_interfaces[mg.step()] ~= nil)
  generator_interfaces[mg.step()](nk_ctx, local_table)
  -- текущая логика такая: после генерации на один кадр мы попадаем в это состояние
  -- здесь степ может стать больше 3, нужно либо переключать стейты еще при генерации,
  -- либо добавить возможность определить какой стейт последний
  -- вообще после любого шага генерации у нас могут появиться данные после генерации
  -- поэтому возможно даже удобно что мы всегда работаем с этими данными в этом стейте (?)
end

local currently_oppened_world_map_window = nil

local function left_world_map_window(nk_ctx)
  if core.type(currently_oppened_world_map_window) ~= "character" then return end
  local fbw, fbh = input.get_framebuffer_size()
  -- это окно уже должно быть на всю высоту окна игры, хотя это в целом зависит от того что у меня будет снизу
  -- какая ширина? в цк2 окно было чуть меньше чем половина экрана
  local sizex, sizey = fbw/2.0, fbh-100
  local bounds = {0, 100, sizex, sizey}
  char_panel(nk_ctx, currently_oppened_world_map_window, bounds)
  if escape_clicked then
    currently_oppened_world_map_window = nil
    escape_clicked = false
  end
end

local world_map_upper_panel_window_flags = nk.WINDOW_NO_SCROLLBAR
local upper_panel_button_width = 100
local upper_panel_buttons_count = 5
local upper_panel_width = upper_panel_buttons_count * upper_panel_button_width
local function world_map_upper_panel(nk_ctx, game_ctx, _) -- timer
  --local mouse_x, mouse_y = input.get_cursor_pos()
  --local fbw, fbh = input.get_framebuffer_size()

  -- хотя возможно на мобиле нужно будет сделать эту панельку слева
  -- какой у нее размер? там должны поместиться, сколько, 5 (?) кнопок как минимум
  -- кнопки одинакового размера, копки состоят из нескольких картинок (дефолт, наведено, нажато)
  --
  if nk.window_begin(nk_ctx, "upper_panel", {0, 0, upper_panel_width, upper_panel_button_width}, world_map_upper_panel_window_flags) then
    nk.layout_row_dynamic(nk_ctx, upper_panel_button_width-10, upper_panel_buttons_count)
    if nk.button(nk_ctx, {0.0, 0.7, 0.0, 1.0}) then
      -- рисуем окно персонажа, возможно мы можем использовать непосредственно персонажа которогго рисуем
      -- или фракцию, или город (хотя город другой интерфейс)
      if currently_oppened_world_map_window ~= game_ctx.player_character then
        currently_oppened_world_map_window = game_ctx.player_character
      else
        currently_oppened_world_map_window = nil
      end
    end
    nk.button(nk_ctx, {0.0, 0.7, 0.0, 1.0})
    nk.button(nk_ctx, {0.0, 0.7, 0.0, 1.0})
    nk.button(nk_ctx, {0.0, 0.7, 0.0, 1.0})
    nk.button(nk_ctx, {0.0, 0.7, 0.0, 1.0})
  end
  nk.window_end(nk_ctx)
end

local right_panel_array = {
  core.character_stats.money, core.character_stats.authority, core.character_stats.esteem, core.character_stats.influence
}

local upper_right_panel_info_width = 50
local upper_right_panel_info_count = #right_panel_array
local upper_right_panel_offset = 5
local upper_right_panel_info_size = upper_right_panel_info_width + upper_right_panel_offset
local upper_right_panel_width = upper_right_panel_info_size * upper_right_panel_info_count
local function world_map_upper_right_panel(nk_ctx, game_ctx, _) -- timer
  local char = game_ctx.player_character
  local fbw, fbh = input.get_framebuffer_size()
  local panel_startx = fbw-upper_right_panel_width
  if nk.window_begin(nk_ctx, "upper_right_panel", {panel_startx, 0, upper_right_panel_width, upper_right_panel_info_width}, world_map_upper_panel_window_flags) then
    -- тут будут несколько картинок с данными: деньги, авторитет, уважение, влияние, величина личных владений
    local bounds = nk.window_get_content_region(nk_ctx)
    nk.layout_space_begin(nk_ctx, 'static', bounds[4], upper_right_panel_info_count*2)
    for i = 1, upper_right_panel_info_count do
      nk.layout_space_push(nk_ctx, {(i-1)*upper_right_panel_info_size, 0, upper_right_panel_info_width, upper_right_panel_info_width-10})
      if nk.button(nk_ctx, {0.7, 0.7, 0.0, 1.0}) then print("pressed " .. i .. " button") end
      nk.layout_space_push(nk_ctx,
        {
          (i-1)*upper_right_panel_info_size,
          upper_right_panel_info_width/2,
          upper_right_panel_info_width,
          upper_right_panel_info_width/2
        })
      local value = char:get_current_stat(right_panel_array[i])
      nk.label(nk_ctx, tostring(value), nk.TEXT_ALIGN_CENTERED, {0.0, 0.0, 0.0, 1.0})
    end
    nk.layout_space_end(nk_ctx)
  end
  nk.window_end(nk_ctx)
end

local next_turn_sizex, next_turn_sizey = 100, 100
local function world_map_bottom_right_panel(nk_ctx, game_ctx, _)
  local fbw, fbh = input.get_framebuffer_size()
  local panel_startx, panel_starty = fbw-next_turn_sizex, fbh-next_turn_sizey
  local font_height = nk_ctx:font():height()
  if nk.window_begin(nk_ctx, "bottom_right_panel", {panel_startx, panel_starty, next_turn_sizex, next_turn_sizey}, world_map_upper_panel_window_flags) then
    local bounds = nk.window_get_content_region(nk_ctx)
    nk.layout_space_begin(nk_ctx, 'static', bounds[4], 1)
    local starty = (bounds[4] - font_height) / 2.0
    nk.layout_space_push(nk_ctx, {0, starty, next_turn_sizex, font_height})
    nk.label(nk_ctx, "Next turn", nk.TEXT_ALIGN_CENTERED)
    nk.layout_space_end(nk_ctx)
  end
  nk.window_end(nk_ctx)
end

local function world_map_state_function(nk_ctx, game_ctx, timer, local_table)
  -- по идее тут вызываем функцию интерфейса игрока, то есть интерфейс персонажа
  -- в интерфейсе игрока придется еще делать переключение хода
  -- а для этого нужно сделать еще одно состояние
  -- прежде чем начать новую игру, нам нужно будет дать игроку выбрать персонажа
  -- например с помощью той информации после генерации
  -- выбор персонажа - это по идее еще одно игровое состояние
  -- хотя может быть и нет, если персонаж нил, то это хорошая идея его выбрать

  -- а тут нужно вызывать меню по кнопке
  --keys_input()
  --mouse_input(timer.current_time)

  -- требуется еще добавить иконки хоть какие нибудь
  --nk.image(ctx, icon_123)
  -- нужно придумать окно персонажа, будем ориентироваться на цк2

  local empty = menu.core_menu(nk_ctx, game_ctx, timer, local_table)

  if empty then
    left_world_map_window(nk_ctx)
    world_map_upper_panel(nk_ctx, game_ctx, timer)
    -- нужно еще сделать правую верхнюю панель, миникарту (кстати как?), нижнюю правую часть
    -- (кнопку с ходом), справа снизу видимо будут игровые сообщения
    -- (ну то есть практически полная калька с цк2)
    world_map_upper_right_panel(nk_ctx, game_ctx, timer)
    world_map_bottom_right_panel(nk_ctx, game_ctx, timer)

    if escape_clicked then menu.init_menu(false); escape_clicked = false end
  end
end

local function main_menu_loading_state_function(_, _, _, _) -- nk_ctx, game_ctx, timer, local_table
  menu.clear()
end

local function loading_state_function(nk_ctx, _, timer, local_table) -- game_ctx
  menu.clear()
  -- нужно еще рисовать бэкграунд
  assert(local_table.loading_table ~= nil)
  progress_bar(nk_ctx, timer, local_table.loading_table)
end

-- local function battle_map_state_function(nk_ctx, game_ctx, timer, local_table)
--  -- интерфейс битвы
-- end
--
-- local function encounter_state_function(nk_ctx, game_ctx, timer, local_table)
--  -- интерфейс геройской битвы
-- end

local interface_hovered = false

local state_functions_table = {
  [game.state.main_menu_loading]           = main_menu_loading_state_function,
  [game.state.main_menu]                   = main_menu_state_function,
  [game.state.world_map_generator_loading] = loading_state_function,
  [game.state.world_map_generator]         = world_map_generator_state_function,
  [game.state.world_map_generating]        = loading_state_function,
  [game.state.world_map_loading]           = loading_state_function,
  [game.state.world_map]                   = world_map_state_function,
  -- [game.state.battle_map_loading] = true,
  -- [game.state.battle_map] = battle_map_state_function,
  -- [game.state.encounter_loading] = true,
  -- [game.state.encounter] = encounter_state_function
}

local function interface_func(nk_ctx, game_ctx, timer, local_table)
  local frame_time = timer:current_time()

  -- interface.is_hovered - check is any window is hovered EXCEPT arg (window name)
  interface_hovered = interface.is_hovered(tooltip_name) and interface.is_hovered(screen_selection_box_name)
  escape_clicked = check_event_click(escape)
  local character = mouse_movement(nk_ctx, game_ctx, frame_time, interface_hovered, screen_selection_box_name)
  key_input(game_ctx, timer)

  if character ~= nil then currently_oppened_world_map_window = character end

  assert(#state_functions_table == 7)
  local state_f = state_functions_table[game_ctx.state]
  assert(type(state_f) == "function")
  state_f(nk_ctx, game_ctx, timer, local_table)

  if core.is_tile_index_valid(game_ctx.tile_index) and not interface_hovered then
    core.highlight_tile(game_ctx.tile_index, utils.make_color(0.9, 0.9, 0.0, 0.5))
    tooltip_func(nk_ctx, game_ctx, timer, local_table, tooltip_name)
  end
end

return interface_func
