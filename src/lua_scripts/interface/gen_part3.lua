local nk = require("moonnuklear")

local window_flags = nk.WINDOW_BORDER | nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_BACKGROUND
local provinces_count_prop = {
  min = 1000,
  default_val = 4000,
  max = 5000,
  step = 50,
  var_name = "Province count"
}

local history_iterations_count_prop = {
  min = 200,
  default_val = 300,
  max = 1000,
  step = 10,
  var_name = "History iterations count"
}

local function gen_property(ctx, user_table, var_name, prop_table, window_width)
  local pixel_step = (prop_table.max - prop_table.min) / window_width
  user_table[var_name] = nk.property(ctx, prop_table.var_name, prop_table.min, user_table[var_name], prop_table.max, prop_table.step, pixel_step)
end

local user_data = nil
local function user_data_init()
  return {
    provinces_count = provinces_count_prop.default_val,
    history_iterations_count = history_iterations_count_prop.default_val
  }
end

local function gen_part3_fun(ctx, local_table)
  if user_data == nil then user_data = user_data_init() end
  local map_generator = local_table.map_generator

  local ret_value = 0

  if nk.window_begin(ctx, "countries_generator", {5, 5, 400, 400}, window_flags) then
    nk.layout_row_static(ctx, 30.0, 400, 1)
    nk.label(ctx, map_generator.step_name(), nk.TEXT_ALIGN_LEFT)

    nk.layout_row_dynamic(ctx, 30.0, 1)
    gen_property(ctx, user_data, "provinces_count", provinces_count_prop, 400)
    gen_property(ctx, user_data, "history_iterations_count", history_iterations_count_prop, 400)

    nk.layout_row_dynamic(ctx, 30.0, 2)
    if nk.button(ctx, nil, "Back") then ret_value = -1 end
    if nk.button(ctx, nil, "Generate") then ret_value = 1 end
  end
  nk.window_end(ctx)

  if ret_value < 0 then map_generator.prev_step() end
  if ret_value == 0 then return end

  local_table.generator_userdata = user_data
  user_data = nil
  map_generator.advance()
end

return gen_part3_fun
