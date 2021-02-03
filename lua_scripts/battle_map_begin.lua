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

  -- do
  --   -- можно добавить также данные частиц + где то указать интенсивность
  --   local particle_table = {
  --     id = "part1",
  --
  --   }
  -- end
  --
  -- do
  --   local state_table = {
  --     id = "basic_state",
  --     -- вообще можно произвольное количество текстурок сделать, все что нужно - это правильно посчитать сторону по количеству текстурок
  --     textures = {"image_lua_test", "image_lua_test", "image_lua_test", "image_lua_test", "image_lua_test"},
  --     -- если укажем 0 - то выполним в том же кадре, если укажем 1 то выполним в следующим кадре, если укажем < 0 то состояние никогда не закончится (например смерть)
  --     time = constants.one_second / 3,
  --     next = "next_state", -- проходит время и мы переключаемся автоматически в следующий стейт, функция необязательна
  --     -- функция запускается ВНАЧАЛЕ стейта, то бишь нужно создать специальные стейты только с функциями
  --     func = [[
  --       return function(unit)
  --         unit:set_state("random_state")
  --         local status = unit.status
  --         if status == core.unit_status.idle then
  --
  --         end
  --       end
  --     ]],
  --     --func = {path = "function_container.lua", name = "func1"} -- либо строкой, либо из файла
  --   }
  --   --utils.add_animation_state(state_table)
  -- end

  -- нужно заполнить биомами тайлы
  -- нужно дать тайлам оставшиеся текстурки, текстурки к сожалению нужно загружать заранее, поэтому конкретную текстурку я дать не могу
  -- (хотя меня что меня останавливает? написанный код и текущий дизайн) поэтому нужно где то хранить строки с изображениями
  -- где их хранить главный вопрос, но по идее такие штуки нужно делать через интерфейс карты, буду хранить его в отдельном контейнере

  do
    local image_table = {
      id = "biome0_ground",
      path = "apates_quest/textures/biomes/a3ab617780e86740e3c0b4053b760c99.jpg",
      scale = {width = 64, height = 64, filter = 0},
      type = 0,
      sampler = 1
    }
    utils.add_image(image_table)
  end

  do
    local image_table = {
      id = "biome1_ground",
      path = "apates_quest/textures/biomes/fd2e92a2428f1b71b67dcae79efc3122.png",
      scale = {width = 64, height = 64, filter = 0},
      type = 0,
      sampler = 1
    }
    utils.add_image(image_table)
  end

  do
    local image_table = {
      id = "biome0_walls",
      path = "apates_quest/textures/biomes/a3ab617780e86740e3c0b4053b760c99.jpg",
      scale = {width = 64, height = 64, filter = 0},
      type = 0,
      sampler = 1
    }
    utils.add_image(image_table)
  end

  do
    local image_table = {
      id = "biome1_walls",
      path = "apates_quest/textures/biomes/fd2e92a2428f1b71b67dcae79efc3122.png",
      scale = {width = 64, height = 64, filter = 0},
      type = 0,
      sampler = 1
    }
    utils.add_image(image_table)
  end

  local biomes_ground_images = {"biome0_ground", "biome1_ground"}
  local biomes_walls_images = {"biome0_walls", "biome1_walls"}
  -- нужно что то придумать со стенами
  local tiles_count = ctx.map.tiles_count
  for i = 0, tiles_count-1 do
    local biome_index = ctx.map:get_tile_biome(i)
    ctx.map:set_tile_texture_id(i, biomes_ground_images[biome_index+1])
    ctx.map:set_tile_wall_texture_id(i, biomes_walls_images[biome_index+1])
  end

  do
    -- тип отряда, вообще по идее мы его скопируем из предыдущего шага
    local troop_type_table = {
      id = "test_troop_type",
      --unit_type = "test_unit_type",
      default_unit_state = "basic_state",
      stats = {
        troop_size = 10,
        max_hp = 1000,
        morale = 100,
        armor = 10,
        siege_armor = 100,
        speed = 5,
        initiative = 10,
        melee_attack = 10,
        melee_defence = 10,
        range_attack = 10,
        range_defence = 10,
        charge = 30,
        morale_damage = 10,
        melee_damage = 30,
        melee_armor_piercing = 30,
        melee_magic = 0,
        melee_fire = 0,
        melee_siege = 0,
        range_damage = 0,
        range_armor_piercing = 0,
        range_magic = 0,
        range_fire = 0,
        range_siege = 0,
        accuracy = 10,
        reloading = 2,
        ammo = 10,
        maintenance = 4,
        provision = 3,
        recovery = 2
      }
    }
    utils.add_troop_type(troop_type_table)
  end

  do
    local troop_table = {
      type = "test_troop_type",
      -- надо как то задать координату, вообще можно использовать (x,y), но пока что наверное просто индексом
      tile_index = 0,
      current_stats = {
        troop_size = 10,
        max_hp = 500,
        morale = 76,
        armor = 10,
        siege_armor = 100,
        speed = 5,
        initiative = 10,
        melee_attack = 10,
        melee_defence = 10,
        range_attack = 10,
        range_defence = 10,
        charge = 30,
        morale_damage = 10,
        melee_damage = 30,
        melee_armor_piercing = 30,
        melee_magic = 0,
        melee_fire = 0,
        melee_siege = 0,
        range_damage = 0,
        range_armor_piercing = 0,
        range_magic = 0,
        range_fire = 0,
        range_siege = 0,
        accuracy = 10,
        reloading = 2,
        ammo = 10,
        maintenance = 4,
        provision = 3,
        recovery = 2
      }
    }
    utils.add_troop(troop_table)
  end

  -- в таком виде конечно не хочется оставлять тип юнита, зачем плодить лишние таблицы
  do
    local unit_type_table = {
      id = "test_unit_type",
      default_state = "basic_state"
    }
  end

  do
    local unit_table = { -- а она и не нужна

    }
  end

  do
    -- нужны текстурки для состояний
    local image_table1 = {
      id = "unit_test_texture1",
      path = "apates_quest/textures/armies_and_heroes/glad1_256.png",
      type = 0,
      sampler = 1
    }
    utils.add_image(image_table1)

    local image_table2 = {
      id = "unit_test_texture2",
      path = "apates_quest/textures/armies_and_heroes/glad2_256.png",
      type = 0,
      sampler = 1
    }
    utils.add_image(image_table2)
  end

  do
    local state_table = {
      id = "basic_state",
      -- вообще можно произвольное количество текстурок сделать, все что нужно - это правильно посчитать сторону по количеству текстурок
      textures = {"unit_test_texture1"},
      -- если укажем 0 - то выполним в том же кадре, если укажем 1 то выполним в следующим кадре, если укажем < 0 то состояние никогда не закончится (например смерть)
      --time = constants.one_second / 3,
      time = -1,
      --next = "next_state", -- проходит время и мы переключаемся автоматически в следующий стейт, функция необязательна
      -- функция запускается ВНАЧАЛЕ стейта, то бишь нужно создать специальные стейты только с функциями
      func = [[
        return function(unit)
          --unit.user_time
          --unit.state
          --local status = unit.status
          --if status == core.unit_status.idle then
          --end

          local rand_val = unit:random()
          local time_k = unit.state_time * (1 / (constants.one_second / 3))
          if time_k > rand_val then unit:set_state("basic_state2") end
        end
      ]],
      --func = {path = "function_container.lua", name = "func1"} -- либо строкой, либо из файла
    }
    utils.add_unit_state(state_table)
  end

  do
    local state_table = {
      id = "basic_state2",
      -- вообще можно произвольное количество текстурок сделать, все что нужно - это правильно посчитать сторону по количеству текстурок
      textures = {"unit_test_texture2"},
      -- если укажем 0 - то выполним в том же кадре, если укажем 1 то выполним в следующим кадре, если укажем < 0 то состояние никогда не закончится (например смерть)
      --time = constants.one_second / 3,
      time = -1,
      --next = "next_state", -- проходит время и мы переключаемся автоматически в следующий стейт, функция необязательна
      -- функция запускается ВНАЧАЛЕ стейта, то бишь нужно создать специальные стейты только с функциями
      func = [[
        return function(unit)
          --unit.user_time
          --unit.state
          --local status = unit.status
          --if status == core.unit_status.idle then
          --end

          local rand_val = unit:random()
          local time_k = unit.state_time * (1 / (constants.one_second / 3))
          if time_k > rand_val then unit:set_state("basic_state") end
        end
      ]],
      --func = {path = "function_container.lua", name = "func1"} -- либо строкой, либо из файла
    }
    utils.add_unit_state(state_table)
  end
end
