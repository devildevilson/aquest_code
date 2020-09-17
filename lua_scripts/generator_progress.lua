local nk = require("moonnuklear")

local window_flags <const> = nk.WINDOW_BORDER | nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_BACKGROUND
local generator_names <const> = {"Tectonic plates generator", "Biomes generator", "Countries generator"}

function gen_progress(ctx, info)
  local fbw, fbh <const> = input.get_framebuffer_size()
  local progress_bar_size <const> = 40
  local sizex, sizey <const> = 400, ctx:font():height() * 2 + progress_bar_size + 20
  local wx, wy <const> = fbw/2.0-sizex/2.0, fbh/2.0-sizey/2.0
  local current_step <const> = info.current_step
  local hint <const> = info.hint
  local step_count <const> = info.step_count
  local current_generator_part <const> = info.current_generator_part
  local progress_hint <const> = tostring(current_step) .. "/" .. tostring(step_count)

  if nk.window_begin(ctx, "progress_bar", {wx, wy, sizex, sizey}, window_flags) then
    nk.layout_row_dynamic(ctx, ctx:font():height(), 1)
    nk.label(ctx, generator_names[current_generator_part+1], nk.TEXT_ALIGN_LEFT);

    local text_width <const> = ctx:font():width(ctx:font():height(), hint)
    local hint2_width <const> = ctx:font():width(ctx:font():height(), "00/00")
    local points_width <const> = ctx:font():width(ctx:font():height(), "...")
    local small_hint <const> = text_width < (sizex-hint2_width)
    local text_ratio <const> = small_hint and text_width/sizex or (sizex-hint2_width-points_width)/sizex
    local ratio <const> = small_hint and {text_ratio, 1.0-text_ratio} or {text_ratio, points_width/sizex, hint2_width/sizex}
    nk.layout_row(ctx, nk.DYNAMIC, ctx:font():height(), ratio)
    nk.label(ctx, hint, nk.TEXT_ALIGN_LEFT)
    if not small_hint then nk.label(ctx, "...", nk.TEXT_ALIGN_LEFT) end
    nk.label(ctx, progress_hint, nk.TEXT_ALIGN_RIGHT)

    nk.layout_row_dynamic(ctx, progress_bar_size, 1)
    nk.progress(ctx, current_step, step_count, nk.FIXED)
  end
  nk.window_end(ctx)

  return false
end
