local nk = require("moonnuklear")

local window_flags = nk.WINDOW_BORDER | nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_BACKGROUND
local noise_multiplier_prop = {
  min = 0.0,
  default_val = 0.1,
  max = 1.0,
  step = 0.01,
  var_name = "Noise multiplier"
}

local blur_ratio_prop = {
  min = 0.0,
  default_val = 0.7,
  max = 1.0,
  step = 0.01,
  var_name = "Blur ratio"
}

local blur_water_ratio_prop = {
  min = 0.0,
  default_val = 1.0,
  max = 2.0,
  step = 0.01,
  var_name = "Water blur ratio"
}

local blur_iterations_count_prop = {
  min = 0,
  default_val = 2,
  max = 5,
  step = 1,
  var_name = "Blur iterations count"
}

local user_data = nil
local function user_data_init()
  return {
    noise_multiplier = noise_multiplier_prop.default_val,
    blur_ratio = blur_ratio_prop.default_val,
    blur_water_ratio = blur_water_ratio_prop.default_val,
    blur_iterations_count = blur_iterations_count_prop.default_val,
  }
end

local function gen_property(ctx, user_table, var_name, prop_table, window_width)
  local pixel_step = (prop_table.max - prop_table.min) / window_width
  user_table[var_name] = nk.property(ctx, prop_table.var_name, prop_table.min, user_table[var_name], prop_table.max, prop_table.step, pixel_step)
end

local function gen_part2_fun(ctx, local_table)
  if user_data == nil then user_data = user_data_init() end
  local map_generator = local_table.map_generator

  local ret_value = 0

  if nk.window_begin(ctx, "biomes_generator", {5, 5, 400, 400}, window_flags) then
    nk.layout_row_static(ctx, 30.0, 400, 1)
    nk.label(ctx, map_generator.step_name(), nk.TEXT_ALIGN_LEFT)

    nk.layout_row_dynamic(ctx, 30.0, 1)
    gen_property(ctx, user_data, "noise_multiplier", noise_multiplier_prop, 400)
    gen_property(ctx, user_data, "blur_ratio", blur_ratio_prop, 400)
    gen_property(ctx, user_data, "blur_water_ratio", blur_water_ratio_prop, 400)
    gen_property(ctx, user_data, "blur_iterations_count", blur_iterations_count_prop, 400)

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

return gen_part2_fun
