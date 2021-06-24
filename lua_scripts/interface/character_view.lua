local nk = require("moonnuklear")

local character_stats_array = {
  "Military:", core.character_stats.military,
  "Managment:", core.character_stats.managment,
  "Diplomacy:", core.character_stats.diplomacy,
  "Strength:", core.character_stats.strength,
  "Agility:", core.character_stats.agility,
  "Intellect:", core.character_stats.intellect
}

-- было бы неплохо здесь запомнить какие нибудь вещи, чтобы не перевычислять их по 500 раз
-- но для этого нужно как то почистить старые данные при выходе

local function character_panel(ctx, character, window_bounds)
  -- на данном этапе (2021.05.13) достаточно сделать только окно персонажа, окно фракции, окно дипломатии
  -- эти окна должны во первых снабжены иконками и другими картинками, а во вторых позволять легкие
  -- переходы между персонажами, у персонажей должны появиться имена и другие строки (например тип титула)
  -- во фракции должна располагатсья экономика и политика государства (строй, сюзерен, вассалы, законы, совет?)
  -- в дипломатии - список всех государств (или по крайней мере соседи) и наши с ними отношения, быстрая возможность
  -- заключить какие нибудь союзы или сделать какую нибудь пакость внешним государствам
  -- бракосочетание наверное отдельно либо с вассалами либо с внешними государствами
  -- примерно эти 3 вещи я должен увидеть в этих окнах

  --local mouse_x, mouse_y = input.get_cursor_pos()

  local char_name = "placeholder_char_name"
  local title_str = "Titles:"
  local claims_str = "Claims:"
  local font = ctx:font()
  local font_height = font:height()

  local char_name_width = font:width(font_height, char_name)
  local title_str_width = font:width(font_height, title_str)
  local claims_str_width = font:width(font_height, claims_str)
  local max_str_width = math.max(title_str_width, claims_str_width) + 5

  local title_size = 50
  local title_claim_place_size = title_size + 5 + title_size + 5
  local char_image_offset = 10

  local hovering_title = nil
  local faction = character.faction
  local titles_count = 0
  core.each_title(faction, function(_) titles_count = titles_count + 1 end)

  -- local main_title = faction.main_title
  -- if main_title.type == core.title_types.king then
  --   local locale_type = localization.get("title.type")
  -- end
  -- local char_name = localization.get(character.name_table_id)[character.name_index]
  -- local full_character_name = utils.compose_string(" ", "King", {character.name_table_id, character.name_index})

  --nk.style_set_float(nk_ctx, "window.group_border", 0.0)

  local ratio1 = char_name_width/window_bounds[3]
  local ratio2 = 0.6
  if nk.window_begin(ctx, "character_panel", window_bounds, nk.WINDOW_NO_SCROLLBAR) then
    -- сверху имя персонажа и возраст, ниже две панельки в одной портрет, в другой культура, религия, статы
    -- ниже портрета титулы и претензии, ниже статов модификаторы и трейты, у этих панелек неплохое обрамление
    -- ниже этой части, семья, вассалы, двор, союзники и проч, каждый в своей "страничке"
    -- какие у меня будут явные отличия от цк2? вассалы переместятся в окно фракции, союзники в окно дипломатии
    -- добавится специальное окно героя, там партия, шмотки и проч
    nk.layout_row(ctx, nk.DYNAMIC, 30.0, {ratio1, 1.0-ratio1})
    nk.label(ctx, char_name, nk.TEXT_ALIGN_LEFT)
    nk.label(ctx, "99", nk.TEXT_ALIGN_RIGHT)
    nk.layout_space_begin(ctx, nk.STATIC, window_bounds[4]/2.0, 5)
    nk.layout_space_push(ctx, {0, 0, window_bounds[3]*ratio2-5, window_bounds[4]/2.0})
    -- получаем эквивалетные значения что и на 2 строчки ниже?
    -- не совсем, group_begin добавляет прозрачную границу, нужно ли с ней както бороться?
    -- было бы неплохо уметь ее выключать
    --local bounds = nk.widget_bounds(nk_ctx)
    if nk.group_begin(ctx, "group_char_portrait_titles", nk.WINDOW_NO_SCROLLBAR) then
      local bounds = nk.window_get_content_region(ctx)
      nk.layout_space_begin(ctx, nk.STATIC, bounds[4], 5)
      nk.layout_space_push(ctx, {0, 0, bounds[3], bounds[4]-(title_claim_place_size+char_image_offset)})
      nk.button(ctx, {0.0, 0.0, 0.7, 1.0})
      nk.layout_space_push(ctx, {0, bounds[4]-title_claim_place_size, bounds[3], title_claim_place_size})
      if nk.group_begin(ctx, "group_char_titles", nk.WINDOW_NO_SCROLLBAR) then
        local local_bounds = nk.window_get_content_region(ctx)
        --local ratio_t = max_str_width/local_bounds[3]
        --local halfy = local_bounds[4] / 2.0 - 5
        local text_start = title_size/2.0 - font_height/2.0
        local titles_place_size = local_bounds[3] - max_str_width
        local title_offset = math.min((titles_place_size-title_size) / (titles_count-1), title_size+5)
        nk.layout_space_begin(ctx, nk.STATIC, local_bounds[4], 99)
        nk.layout_space_push(ctx, {0, text_start, max_str_width, font_height})
        nk.label(ctx, title_str, nk.TEXT_ALIGN_LEFT)
        local title_counter = 0
        core.each_title(faction, function(title)
          nk.layout_space_push(ctx, {max_str_width+title_counter*title_offset, 0, title_size, title_size})
          local title_bounds = nk.widget_bounds(ctx)
          interface.heraldy_image(ctx, title)
          --if interface.heraldy_button(ctx, title) then print("pressed " .. title.id) end -- не работает (кнопка постоянно выдает 1)
          if ctx:is_mouse_hovering_rect(title_bounds) then hovering_title = title end
          title_counter = title_counter + 1
        end)

        nk.layout_space_push(ctx, {0, title_size+5+text_start, max_str_width, font_height})
        nk.label(ctx, claims_str, nk.TEXT_ALIGN_LEFT)
        nk.layout_space_push(ctx, {max_str_width, title_size+5, title_size, title_size})
        nk.button(ctx, {0.7, 0.0, 0.0, 1.0})

        nk.layout_space_end(ctx)

        if hovering_title ~= nil then nk.tooltip(ctx, hovering_title.id) end

        nk.group_end(ctx)
      end
      nk.layout_space_end(ctx)

      nk.group_end(ctx)
    end
    nk.layout_space_push(ctx, {window_bounds[3]*ratio2+5, 0, window_bounds[3]*(1.0-ratio2)-30, window_bounds[4]/2.0})
    if nk.group_begin(ctx, "group_char_stats", nk.WINDOW_NO_SCROLLBAR) then
      nk.layout_row(ctx, nk.DYNAMIC, font_height, {0.5, 0.5})
      nk.label(ctx, "Dynasty:", nk.TEXT_ALIGN_LEFT)
      nk.label(ctx, "testing", nk.TEXT_ALIGN_RIGHT)
      nk.label(ctx, "Religion:", nk.TEXT_ALIGN_LEFT)
      nk.label(ctx, "testing", nk.TEXT_ALIGN_RIGHT)
      nk.label(ctx, "Culture:", nk.TEXT_ALIGN_LEFT)
      nk.label(ctx, "testing", nk.TEXT_ALIGN_RIGHT)
      for i = 1, #character_stats_array, 2 do
        nk.label(ctx, character_stats_array[i], nk.TEXT_ALIGN_LEFT)
        nk.label(ctx, character:get_current_stat(character_stats_array[i+1]), nk.TEXT_ALIGN_RIGHT)
      end

      nk.group_end(ctx)
    end
    nk.layout_space_end(ctx)
  end
  nk.window_end(ctx)
end

return character_panel
