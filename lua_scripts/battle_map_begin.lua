-- по идее в контексте должны быть указаны армии, из них можно получить индексы тайлов
-- по индексам тайлов найдем армии рядом, создадим контейнер,
function reading_map_info(map_ctx, container, table)
  -- контейнер придется создать тут, но не думаю что это проблема
  -- фукции обхода тайлов, получим соседей тайла, тип tile_get_neighbours(tile_index)
  --
end

function begin(ctx, table)
  -- откуда данные которые мы собрали? неплохо было бы чтобы они лежали в таблице
  -- что нибудь вроде table.map_data.something, с другой стороны нам может
  -- потребоваться собрать много данных, что может влететь в копеечку по памяти
  -- много это сколько? наверное 2-3 армии с каждой стороны по сколько отрядов?
  -- 6 армий по 20 отрядов нормальная прикидка? нужно сохранить все аттрибуты
  -- контейнер контекста должен быть доступен и там, тогда получится подсократить
  -- использование памяти,

  local tiles_count = ctx.map.tiles_count
  print(tiles_count)

  for i = 0, tiles_count-1 do
    local h = ctx.random:closed(0.8, 1.2)
    ctx.map:set_tile_height(i, h) -- не так много вещей напрямую с картой взамодействуют
    local b = ctx.random:closed(0, 1)
    ctx.map:set_tile_biome(i, b) -- делать по индексу или как?
  end
end

function make_biomes(ctx, table)
  do
    local image_table = {
      id = "image_upper_tree11",
      path = "apates_quest/textures/biomes/upper_tree.png",
      type = 0,
      sampler = 1
    }
    utils.add_image(image_table)
  end

  do
    local image_table = {
      id = "image_upper_tree12",
      path = "apates_quest/textures/biomes/upper_tree4.png",
      type = 0,
      sampler = 1
    }
    utils.add_image(image_table)
  end

  do
    local image_table = {
      id = "image_upper_tree21",
      path = "apates_quest/textures/biomes/upper_spruce2.png",
      type = 0,
      sampler = 1
    }
    utils.add_image(image_table)
  end

  do
    local image_table = {
      id = "image_upper_tree22",
      path = "apates_quest/textures/biomes/upper_sakura2.png",
      type = 0,
      sampler = 1
    }
    utils.add_image(image_table)
  end

  do
    -- нужно ли какнибудь группировать все эти данные?
    local biomes_table = {
      -- скорее всего нужен будет id, с другой стороны мы возвращаем индекс, но мне кажется
      -- что индексы будут как то плохо работать в условиях модификации генератора, тут опять же
      -- нужно добавлять индексы в соответствующие переменные
      id = "basic_biome",
      texture1_face = "image_upper_tree11", texture1_top = "image_upper_tree12",
      --texture2_face = "image_lua_test", texture2_top = "image_lua_test",
      --texture3_face = "image_lua_test", texture3_top = "image_lua_test",
      -- нужно ли нам учитывать возможность нарисовать ничего? наверное, мы можем это сделать с помощью скейла
      density = 52, probability1 = 12, probability2 = 0, probability3 = 0,
      min_scale1 = 0.1, max_scale1 = 0.3,
      min_scale2 = 0.1, max_scale2 = 0.3,
      min_scale3 = 0.1, max_scale3 = 0.3,
      -- может зум только один? а может это вообще константа? не уверен, нет, кажется что
      -- разные зумы - это верное решение
      max_zoom1 = 0.1, max_zoom2 = 0.1, max_zoom3 = 0.1
      -- тут должны добавится дополнительные данные, например стоимость перехода
    }
    -- наверное можно придумать аккуратную таблицу биомов так чтобы мы могли использовать
    -- индексы вне зависимости от текущей конфигурации, биомов максимум 256
    local biome_index = utils.add_biome(biomes_table)
  end

  do
    -- нужно ли какнибудь группировать все эти данные?
    local biomes_table = {
      -- скорее всего нужен будет id, с другой стороны мы возвращаем индекс, но мне кажется
      -- что индексы будут как то плохо работать в условиях модификации генератора, тут опять же
      -- нужно добавлять индексы в соответствующие переменные
      id = "basic_biome1",
      texture1_face = "image_upper_tree21", texture1_top = "image_upper_tree22",
      --texture2_face = "image_lua_test", texture2_top = "image_lua_test",
      --texture3_face = "image_lua_test", texture3_top = "image_lua_test",
      -- нужно ли нам учитывать возможность нарисовать ничего? наверное, мы можем это сделать с помощью скейла
      density = 34, probability1 = 12, probability2 = 0, probability3 = 0,
      min_scale1 = 0.1, max_scale1 = 0.3,
      min_scale2 = 0.1, max_scale2 = 0.3,
      min_scale3 = 0.1, max_scale3 = 0.3,
      -- может зум только один? а может это вообще константа? не уверен, нет, кажется что
      -- разные зумы - это верное решение
      max_zoom1 = 0.1, max_zoom2 = 0.1, max_zoom3 = 0.1
      -- тут должны добавится дополнительные данные, например стоимость перехода
    }
    -- наверное можно придумать аккуратную таблицу биомов так чтобы мы могли использовать
    -- индексы вне зависимости от текущей конфигурации, биомов максимум 256
    local biome_index = utils.add_biome(biomes_table)
  end

  do
    -- можно добавить также данные частиц + где то указать интенсивность
    local particle_table = {
      id = "part1",

    }
  end

  do
    local state_table = {
      id = "basic_state",
      -- вообще можно произвольное количество текстурок сделать, все что нужно - это правильно посчитать сторону по количеству текстурок
      textures = {"image_lua_test", "image_lua_test", "image_lua_test", "image_lua_test", "image_lua_test"},
      -- если укажем 0 - то выполним в том же кадре, если укажем 1 то выполним в следующим кадре, если укажем < 0 то состояние никогда не закончится (например смерть)
      time = constants.one_second / 3,
      next = "next_state", -- проходит время и мы переключаемся автоматически в следующий стейт, функция необязательна
      -- функция запускается ВНАЧАЛЕ стейта, то бишь нужно создать специальные стейты только с функциями
      func = [[
        return function(unit)
          unit:set_state("random_state")
          local status = unit.status
          if status == core.unit_status.idle then

          end
        end
      ]],
      func = {path = "function_container.lua", name = "func1"} -- либо строкой, либо из файла
    }
    --utils.add_animation_state(state_table)
  end

  -- нужно заполнить биомами тайлы
  -- test test lmdfklsh;lga
end
