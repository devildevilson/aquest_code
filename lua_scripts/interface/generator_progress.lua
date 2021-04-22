local nk = require("moonnuklear")

local window_flags = nk.WINDOW_BORDER | nk.WINDOW_NO_SCROLLBAR
--local generator_names = {"Tectonic plates generator", "Biomes generator", "Countries generator"}

local function progress_bar(ctx, _, info) -- timer
  assert(info ~= nil)

  local fbw, fbh = input.get_framebuffer_size()
  local progress_bar_size = 35
  local sizex, sizey = 400, ctx:font():height() * 2 + progress_bar_size + 20
  --local wx, wy = fbw/2.0-sizex/2.0, fbh/2.0-sizey/2.0
  local wx, wy = fbw-sizex-10, fbh-sizey-10
  local current_step = info.current_step
  local current_progress_name = info.hint1
  local hint = info.hint2
  local step_count = info.step_count
  local progress_hint = tostring(current_step) .. "/" .. tostring(step_count)

  --assert(hint ~= nil and hint ~= "")
  --print(hint)

  if nk.window_begin(ctx, "progress_bar", {wx, wy, sizex, sizey}, window_flags) then
    nk.layout_row_dynamic(ctx, ctx:font():height(), 1)
    nk.label(ctx, current_progress_name, nk.TEXT_ALIGN_LEFT);

    local text_width = ctx:font():width(ctx:font():height(), hint)
    local hint2_width = ctx:font():width(ctx:font():height(), "00/00")
    local points_width = ctx:font():width(ctx:font():height(), "...")
    local small_hint = text_width < (sizex-hint2_width)
    local text_ratio = small_hint and text_width/sizex or (sizex-hint2_width-points_width)/sizex
    local ratio = small_hint and {text_ratio, 1.0-text_ratio} or {text_ratio, points_width/sizex, hint2_width/sizex}
    nk.layout_row(ctx, nk.DYNAMIC, ctx:font():height(), ratio)
    nk.label(ctx, hint, nk.TEXT_ALIGN_LEFT)
    if not small_hint then nk.label(ctx, "...", nk.TEXT_ALIGN_LEFT) end
    nk.label(ctx, progress_hint, nk.TEXT_ALIGN_RIGHT)

    nk.layout_row_dynamic(ctx, progress_bar_size, 1)
    nk.progress(ctx, current_step, step_count, nk.FIXED)
  end
  nk.window_end(ctx)
end

return progress_bar
