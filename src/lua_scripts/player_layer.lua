local nk = require("moonnuklear") -- нужно попробовать убрать из луа require используя sol state require_file

local choose = input.get_event("menu_choose")

local function check_event_click(event)
  return input.check_event(event, input.state_click | input.state_double_click | input.state_long_click)
end

--local window_flags = nk.WINDOW_NO_SCROLLBAR | nk.WINDOW_BACKGROUND

local layer_windows = {
  "resources",
  "next_turn"
}

function main_interface_layer(ctx, interface, data)
  local fbw, fbh = input.get_framebuffer_size()
  local wcsx, wcsy = input.get_window_content_scale();
  local mcsx, mcsy = input.get_monitor_content_scale();
  local psx, psy = input.get_monitor_physical_size();

  local current_player_turn = game.current_player_turn()

  local sx, sy = 300, 30
  if nk.window_begin(ctx, "resources_layer", {fbw-sx, 0, sx, sy}, nk.WINDOW_NO_SCROLLBAR|nk.WINDOW_BORDER) then
    nk.layout_space_begin(ctx, nk.STATIC, sy, 10);
    nk.layout_space_push(ctx, {5, 5, (sx-40)/4, sy-10}) -- наверное тут можно использовать nk dynamic
    nk.label(ctx, "1234", nk.TEXT_ALIGN_CENTERED, current_player_turn and {255, 0, 0, 255} or {180, 180, 180, 255})
    local bounds1 = nk.widget_bounds(ctx)
    if ctx:is_mouse_hovering_rect(bounds1) then
      nk.tooltip(ctx, "money")
    end

    nk.layout_space_push(ctx, {5+((sx-40)/4+10)*1, 5, (sx-40)/4, sy-10})
    nk.label(ctx, "4321", nk.TEXT_ALIGN_CENTERED, current_player_turn and {255, 0, 0, 255} or {180, 180, 180, 255})
    local bounds2 = nk.widget_bounds(ctx)
    if ctx:is_mouse_hovering_rect(bounds2) then
      nk.tooltip(ctx, "authority")
    end

    nk.layout_space_push(ctx, {5+((sx-40)/4+10)*2, 5, (sx-40)/4, sy-10})
    nk.label(ctx, "4321", nk.TEXT_ALIGN_CENTERED, current_player_turn and {255, 0, 0, 255} or {180, 180, 180, 255})
    local bounds3 = nk.widget_bounds(ctx)
    if ctx:is_mouse_hovering_rect(bounds3) then
      nk.tooltip(ctx, "esteem")
    end

    nk.layout_space_push(ctx, {5+((sx-40)/4+10)*3, 5, (sx-40)/4, sy-10})
    nk.label(ctx, "4321", nk.TEXT_ALIGN_CENTERED, current_player_turn and {255, 0, 0, 255} or {180, 180, 180, 255})
    local bounds4 = nk.widget_bounds(ctx)
    if ctx:is_mouse_hovering_rect(bounds4) then
      nk.tooltip(ctx, "infuence")
    end

    nk.layout_space_end(ctx)
  end
  nk.window_end(ctx)

  -- мне нужно проверять есть ли у меня фокус на каком либо окне, чтобы обрабатывать непосредственно взаимодействие с игрой
  -- поэтому я не могу сделать слой интерфейса поверх всего окна, мне нужно делать множество мелких окон
  local sx2, sy2 = 100, 100
  if nk.window_begin(ctx, "next_turn_layer", {fbw-sx2, fbh-sy2, sx2, sy2}, nk.WINDOW_NO_SCROLLBAR|nk.WINDOW_BORDER) then
    nk.layout_space_begin(ctx, nk.STATIC, sy2, 10);
    -- нам нужно нарисовать юзер интерфейс непосредственно игры
    -- как он выглядит? во первых я подозреваю что он должен скейлиться от размеров окна
    -- в этом случае нам не помешает физический размер окна в дюймах
    -- в glfw есть текущий контент скейл - это отношение текущего DPI на дефолтное
    -- если сделать UI с этим скейлом то он будет смотреться на всех остальных машинах нормально
    -- но я сомневаюсь что это то с чем у меня будут проблемы, скорее мне пригодится размер в миллиметрах
    -- чтобы определить на каком устройстве мы запускаем
    -- в зависмости от устройства можно будет менять тип интерфейса, можно сделать константой

    nk.layout_space_push(ctx, {0, 0, sx2, sy2})
    local bounds = nk.widget_bounds(ctx)
    local hovering = ctx:is_mouse_hovering_rect(bounds)
    if hovering then
      nk.tooltip(ctx, "next turn")
    end
    if current_player_turn and ctx:is_mouse_click_in_rect(nk.BUTTON_LEFT, bounds) then
      game.player_end_turn()
      print("advance turn")
    end

    local color = current_player_turn and {255, 0, 0, 255} or {180, 180, 180, 255}
    color = (hovering and not current_player_turn) and {255, 255, 0, 255} or color

    nk.layout_space_push(ctx, {0, sy2/2-5, sx2, sy2-(sy2/2-5)})
    nk.label(ctx, "Next turn", nk.TEXT_ALIGN_CENTERED, color)

    nk.layout_space_end(ctx)
  end
  nk.window_end(ctx)

  return false
end
