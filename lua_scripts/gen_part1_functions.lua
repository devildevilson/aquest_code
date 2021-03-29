-- первые три функции генерации карты, генерирует плиты и данные для плит
-- в среднем работает за 30 секунд (ужас), не уверен как это будет выглядеть в будущем
-- создание плит в функции generate_plates занимает больше всего времени, как можно ускорить?
-- мне кажется что я еще не доконца оптимизировал СИ составляющую луа
-- работает за ~25 секунд, исправил все баги
-- jit? для jit нужен луа 5.1

local maf = require("apates_quest.scripts.maf")
local types = require("apates_quest.scripts.entities_types_table")

local function bool_to_number(value)
  assert(type(value) == "boolean")
  return value and 1 or 0
end

function setup_generator(ctx, local_table)
  local function_timer = generator.timer_t.new("seting up generator")
  --collectgarbage("collect")

  local uint_t = generator.data_type.uint_t
  local float_t = generator.data_type.float_t
  local int_t = generator.data_type.int_t

  ctx.container:set_tile_template({
    uint_t,  -- color,
    uint_t,  -- plate_index,
    uint_t,  -- edge_index,
    float_t, -- edge_dist,
    uint_t,  -- mountain_index,
    float_t, -- mountain_dist,
    uint_t,  -- ocean_index,
    float_t, -- ocean_dist,
    uint_t,  -- coastline_index,
    float_t, -- coastline_dist,
    uint_t,  -- water_index,
    uint_t,  -- water_dist,
    float_t, -- elevation,
    float_t, -- heat,
    float_t, -- moisture,
    uint_t,  -- biome,
    uint_t,  -- province_index,
    uint_t,  -- culture_id,
    uint_t,  -- country_index
    uint_t,  -- duchie_index
    uint_t,  -- kingship_index
    uint_t   -- empire_index
  })

  do
    local index = ctx.container:set_entity_template({
      float_t, -- drift_axis,
      float_t, -- drift_axis1,
      float_t, -- drift_axis2,
      float_t, -- drift_rate,
      float_t, -- spin_axis,
      float_t, -- spin_axis1,
      float_t, -- spin_axis2,
      float_t, -- spin_rate,
      float_t, -- base_elevation,
      uint_t   -- oceanic
    })

    assert(index == types.entities.plate)
  end

  do
    local index = ctx.container:set_entity_template({
      uint_t,  -- first_tile
      uint_t,  -- second_tile
      float_t, -- plate0_movement,
      float_t, -- plate0_movement1,
      float_t, -- plate0_movement2,
      float_t, -- plate1_movement,
      float_t, -- plate1_movement1,
      float_t  -- plate1_movement2,
    })

    assert(index == types.entities.edge)
  end

  do
    local index = ctx.container:set_entity_template({
      uint_t, -- country_index,
      uint_t  -- title_index
    })

    assert(index == types.entities.province)
  end

  do
    local index = ctx.container:set_entity_template({});
    assert(index == types.entities.province_neighbors)
  end

  do
    local index = ctx.container:set_entity_template({});
    assert(index == types.entities.culture)
  end

  do
    local index = ctx.container:set_entity_template({});
    assert(index == types.entities.country)
  end

  do
    local index = ctx.container:set_entity_template({
      uint_t, -- parent
      uint_t, -- owner
    })

    assert(index == types.entities.title)
  end

  -- тут создадим хранилища строк, хотя может и не тут
  --print("setup_generator")

  -- долго считает, нужно придумать возможно толи 'map' функцию
end

local function mix_val(x, y, val)
  --return x * (1.0 - val) + y * val
  return x + val * (y - x)
end

-- 64бит луа позволяет нам делать так
local function make_index_pair(index1, index2)
  return (index1 << 32) | index2
end

local function get_index_pair(index_pair)
  return (index_pair >> 32) & constants.uint32_max, index_pair & constants.uint32_max
end

function generate_plates(ctx, local_table)
  local function_timer = generator.timer_t.new("plates generation")
  --collectgarbage("collect")

  local plates_count = local_table.userdata.plates_count
  local tiles_count = ctx.map:tiles_count()
  local tile_plate = utils.init_array(tiles_count, -1)
  local plate_tiles = utils.init_array(plates_count, {})

  local maxf = math.max
  local minf = math.min
  local absf = math.abs

  do
    -- эта часть похоже что самая долгая
    -- по идее сделал все что мог для оптимизации, все же луа это не с++

    -- около 3 секунд экономит (тоже неплохо)
    utils.int_random_queue(ctx, plates_count, function(index, queue_push)
      local attempts = 0
      local success = false
      repeat
        attempts = attempts + 1
      repeat
        -- возвращает целочисленный рандом между 1 и ctx.map:tiles_count() включительно
        local rand_index = ctx.random:index(ctx.map:tiles_count())
        if tile_plate[rand_index] ~= -1 then break end

        -- возвращаем константный указатель, по идее я исправил все функции карты
        -- так чтобы они работали нормально с луа индексами
        local tile = ctx.map:get_tile(rand_index) -- то есть валидно даже если rand_index == ctx.map:tiles_count()
        local continue_b = false
        for j = 1, tile.n_count do
          -- все массивы в луа индексируются с 1, даже си массивы вроде tile.neighbors
          -- принимает луа_индекс и возвращает луа_индекс
          local n_index = tile:get_neighbor_index(j)
          if tile_plate[n_index] ~= -1 then continue_b = true end
        end

        if continue_b then break end

        success = true
        tile_plate[rand_index] = index
        --plate_tiles[i][#plate_tiles[i]+1] = rand_index
        for j = 1, tile.n_count do
          local n_index = tile:get_neighbor_index(j)
          local pair = make_index_pair(index, n_index)
          queue_push(pair)
        end
      until true
      until success or attempts >= 100
    end, function(data, queue_push)
      local plate_index, tile_index = get_index_pair(data)

      if tile_plate[tile_index] == -1 then
        tile_plate[tile_index] = plate_index

        -- следующие 3 строчки, исключая коменты, скорее всего занимают больше всего времени

        -- возвращаем константный указатель
        local tile = ctx.map:get_tile(tile_index)
        -- tile.n_count лучше сделать проперти
        for j = 1, tile.n_count do
          local n_index = tile:get_neighbor_index(j)
          if tile_plate[n_index] == -1 then
            queue_push(make_index_pair(plate_index, n_index))
          end
        end
      end
    end)

    -- занимает секунд 13 судя по таймеру
    -- local active_tile_indices = utils.create_table(tiles_count, 0)
    -- assert(#active_tile_indices == 0)
    -- local unique_tiles = utils.init_array(tiles_count, false)
    -- for i = 1, plates_count do
    --   local attempts = 0
    --   local success = false
    --   repeat
    --     attempts = attempts + 1
    --   repeat
    --     -- возвращает целочисленный рандом между 1 и ctx.map:tiles_count() включительно
    --     local rand_index = ctx.random:index(ctx.map:tiles_count())
    --     if tile_plate[rand_index] ~= -1 then break end
    --
    --     -- возвращаем константный указатель, по идее я исправил все функции карты
    --     -- так чтобы они работали нормально с луа индексами
    --     local tile = ctx.map:get_tile(rand_index) -- то есть валидно даже если rand_index == ctx.map:tiles_count()
    --     local continue_b = false
    --     for j = 1, tile.n_count do
    --       -- все массивы в луа индексируются с 1, даже си массивы вроде tile.neighbors
    --       -- принимает луа_индекс и возвращает луа_индекс
    --       local n_index = tile:get_neighbor_index(j)
    --       if tile_plate[n_index] ~= -1 then continue_b = true end
    --     end
    --
    --     if continue_b then break end
    --
    --     success = true
    --     unique_tiles[rand_index] = true
    --     tile_plate[rand_index] = i
    --     --plate_tiles[i][#plate_tiles[i]+1] = rand_index
    --     for j = 1, tile.n_count do
    --       local n_index = tile:get_neighbor_index(j)
    --       local pair = make_index_pair(i, n_index)
    --       active_tile_indices[#active_tile_indices+1] = pair
    --     end
    --   until true
    --   until success or attempts >= 100
    -- end
    --
    -- -- скорее всего основной виновник низкой производительности вот он
    -- -- рандомно раскидываем все пространство по плитам
    -- while #active_tile_indices ~= 0 do
    -- repeat
    --   -- берем рандомные даные из массива
    --   local random_index = ctx.random:index(#active_tile_indices)
    --   local plate_index, tile_index = get_index_pair(active_tile_indices[random_index])
    --   active_tile_indices[random_index] = active_tile_indices[#active_tile_indices]
    --   active_tile_indices[#active_tile_indices] = nil
    --
    --   if tile_plate[tile_index] ~= -1 then break end
    --
    --   tile_plate[tile_index] = plate_index
    --   --plate_tiles[plate_index][#plate_tiles[plate_index]+1] = tile_index
    --   unique_tiles[tile_index] = true
    --
    --   -- возвращаем константный указатель
    --   local tile = ctx.map:get_tile(tile_index)
    --   -- tile.n_count лучше сделать проперти
    --   for j = 1, tile.n_count do
    --     local n_index = tile:get_neighbor_index(j)
    --     if not unique_tiles[n_index] then
    --       active_tile_indices[#active_tile_indices+1] = make_index_pair(plate_index, n_index)
    --     end
    --   end
    -- until true
    -- end

    -- маленькая оптимизация
    for i = 1, tiles_count do
      local plate_index = tile_plate[i]
      assert(plate_index >= 1 and plate_index <= plates_count)
      plate_tiles[plate_index][#plate_tiles[plate_index]+1] = i
    end

    --unique_tiles = nil
    --active_tile_indices = nil
    function_timer:checkpoint("plates creation")
  end

  do
    -- проверка
    local abc = 0
    for i = 1, plates_count do
      local count = #plate_tiles[i]
      abc = abc + count
    end
    print("tile_count " .. tiles_count)
    print("counter    " .. abc)
    assert(tiles_count == abc)
  end

  -- собираем границы, границ должно быть < 100к (в среднем ~50-70)
  local tiles_edges = utils.create_table(100000, 0)
  do -- тут получается цикл на 3кк итераций
    local unique_edges = utils.create_table(0, 100000)
    -- почти в два раза ускорение, по сути убрал только циклы (?)
    utils.each_tile_neighbor(ctx, function(tile_index, neighbor_index)
      local plate1 = tile_plate[tile_index]
      local plate2 = tile_plate[neighbor_index]
      local min_index = minf(tile_index, neighbor_index)
      local max_index = maxf(tile_index, neighbor_index)
      local tile_indices_pair = make_index_pair(min_index, max_index)

      if plate1 ~= plate2 and unique_edges[tile_indices_pair] == nil then
        unique_edges[tile_indices_pair] = true
        tiles_edges[#tiles_edges+1] = tile_indices_pair
      end
    end)
    -- for i = 1, tiles_count do
    --   local tile = ctx.map:get_tile(i)
    --   for j = 1, tile.n_count do
    --     local tile_neighbour_index = tile:get_neighbor_index(j)
    --
    --     local plate1 = tile_plate[i]
    --     local plate2 = tile_plate[tile_neighbour_index]
    --     local min_index = minf(i, tile_neighbour_index)
    --     local max_index = maxf(i, tile_neighbour_index)
    --     local tile_indices_pair = make_index_pair(min_index, max_index)
    --
    --     if plate1 ~= plate2 and unique_edges[tile_indices_pair] == nil then
    --       unique_edges[tile_indices_pair] = true
    --       tiles_edges[#tiles_edges+1] = tile_indices_pair
    --     end
    --   end
    -- end

    unique_edges = nil
  end

  function_timer:checkpoint("edges found")

  local min_plates_count = local_table.userdata.plates_connection_limit
  local max_iterations = local_table.userdata.plates_connection_iteration
  local current_plates_count = plates_count
  local current_iter = 0

  -- даже при том что тут много нужно сделать, этот цикл занимает не очень много времени
  -- тут циклы с малым количеством итераций в основном (итерации по границам + по количеству плит)
  while current_plates_count > min_plates_count and current_iter < max_iterations do
    local next_plates = utils.init_array(plates_count, {})
    current_iter = current_iter + 1

    -- по идее вот эта часть занимает больше всего времени
    -- как оказывается нет
    for i = 1, #tiles_edges do
      local pair = tiles_edges[i]
      local tile_index1, tile_index2 = get_index_pair(pair)

      assert(tile_index1 >= 1 and tile_index1 <= tiles_count)
      assert(tile_index2 >= 1 and tile_index2 <= tiles_count)
      assert(tile_index1 ~= tile_index2)
      local plate1 = tile_plate[tile_index1]
      local plate2 = tile_plate[tile_index2]
      if plate1 ~= plate2 then
        next_plates[plate1][plate2] = true
        next_plates[plate2][plate1] = true
      end
    end

    local max_tiles_count = 0
    local min_tiles_count = 121523152
    for i = 1, #plate_tiles do
      local tiles_count = #plate_tiles[i]
      if tiles_count >= 2 then
        max_tiles_count = maxf(max_tiles_count, tiles_count)
        min_tiles_count = minf(min_tiles_count, tiles_count)
      end
    end

    local prob_count = 0
    local koef_summ = 0

    -- ниже код определяет какие плиты соединятся друг с другом на текущей итерации
    local plates_union = utils.init_array(plates_count, false)
    for i = 1, #plate_tiles do
    repeat
      local tiles_count = #plate_tiles[i]
      if tiles_count < 2 then break end

      -- я хочу чтобы как можно больше плит соединились на полюсах
      -- для этого я беру небольшой коэффициент на основе y компоненты позиции корневого тайла
      local root_tile_index = plate_tiles[i][1]
      local tile = ctx.map:get_tile(root_tile_index)
      local root_pos = maf.vector(ctx.map:get_point(tile.center))
      --local norm = glm::normalize(root_pos * glm::vec4(1.0f, 1.0f, 1.0f, 0.0f))
      root_pos:normalize()
      local y_k = absf(root_pos.y)
      assert(y_k >= 0.0 and y_k <= 1.0)
      local poles_k = mix_val(0.2, 0.8, absf(root_pos.y))
      assert(poles_k >= 0.2 and poles_k <= 0.8)

      -- и так же я хочу чтобы большие плиты не продолжали бесконечно соединятся
      local k = (tiles_count - min_tiles_count) / (max_tiles_count - min_tiles_count)
      assert(k <= 1.0 and k >= 0.0)
      local inv_k = 1.0 - k
      local sum_inv_k = 0.6
      local sum = inv_k * sum_inv_k + poles_k * (1.0 - sum_inv_k)
      local final_k = mix_val(0.1, 0.8, sum)
      assert(final_k >= 0.1 and final_k <= 0.8)

      local need_unite = ctx.random:probability(final_k)
      plates_union[i] = need_unite

      if plates_union[i] then prob_count = prob_count + 1 end
      koef_summ = koef_summ + final_k
    until true
    end

    -- print("prob_count " .. prob_count)
    -- print("koef_summ  " .. koef_summ)

    -- по идее при обходе мы почти все плиты которые нужно соединяем
    for i = 1, #plate_tiles do
    repeat
      if #plate_tiles[i] < 2 then break end
      if not plates_union[i] then break end

      local plate_index = -1
      do
        for key, value in pairs(next_plates[i]) do
          assert(type(key) == "number")
          -- первого соседа?
          if plates_union[key] then plate_index = key; break; end
        end
      end

      if plate_index == -1 then break end

      -- можем ли мы вообще попасть на невалидную плиту?
      while #plate_tiles[plate_index] < 2 do
        plate_index = plate_tiles[plate_index][1];
      end

      local function append_table(table1, table2)
        for i,value in ipairs(table2) do
          table1[#table1+1] = value
        end
      end

      append_table(plate_tiles[i], plate_tiles[plate_index])
      plate_tiles[plate_index] = nil
      plate_tiles[plate_index] = {i}

      current_plates_count = current_plates_count - 1
    until true
    end

    -- обновляем индексы
    for i = 1, #plate_tiles do
      if #plate_tiles[i] >= 2 then
        for j, value in ipairs(plate_tiles[i]) do tile_plate[value] = i end
      end
    end

  end -- while

  function_timer:checkpoint("plates connected")

  tiles_edges = nil

  local max_tiles_count = 0
  local min_tiles_count = 121523152
  local plates_counter = 0
  for i, value in ipairs(plate_tiles) do
    local tiles_count = #value
    if tiles_count >= 2 then
      max_tiles_count = maxf(max_tiles_count, tiles_count);
      min_tiles_count = minf(min_tiles_count, tiles_count);
      plates_counter = plates_counter + 1
    end
  end

  local united_plates = 0
  local plate_tiles_local = {}
  for i, value in ipairs(plate_tiles) do
    local tiles_count = #value
    if tiles_count > 1 then
      plate_tiles_local[#plate_tiles_local+1] = value
    else
      --print("small plate index " .. (i-1))
      --print("small plate tile  " .. value[1]-1)
      united_plates = united_plates + 1
    end
  end

  plate_tiles = nil
  print("united plates " .. united_plates)
  print("valid plates  " .. #plate_tiles_local)
  print("summ          " .. (united_plates + #plate_tiles_local))

  -- не могу получить индекс старых плит внутри суперплиты =(

  local tiles_counter = 0
  for i = 1, #plate_tiles_local do
    assert(#plate_tiles_local[i] > 1)

    for j = 1, #plate_tiles_local[i] do
      local tile_index = plate_tiles_local[i][j]
      tile_plate[tile_index] = i
      assert(tile_index >= 1 and tile_index <= tiles_count)
      tiles_counter = tiles_counter + 1
    end
  end

  assert(tiles_counter == ctx.map:tiles_count())

  -- вот так я добавляю данные в контейнер
  -- в контейнере данные будут занимать меньше места
  -- контейнер тоже работает с луа_индексами
  for i = 1, tiles_count do
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.plate_index, tile_plate[i])
  end

  local_table.final_plates_count = #plate_tiles_local
  ctx.container:set_entity_count(types.entities.plate, local_table.final_plates_count);
  for i = 1, #plate_tiles_local do
    for j, value in ipairs(plate_tiles_local[i]) do
      ctx.container:add_child(types.entities.plate, i, value);
    end
  end

  print("tectonic plates count " .. plates_counter)
  print("max tiles count       " .. max_tiles_count)
  print("min tiles count       " .. min_tiles_count)

  tile_plate = nil
  plate_tiles = nil
  active_tile_indices = nil
  unique_tiles = nil
end -- generate_plates

-- выполняется примерно за 71 мс, более менее терпимо
function generate_plate_datas(ctx, local_table)
  local function_timer = generator.timer_t.new("plate datas generating")
  --collectgarbage("collect")

  local maxf = math.max
  local minf = math.min

  local max_tiles_count = 0
  local min_tiles_count = 1215152
  local plates_count = local_table.final_plates_count
  local local_plates_indices = utils.init_array(plates_count, -1)
  local tectonic_plate_props = utils.init_array(plates_count, {})
  for i = 1, plates_count do
    local_plates_indices[i] = i
    local tiles_count = ctx.container:get_childs_count(types.entities.plate, i)
    max_tiles_count = maxf(max_tiles_count, tiles_count)
    min_tiles_count = minf(min_tiles_count, tiles_count)
  end

  assert(min_tiles_count > 1)

  local ocean_percentage = local_table.userdata.ocean_percentage
  local oceanic_tiles = ocean_percentage * ctx.map:tiles_count()
  local oceanic_tiles_count = 0

  local map = ctx.map
  local rand = ctx.random

  -- эти вещи по идее можно вынести в данные генерации
  local min_drift_rate = -constants.pi / 30.0
  local max_drift_rate =  constants.pi / 30.0
  local min_spin_rate  = -constants.pi / 30.0
  local max_spin_rate  =  constants.pi / 30.0

  local min_oceanic_elevation = -0.8
  local max_oceanic_elevation = -0.3
  local min_continental_elevation = 0.15
  local max_continental_elevation = 0.5
  assert(ctx.container:entities_count(types.entities.plate) ~= 0)

  for i = 1, plates_count do
    -- взятие случайных плит: результат выгялит гораздо лучше
    local rand_plate_index = rand:index(#local_plates_indices)
    local plate_index = local_plates_indices[rand_plate_index]
    local_plates_indices[rand_plate_index] = local_plates_indices[#local_plates_indices]
    local_plates_indices[#local_plates_indices] = nil

    assert(plate_index <= plates_count)

    local tile_indices_count = ctx.container:get_childs_count(types.entities.plate, plate_index)
    assert(tile_indices_count > 1)

    local k = (tile_indices_count - min_tiles_count) / (max_tiles_count - min_tiles_count)
    assert(k <= 1.0 and k >= 0.0)
    local inv_k = k
    local final_k = mix_val(0.3, 0.95, inv_k)

    local oceanic = rand:probability(final_k)
    if oceanic and (oceanic_tiles_count + tile_indices_count <= oceanic_tiles) then
      oceanic_tiles_count = oceanic_tiles_count + tile_indices_count
    elseif oceanic and (oceanic_tiles_count + tile_indices_count > oceanic_tiles) then
      oceanic = false;
    end

    -- нужно использовать стороннюю либу для векторной математики
    local drift_axis = maf.vector(rand:unit3())
    drift_axis:normalize()
    local drift_rate = rand:closed(min_drift_rate, max_drift_rate)
    -- тут берем случайный тайл на плите, но вообще можно любой случайный тайл брать
    local idx = rand:index(tile_indices_count)
    assert(idx <= tile_indices_count)
    local rand_index = ctx.container:get_child(types.entities.plate, plate_index, idx)
    local tile = map:get_tile(rand_index)
    -- get_point возвращает 3 числа, maf.vector принимает 3 числа
    local spin_axis = maf.vector(map:get_point(tile.center))
    spin_axis:normalize()
    local spin_rate = rand:closed(min_spin_rate, max_spin_rate)
    local base_elevation = rand:closed(min_oceanic_elevation, max_oceanic_elevation)
    if not oceanic then base_elevation = rand:closed(min_continental_elevation, max_continental_elevation) end

    tectonic_plate_props[plate_index] = {
      drift_axis = drift_axis,
      drift_rate = drift_rate,
      spin_axis = spin_axis,
      spin_rate = spin_rate,
      base_elevation = base_elevation,
      oceanic = oceanic
    }
  end

  for i = 1, plates_count do
    ctx.container:set_data_vec3(types.entities.plate, i, types.properties.plate.drift_axis,     tectonic_plate_props[i].drift_axis:unpack())
    ctx.container:set_data_f32 (types.entities.plate, i, types.properties.plate.drift_rate,     tectonic_plate_props[i].drift_rate)
    ctx.container:set_data_vec3(types.entities.plate, i, types.properties.plate.spin_axis,      tectonic_plate_props[i].spin_axis:unpack())
    ctx.container:set_data_f32 (types.entities.plate, i, types.properties.plate.spin_rate,      tectonic_plate_props[i].spin_rate)
    ctx.container:set_data_f32 (types.entities.plate, i, types.properties.plate.base_elevation, tectonic_plate_props[i].base_elevation)
    ctx.container:set_data_u32 (types.entities.plate, i, types.properties.plate.oceanic,        bool_to_number(tectonic_plate_props[i].oceanic))
  end

  print("oceanic tiles count " .. oceanic_tiles_count)
  print("ground  tiles count " .. ctx.map:tiles_count() - oceanic_tiles_count)

  assert(plates_count == ctx.container:entities_count(types.entities.plate))
  -- хорошая оптимизация, но не думаю что я смогу всегда ее применять
  local water_counter = 0
  for i = 1, plates_count do
    local oceanic = ctx.container:get_data_u32(types.entities.plate, i, types.properties.plate.oceanic)
    local height = ctx.container:get_data_f32(types.entities.plate, i, types.properties.plate.base_elevation)
    local childs = ctx.container:get_childs(types.entities.plate, i)

    water_counter = water_counter + oceanic * #childs

    -- рандомное число из предыдущего значения (период 2^32)
    -- не используем ctx.random
    assert(i >= 0 and i <= constants.uint32_max) -- valid range for utils.prng
    local rand_num1 = utils.prng(i)
    local rand_num2 = utils.prng(rand_num1)
    local rand_num3 = utils.prng(rand_num2)
    local color_r = utils.prng_normalize(rand_num1)
    local color_g = utils.prng_normalize(rand_num2)
    local color_b = utils.prng_normalize(rand_num3)
    local color = utils.make_color(color_r, color_b, color_g, 1.0)
    -- применяем сразу ко всем тайлам плиты
    -- к сожалению это наверное единственно место где очевидно применение childs
    ctx.map:set_plate_color_and_height(childs, color, height)
  end

  print("oceanic tiles after recompute " .. water_counter)
  print("oceanic tiles k               " .. water_counter / ctx.map:tiles_count())

  tectonic_plate_props = nil
  local_plates_indices = nil
  -- тяжело переписывать 6к строк =(
end -- generate_plate_datas
