local nk = require("moonnuklear")

local generator_name = "Biomes generator"
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

local function gen_property(ctx, table, var_name, prop_table, window_width)
  local pixel_step = (prop_table.max - prop_table.min) / window_width
  table[var_name] = nk.property(ctx, prop_table.var_name, prop_table.min, table[var_name], prop_table.max, prop_table.step, pixel_step)
end

function gen_part2_fun(ctx, table)
  if table["noise_multiplier"] == nil then
    table["noise_multiplier"] = noise_multiplier_prop.default_val
    table["blur_ratio"] = blur_ratio_prop.default_val
    table["blur_water_ratio"] = blur_water_ratio_prop.default_val
    table["blur_iterations_count"] = blur_iterations_count_prop.default_val
  end

  local ret_value = 0

  if nk.window_begin(ctx, "biomes_generator", {5, 5, 400, 400}, window_flags) then
    nk.layout_row_static(ctx, 30.0, 400, 1)
    nk.label(ctx, generator_name, nk.TEXT_ALIGN_LEFT)

    nk.layout_row_dynamic(ctx, 30.0, 1)
    gen_property(ctx, table, "noise_multiplier", noise_multiplier_prop, 400)
    gen_property(ctx, table, "blur_ratio", blur_ratio_prop, 400)
    gen_property(ctx, table, "blur_water_ratio", blur_water_ratio_prop, 400)
    gen_property(ctx, table, "blur_iterations_count", blur_iterations_count_prop, 400)

    nk.layout_row_dynamic(ctx, 30.0, 2)
    if nk.button(ctx, nil, "Back") then ret_value = -1 end
    if nk.button(ctx, nil, "Generate") then ret_value = 1 end
  end
  nk.window_end(ctx)

  return ret_value
end
