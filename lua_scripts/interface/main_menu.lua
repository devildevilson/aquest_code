-- мне нужно тип придумать какойто класс чтобы легко понимать когда рисовать меню и какое
--

local queue = require("apates_quest.scripts.queue")
local menu_table = require("apates_quest.scripts.interface.user")

local menu_stack = queue.new()

-- меню состоит из двух частей: изображение на заднем фоне + непосредтсвенно меню
-- тут мы должны создать стек меню, как заполнить?
local function init_menu(add_core)
  if add_core then menu_stack:push_right(menu_table.core_menu)
  else menu_stack:push_right(menu_table.map_menu) end
end

local function core_menu(nk_ctx, game_ctx, timer, local_table)
  if menu_stack:is_empty() then return true end

  local cur = menu_stack:right()
  cur(nk_ctx, game_ctx, timer, local_table, menu_stack)
  return false
end

local function exist()
  return not menu_stack:is_empty()
end

local function clear()
  menu_stack:clear()
end

return {core_menu = core_menu, init_menu = init_menu, exist = exist, clear = clear}
