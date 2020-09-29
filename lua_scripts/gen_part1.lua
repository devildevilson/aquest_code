local nk = require("moonnuklear")

local generator_name = "Tectonic plates generator"
local window_flags = nk.WINDOW_BORDER | nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_BACKGROUND
local plates_count_prop = {
  min = 40,
  default_val = 200,
  max = 300,
  step = 1,
  var_name = "Plates count"
}

local plates_connection_limit_prop = {
  min = plates_count_prop.min/2,
  default_val = plates_count_prop.default_val/2,
  max = plates_count_prop.max,
  step = 1,
  var_name = "Plates connection limit"
}

local plates_connection_iteration_prop = {
  min = 2,
  default_val = 5,
  max = 15,
  step = 1,
  var_name = "Plates connection iteration"
}

local ocean_percentage_prop = {
  min = 0.0,
  default_val = 0.7,
  max = 1.0,
  step = 0.01,
  var_name = "Ocean ratio"
}

local current_seed_buf = "1"
local current_seed = 1

local function gen_property(ctx, table, var_name, prop_table, window_width)
  local pixel_step = (prop_table.max - prop_table.min) / window_width
  table[var_name] = nk.property(ctx, prop_table.var_name, prop_table.min, table[var_name], prop_table.max, prop_table.step, pixel_step)
end

local function decimal_filter(edit, codepoint)
  -- print(type(edit))
  -- print(edit)
  -- print(codepoint)
  if (codepoint < utf8.codepoint('0') or codepoint > utf8.codepoint('9')) and codepoint ~= utf8.codepoint('-') then return false end
  return true
end

local function isempty(s)
  return s == nil or s == ''
end

local world_name_str = ''
local world_name_str_prev = ''
local world_folder_str = ''

local code_empty_string = 0
local code_bad_string = 1
local code_world_exists = 2
local code_world_does_not_exist = 3

function gen_part1_fun(ctx, table)
  if table["plates_count"] == nil then
    table["plates_count"] = plates_count_prop.default_val
    table["plates_connection_limit"] = plates_connection_limit_prop.default_val
    table["plates_connection_iteration"] = plates_connection_iteration_prop.default_val
    table["ocean_percentage"] = ocean_percentage_prop.default_val
  end

  local ret_value = 0
  local states_flag

  local valid_name = false
  local valid_folder = false

  if nk.window_begin(ctx, "plates_gen", {5, 5, 400, 400}, window_flags) then
    nk.layout_row_dynamic(ctx, 30.0, 1)
    nk.label(ctx, generator_name, nk.TEXT_ALIGN_LEFT)

    nk.layout_row(ctx, nk.DYNAMIC, 30.0, {0.6, 0.4})
    --current_seed_buf, states_flag = nk.edit_string(ctx, nk.EDIT_FIELD, current_seed_buf, 11, decimal_filter)
    current_seed_buf, states_flag = nk.edit_string(ctx, nk.EDIT_FIELD, current_seed_buf, 11, nk.filter_decimal)
    if nk.button(ctx, nil, "Randomize") then
      local val = generator.get_random_int()
      current_seed = val
      current_seed_buf = tostring(current_seed)
    end

    nk.layout_row(ctx, nk.DYNAMIC, 30.0, {0.9, 0.1})
    world_name_str_prev, states_flag = nk.edit_string(ctx, nk.EDIT_FIELD, world_name_str_prev, 50)
    local bounds1 = nk.widget_bounds(ctx)
    valid_name = not isempty(world_name_str)
    if not valid_name then
      nk.label(ctx, "[!]", nk.TEXT_ALIGN_RIGHT, {1,0,0,1})
      if ctx:is_mouse_hovering_rect(bounds1) then
        nk.tooltip(ctx, "Please provide a world name")
      end
    else
      nk.spacing(ctx, 1)
    end

    world_folder_str, states_flag = nk.edit_string(ctx, nk.EDIT_FIELD, world_folder_str, 50)
    local bounds2 = nk.widget_bounds(ctx)

    if world_name_str_prev ~= world_name_str then
      world_name_str = world_name_str_prev
      -- здесь пытаемся собрать строку из входной так чтобы она имела только английские символы
      -- + '_' + '.' (наверное допустимы еще какие то символы но лучше так)
      -- делаем это если первый символ совпадает у строк
      if isempty(world_folder_str) or world_folder_str:sub(0, 0) == world_name_str:sub(0, 0) then
        world_folder_str = ''
        for c in world_name_str:gmatch(".") do
          --print(string.match(c, "[a-zA-Z0-9_.]")) -- если ни один символ не попадает в выражение, то возвращает nil
          if c == ' ' then world_folder_str = world_folder_str .. '_'
          elseif string.match(c, "[a-zA-Z0-9_.]") then world_folder_str = world_folder_str .. c:lower() end
        end
      end
    end

    valid_folder = not isempty(world_folder_str) and string.match(world_folder_str, "[a-zA-Z0-9_.]+") == world_folder_str -- нужно проверить с иходной сторокой
    local existance = not isempty(world_folder_str) and generator.check_world_existance(world_folder_str) == code_world_exists

    if not valid_folder or existance then
      nk.label(ctx, "[!]", nk.TEXT_ALIGN_RIGHT, {1,0,0,1})
      if ctx:is_mouse_hovering_rect(bounds2) then
        if existance then
          nk.tooltip(ctx, "Another world exists in folder " .. world_folder_str)
        else nk.tooltip(ctx, "Please provide a folder name. Folder name must match [a-zA-Z0-9_.]") end
      end
    else
      --nk.spacing(ctx, 1)
    end

    -- переменные
    nk.layout_row_dynamic(ctx, 30.0, 1)
    local bounds4 = nk.widget_bounds(ctx)
    gen_property(ctx, table, "plates_count", plates_count_prop, bounds4[3])
    gen_property(ctx, table, "plates_connection_limit", plates_connection_limit_prop, bounds4[3])
    gen_property(ctx, table, "plates_connection_iteration", plates_connection_iteration_prop, bounds4[3])
    gen_property(ctx, table, "ocean_percentage", ocean_percentage_prop, bounds4[3])

    nk.layout_row_dynamic(ctx, 30.0, 2)
    if nk.button(ctx, nil, "Back") then ret_value = -1 end
    if nk.button(ctx, nil, "Generate") then ret_value = 1 end
  end
  nk.window_end(ctx)

  current_seed = tonumber(current_seed_buf)
  if current_seed ~= nil and current_seed > 4294967296 then
    current_seed = 4294967296 -- maximum seed is UINT32_MAX (2^32)
    current_seed_buf = tostring(current_seed)
  end

  if ret_value < 0 then return -1 end
  if ret_value == 0 or not valid_name or not valid_folder then return 0 end
  generator.setup_random_seed(current_seed == nil and 1 or current_seed)
  generator.setup_noise_seed(current_seed == nil and 1 or current_seed)
  generator.set_world_name(world_name_str)    -- пустая строка == вылет
  generator.set_folder_name(world_folder_str)
  current_seed_buf = "1"
  current_seed = 1
  return ret_value
end
