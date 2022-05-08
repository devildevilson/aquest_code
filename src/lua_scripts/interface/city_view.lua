local nk = require("moonnuklear")

-- character - текущий персонаж, для проверок можем ли мы чего построить
local function city_view(ctx, character, city)
  local title = city.title
  local type = city.type
  local name = localization.get(title.name_id)

  local window_bounds = { 400, 400, 400, 400 }
  if nk.window_begin(ctx, name, window_bounds, nk.WINDOW_NO_SCROLLBAR) then
    -- как выглядит? название панели это название города? иконка города в рамочке слева
    -- рядом описание (оно не должно занимать много места), под ним статы
    -- под статами список построек, как в списке построек огранизовать цепочку потроек?
    -- в цк2 я так понимаю upgrades_from сообщал интерфейсу, что у здания есть следующий уровень

    nk.layout_space_begin(ctx, nk.STATIC, window_bounds[4] - 100, 1)
    nk.layout_space_push(ctx, {5, 100, window_bounds[3]-10, window_bounds[4] - 100})

    if nk.group_begin(ctx, "city_buildings", 0) then
      nk.layout_space_begin(ctx, nk.STATIC, 100, 99)

      -- тут неудобно делать итератор
      for i = 1, type.buildings_count do
        local building = type.buildings[i]
        local compl = city:completed_building(i)
        local avail = city:available_building(i)
        local visib = city:visible_building(i)
        local currently_building = i == city.building_project_index

        if visib and building.upgrades_from == nil then

          if compl then -- здание построенно
            local counter = 1 -- уровень улучшения
            local upgrade = city:find_building_upgrade(building)
            local building_obj = nil
            while upgrade ~= -1 and city:completed_building(upgrade) do
              building_obj = type.buildings[upgrade]
              upgrade = city:find_building_upgrade(building_obj)
              counter = counter + 1
            end

            if upgrade == -1 and building_obj == nil then
              -- улучшений нет
            elseif upgrade == -1 and building_obj ~= nil then
              -- доступно последнее улучшение
            elseif upgrade ~= -1 then
              -- есть непостроенное улучшение
            end
          else -- здание не построенно
            if city:can_build(character, i) then

            end
          end

          local h = font_size + 20
          local w = window_bounds[3]-10
          nk.layout_space_push(ctx, {0, (i-1) * 100, w, h})
          if nk.group_begin(ctx, "building", nk.WINDOW_NO_SCROLLBAR) then
            local offset = 5
            local icon_w = h
            local icon2_x = w/2-h/2
            local icon3_x = w/2+w/4-h/2
            local text1_w = icon2_x - offset - icon_w
            local cost_w = icon3_x - (icon2_x + icon_w)
            local time_w = w - (offset + icon_w + text1_w + icon_w + cost_w + icon_w)
            -- возможно все равно имеет смысл сделать layout_space_begin
            nk.layout_row(ctx, nk.DYNAMIC, h, {offset / w, icon_w / w, text1_w / w, icon_w / w, cost_w / w, icon_w / w, time_w / w})
            nk.space()
            interface.image()
            nk.label(ctx, "name", nk.TEXT_ALIGN_LEFT)
            interface.image()
            nk.label(ctx, "cost", nk.TEXT_ALIGN_RIGHT)
            interface.image()
            nk.label(ctx, "time", nk.TEXT_ALIGN_CENTER)

            nk.group_end(ctx)
          end

        end -- if visib and building.upgrades_from == nil then
      end -- for i = 1, type.buildings_count do
      nk.layout_space_end(ctx)

      nk.group_end(ctx)
    end -- if nk.group_begin(ctx, "city_buildings", 0) then

    nk.layout_space_end(ctx)
  end -- nk.window_begin
  nk.window_end(ctx)
end

return city_view
