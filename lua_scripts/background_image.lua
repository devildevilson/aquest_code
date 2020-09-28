local nk = require("moonnuklear")

local window_flags <const> = nk.WINDOW_BACKGROUND

function background_drawer(ctx)
  local fbw, fbh <const> = input.get_framebuffer_size()
  local image = utils.background_image() -- эта функция наверное может возвращать nil, либо она должна возвращать nk_image
  if nk.window_begin(ctx, "background", {0, 0, fbw, fbh}, window_flags) then
    nk.layout_row(ctx, nk.DYNAMIC, fbh, {1.0})
    nk.image(ctx, image)
  end
  nk.window_end(ctx)
end

-- собственно здесь можно проверить как работает имейдж контейнер
