local nk = require("moonnuklear")

local generator_name = "Countries generator"
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

local function gen_property(ctx, table, var_name, prop_table, window_width)
  local pixel_step = (prop_table.max - prop_table.min) / window_width
  table[var_name] = nk.property(ctx, prop_table.var_name, prop_table.min, table[var_name], prop_table.max, prop_table.step, pixel_step)
end

function gen_part3_fun(ctx, table)
  if table["provinces_count"] == nil then
    table["provinces_count"] = provinces_count_prop.default_val
    table["history_iterations_count"] = history_iterations_count_prop.default_val
  end

  local ret_value = 0

  if nk.window_begin(ctx, "countries_generator", {5, 5, 400, 400}, window_flags) then
    nk.layout_row_static(ctx, 30.0, 400, 1)
    nk.label(ctx, generator_name, nk.TEXT_ALIGN_LEFT)

    nk.layout_row_dynamic(ctx, 30.0, 1)
    gen_property(ctx, table, "provinces_count", provinces_count_prop, 400)
    gen_property(ctx, table, "history_iterations_count", history_iterations_count_prop, 400)

    nk.layout_row_dynamic(ctx, 30.0, 2)
    if nk.button(ctx, nil, "Back") then ret_value = -1 end
    if nk.button(ctx, nil, "Generate") then ret_value = 1 end
  end
  nk.window_end(ctx)

  return ret_value
end
