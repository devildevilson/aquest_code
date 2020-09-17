local nk = require("moonnuklear")

local generator_name <const> = "Tectonic plates generator"
local window_flags <const> = nk.WINDOW_BORDER | nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_BACKGROUND
local plates_count_prop <const> = {
  min = 40,
  default_val = 199,
  max = 300,
  step = 1,
  var_name = "Plates count"
}

local ocean_percentage_prop <const> = {
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

function gen_part1_fun(ctx, table)
  if table["plates_count"] == nil then
    table["plates_count"] = plates_count_prop.default_val
    table["ocean_percentage"] = ocean_percentage_prop.default_val
  end

  local ret_value = 0
  local states_flag

  if nk.window_begin(ctx, "plates_gen", {5, 5, 400, 400}, window_flags) then
    nk.layout_row_static(ctx, 30.0, 400, 1)
    nk.label(ctx, generator_name, nk.TEXT_ALIGN_LEFT)

    nk.layout_row(ctx, nk.DYNAMIC, 30.0, {0.6, 0.4})
    --current_seed_buf, states_flag = nk.edit_string(ctx, nk.EDIT_FIELD, current_seed_buf, 11, decimal_filter)
    current_seed_buf, states_flag = nk.edit_string(ctx, nk.EDIT_FIELD, current_seed_buf, 11, nk.filter_decimal)
    if nk.button(ctx, nil, "Randomize") then
      local val = generator.get_random_int()
      current_seed = val
      current_seed_buf = tostring(current_seed)
    end

    -- переменные
    nk.layout_row_dynamic(ctx, 30.0, 1)
    gen_property(ctx, table, "plates_count", plates_count_prop, 400)
    gen_property(ctx, table, "ocean_percentage", ocean_percentage_prop, 400)

    nk.layout_row_static(ctx, 30.0, 199, 2)
    if nk.button(ctx, nil, "Back") then ret_value = -1 end
    if nk.button(ctx, nil, "Generate") then ret_value = 1 end
  end
  nk.window_end(ctx)

  current_seed = tonumber(current_seed_buf)
  if current_seed ~= nil and current_seed > 4294967296 then
    current_seed = 4294967296 -- maximum seed is UINT32_MAX (2^32)
    current_seed_buf = tostring(current_seed)
  end

  if ret_value == 0 then return 0 end
  generator.setup_random_seed(current_seed == nil and 1 or current_seed)
  generator.setup_noise_seed(current_seed == nil and 1 or current_seed)
  current_seed_buf = "1"
  current_seed = 1
  return ret_value
end
