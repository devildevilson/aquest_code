--local nk = require("moonnuklear") -- это зарегестрированный Си модуль
--local fmt = require("fmt")        -- это зарегестрированный Си модуль
--local queue = require("apates_quest.scripts.queue")
local menu = require("apates_quest.scripts.interface.main_menu")
-- было бы неплохо чистить как то эти функции, так как это require
-- они хранятся во внутренней таблице, для того чтобы их почистить
-- из этой таблице, нужно наверное завести еще одну функцию
-- которая тупо пройдется по модулям и удалит их из таблицы
local progress_bar = require("apates_quest.scripts.interface.generator_progress")
local gen_func1 = require("apates_quest.scripts.interface.gen_part1")
local gen_func2 = require("apates_quest.scripts.interface.gen_part2")
local gen_func3 = require("apates_quest.scripts.interface.gen_part3")

-- для того чтобы почистить модуль можно использовать что то вроде
--utils.clear_module("apates_quest.scripts.interface.generator_progress")
-- должна ли эта функция вызывать чистку мусора?

local generator_interfaces = {gen_func1, gen_func2, gen_func3}

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
  if mg.is_finished() then return end

  assert(mg ~= nil)
  assert(generator_interfaces[mg.step()] ~= nil)
  generator_interfaces[mg.step()](nk_ctx, local_table)
  -- текущая логика такая: после генерации на один кадр мы попадаем в это состояние
  -- здесь степ может стать больше 3, нужно либо переключать стейты еще при генерации,
  -- либо добавить возможность определить какой стейт последний
  -- вообще после любого шага генерации у нас могут появиться данные после генерации
  -- поэтому возможно даже удобно что мы всегда работаем с этими данными в этом стейте (?)
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
end

-- local function battle_map_state_function(nk_ctx, game_ctx, timer, local_table)
--  -- интерфейс битвы
-- end
--
-- local function encounter_state_function(nk_ctx, game_ctx, timer, local_table)
--  -- интерфейс геройской битвы
-- end

local state_functions_table = {
  [game.state.main_menu] = main_menu_state_function,
  [game.state.world_map_generator] = world_map_generator_state_function,
  [game.state.world_map] = world_map_state_function,
  -- [game.state.battle_map] = battle_map_state_function,
  -- [game.state.encounter] = encounter_state_function,
}

local function interface(nk_ctx, game_ctx, timer, local_table)
  if game_ctx:is_loading() or game_ctx.state == game.state.world_map_generating then
    menu.clear()
    -- нужно еще рисовать бэкграунд
    if game_ctx.state ~= game.state.main_menu_loading then
      assert(local_table.loading_table ~= nil)
      progress_bar(nk_ctx, timer, local_table.loading_table)
    end

    return
  end

  local state_f = state_functions_table[game_ctx.state]
  assert(state_f ~= nil)
  state_f(nk_ctx, game_ctx, timer, local_table)
end

return interface
