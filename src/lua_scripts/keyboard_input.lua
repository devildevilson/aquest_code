local border_render = input.get_event("border_render")
local go_to_capital = input.get_event("home")

--local click_state = input.state_click | input.state_double_click | input.state_long_click

local function key_input(game_ctx, _) -- timer
  input.frame_events(function(event, type)
    if type == input.press then
      if event == border_render then
        core.toggle_border_rendering()
      end

      if event == go_to_capital then
        local p = game_ctx.player_character
        local r = p.realm
        local capital = nil
        core.each_title(r, function(title)
          --print(title.id .. " " .. title.type .. " " .. core.titulus_type.city)
          if title.type == core.titulus_type.city then capital = title:get_city(); return true end
          return false
        end);

        assert(capital ~= nil)

        local tile = core.get_tile(capital.tile_index)
        local centerx, centery, centerz = tile:center()
        camera.set_end_point(centerx, centery, centerz)
      end
    end
  end)
end

return key_input
