--luacheck: no max line length
--luacheck: ignore local_table

-- подумал тут что нужно отделить название мода от ресурсов с помощью ':'
local types = require("apates_quest.scripts.entities_types_table")
local queue = require("apates_quest.scripts.queue")

local function make_index_pair(index1, index2)
  return (index1 << 32) | index2
end

local function get_index_pair(index_pair)
  return (index_pair >> 32) & constants.uint32_max, index_pair & constants.uint32_max
end

local function pack_n_index(province_n_index)
  if province_n_index >= 0 then return province_n_index end

  local tmp = math.abs(province_n_index)
  local mask = constants.uint32_max & ~constants.int32_max
  return mask | tmp
end

local function unpack_n_index(province_n_index)
  if province_n_index >= constants.int32_max then return -(province_n_index & constants.int32_max) end
  return province_n_index
end

local function bool_to_number(value)
  --assert(type(value) == "boolean")
  return value and 1 or 0
end

local function is_tile_valid(ctx, tile_index)
  local t = ctx.container:get_data_f32(types.entities.tile, tile_index, types.properties.tile.heat)
  local h = ctx.container:get_data_f32(types.entities.tile, tile_index, types.properties.tile.elevation)
  return t > 0.15 and h >= 0.0 -- and h < 0.5
end

local function remove_from_array(array, object)
  local size = #array
  for i = 1, size do
    if array[i] == object then
      array[i] = array[size]
      array[size] = nil
      break
    end
  end

  assert(#array == size-1)
end

local function check_container_unique(array)
  local size = #array
  local unique_elems = utils.create_table(0, size * 10)
  for i = 1, size do
    local inner_size = #array[i]
    for j = 1, inner_size do
      local index = array[i][j]
      assert(unique_elems[index] == nil)
      unique_elems[index] = true
    end
  end
end

local function array_contain(array, data)
  local size = #array
  for i = 1, size do
    if array[i] == data then return true end
  end
  return false
end

local function find_index(array, obj)
  if type(obj) == "function" then
    for i = 1, #array do
      if obj(array[i]) then return i end
    end
  else
    for i = 1, #array do
      if array[i] == obj then return i end
    end
  end

  return -1
end

local function update_container(tiles_count, tile_province, province_tiles)
  for i = 1, #province_tiles do
    province_tiles[i] = utils.create_table(100, 0)
  end

  for i = 1, tiles_count do
    local prov_index, _ = get_index_pair(tile_province[i])
    if prov_index ~= constants.uint32_max then
      table.insert(province_tiles[prov_index], i)
    end
  end
end

local function unite_small_provinces(ctx, province_tiles_min, province_tiles_avg, tile_province, province_tiles)
  local maxf = math.max
  local minf = math.min
  local tiles_count = ctx.map:tiles_count()

  local provinces_edges = utils.create_table(100000, 0)
  do
    local unique_edges = utils.create_table(0, 100000)
    utils.each_tile_neighbor(ctx, function(tile_index, neighbor_index)
      local prov_index1, _ = get_index_pair(tile_province[tile_index])
      local prov_index2, _ = get_index_pair(tile_province[neighbor_index])
      if prov_index1 == constants.uint32_max or prov_index2 == constants.uint32_max then return end

      local min_index = minf(tile_index, neighbor_index)
      local max_index = maxf(tile_index, neighbor_index)
      local tile_indices_pair = make_index_pair(min_index, max_index)
      assert(prov_index1 <= #province_tiles)
      assert(prov_index2 <= #province_tiles)
      assert(#province_tiles[prov_index1] >= 2)
      assert(#province_tiles[prov_index2] >= 2)

      if prov_index1 ~= prov_index2 and unique_edges[tile_indices_pair] == nil then
        unique_edges[tile_indices_pair] = true
        table.insert(provinces_edges, tile_indices_pair)
      end
    end)
    unique_edges = nil
  end

  print("provinces_edges count " .. #provinces_edges)

  --function_timer:checkpoint("provinces neighbors found")

  -- предварительно соединим малые провинции
  local max_iterations = 15
  local current_iter = 0;
  while current_iter < max_iterations do
    local next_provinces = utils.init_array(#province_tiles, {})
    current_iter = current_iter + 1

    local minimum_counter = 0
    local minimum_tiles_counter = 0
    local minimum_tiles = 32552325

    for i = 1, #provinces_edges do
      local pair = provinces_edges[i]
      local tile_index1, tile_index2 = get_index_pair(pair)

      assert(tile_index1 >= 1 and tile_index1 <= tiles_count)
      assert(tile_index2 >= 1 and tile_index2 <= tiles_count)
      assert(tile_index1 ~= tile_index2)
      local prov_index1, _ = get_index_pair(tile_province[tile_index1])
      local prov_index2, _ = get_index_pair(tile_province[tile_index2])
      assert(#province_tiles[prov_index1] >= 2)
      assert(#province_tiles[prov_index2] >= 2)
      if prov_index1 ~= prov_index2 then
        next_provinces[prov_index1][prov_index2] = true
        next_provinces[prov_index2][prov_index1] = true
      end
    end

    local max_tiles_count = 0
    local min_tiles_count = 121523152
    for i = 1, #province_tiles do
      local province_tiles_count = #province_tiles[i]
      if province_tiles_count >= 2 then
        max_tiles_count = maxf(max_tiles_count, province_tiles_count)
        min_tiles_count = minf(min_tiles_count, province_tiles_count)
      end
    end

    for i = 1, #province_tiles do
    repeat
      if #province_tiles[i] < 2 then break end
      if #province_tiles[i] >= province_tiles_min then break end

      local province_too_small = #province_tiles[i] < province_tiles_min
      if province_too_small then
        minimum_counter = minimum_counter + 1
        minimum_tiles_counter = minimum_tiles_counter + #province_tiles[i]
        minimum_tiles = minf(minimum_tiles, #province_tiles[i])
      end

      -- найдем соседей с которыми мы можем соединиться
      local province_index = -1
      local local_minimum_tiles = 349285325
      for idx,_ in pairs(next_provinces[i]) do
        local province_tiles_count = #province_tiles[idx]
        if province_tiles_count >= 2 and province_tiles_count < province_tiles_avg then
          --province_index = idx
          --break
          if local_minimum_tiles > province_tiles_count then
            local_minimum_tiles = province_tiles_count
            province_index = idx
          end
        end

        -- if province_too_small then
        --   print("idx " .. idx .. " tiles_count " .. tiles_count)
        -- end
      end

      if province_index == -1 then break end

      assert(#province_tiles[province_index] >= 2)

      local function append_table(table1, table2)
        for index = 1, #table2 do
          table.insert(table1, table2[index])
        end
      end

      append_table(province_tiles[i], province_tiles[province_index])
      province_tiles[province_index] = nil
      province_tiles[province_index] = {i}
    until true
    end

    -- обновляем индексы
    for i = 1, #province_tiles do
      if #province_tiles[i] >= 2 then
        for j = 1, #province_tiles[i] do
          tile_province[province_tiles[i][j]] = make_index_pair(i, constants.uint32_max)
        end
      end
    end

    -- print("iter " .. current_iter)
    -- print("minimum_counter " .. minimum_counter)
    -- print("minimum_tiles_counter_avg " .. minimum_tiles_counter / minimum_counter)
    -- print("minimum_tiles " .. minimum_tiles)
    --next_plates = nil
  end -- while
end

local function gen_title_color(title_table, index)
  local val1 = utils.prng32(index)
  local val2 = utils.prng32(val1)
  local val3 = utils.prng32(val2)
  local val4 = utils.prng32(val3)
  local val5 = utils.prng32(val4)
  local f1 = utils.prng_normalize32(val1);
  local f2 = utils.prng_normalize32(val2);
  local f3 = utils.prng_normalize32(val3);
  local f4 = utils.prng_normalize32(val4);
  local f5 = utils.prng_normalize32(val5);
  title_table.main_color = utils.make_color(f1, f2, f3, 1.0)
  title_table.border_color1 = utils.make_color(f2, f3, f4, 1.0)
  title_table.border_color2 = utils.make_color(f3, f4, f5, 1.0)
end

local function generate_provinces(ctx, local_table)
  local function_timer = generator.timer_t.new("provinces generation")

  local maxf = math.max
  local minf = math.min

  local tiles_count = ctx.map:tiles_count()
  local tile_pool = utils.init_array(tiles_count, -1)
  local pool_id = 0
  for i = 1, tiles_count do
  repeat
    if tile_pool[i] ~= -1 then break end
    do
      local h = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.elevation)
      if h < 0.0 then break end
    end

    pool_id = pool_id + 1
    tile_pool[i] = pool_id

    local ground_queue = queue.new()
    ground_queue:push_right(i)

    while not ground_queue:is_empty() do
      local current_tile = ground_queue:pop_left()
      local tile = ctx.map:get_tile(current_tile)
      for j = 1, tile.n_count do
        local n_index = tile:get_neighbor_index(j)
        local h = ctx.container:get_data_f32(types.entities.tile, n_index, types.properties.tile.elevation)
        if h >= 0.0 and tile_pool[n_index] == -1 then
          tile_pool[n_index] = pool_id
          ground_queue:push_right(n_index)
        end
      end
    end -- while

  until true
  end -- for

  local ground_pools = utils.init_array(pool_id, {})
  for i = 1, tiles_count do
    local pool_index = tile_pool[i]
    if pool_index ~= -1 then
      table.insert(ground_pools[pool_index], i)
    end
  end

  function_timer:checkpoint("ground tile pools is setted")

  -- тут раньше вычислялись веса у тайла, возможно мне тоже нужно сделать веса, но другие
  local good_tiles_count = 0
  local tile_weights = utils.init_array(tiles_count, make_index_pair(0, 0))
  for i = 1, tiles_count do
    local h = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.elevation)
    local t = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.heat)

    good_tiles_count = good_tiles_count + bool_to_number(is_tile_valid(ctx, i))

    -- в оригинале вес зависел от биома и давалось больше очков теплому биому
    local valid_h = h >= 0.0 and h < 0.5
    local valid_t = t > 0.15
    local h_k = bool_to_number(valid_h)
    local h_t = bool_to_number(valid_t)
    local points = math.floor(100 * (h_k * h_t * t)) -- было бы неплохо придумать что нибудь поразумнее
    --print(points)
    tile_weights[i] = make_index_pair(i, points)
  end

  local provinces_count = local_table.userdata.provinces_count
  local province_tiles_avg = good_tiles_count / provinces_count
  print("good_tiles_count   " .. good_tiles_count)
  print("province_tiles_avg " .. province_tiles_avg)
  local province_tiles_ratio = 0.75
  local province_tiles_min = province_tiles_ratio * province_tiles_avg
  local province_tiles_max = (2.0 - province_tiles_ratio) * province_tiles_avg
  print("province_tiles_min " .. province_tiles_min)
  print("province_tiles_max " .. province_tiles_max)

  local tile_weights_accum = utils.init_array(tiles_count, make_index_pair(0, 0))
  local accum = 0
  for i = 1, tiles_count do
    local id, points = get_index_pair(tile_weights[i])
    accum = accum + points
    tile_weights_accum[i] = make_index_pair(id, accum)
  end

  table.sort(tile_weights_accum, function(first, second)
    local _, points1 = get_index_pair(first)
    local _, points2 = get_index_pair(second)
    return points1 < points2
  end)

  function_timer:checkpoint("tiles weights computation")

  local unique_tiles = utils.init_array(tiles_count, false)
  local pool_prov_count = utils.init_array(#ground_pools, 0)
  local tile_province = utils.init_array(tiles_count, make_index_pair(constants.uint32_max, constants.uint32_max))
  local province_tiles = utils.init_array(provinces_count, {})
  utils.int_queue(provinces_count, function(index, queue_push)
    local tile_index = -1
    local attempts = 0

    while tile_index == -1 and attempts < 100 do
    repeat
      local rand_index = ctx.random:index(#tile_weights_accum)
      local id, _ = get_index_pair(tile_weights_accum[rand_index])
      attempts = attempts + 1

      if unique_tiles[id] then break end

      local valid_tile = is_tile_valid(ctx, id)
      if not valid_tile then break end

      local pool_index = tile_pool[id]
      if pool_index == -1 then break end
      if #ground_pools[pool_index] < province_tiles_avg / 2 then break end
      if #ground_pools[pool_index] < province_tiles_avg and pool_prov_count[pool_index] >= 1 then break end
      -- если остров меньше чем четверть от минимального (максимального? среднего?) количества тайлов в провке,
      -- то пропускаем этот остров, если между этим значением и минимального
      -- (максимального? среднего?) количества тайлов, то на этом острове должно быть только одна провинция
      -- иначе любое колиество провинций

      local dist_to_water_accum = 0
      local found = true
      local tile = ctx.map:get_tile(id)
      for j = 1, tile.n_count do
        local n_index = tile:get_neighbor_index(j)
        if unique_tiles[id] then
          found = false
          break
        end

        local tile = ctx.map:get_tile(n_index) -- luacheck: ignore tile
        for k = 1, tile.n_count do
          local n_index = tile:get_neighbor_index(k) -- luacheck: ignore n_index
          if unique_tiles[id] then
            found = false
            break
          end
        end

        local dist_to_water = ctx.container:get_data_u32(types.entities.tile, id, types.properties.tile.water_dist)
        dist_to_water_accum = dist_to_water_accum + dist_to_water
      end

      if dist_to_water_accum <= tile.n_count then break end -- continue
      if not found then break end -- continue

      pool_prov_count[pool_index] = pool_prov_count[pool_index] + 1
      tile_index = id
    until true
    end

    if tile_index == -1 then return end
    unique_tiles[tile_index] = true

    local tile = ctx.map:get_tile(tile_index)
    for j = 1, tile.n_count do
      local n_index = tile:get_neighbor_index(j)
      unique_tiles[n_index] = true
      local tile = ctx.map:get_tile(n_index) -- luacheck: ignore tile
      for k = 1, tile.n_count do
        local n_index = tile:get_neighbor_index(k) -- luacheck: ignore n_index
        unique_tiles[n_index] = true
      end
    end

    queue_push(tile_index)
    tile_province[tile_index] = make_index_pair(index, 0)
    --table.insert(province_tiles[i], tile_index)
  end, function(current_tile, queue_push)
    local province_index, dist = get_index_pair(tile_province[current_tile])
    local tile = ctx.map:get_tile(current_tile)
    for j = 1, tile.n_count do
    repeat
      local n_index = tile:get_neighbor_index(j)

      local valid_tile = is_tile_valid(ctx, n_index)
      if not valid_tile then break end

      local _, n_dist = get_index_pair(tile_province[n_index])
      if n_dist ~= constants.uint32_max then break end

      tile_province[n_index] = make_index_pair(province_index, dist + 1)
      --table.insert(province_tiles[i], n_index)

      queue_push(n_index)
    until true
    end
  end) -- utils.int_queue

  -- нам придется обновить province_tiles несколько раз в этой функции
  update_container(tiles_count, tile_province, province_tiles)

  function_timer:checkpoint("previous province computation")

  unite_small_provinces(ctx, province_tiles_min, province_tiles_avg, tile_province, province_tiles)

  function_timer:checkpoint("small provinces united")

  local ceilf = math.ceil
  -- нам необходимо разделить слишком большие провинции на несколько
  -- на сколько? по минимальным значениям? или по средним?
  -- лучше по средним, чем больше тайлов в провинции тем лучше
  -- в этом случае мы сможем даже захватить какие нибудь вещи не входящие ни в какие провинции
  -- (такие есть?)
  for i = 1, #province_tiles do
  repeat
    if #province_tiles[i] < 2 then break end
    if #province_tiles[i] < province_tiles_max then break end

    local new_provinces_count = ceilf(#province_tiles[i] / province_tiles_avg)
    assert(new_provinces_count > 0)
    if new_provinces_count == 1 then break end

    local tiles_array = province_tiles[i]
    province_tiles[i] = {}
    for j = 1, #tiles_array do
      local tile_index = tiles_array[j]
      tile_province[tile_index] = make_index_pair(constants.uint32_max, constants.uint32_max)
    end

    local unique_tiles = utils.create_table(0, good_tiles_count) -- luacheck: ignore unique_tiles
    utils.int_queue(new_provinces_count, function(index, queue_push)
      local tile_index = -1
      local attempts = 0

      while tile_index == -1 and attempts < 100 do
      repeat
        local rand_index = ctx.random:index(#tiles_array)
        local current_tile_index = tiles_array[rand_index]
        attempts = attempts + 1

        if unique_tiles[current_tile_index] ~= nil then break end

        local valid_tile = is_tile_valid(ctx, current_tile_index)
        if not valid_tile then break end

        local dist_to_water_accum = 0
        local tile = ctx.map:get_tile(current_tile_index)
        local found = true
        for m = 1, tile.n_count do
          local n_index = tile:get_neighbor_index(m)
          if unique_tiles[current_tile_index] ~= nil then
            found = false
            break
          end

          local tile = ctx.map:get_tile(n_index) -- luacheck: ignore tile
          for k = 1, tile.n_count do
            local n_index = tile:get_neighbor_index(k) -- luacheck: ignore n_index
            if unique_tiles[current_tile_index] ~= nil then
              found = false
              break
            end
          end

          assert(current_tile_index >= 1 and current_tile_index <= tiles_count)
          local dist_to_water = ctx.container:get_data_u32(types.entities.tile, current_tile_index, types.properties.tile.water_dist)
          dist_to_water_accum = dist_to_water_accum + dist_to_water
        end

        --if dist_to_water_accum <= tile.n_count then break end
        if not found then break end

        tile_index = current_tile_index
      until true
      end -- while

      assert(tile_index ~= -1)

      if index == 1 then
        tile_province[tile_index] = make_index_pair(i, 0)
      else
        table.insert(province_tiles, {})
        local new_province_index = #province_tiles
        tile_province[tile_index] = make_index_pair(new_province_index, 0)
      end

      queue_push(tile_index)
      unique_tiles[tile_index] = true
      local tile = ctx.map:get_tile(tile_index)
      for j = 1, tile.n_count do
        local n_index = tile:get_neighbor_index(j)
        unique_tiles[n_index] = true
        local tile = ctx.map:get_tile(n_index) -- luacheck: ignore tile
        for k = 1, tile.n_count do
          local n_index = tile:get_neighbor_index(k) -- luacheck: ignore n_index
          unique_tiles[n_index] = true
        end
      end
    end, function(current_tile, queue_push)
      -- тут я попробовал разные очереди: фифо очередь, рандомная очередь, очередь-куча
      -- хорошо работает обычная фифо очередь

      local province_index, dist = get_index_pair(tile_province[current_tile])
      local tile = ctx.map:get_tile(current_tile)
      for j = 1, tile.n_count do
      repeat
        local n_index = tile:get_neighbor_index(j)
        local n_province_index,_ = get_index_pair(tile_province[n_index])

        local valid_tile = is_tile_valid(ctx, n_index)
        if not valid_tile then break end

        if n_province_index ~= constants.uint32_max then break end

        tile_province[n_index] = make_index_pair(province_index, dist + 1)
        --table.insert(province_tiles[province_index], n_index)

        queue_push(n_index)
      until true
      end
    end) -- utils.int_queue
  until true
  end -- for

  update_container(tiles_count, tile_province, province_tiles)

  -- for i = 1, #province_tiles do
  --   if #province_tiles[i] < province_tiles_max then
  --     print("id " .. i .. " tilles count " .. #province_tiles[i])
  --   end
  -- end

  function_timer:checkpoint("big provinces divided")

  -- если у нас по прежнему остаются провинции с малым количеством тайлов
  -- то нужно позаимствовать их у соседа с большим количеством тайлов
  -- провинции-коммунисты

  -- как заимствовать?
  -- нужно найти соседей с количеством тайлов минимум в два раза большим?
  -- или соседей с количеством больше avg? или больше макс?
  -- третий вариант самый адекватный, но с другой стороны
  -- таких соседей может и не быть
  for i = 1, #province_tiles do
  repeat
    if #province_tiles[i] < 2 then break end
    if #province_tiles[i] >= province_tiles_min then break end

    local border = {}
    for j = 1, #province_tiles[i] do
      local current_tile_index = province_tiles[i][j]
      local province_index1,_ = get_index_pair(tile_province[current_tile_index])
      assert(i == province_index1)
      local tile = ctx.map:get_tile(current_tile_index)
      for k = 1, tile.n_count do
        local n_index = tile:get_neighbor_index(k)
        local province_index2,_ = get_index_pair(tile_province[n_index])
        if province_index2 ~= constants.uint32_max and province_index1 ~= province_index2 then
          table.insert(border, make_index_pair(current_tile_index, n_index))
        end
      end
    end

    -- проверяем границы, берем тайлы только у больших соседей
    local current_index = 1
    while #province_tiles[i] < province_tiles_min and current_index <= #border do
    repeat
      local local_index = current_index
      current_index = current_index + 1

      -- раньше неправильно находился opposite_index
      local tile_index1, tile_index2 = get_index_pair(border[local_index])
      assert(tile_index1 >= 1 and tile_index1 <= tiles_count)
      assert(tile_index2 >= 1 and tile_index2 <= tiles_count)
      local province1,_ = get_index_pair(tile_province[tile_index1])
      local province2,_ = get_index_pair(tile_province[tile_index2])
      local opposite_index = province1 == i and province2 or province1
      local choosen_tile_index = province1 == i and tile_index2 or tile_index1

      assert(opposite_index >= 1 and opposite_index <= #province_tiles)
      if #province_tiles[opposite_index] <= province_tiles_avg then break end

      -- удаляем тайл
      remove_from_array(province_tiles[opposite_index], choosen_tile_index)

      -- добавляем тайл
      table.insert(province_tiles[i], choosen_tile_index)
      do
        local _, old_b = get_index_pair(tile_province[choosen_tile_index])
        tile_province[choosen_tile_index] = make_index_pair(i, old_b)
      end

      local tile = ctx.map:get_tile(choosen_tile_index)
      for j = 1, tile.n_count do
        local n_index = tile:get_neighbor_index(j)
        local province_index2, _ = get_index_pair(tile_province[n_index])
        if province_index2 ~= constants.uint32_max and i ~= province_index2 then
          table.insert(border, make_index_pair(choosen_tile_index, n_index))
        end
      end
    until true
    end -- while

  until true
  end -- for

  function_timer:checkpoint("small provinces takes from big")

  unite_small_provinces(ctx, province_tiles_min, province_tiles_avg, tile_province, province_tiles)

  function_timer:checkpoint("last unification")

  -- всякая информация
  local province_min = 10000
  local province_max = 0
  local count = 0
  local count_more_max = 0
  local count_less_min = 0
  local accum_max = 0
  local accum_min = 0
  local accum_tiles_count = 0
  for i = 1, #province_tiles do
  repeat
    if #province_tiles[i] == 0 then break end
    if #province_tiles[i] == 1 then break end

    province_min = minf(#province_tiles[i], province_min)
    province_max = maxf(#province_tiles[i], province_max)
    count = count + 1
    accum_tiles_count = accum_tiles_count + #province_tiles[i]

    if #province_tiles[i] > province_tiles_max then
      count_more_max = count_more_max + 1
      accum_max = accum_max + #province_tiles[i]
    end

    if #province_tiles[i] < province_tiles_min then
      count_less_min = count_less_min + 1
      accum_min = accum_min + #province_tiles[i]
    end
  until true
  end

  -- обязательное условие хорошего результата
  assert(accum_tiles_count <= good_tiles_count)

  ctx.container:clear_entities(types.entities.province)
  ctx.container:clear_entities(types.entities.province_neighbors)

  for i = 1, ctx.map:tiles_count() do
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.province_index, constants.uint32_max)
  end

  -- в контейнере переход от провинции к тайлу держать совсем не обязательно
  -- более важно держать соседей провинции

  do
    local size = #province_tiles
    local province_tiles_copy = utils.create_table(size, 0)
    local counter = 0
    for i = 1, size do
      local inner_size = #province_tiles[i]
      if inner_size >= 2 then
        counter = counter + 1
        province_tiles_copy[counter] = province_tiles[i]
        for j = 1, inner_size do
          local index = province_tiles[i][j]
          tile_province[index] = counter
        end
      end
    end

    province_tiles = province_tiles_copy
  end

  check_container_unique(province_tiles)

  assert(count == #province_tiles)
  ctx.container:set_entity_count(types.entities.province, count)
  ctx.container:set_entity_count(types.entities.province_neighbors, count)
  for i = 1, #province_tiles do
    local inner_size = #province_tiles[i]
    assert(inner_size >= 2)
    for j = 1, inner_size do
      ctx.container:add_child(types.entities.province, i, province_tiles[i][j]);
    end

    assert(inner_size == ctx.container:get_childs_count(types.entities.province, i))
  end

  for i = 1, #province_tiles do
    for j = 1, #province_tiles[i] do
      local tile_index = province_tiles[i][j]
      ctx.container:set_data_u32(types.entities.tile, tile_index, types.properties.tile.province_index, i)
    end
  end

  print("#provinces        " .. #province_tiles)
  print("provinces         " .. count)
  print("province_min      " .. province_min)
  print("province_max      " .. province_max)
  print("count_more_max    " .. count_more_max)
  print("count_less_min    " .. count_less_min)
  print("accum_tiles_count " .. accum_tiles_count)
  print("more_max_avg      " .. count_more_max == 0 and accum_max / count_more_max or 0)
  print("less_min_avg      " .. count_less_min == 0 and accum_min / count_less_min or 0)
  print("final_avg         " .. accum_tiles_count / count)

  function_timer:finish()
end -- generate_provinces

local function check_unique_province_tiles(ctx)
  local map_tiles_count = ctx.map:tiles_count()
  local province_count = ctx.container:entities_count(types.entities.province)
  local unique_indices = utils.create_table(0, map_tiles_count)
  for i = 1, province_count do
    local tiles_count = ctx.container:get_childs_count(types.entities.province, i)
    for j = 1, tiles_count do
      local index = ctx.container:get_child(types.entities.province, i, j)
      assert(unique_indices[index] == nil)
      unique_indices[index] = true
    end
  end
end

local function province_postprocessing(ctx, _)
  local function_timer = generator.timer_t.new("province postprocessing")

  -- ищем подходящие неразмеченные области
  local tiles_count = ctx.map:tiles_count()
  local tile_free_area = utils.create_table(tiles_count, 0)
  local free_area_tile = utils.create_table(tiles_count, {})
  for current_tile_index = 1, tiles_count do
  repeat
    local valid_tile = is_tile_valid(ctx, current_tile_index)
    if not valid_tile then break end

    local prov_index = ctx.container:get_data_u32(types.entities.tile, current_tile_index, types.properties.tile.province_index)
    if prov_index ~= constants.uint32_max then break end -- continue

    local found = false
    for j = 1, #tile_free_area do
      local _, tile_index = get_index_pair(tile_free_area[j])
      if tile_index == current_tile_index then
        found = true
        break
      end
    end

    if found then break end -- continue

    local tiles_queue = queue.new()
    local unique_tiles = utils.create_table(0, 1000)
    do
      local current_free_area = #free_area_tile+1
      unique_tiles[current_tile_index] = true
      tiles_queue:push_right(make_index_pair(current_free_area, current_tile_index))

      table.insert(tile_free_area, make_index_pair(current_free_area, current_tile_index))
      free_area_tile[current_free_area] = utils.create_table(100, 0)
      table.insert(free_area_tile[current_free_area], current_tile_index)
    end

    while not tiles_queue:is_empty() do
      local current_free_area, tile_index = get_index_pair(tiles_queue:pop_left())
      local tile = ctx.map:get_tile(tile_index)
      for j = 1, tile.n_count do
      repeat
        local n_index = tile:get_neighbor_index(j)
        if unique_tiles[n_index] ~= nil then break end -- continue

        local valid_tile = is_tile_valid(ctx, n_index) -- luacheck: ignore valid_tile
        if not valid_tile then break end -- continue

        local prov_index = ctx.container:get_data_u32(types.entities.tile, n_index, types.properties.tile.province_index) -- luacheck: ignore prov_index
        assert(prov_index == constants.uint32_max) -- это 100%? я в этом сомневаюсь

        table.insert(free_area_tile[current_free_area], n_index)
        table.insert(tile_free_area, make_index_pair(current_free_area, n_index))
        unique_tiles[n_index] = true
        tiles_queue:push_right(make_index_pair(current_free_area, n_index))
      until true
      end -- for
    end -- while
  until true
  end -- for

  function_timer:checkpoint("free area found")

  local good_tiles_count = 0
  for i = 1, tiles_count do
    local valid_tile = is_tile_valid(ctx, i)
    good_tiles_count = good_tiles_count + bool_to_number(valid_tile)
  end

  local provinces_count = ctx.container:entities_count(types.entities.province)
  local province_tiles_avg = good_tiles_count / provinces_count
  local province_tiles_ratio = 0.75
  local province_tiles_min = province_tiles_ratio * province_tiles_avg
  local province_tiles_max = (2.0 - province_tiles_ratio) * province_tiles_avg

  print("provinces_count    " .. provinces_count)
  print("good_tiles_count   " .. good_tiles_count)
  print("province_tiles_avg " .. province_tiles_avg)
  print("#tile_free_area    " .. #tile_free_area)

  -- у нас на карте есть еще не все подходящие области раскиданы под провинции
  -- остаются еще безхозные однотайловые острова, и довольно крупные острова
  -- обходим все граунд пулы, чекаем есть ли среди них крупные сначало

  -- тут могут быть кусочки континентов на полюсах, которые чуть чуть пригодны для жизни
  -- и че с ними делать? в общем видимо нужно найти неразмеченные области вне зависимости ни от чего
  for i = 1, #free_area_tile do
    if #free_area_tile[i] >= province_tiles_min * 0.75 then

      -- области могут бы размерами гораздо большими чем обычная провинция, нужно ли что нибудь делать?
      --print("add free area " .. i .. " size " .. #free_area_tile[i])
      -- мы уже нашли по идее все области
      -- осталось только их добавить
      local new_province_index   = ctx.container:add_entity(types.entities.province)
      local new_province_n_index = ctx.container:add_entity(types.entities.province_neighbors)
      assert(new_province_index == new_province_n_index)
      for j = 1, #free_area_tile[i] do
        local tile_index = free_area_tile[i][j]
        ctx.container:add_child(types.entities.province, new_province_index, tile_index)
        ctx.container:set_data_u32(types.entities.tile, tile_index, types.properties.tile.province_index, new_province_index)
      end

    end -- if
  end -- for

  provinces_count = ctx.container:entities_count(types.entities.province)
  print("provinces_count    " .. provinces_count)
  function_timer:checkpoint("spawn new provinces")

  local small_islands_iteration_count = 15
  local small_islands_iteration = 0
  while small_islands_iteration < small_islands_iteration_count do
    small_islands_iteration = small_islands_iteration + 1
    for i = 1, #free_area_tile do
    repeat
      if #free_area_tile[i] >= province_tiles_min * 0.75 then break end -- continue
      local first_index = free_area_tile[i][1]
      local prov_index = ctx.container:get_data_u32(types.entities.tile, first_index, types.properties.tile.province_index)
      if prov_index ~= constants.uint32_max then break end -- continue

      -- нужно добавить к ближайшей провинции
      local tiles_queue = queue.new()
      local unique_tiles = utils.create_table(0, 1000)
      for j = 1, #free_area_tile[i] do
        local tile_index = free_area_tile[i][j]
        tiles_queue:push_right(make_index_pair(tile_index, 0))
        unique_tiles[tile_index] = true
      end

      local neighbor_province_index = -1
      while not tiles_queue:is_empty() and neighbor_province_index == -1 do
        local current_tile_index, dist = get_index_pair(tiles_queue:pop_left())

        if dist <= 5 then -- понятное дело нужно будет поменять это число если тайлов будет больше

          local tile = ctx.map:get_tile(current_tile_index)
          for j = 1, tile.n_count do
            local n_index = tile:get_neighbor_index(j)

            if unique_tiles[n_index] == nil then
              local temp_index = ctx.container:get_data_u32(types.entities.tile, n_index, types.properties.tile.province_index)
              if temp_index ~= constants.uint32_max then
                local valid_tile = is_tile_valid(ctx, n_index)
                assert(valid_tile)
                neighbor_province_index = temp_index
                break
              end

              tiles_queue:push_right(make_index_pair(n_index, dist+1))
              unique_tiles[n_index] = true
            end -- if
          end -- for
        end -- if
      end -- while

      if neighbor_province_index == -1 then break end -- continue

      for j = 1, #free_area_tile[i] do
        local tile_index = free_area_tile[i][j]
        ctx.container:set_data_u32(types.entities.tile, tile_index, types.properties.tile.province_index, neighbor_province_index)
        ctx.container:add_child(types.entities.province, neighbor_province_index, tile_index)
      end
    until true
    end -- for
  end -- while

  check_unique_province_tiles(ctx)

  local maxf = math.max
  local minf = math.min

  -- всякая информация
  local province_min = 1000000
  local province_max = 0
  local count = 0
  local count_more_max = 0
  local count_less_min = 0
  local accum_max = 0
  local accum_min = 0
  local accum_tiles_count = 0
  for i = 1, provinces_count do
    local province_tiles_count = ctx.container:get_childs_count(types.entities.province, i)
    province_min = minf(province_tiles_count, province_min)
    province_max = maxf(province_tiles_count, province_max)
    count = count + 1
    accum_tiles_count = accum_tiles_count + province_tiles_count

    if province_tiles_count > province_tiles_max then
      count_more_max = count_more_max + 1
      accum_max = accum_max + province_tiles_count
    end

    if province_tiles_count < province_tiles_min then
      count_less_min = count_less_min + 1
      accum_min = accum_min + province_tiles_count
    end
  end

  assert(accum_tiles_count <= good_tiles_count)

  print("provinces count   " .. provinces_count)
  print("province_min      " .. province_min)
  print("province_max      " .. province_max)
  print("count_more_max    " .. count_more_max)
  print("count_less_min    " .. count_less_min)
  print("accum_tiles_count " .. accum_tiles_count)
  print("more_max_avg      " .. count_more_max == 0 and accum_max / count_more_max or 0)
  print("less_min_avg      " .. count_less_min == 0 and accum_min / count_less_min or 0)
  print("final_avg         " .. accum_tiles_count / count)

  function_timer:finish()
end -- province_postprocessing

local function calculating_province_neighbors(ctx, _)
  local function_timer = generator.timer_t.new("province neighbors calculation")

  local max_neighbour_dist = 5

  local tiles_count = ctx.map:tiles_count()
  local provinces_count = ctx.container:entities_count(types.entities.province)
  local provinces_neighbours = utils.init_array(provinces_count, {})
  for i = 1, tiles_count do
    local current_province_index = ctx.container:get_data_u32(types.entities.tile, i, types.properties.tile.province_index)
    if current_province_index ~= constants.uint32_max then
      local tile = ctx.map:get_tile(i)
      for j = 1, tile.n_count do
      repeat
        local n_index = tile:get_neighbor_index(j)
        local n_province_index = ctx.container:get_data_u32(types.entities.tile, n_index, types.properties.tile.province_index)

        if n_province_index == constants.uint32_max then break end -- continue
        if current_province_index == n_province_index then break end -- continue

        -- эти условия не выполняются
        assert(current_province_index <= provinces_count)
        assert(n_province_index <= provinces_count)

        provinces_neighbours[current_province_index][n_province_index] = true
        provinces_neighbours[n_province_index][current_province_index] = true
      until true
      end -- for
    end -- if
  end -- for

  for i = 1, provinces_count do
    local tile_queue = queue.new()
    local unique_tiles = utils.create_table(0, tiles_count)

    local province_tiles = ctx.container:get_childs(types.entities.province, i)
    for j = 1, #province_tiles do
      local tile_index = province_tiles[j]
      unique_tiles[tile_index] = true

      local tile = ctx.map:get_tile(tile_index)
      for k = 1, tile.n_count do
        local n_index = tile:get_neighbor_index(k)

        local h = ctx.container:get_data_f32(types.entities.tile, n_index, types.properties.tile.elevation)
        if h < 0.0 then
          unique_tiles[n_index] = true
          tile_queue:push_right({n_index, i, 0})
        end -- if
      end -- for
    end -- for

    while not tile_queue:is_empty() do
      local current_tile_index, province_index, dist = table.unpack(tile_queue:pop_left())

      if dist <= max_neighbour_dist then
        local tile = ctx.map:get_tile(current_tile_index)
        for k = 1, tile.n_count do
        repeat
          local n_index = tile:get_neighbor_index(k)
          if unique_tiles[n_index] ~= nil then break end -- continue

          local h = ctx.container:get_data_f32(types.entities.tile, n_index, types.properties.tile.elevation)
          if h < 0.0 then
            tile_queue:push_right({n_index, province_index, dist+1})
            unique_tiles[n_index] = true
          else
            local found_province_index = ctx.container:get_data_u32(types.entities.tile, n_index, types.properties.tile.province_index)
            if found_province_index == constants.uint32_max then break end -- continue
            if provinces_neighbours[province_index][found_province_index] ~= nil then break end -- continue

            assert(i == province_index)
            local n = -found_province_index -- через воду, у нас наверняка будут коллизии с соседями по земле
            provinces_neighbours[province_index][-n] = true
          end
        until true
        end -- for
      end -- if
    end -- while
  end -- for

  for prov_index,t in ipairs(provinces_neighbours) do
    assert(prov_index >= 1 and prov_index <= provinces_count)
    for k,_ in pairs(t) do
      -- тут к нам может придти отрицательный индекс нужно его просто пихнуть в положительное число
      local n_index = pack_n_index(k)
      if k < 0 then assert(n_index >= constants.int32_max and n_index < constants.uint32_max) end
      ctx.container:add_child(types.entities.province_neighbors, prov_index, n_index)
    end
  end

  function_timer:finish()
end -- calculating_province_neighbors

-- скорее мы можем распространить культуры при генерации стран
local function generate_cultures(ctx, _)
  local function_timer = generator.timer_t.new("cultures generation")

  -- культуры, что по культурам?
  -- культуры распространяются почти по форме диаграмы воронного
  -- и довольно редко изменяются
  -- причем часть культур может выпасть на острова, но при этом
  -- попасть и немного на берег
  -- поэтому я не ошибусь если сгенерирую диаграму вороного просто по всей поверхности земли
  -- и по идее это должно будет выглядеть более менее с точки зрения распространения культур
  -- (либо рандом флудфил, по идее тоже неплохо будет выглядеть)

  -- следующий вопрос сколько культур генерировать?
  -- в еу4 существует 68 культурных груп
  -- и сколько-то (68*(от 4 до 7)) отдельных культур
  -- в цк2 25 культурных групп и примерно 121 отдельная культура
  -- (по 5 в среднем)
  -- на весь мир около 340 культур
  -- в общем нужно генерировать чуть больше чем заявленное количество культур
  -- + нужно удостовериться что культуры не будут вылезать из другого края карты
  -- (то есть вылезать за пределы ледяной шапки)

  -- если генерировать сразу много культур, то получается лучше
  -- возможно после предварительной генерации нужно будет соединить
  -- несколько областей (по аналогии с континентами)
  -- а затем снова разбить на непосредственно культуры
  -- нужно как то учесть водные пространства
  -- короче на самом деле культуры поди нужно доделать чуть после того как
  -- сделаны будут государства и религии, потому что они важнее

  local culture_groups_count = 70 * 5
  local tiles_count = ctx.map:tiles_count()

  local tiles_culture = utils.init_array(tiles_count, -1)
  local culture_tiles = utils.init_array(culture_groups_count, {})
  local culture_provinces = utils.init_array(culture_groups_count, {})
  local unique_tiles = utils.create_table(0, tiles_count)

  utils.int_random_queue(ctx.random:num(), culture_groups_count, function(index, queue_push)
    local tile_index = -1
    local attempts = 0

    while tile_index == -1 and attempts < 100 do
    repeat
      attempts = attempts + 1

      local rand_index = ctx.random:index(tiles_count)
      local current_tile_index = rand_index

      do
        local t = ctx.container:get_data_f32(types.entities.tile, current_tile_index, types.properties.tile.heat)
        if t <= 0.15 then break end -- continue
      end

      if unique_tiles[current_tile_index] ~= nil then break end -- continue

      local found = true
      local tile = ctx.map:get_tile(current_tile_index)
      for j = 1, tile.n_count do
        local n_index = tile:get_neighbor_index(j)
        local t = ctx.container:get_data_f32(types.entities.tile, n_index, types.properties.tile.heat)
        if t <= 0.15 then
          found = false
          break
        end

        if unique_tiles[n_index] ~= nil then
          found = false
          break
        end
      end

      if not found then break end -- continue

      tile_index = current_tile_index
      queue_push(tile_index)
      tiles_culture[tile_index] = index
      table.insert(culture_tiles[index], tile_index)
      unique_tiles[tile_index] = true
      for j = 1, tile.n_count do
        local n_index = tile:get_neighbor_index(j)
        unique_tiles[n_index] = true
      end
    until true
    end

    assert(tile_index ~= -1)
  end, function(current_tile_index, queue_push)
    local current_culture_index = tiles_culture[current_tile_index]
    local tile = ctx.map:get_tile(current_tile_index)
    for j = 1, tile.n_count do
      local n_index = tile:get_neighbor_index(j)
      if tiles_culture[n_index] == -1 then
        tiles_culture[n_index] = current_culture_index
        table.insert(culture_tiles[current_culture_index], n_index)
        queue_push(n_index)
      end
    end
  end)

  local provinces_count = ctx.container:entities_count(types.entities.province)
  for i = 1, provinces_count do
    ctx.container:set_data_u32(types.entities.province, i, types.properties.province.culture_id, constants.uint32_max)
  end

  -- рандомно разбросаны культуры по тайлам, что теперь можно придумать для того
  -- чтобы раскидать культуры по провинциям? для начала нужно проверить какая степень
  -- распространенности культуры и возможно что то с чем то соединить
  -- было бы неплохо развесовать культуры и дать шанс мелким культурам в спорных ситуациях
  local culture_wights = utils.init_array(culture_groups_count, 0)
  local accumulated_count = 0
  for i = 1, culture_groups_count do
    local count = 0
    for j = 1, #culture_tiles[i] do
    repeat
      local tile_index = culture_tiles[i][j]
      local v = is_tile_valid(ctx, tile_index)
      if not v then break end -- continue
      -- local h = ctx.container:get_data_f32(types.entities.tile, tile_index, types.properties.tile.elevation)
      -- if h < 0.0 then break end -- continue
      -- local t = ctx.container:get_data_f32(types.entities.tile, tile_index, types.properties.tile.heat)
      -- if t < 0.15 then break end -- continue
      count = count + 1
    until true
    end
    culture_wights[i] = count
    --max_count = math.max(max_count, count)
    accumulated_count = accumulated_count + count
  end

  for i = 1, culture_groups_count do
    culture_wights[i] = culture_wights[i] / accumulated_count
  end

  for i = 1, provinces_count do
    local childs_count = ctx.container:get_childs_count(types.entities.province, i)
    local cultures_table = {}
    for j = 1, childs_count do
      local tile_index = ctx.container:get_child(types.entities.province, i, j)
      local culture_id = tiles_culture[tile_index]
      if cultures_table[culture_id] == nil then cultures_table[culture_id] = 0 end
      cultures_table[culture_id] = cultures_table[culture_id] + 1
      --table.insert(cultures_table, { culture_id, 0 })
      --for k = 1, #cultures_table do
      --  if cultures_table[k][1] == culture_id then cultures_table[k][2] = cultures_table[k][2] + 1 end
      --end
    end

    -- наверное лучше добавить в массив? не
    local summ = 0
    for _,v in pairs(cultures_table) do
      --max = math.max(max, v)
      summ = summ + v
    end

    for k,v in pairs(cultures_table) do
      cultures_table[k] = v / summ
    end

    local province_culture = -1
    local attemps = 0
    while province_culture == -1 and attemps < 10 do
      attemps = attemps + 1
      for k,v in pairs(cultures_table) do
        local tiles_weight = culture_wights[k]
        local province_weight = v

        -- тут нужно что то с чем то замешать
        local final_tiles_weight = 1.0 - tiles_weight
        local final_k = (final_tiles_weight + province_weight) / 2.0
        local prob = ctx.random:probability(final_k)
        if prob then province_culture = k break end
      end
    end
    assert(province_culture ~= -1)

    table.insert(culture_provinces[province_culture], i)
    ctx.container:set_data_u32(types.entities.province, i, types.properties.province.culture_id, province_culture)
  end

  for i = 1, #tiles_culture do
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.culture_id, tiles_culture[i])
  end

  assert(#culture_tiles == #culture_provinces)
  ctx.container:clear_entities(types.entities.culture)
  ctx.container:set_entity_count(types.entities.culture, #culture_tiles)
  for i = 1, #culture_provinces do
    local local_table = culture_provinces[i]
    local size = #local_table
    for j = 1, size do
      ctx.container:add_child(types.entities.culture, i, local_table[j])
    end
  end

  local features = {
    horde = false,
    used_for_random = false,
    allow_in_ruler_designer = false,
    dukes_called_kings = false,
    baron_titles_hidden = false,
    count_titles_hidden = false,
    founder_named_dynasties = false,
    dynasty_title_names = false,
    disinherit_from_blinding = false,
    allow_looting = false,
    seafarer = false,
    dynasty_name_first = false,
    feminist = false,

    has_master_gender = true,
    has_lord_gender = true,
    has_king_gender = true,
    has_emperor_gender = true,
    has_hero_gender = true,
    has_wizard_gender = true,
    has_duke_gender = true,
    has_count_gender = true,
    has_heir_gender = true,
    has_prince_gender = true,
    has_baron_gender = true
  }

  local default_culture_opinion = { character_opinion = -15, popular_opinion = -15 }

  local culture_group_table = {
    id = "culture_group1",
    name_id = "culture_groups.culture_group1.name",
    description_id = "culture_groups.culture_group1.description",
    different_groups = default_culture_opinion,
    different_cultures = default_culture_opinion,
    different_child_cultures = default_culture_opinion,
  }

  utils.add_culture_group(culture_group_table)

  -- теперь нужно добавить культуры в игру
  for i = 1, culture_groups_count do
    local prov_array = culture_provinces[i]
    local prov_count = #prov_array
    for j = 1, prov_count do
      local prov_index = prov_array[j]
      ctx.container:set_data_u32(types.entities.province, prov_index, types.properties.province.culture_id, i)
    end

    local culture_table = {
      id = "culture" .. i,
      name_id = "cultures.culture1.name",
      description_id = "cultures.culture1.description",
      names_table_id = "cultures.english.character_names",
      -- пока с трудом понимаю как работают патронимы, видимо будет гораздо более трудная задача
      patronims_table_id = "cultures.culture1.patronim_table",
      additional_table_id = "cultures.culture1.additional_table",
      grandparent_name_chance = 0.2,
      group = "culture_group1",
      --parent = "",
      --image = "culture1_icon", -- должна ли у культур быть иконка? вряд ли
      color = utils.make_color(0.1, 0.1, 0.1, 1.0),
      bonuses = {},
      features = features,
      different_groups = default_culture_opinion,
      different_cultures = default_culture_opinion,
      different_child_cultures = default_culture_opinion,
      -- задаем культуру из провинции
      --provinces = culture_provinces[i]
    }

    utils.add_culture(culture_table)
  end

  function_timer:finish()
end -- generate_cultures

local history_step = { type = { end_of_empire = 1, becoming_empire = 2, count = 3 } }

local function generate_countries(ctx, local_table)
  local function_timer = generator.timer_t.new("countries generation")
  local maxf = math.max
  local minf = math.min
  local absf = math.abs

  local history = {}

  -- наверное нужно генерировать заного девелопмент и удачу
  local provinces_count = ctx.container:entities_count(types.entities.province)
  local province_country = utils.init_array(provinces_count, -1)
  local country_province = utils.init_array(provinces_count, {})
  local luckness = utils.init_array(provinces_count, 0.0)
  local development = utils.init_array(provinces_count, 0.0)
  for i = 1, #province_country do
    province_country[i] = i
    table.insert(country_province[i], i)
    luckness[i] = ctx.random:closed(0.1, 0.8)
    development[i] = ctx.random:closed(0.1, 0.8)
  end

  -- нужно раскидать провинции по культурам (ну или по культурным группам)
  -- каждая провинция получает по культурной группе, и каждый раунд мы должны посмотреть как
  -- заменится культура на новую, другое дело что есть еще и религии,
  -- и тут вообще они появлялись от балды, возможно имеет смысл их раскидывать также как страны
  -- религии правда разваливались крайне редко и развивались скорее в зависимости от своего престижа
  -- (то есть наличия крупных государств с которыми договаривались таким образом)
  -- как это влияние передать?

  local avg_temp = utils.init_array(provinces_count, 0.0)
  for i = 1, provinces_count do
    local tiles_count = ctx.container:get_childs_count(types.entities.province, i)
    for j = 1, tiles_count do
      local tile_index = ctx.container:get_child(types.entities.province, i, j)
      local t = ctx.container:get_data_f32(types.entities.tile, tile_index, types.properties.tile.heat)
      avg_temp[i] = avg_temp[i] + t
    end

    avg_temp[i] = avg_temp[i] / tiles_count
  end

  local iteration_count = local_table.userdata.history_iterations_count -- итераций должно быть явно больше
  local iteration = 0
  while iteration < iteration_count do
    iteration = iteration + 1

    -- обходим каждую страну, смотрим с каким шансом мы можем что захватить
    for country_index = 1, #country_province do
      if #country_province[country_index] ~= 0 then
        local country_luck = luckness[country_index]
        local country_development = development[country_index]

        -- большая удача и большее развитие - больший шанс что нибудь захватить
        -- но все равно при этом он не должен быть слишком большим
        -- нужно еще предусмотреть чтобы все это дело не вытягивалось в соплю
        -- а было как можно больше похожим на окружность

        -- находим соседей, какой шанс захватить провинцию соседа?
        -- по идее это должно быть маленькое число, около 0.1
        -- то есть примерно 0.05 * country_luck + 0.05 * country_development
        -- + какая то географическая состовляющая
        -- + должна учитываться удача и развитие противника

        local root_province = country_province[country_index][1]
        local avg_t = avg_temp[root_province]

        local current_size = #country_province[country_index]
        for j = 1, current_size do
          local province_index = country_province[country_index][j];

          local neighbors = ctx.container:get_childs(types.entities.province_neighbors, province_index)
          for k = 1, #neighbors do
          repeat
            local n_index_container = unpack_n_index(neighbors[k])
            local n_index = absf(n_index_container)

            local opposing_country = province_country[n_index]
            if country_index == opposing_country then break end -- continue

            local n_country_luck = luckness[opposing_country]
            local n_country_development = development[opposing_country]

            local final_luck = maxf(country_luck - n_country_luck, 0.1)
            local final_development = maxf(country_development - n_country_development, 0.1)
            local chance = 0.05 * final_luck * avg_t + 0.05 * final_development
            local probability = ctx.random:probability(chance)
            if not probability then break end -- continue

            local found = false
            for i = 1, #country_province[opposing_country] do
              if country_province[opposing_country][i] == n_index then
                country_province[opposing_country][i] = country_province[opposing_country][#country_province[opposing_country]]
                country_province[opposing_country][#country_province[opposing_country]] = nil
                found = true
                break
              end
            end

            assert(found)

            table.insert(country_province[country_index], n_index)
            province_country[n_index] = country_index
          until true
          end -- for
        end -- for

        if #country_province[country_index] > 80 then
          local index = -1
          for j = 1, #history do
            if history[j].t ~= history_step.type.end_of_empire and country_index == history[j].country then
              index = j
              break
            end
          end

          if index == -1 then
            index = #history+1
            history[index] = {
              t = history_step.type.count,
              country = country_index,
              size = 0,
              destroy_size = -1,
              empire_iteration = -1,
              end_iteration = -1,
              country_provinces = {}
            }
          end

          history[index].size = maxf(#country_province[country_index], history[index].size)
          if #country_province[country_index] > 100 and history[index].t ~= 2 then
            history[index].t = history_step.type.becoming_empire
            history[index].empire_iteration = iteration
          end

          -- ??
          -- нужно добавить все страны (провинции?)
          if history[index].t == history_step.type.becoming_empire then
            for k = 1, #country_province[country_index] do
              table.insert(history[index].country_provinces, country_province[country_index][k])
            end
          end
        end -- if
      end -- if
    end -- for

    -- в какой то момент мы должны обойти все государства и прикинуть примерно когда они должны развалиться
    -- среди государств есть более удачливые: они с большим шансом захватывают соседа и с меньшим шансом разваливаются
    -- где то здесь же я должен генерировать распространение религий
    for country_index = 1, #country_province do
    repeat
      if #country_province[country_index] == 0 then break end -- continue
      if #country_province[country_index] == 1 then break end -- continue
      local country_luck = 1.0 - luckness[country_index]
      local country_development = 1.0 - development[country_index]

      -- как развалить страну? по идее от девелопмента можно вычислить
      -- количество частей на которые она разваливается
      -- удача контролирует с каким шансом это дело развалится
      -- развал страны должен происходить от размера страны
      -- некий максимальный развер страны?

      local avg_t = avg_temp[country_province[country_index][1]]
      local size_k = minf(#country_province[country_index] / 100.0, 1.0)
      local final_k = country_luck * size_k * avg_t * 0.3 -- country_development ?

      local probability = ctx.random:probability(final_k)
      if not probability then break end

      local count = minf(maxf(#country_province[country_index] * country_development, 2), 30)
      assert(count ~= nil)

      local history_index = -1
      for j = 1, #history do
        if history[j].t ~= history_step.type.end_of_empire and country_index == history[j].country then
          history_index = j
          break
        end
      end

      if history_index ~= -1 then
        history[history_index].destroy_size = count
        history[history_index].t = history_step.type.end_of_empire
        history[history_index].end_iteration = iteration;
      end

      local local_country = country_province[country_index]
      country_province[country_index] = nil
      country_province[country_index] = utils.create_table(100, 0)
      for j = 1, #local_country do
        local prov_index = local_country[j]
        province_country[prov_index] = -1
      end

      -- некоторые провинции не получают своего государства и остаются UINT32_MAX

      local unique_provinces = {}
      utils.int_queue(count, function(_, queue_push)
        local attempts = 0
        local province_index = -1
        while attempts < 100 and province_index == -1 do
          attempts = attempts + 1
          local rand_index = ctx.random:index(#local_country)
          local country_province_index = local_country[rand_index]
          if unique_provinces[country_province_index] == nil then
            if #country_province[country_province_index] ~= 0 and country_province[country_province_index][1] == country_province_index then
              local s = #country_province[country_province_index]
              country_province[country_province_index] = nil
              country_province[country_province_index] = utils.create_table(s, 0)
            else
              local tmp = {}
              country_province[country_province_index], tmp = tmp, country_province[country_province_index]
              while #tmp ~= 0 do
                local first_province = tmp[1]
                for k = 1, #tmp do
                  local prov = tmp[k]
                  province_country[prov] = first_province
                end

                country_province[first_province], tmp = tmp, country_province[first_province]
                --assert(counter < 25)
              end -- while

              assert(#country_province[country_province_index] == 0)
            end -- if

            province_index = country_province_index
          end -- if
        end -- while

        assert(province_index ~= -1)

        table.insert(country_province[province_index], province_index)
        province_country[province_index] = province_index;
        queue_push(make_index_pair(province_index, province_index))
        unique_provinces[province_index] = true
      end, function(data, queue_push)
        local current_index, second_index = get_index_pair(data)

        local neighbours = ctx.container:get_childs(types.entities.province_neighbors, second_index)
        for j = 1, #neighbours do
        repeat
          local n_index = unpack_n_index(neighbours[j])
          n_index = absf(n_index)
          if province_country[n_index] ~= -1 then break end -- continue
          if unique_provinces[n_index] ~= nil then break end -- continue

          queue_push(make_index_pair(current_index, n_index))
          unique_provinces[n_index] = true
          province_country[n_index] = current_index
          table.insert(country_province[current_index], n_index)
        until true
        end
      end)

      for j = 1, #local_country do
      repeat
        local prov_index = local_country[j]
        -- такое может быть если у страны не осталось прямого выхода на эту провинцию
        -- наверное лучше ее возвращать обратно в свое государство
        if province_country[prov_index] ~= -1 then break end -- continue

        -- такого быть не должно
        if #country_province[prov_index] ~= 0 and country_province[prov_index][1] == prov_index then
          province_country[prov_index] = prov_index
          assert(#country_province[prov_index] == 1)
          break -- continue
        end

        local tmp = {}
        country_province[prov_index], tmp = tmp, country_province[prov_index]
        while #tmp ~= 0 do
          local first_province = tmp[1]
          for k = 1, #tmp do
            local prov = tmp[k]
            province_country[prov] = first_province
          end

          country_province[first_province], tmp = tmp, country_province[first_province]
          --assert(counter < 25);
        end

        assert(#country_province[prov_index] == 0)
        table.insert(country_province[prov_index], prov_index)
        province_country[prov_index] = prov_index;

        --luckness[prov_index] = ctx.random:norm()
        --development[prov_index] = ctx.random:norm()
      until true
      end -- for
    until true
    end -- for

  end -- while iteration < iteration_count do

  -- пытаемся найти какой стране принадлежат провинции по "рутовой" провинции
  for current_index = 1, #country_province do
  repeat
    if #country_province[current_index] == 0 then break end -- continue
    if country_province[current_index][1] == current_index then break end -- continue

    local root_index = country_province[current_index][1]
    country_province[current_index], country_province[root_index] = country_province[root_index], country_province[current_index]
    luckness[current_index], luckness[root_index] = luckness[root_index], luckness[current_index]
    development[current_index], development[root_index] = development[root_index], development[current_index]

    for j = 1, #country_province[current_index] do
      local prov_index = country_province[current_index][j]
      province_country[prov_index] = current_index
    end

    for j = 1, #country_province[root_index] do
      local prov_index = country_province[root_index][j]
      province_country[prov_index] = root_index
    end

    while current_index ~= -1 do
      -- NO REPEAT
      if #country_province[current_index] == 0 then break end
      local root_index = country_province[current_index][1] -- luacheck: ignore root_index
      if root_index == current_index then break end

      country_province[current_index], country_province[root_index] = country_province[root_index], country_province[current_index]
      luckness[current_index], luckness[root_index] = luckness[root_index], luckness[current_index]
      development[current_index], development[root_index] = development[root_index], development[current_index]
      for j = 1, #country_province[current_index] do
        local prov_index = country_province[current_index][j]
        province_country[prov_index] = current_index
      end

      for j = 1, #country_province[root_index] do
        local prov_index = country_province[root_index][j]
        province_country[prov_index] = root_index
      end

    end -- while
  until true
  end -- for current_index = 1, #country_province do

  -- множество провинций получаются оторваны от государств на суше
  -- нужно их либо выделить в отдельные государтсва
  -- возможно даже дать им дополнительных провинций
  -- либо добавить в государство их окружающее
  local new_countries = {}
  for i = 1, #country_province do
  repeat
    if #country_province[i] < 2 then break end -- continue
    local country_root = country_province[i][1]
    assert(country_root == i)
    for j = 2, #country_province[i] do
    repeat
      local province_index = country_province[i][j]

      local unique_provinces = {}
      local provinces_queue = queue.new();

      unique_provinces[province_index] = true
      -- интересно луа чистит память части массива в случае его постепенного удаления сначала
      provinces_queue:push_right(province_index)

      local found = false
      while not provinces_queue:is_empty() and not found do
        local current_province_index = provinces_queue:pop_left()

        local neighbours = ctx.container:get_childs(types.entities.province_neighbors, current_province_index)
        for k = 1, #neighbours do
          local n_index = unpack_n_index(neighbours[k])
          if n_index >= 0 and province_country[n_index] == i and unique_provinces[n_index] == nil then

            if n_index == country_root then
              found = true
              break
            end

            unique_provinces[n_index] = true
            provinces_queue:push_right(n_index);
          end -- if
        end -- for
      end -- while

      if found then break end -- continue

      -- нет прямого доступа к провинции
      -- это значит что эта провинция (и все возможные соседи)
      -- должны быть другим государством
      table.insert(new_countries, province_index)
    until true
    end -- for
  until true
  end -- for i = 1, #country_province do

  for i = 1, #new_countries do
    local province_index = new_countries[i]
    local old_country = province_country[province_index]
    assert(old_country ~= constants.uint32_max)
    local found = false
    for j = 1, #country_province[old_country] do
      if country_province[old_country][j] == province_index then
        country_province[old_country][j] = country_province[old_country][#country_province[old_country]]
        country_province[old_country][#country_province[old_country]] = nil
        found = true
        break
      end
    end

    assert(found)

    province_country[province_index] = constants.uint32_max
  end -- for

  for i = 1, #new_countries do
  repeat
    local province_index = new_countries[i]
    if province_country[province_index] ~= constants.uint32_max then break end -- continue

    local unique_provinces = {}
    local provinces_queue = queue.new()
    local new_country_provinces = {}
    unique_provinces[province_index] = true
    provinces_queue:push_right(province_index)
    table.insert(new_country_provinces, province_index)

    while not provinces_queue:is_empty() do
      local current_province_index = provinces_queue:pop_left()

      local neighbours = ctx.container:get_childs(types.entities.province_neighbors, current_province_index)
      for k = 1, #neighbours do
        local raw_n_index = unpack_n_index(neighbours[k])
        local n_index = absf(raw_n_index)
        if province_country[n_index] == constants.uint32_max and unique_provinces[n_index] == nil then
          unique_provinces[n_index] = true
          provinces_queue:push_right(n_index)
          table.insert(new_country_provinces, n_index)
        end
      end -- for
    end -- while

    for j = 1, #new_country_provinces do
      local index = new_country_provinces[j];
      assert(province_country[index] == constants.uint32_max)
      province_country[index] = province_index
    end

    country_province[province_index], new_country_provinces = new_country_provinces, country_province[province_index]
    assert(#new_country_provinces == 0)
  until true
  end --for

  local province_count = 0
  local country_count = 0
  local max_country = 0
  for country = 1, #country_province do
  repeat
    if #country_province[country] == 0 then break end -- continue

    province_count = province_count + #country_province[country]
    country_count = country_count + 1
    max_country = maxf(max_country, #country_province[country])
    assert(country_province[country][1] == country)
    for j = 1, #country_province[country] do
      local province = country_province[country][j];
      assert(province_country[province] == country)
    end
  until true
  end

  -- тут у нас примерно 6-7 стран с размерами больше 100
  -- + спавнятся однопровинчатые государства по середине более крупных
  -- мне нужно как то сильнее распределить провинции + что то сделать с
  -- однопровинчатыми государствами
  -- возможно эти крупные государства нужно как то развалить?
  -- минорки нужно видимо увеличить

  -- некоторые из крупных государств только каким то чудом не развалились
  -- (ну хотя я может быть напортачил с формулой)

  -- нужно сначало минорки определить
  -- то государство, которое мало по размеру и у которого только один крупный сосед
  -- если два соседа? в этом случае ситуация как у ростова в европке

  -- маленький размер - это сколько? может ли заспавнится большая страна внутри другой большой?
  -- тут скорее нужно проверить как то есть соседи у государства кроме другого
  -- если соседей = 1 и нет выхода к морю, то вот наш кандидат

  for country_index = 1, #country_province do
  repeat
    local neighbors = {}
    neighbors[country_index] = true
    for j = 1, #country_province[country_index] do
      local province_index = country_province[country_index][j]

      local neighbours_count = ctx.container:get_childs_count(types.entities.province_neighbors, province_index)
      for k = 1, neighbours_count do
        local raw_n_index = unpack_n_index(ctx.container:get_child(types.entities.province_neighbors, province_index, k))
        local n_index = absf(raw_n_index)
        local neighbour_country = province_country[n_index]
        neighbors[neighbour_country] = true
      end -- for
    end -- for

    local neighbors_size = 0
    for _,_ in pairs(neighbors) do neighbors_size = neighbors_size + 1 end
    assert(neighbors_size ~= 0)

    -- тут проблема в том что есть несколько провинций которые находятся внутри огромного государства
    -- как определить эти провинции? у нас есть несколько проблем,
    -- провинция просто может быть сильно внутри, но при этом соединена с другими провинциями
    -- типо можно посчитать количество стран
    if neighbors_size > 2 then break end -- continue

    -- нужно проверить выход к морю
    -- если он есть, то не все потеряно?
    local found = false
    for j = 1, #country_province[country_index] do
      local province_index = country_province[country_index][j]
      local tiles_count = ctx.container:get_childs_count(types.entities.province, province_index)
      for k = 1, tiles_count do
        local tile_index = ctx.container:get_child(types.entities.province, province_index, k)
        local tile = ctx.map:get_tile(tile_index)
        for c = 1, tile.n_count do
          local n_index = tile:get_neighbor_index(c)

          local h = ctx.container:get_data_f32(types.entities.tile, n_index, types.properties.tile.elevation)
          if h < 0.0 then
            found = true
            break
          end -- if
        end -- for

        if found then break end
      end -- for

      if found then break end
    end -- for

    -- вообще здесь мы еще можем исключить тех кто имеет выход к озеру
    if found then break end -- continue

    -- теперь у нас есть государства без выхода к морю и только с одним соседом
    -- что нужно сделать?
    -- если это минорка, нужно посмотреть расстояние до других государств

    if neighbors_size == 1 then break end -- если вообще нет соседей

    --print("country_index " .. country_index)
    --print("n " .. k)
    local opposing_country = -1
    for k,_ in pairs(neighbors) do opposing_country = k == country_index and opposing_country or k end

    assert(opposing_country ~= -1 and opposing_country ~= country_index)

    -- скорее всего в такую ситуацию может попасть не только одна провинция, но и более крупное государство

    if #country_province[country_index] < 2 then
      local first_province = country_province[country_index][1]
      province_country[first_province] = opposing_country
      table.insert(country_province[opposing_country], first_province)
      country_province[country_index] = {}
    else
      -- если провинций больше
      -- то может быть раздать этому государству провинций?

      local unique_provinces = {}
      local province_local = queue.new()
      local province_local2 = queue.new()
      for j = 1, #country_province[country_index] do
        local province_index = country_province[country_index][j]
        local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)
        for k = 1, #neighbours do
          local n_index = absf(unpack_n_index(neighbours[k]))
          province_local:push_right(n_index)
          unique_provinces[n_index] = true
        end
      end

      local new_n = -1
      while not province_local:is_empty() and new_n == -1 do
      repeat
        local province_index = province_local:pop_left()

        if province_country[province_index] == country_index then break end -- continue

        local index = province_country[province_index]
        assert(index == opposing_country)
        province_country[province_index] = country_index
        remove_from_array(country_province[index], province_index)

        table.insert(country_province[country_index], province_index)

        local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)
        for j = 1, #neighbours do
        repeat
          local n_index = absf(unpack_n_index(neighbours[j]))

          if province_country[n_index] == country_index then break end -- continue
          if unique_provinces[n_index] ~= nil then break end -- continue

          province_local2:push_right(n_index)
          unique_provinces[n_index] = true
        until true
        end

        while not province_local2:is_empty() do
          local province_index = province_local2:pop_left() -- luacheck: ignore province_index

          local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)  -- luacheck: ignore neighbours
          for j = 1, #neighbours do
            local n_index = absf(unpack_n_index(neighbours[j]))

            if province_country[n_index] ~= opposing_country then
              new_n = province_country[n_index]
              break
            end
          end

          province_local:push_right(province_index)
        end -- while
      until true
      end -- while


    end -- if
  until true
  end -- for country_index = 1, #country_province do

  print("new_countries  size " .. #new_countries)
  print("province_count calc " .. province_count)
  print("province_count      " .. #province_country)
  print("country_count       " .. country_count)
  print("max_country         " .. max_country)
  print("avg provinces       " .. province_count / country_count)

  for country = 1, #country_province do
  repeat
    if #country_province[country] < 100 then break end

    print("\n")
    print("country     " .. country)
    print("provinces   " .. #country_province[country])
    print("development " .. development[country])
    print("luck        " .. luckness[country])
    local root = country_province[country][1]
    local avg_t = avg_temp[root]
    print("avg_t       " .. avg_t)
  until true
  end

  for i = 1, ctx.map:tiles_count() do
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.country_index, constants.uint32_max)
  end

  for i = 1, #province_country do
    ctx.container:set_data_u32(types.entities.province, i, types.properties.province.country_index, province_country[i])
    local childs = ctx.container:get_childs(types.entities.province, i)
    for j = 1, #childs do
      local tile_index = childs[j]
      ctx.container:set_data_u32(types.entities.tile, tile_index, types.properties.tile.country_index, province_country[i])
    end
  end

  local new_country_counter = 0
  ctx.container:clear_entities(types.entities.country)
  ctx.container:set_entity_count(types.entities.country, #country_province)
  for i = 1, #country_province do
    local childs_count = ctx.container:get_childs_count(types.entities.country, i)
    assert(childs_count == 0)

    if #country_province[i] ~= 0 then
      for j = 1, #country_province[i] do
        ctx.container:add_child(types.entities.country, i, country_province[i][j])
      end
      new_country_counter = new_country_counter + 1
    end
  end

  local tiles_count_color = ctx.container:entities_count(types.entities.tile)
  for i = 1, tiles_count_color do
    local country_index = ctx.container:get_data_u32(types.entities.tile, i, types.properties.tile.country_index)
    local rand_num1 = utils.prng32(country_index)
    local rand_num2 = utils.prng32(rand_num1)
    local rand_num3 = utils.prng32(rand_num2)
    local color_r = utils.prng_normalize32(rand_num1)
    local color_g = utils.prng_normalize32(rand_num2)
    local color_b = utils.prng_normalize32(rand_num3)
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.color, utils.make_color(color_r, color_b, color_g, 1.0))
  end

  ctx.map:set_tiles_color(ctx.container, types.properties.tile.color)

  local_table.country_count = new_country_counter

  -- меня уже более менее устраивает результат
  -- на следующих шагах мне нужно как использовать историю сгененированную здесь
  -- причем, мне нужно будет определить какие-то государства как нестабильные
  -- и как стабильные, при начале игры нестабильные государства должны как то эпично разваливаться
  -- а стабильные большие государства должны представлять угрозу для игрока или быть легким уровнем сложности

  -- какие страны стабильные а какие нет? у меня довольно много государств спавнится в которых
  -- очень сложная география (то есть как будто целая страна нарезана на лоскуты)
  -- если граница большая? как проверить большую границу? идеальное государство - это такое
  -- которое имеет границу максимально подобную окружности
  -- периметр такой фигуры будет сильно меньше чем периметр произвольной фигуры
  -- помимо периметра есть еще неплохой способ: дальность от границы
  -- в окружности центр - это максимальная дальность
  -- и что мне нужно в итоге получить? по идее государства - это несколько вассалов
  -- (у которых есть свои вассалы и проч), в нестабильном государстве вассалы
  -- входят в разные оппозиционные фракции и разрывают страну на части
  -- в стабильном государстве вся такая деятельность подавлена
  -- и мне нужно сгенерировать по сути стартовый набор вассалов внутри государства
  -- со своими амбициями и желаниями
  -- у сильного государства вассалы либо подавлены либо юридически у них очень мало прав
  -- у слабого государства вассалы чувствуют себя лучше вплоть до игнорирования приказов сюзерена
  -- например, довольно сильное государство - византийская империя
  -- (имперская администрация как то ограничивала возможности вассалов)
  -- слабое государство - это франкская империя, которая после смерти императора развалилась на куски

  -- у меня есть какая то небольшая история (возможно нужно ее сделать больше, но это позже)
  -- в общем то теперь я плавно перехожу к геймплею
  -- для этого мне нужно: починить рендер, сделать несколько типов рендеров, интерфейс
  -- как сделать пошаговость?, заспавнить города, мне еще специальные постройки нужно поставить (шахты, данжи)
  -- и наверное нужно сразу разделить государства на
  -- земли кочевников, государства племен, и феодальные государства
  -- неплохо было заспавнить несколько технологических центров
  -- (ну то есть сделать условный китай и итальянские республики)

  -- нужно создать где то рядом персонажей и раздать им титулы

  function_timer:finish()
end -- generate_countries

-- сверху государств сгенерируем религии и культуры
-- честно говоря толком ничего не могу путного придумать для сложного генератора
-- самый адекватный способ на мой взгляд это просто пройтись рандом флуд фил или воронным
local function generate_religions(ctx, local_table)
  local function_timer = generator.timer_t.new("religions generation")
  -- сколько всего религий? можно сказать что несколько, их штук 10
  -- нужно по крайней мере разделить религии на мажорные и минорные
  -- мажорные это тип ислам, христиантсво и проч, минорные - иудаизм
  -- (несколько провок по миру + иногда может заспанится придворный с этой религией)
  -- еще можно сразу сгенерировать дочернии религии, осталось только понять как что генерировать
  -- было бы неплохо использовать информацию при генерации страны для генерации религий
  -- крупные страны должны с большей вероятностью получить новую религию
  -- но при этом должны быть группы крупных государств со сходной религией
  -- у нас причем должно быть довольно много религий которые находятся под контролем каких то
  -- империй другой религии
  local religions_count = 10

  local unique_indices = utils.create_table(0, religions_count)
  --local religions_table = utils.init_array(religions_count, {})
  local provinces_count = ctx.container:entities_count(types.entities.province)
  local religion_provinces = utils.init_array(religions_count, {})
  local province_religion = utils.init_array(provinces_count, -1)
  utils.int_queue(religions_count, function(index, queue_push)
    local province_index = -1
    local attempts = 0
    while province_index == -1 and attempts < 100 do
      attempts = attempts + 1
      local rand_index = ctx.random:index(provinces_count)
      if unique_indices[rand_index] == nil then
        province_index = rand_index
        local childs = ctx.container:get_childs(types.entities.province_neighbors, province_index)
        for i = 1, #childs do
          local n_province = math.abs(unpack_n_index(childs[i]))
          unique_indices[n_province] = true
        end
      end
    end

    if province_index == -1 then return end

    unique_indices[province_index] = true
    table.insert(religion_provinces[index], province_index)
    province_religion[province_index] = index
    queue_push(province_index)
  end, function(data, queue_push)
    local rel_index = province_religion[data]
    local childs_count = ctx.container:get_childs_count(types.entities.province_neighbors, data)
    for i = 1, childs_count do
      local child = ctx.container:get_child(types.entities.province_neighbors, data, i)
      local n_province = math.abs(unpack_n_index(child))
      if province_religion[n_province] == -1 then
        province_religion[n_province] = rel_index
        table.insert(religion_provinces[rel_index], n_province)
        queue_push(n_province)
      end
    end
  end)

  local hostile = {
    character_opinion = -20,
    popular_opinion = -30,
    intermarriage = true,
    title_usurpation = false,
    holy_wars = true
  }

  local evil = {
    character_opinion = -30,
    popular_opinion = -45,
    intermarriage = false,
    title_usurpation = false,
    holy_wars = true
  }


  local group_table = {
    id = "religion_group1",
    name_id = "religions.group1.name",
    description_id = "religions.group1.name",
    different_groups = evil,
    different_religions = evil,
    different_faiths = hostile
  }

  utils.add_religion_group(group_table)

  local function gen_color(val)
    local val1 = utils.prng32(val)
    local val2 = utils.prng32(val1)
    local val3 = utils.prng32(val2)
    local c1 = utils.prng_normalize32(val1)
    local c2 = utils.prng_normalize32(val2)
    local c3 = utils.prng_normalize32(val3)
    return utils.make_color(c1, c2, c3, 1.0)
  end

  for i = 1, provinces_count do
    ctx.container:set_data_u32(types.entities.province, i, types.properties.province.religion_id, constants.uint32_max)
  end

  -- так что теперь, я примерно раскинул провинции по религиям
  -- когда я раскидываю религии по людям нужно прикидывать сколько занимает религия провинций в стране
  -- существуют провинции в которых религий нет (острова например), для них нужно сгенерить одтельную религию
  for i = 1, #religion_provinces do
    local prov_array = religion_provinces[i]
    local prov_count = #prov_array
    for j = 1, prov_count do
      local prov_index = prov_array[j]
      ctx.container:set_data_u32(types.entities.province, prov_index, types.properties.province.religion_id, i)
    end

    -- "religion1"
    local rel_color = gen_color(i)
    local rel_table = {
      id = "religion" .. i,
      name_id = "religions.rel1.name",
      description_id = "religions.rel1.description",
      --parent = "parent_religion1", -- может не быть
      group = "religion_group1", -- группа должна существовать в любом случае
      aggression = 0.5, -- с агрессией пока что ничего не понятно
      crusade_name_id = "religions.rel1.crusade",
      holy_order_names_table_id = "religions.rel1.orders", -- указатель на таблицу
      scripture_name_id = "religions.rel1.scripture",
      good_gods_table_id = "religions.rel1.gods",
      evil_gods_table_id = "religions.rel1.evil_gods",
      high_god_name_id = "religions.rel1.crusade",
      piety_name_id = "religions.rel1.piety",
      priest_title_name_id = "religions.rel1.priest_title",
      reserved_male_names_table_id = "religions.rel1.male_names",
      reserved_female_names_table_id = "religions.rel1.female_names",
      color = rel_color,
      -- картинка
      bonuses = {}, -- тип стата, собственно бонус
      opinion_bonuses = {}, -- ???
      -- тут переопределяются данные из религиозной группы или из родителькой религии
      different_groups = evil,
      different_religions = evil,
      different_faiths = hostile,

      -- чет не подумал про это, нужно все же перенести в провинцию указание религии и культуры
      -- нужно переделать чутка
      --provinces = religion_provinces[i]
    }

    utils.add_religion(rel_table)
  end

  local current_size = #religion_provinces
  for i = 1, provinces_count do
  repeat
    local rel_index = ctx.container:get_data_u32(types.entities.province, i, types.properties.province.religion_id)
    if rel_index ~= constants.uint32_max then break end -- continue

    current_size = current_size + 1
    ctx.container:set_data_u32(types.entities.province, i, types.properties.province.religion_id, current_size)
    local rel_color = gen_color(i)
    local rel_table = {
      id = "religion" .. current_size,
      name_id = "religions.rel1.name",
      description_id = "religions.rel1.description",
      --parent = "parent_religion1", -- может не быть
      group = "religion_group1", -- группа должна существовать в любом случае
      aggression = 0.5, -- с агрессией пока что ничего не понятно
      crusade_name_id = "religions.rel1.crusade",
      holy_order_names_table_id = "religions.rel1.orders", -- указатель на таблицу
      scripture_name_id = "religions.rel1.scripture",
      good_gods_table_id = "religions.rel1.gods",
      evil_gods_table_id = "religions.rel1.evil_gods",
      high_god_name_id = "religions.rel1.crusade",
      piety_name_id = "religions.rel1.piety",
      priest_title_name_id = "religions.rel1.priest_title",
      reserved_male_names_table_id = "religions.rel1.male_names",
      reserved_female_names_table_id = "religions.rel1.female_names",
      color = rel_color,
      -- картинка
      bonuses = {}, -- тип стата, собственно бонус
      opinion_bonuses = {}, -- ???
    }

    utils.add_religion(rel_table)
  until true
  end
  function_timer:finish()
end

local function add_provinces_data(ctx, local_table)
  local function_timer = generator.timer_t.new("forming provinces data")
  -- в будущем нужно положить id культур и религий в local_table
  -- в принципе провинций не больше 5к будет + можно подчистить таблицу после этой функции
  local provinces_creation_count = ctx.container:entities_count(types.entities.province)
  print("provinces_creation_count" .. provinces_creation_count)
  for i = 1, provinces_creation_count do
    local culture_id = ctx.container:get_data_u32(types.entities.province, i, types.properties.province.culture_id)
    local religion_id = ctx.container:get_data_u32(types.entities.province, i, types.properties.province.religion_id)
    local province = {
      max_cities_count = 1,
      title = "baron" .. i .. "_title",
      culture = "culture" .. culture_id,
      religion = "religion" .. religion_id,
      neighbors = {},
      tiles = {}
    }

    local neighbors_count = ctx.container:get_childs_count(types.entities.province_neighbors, i)
    -- почему возвращается 0? соседей у тайла действительно может быть 0, а вот тайлы должны быть всегда
    for j = 1, neighbors_count do
      local index = ctx.container:get_child(types.entities.province_neighbors, i, j)
      local n = unpack_n_index(index)
      province.neighbors[j] = math.abs(n)
    end

    -- не туда тайлы добавлял =(
    local tiles_count = ctx.container:get_childs_count(types.entities.province, i)
    assert(tiles_count ~= 0, "no tiles, province " .. i)
    for j = 1, tiles_count do
      local index = ctx.container:get_child(types.entities.province, i, j)
      province.tiles[j] = index
    end

    utils.add_province(province)
  end

  function_timer:finish()
end

local function generate_heraldy(ctx, local_table) -- luacheck: ignore
  local function_timer = generator.timer_t.new("heraldy generation")

  utils.load_heraldy_layers("apates_quest/scripts/heraldies_config.lua")

  function_timer:finish()
end -- generate_heraldy

local function append_table(table1, table2)
  local count = #table2
  local cur_size = #table1
  for i = 1, count do
    table1[cur_size + i] = table2[i]
    --table.insert(table1, table2[i])
  end
end

local function generate_titles(ctx, local_table)
  local function_timer = generator.timer_t.new("titles generation")
  local maxf = math.max
  local minf = math.min
  local absf = math.abs

  -- титулы по идее должны как то учитывать сгенерированные государства
  -- сначало наверное нужно придумать механизм взаимодействия со строками
  -- должен быть некий банк строк либо алгоритм их вывода
  -- при совпадении типа и индекса выдается строка
  -- тип: строка для провинции, персонажа, титула и проч
  -- короче должен быть способ запоминать строки использованные в игре
  -- что делать с локализацией? нужны переменные локализации и
  -- зарегистрировать особые функции связанные с локализацией
  -- при обращении к ним получать строку, а потом ее запихивать в стек

  -- нашел хороший код который дал мне идею генератора (https://github.com/skeeto/fantasyname)
  -- основная задача генераторов названий: соединить несколько строк с определенными типами друг с другом
  -- например алфавит (арит|мерит)(нимир|риар) даст соответственно 4 имени
  -- помимо этого нужно добавить возможность делать первый символ большим, ставить символы в обратном порядке,
  -- брать символы из таблицы, пустой символ
  -- синтаксис !~<table_name|>(name_part|), так ли нужны таблицы?
  -- таблицы нужны прежде всего когда нужно повторить какой то паттерн
  -- вообще можно использовать % + символ для вызова таблиц
  -- (go%b|ro(bu|nu)|)~(%b|%a)!(byu|asd)
  -- не понял пока еще что такое Collapse Triples
  -- вроде как сделал

  -- еще нужно придумать способ задавать переменные в текст
  -- то есть: Дженерик текст %character_name% дженерик текст
  -- причем еще нужно продумать какую то возможность учитывать падежи
  -- вообще идеально как-то помечать какой то участок текста и выводить например тултип
  -- я могу так сделать разделив текст на несколько частей
  -- но это видимо что-то из разряда фантастики
  -- (наверное можно вычислить квадрат где находится этот участок)

  -- осталось понять как адекватно сделать базы со строками
  -- я думаю что нужно предсгенерировать много имен, положить их в массивы строк по типам
  -- и во время игры просто доставать их оттуда

  -- нужно сгенерировать геральдики: для этого нужно сделать и загрузить большое количество трафаретов
  -- трафарет - это, в моем случае, картинка с альфа каналом + зарегестрированы 4 цвета (чистые ргб и черный)
  -- мне нужны трафареты щитов, королевских животных, религиозных символик и проч
  -- все эти трафареты наверное нужно ужать до 128x128... какой размер использует цк2? 88х88 какой то такой (в атласе)

  -- нужно сгенерировать титулы герцогские (несколько баронств), королевские (несколько герцогств) и имперские (несколько королевств)
  -- возможно добавить еще одну иерархию титулов (что-то вроде благословления богини, но ее можно создать только внутриигровым эвентом)
  -- я еще не придумал как титул обратно получать, из провинции
  local provinces_count = ctx.container:entities_count(types.entities.province)
  local tiles_count = ctx.map:tiles_count()
  -- нужно посчитать тайлы соприкосновения
  local border_size_const = 5
  local neighbours_border_size = utils.init_array(provinces_count, {})
  do
    for i = 1, #neighbours_border_size do
      local n = ctx.container:get_childs(types.entities.province_neighbors, i)
      neighbours_border_size[i] = utils.init_array(#n, 0)
      for j = 1, #neighbours_border_size[i] do
        neighbours_border_size[i][j] = make_index_pair(n[j], 0) -- NO UNPACK
      end
    end

    local unique_border = {}
    for i = 1, tiles_count do
    repeat
      local province_index = ctx.container:get_data_u32(types.entities.tile, i, types.properties.tile.province_index)
      if province_index == constants.uint32_max then break end -- continue
      local tile = ctx.map:get_tile(i)
      for j = 1, tile.n_count do
      repeat
        local n_index = tile:get_neighbor_index(j)
        local n_province_index = ctx.container:get_data_u32(types.entities.tile, n_index, types.properties.tile.province_index)
        if n_province_index == constants.uint32_max then break end -- continue
        if province_index == n_province_index then break end -- continue

        local first_index = maxf(i, n_index)
        local second_index = minf(i, n_index)
        local key = make_index_pair(second_index, first_index);
        if unique_border[key] ~= nil then break end -- continue

        do
          local n_prov_index = -1
          local n = ctx.container:get_childs(types.entities.province_neighbors, province_index)
          for k = 1, #n do
            if absf(unpack_n_index(n[k])) == n_province_index then
              n_prov_index = k
              break
            end
          end

          assert(n_prov_index ~= -1)
          local prov_index, count = get_index_pair(neighbours_border_size[province_index][n_prov_index])
          neighbours_border_size[province_index][n_prov_index] = make_index_pair(prov_index, count + 1)
        end

        do
          local n_prov_index = -1
          local n = ctx.container:get_childs(types.entities.province_neighbors, n_province_index)
          for k = 1, #n do
            if absf(unpack_n_index(n[k])) == province_index then
              n_prov_index = k
              break
            end
          end

          assert(n_prov_index ~= -1)
          local prov_index, count = get_index_pair(neighbours_border_size[n_province_index][n_prov_index])
          neighbours_border_size[n_province_index][n_prov_index] = make_index_pair(prov_index, count + 1)
        end

        unique_border[key] = true
      until true
      end -- for
    until true
    end -- for

    for i = 1, #neighbours_border_size do
      table.sort(neighbours_border_size[i], function(first, second)
        local _, count1 = get_index_pair(first)
        local _, count2 = get_index_pair(second)
        return count1 > count2
      end)
    end
  end -- do

  -- герцогский титул это что? в цк2 в герцогстве находится до 6 баронств, в цк2 394 гергогств
  local duchies_count = provinces_count / 4 -- среднее количество находится в районе 3-4 (в цк2 почти 4)
  local unique_indices = utils.create_table(0, provinces_count)
  local duchies = utils.create_table(duchies_count, 0)
  utils.int_queue(duchies_count, function(index, queue_push) -- luacheck: ignore index
    local province_index = -1
    local attempts = 0
    while province_index == -1 and attempts < 100 do
      attempts = attempts + 1
      local rand_index = ctx.random:index(provinces_count)
      if unique_indices[rand_index] == nil then
        -- проверить соседей? соседей больше чем может быть в герцогстве баронств
        province_index = rand_index
      end
    end

    if province_index == -1 then return end

    unique_indices[province_index] = true
    local duchie_index = #duchies+1
    queue_push(make_index_pair(duchie_index, province_index))
    duchies[duchie_index] = utils.create_table(10, 0)
    table.insert(duchies[duchie_index], province_index)
  end, function(data, queue_push)
    local duchie_index, province_index = get_index_pair(data)
    local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)
    if #neighbours == 0 then return end
    if #duchies[duchie_index] > 5 then return end
    assert(#duchies[duchie_index] > 0)
    if #duchies[duchie_index] == 1 then
      local new_province_index = -1
      local attempts = 0
      while new_province_index == -1 and attempts < 15 do
      repeat
        attempts = attempts + 1
        local rand_index = ctx.random:index(#neighbours)
        local _, border_size = get_index_pair(neighbours_border_size[province_index][rand_index])
        if border_size < border_size_const then break end -- continue
        local n_province_index = unpack_n_index(neighbours[rand_index])
        if n_province_index < 0 then break end -- across_water
        if unique_indices[absf(n_province_index)] ~= nil then break end -- continue

        new_province_index = absf(n_province_index)
      until true
      end

      -- вообще ситуация возможна при которой мы не сможем взять рандомного соседа
      -- это герцогство тогда должно пойти в состав другого, или позаимствовать

      if new_province_index == -1 then return end

      unique_indices[new_province_index] = true
      -- тут наверное нужно опять просмотреть соседей текущего
      -- то, что соседи могут "закончится" и у текущей провинции и у соседней, кажется равновероятным
      queue_push(make_index_pair(duchie_index, new_province_index))
      table.insert(duchies[duchie_index], new_province_index)
    else
      for i = 1, #neighbours do
        local breakb = false
      repeat
        local prov_index, border_size = get_index_pair(neighbours_border_size[province_index][i])
        local n_province_index = unpack_n_index(prov_index)
        if border_size < border_size_const then break end
        if unique_indices[absf(n_province_index)] ~= nil then break end -- continue

        local found = false
        local n_neighbours = ctx.container:get_childs(types.entities.province_neighbors, absf(n_province_index))
        for j = 1, #n_neighbours do
        repeat
          local n_n_province_index = unpack_n_index(n_neighbours[j])
          if absf(n_n_province_index) == province_index then break end -- continue
          if n_n_province_index < 0 then break end -- continue

          for k = 1, #neighbours do
            if neighbours[k] == n_neighbours[j] then found = true break end
          end
        until true
        end

        if not found then break end -- continue

        unique_indices[absf(n_province_index)] = true
        queue_push(make_index_pair(duchie_index, absf(n_province_index)))
        table.insert(duchies[duchie_index], absf(n_province_index))
        breakb = true -- break
      until true
        if breakb then break end
      end -- for
    end -- if
  end) -- utils.int_queue

  do
    local duchies_provinces_count = 0
    for _,_ in pairs(unique_indices) do duchies_provinces_count = duchies_provinces_count + 1 end
    print("duchies provinces count " .. duchies_provinces_count)
    print("free provinces    count " .. provinces_count - duchies_provinces_count)
  end

  -- теперь нужно посмотреть чтобы было примерно равное распределение
  -- и наверное добавить свободные провинции

  for i = 1, #duchies do
  repeat
    if #duchies[i] >= 6 then break end -- continue

    local count = #duchies[i]
    for j = 1, count do
      local province_index = duchies[i][j]
      --if #duchies[i] >= 6 then break end

      local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)
      for k = 1, #neighbours do
      repeat
        if #duchies[i] >= 6 then break end -- continue
        local prov_index, border_size = get_index_pair(neighbours_border_size[province_index][k])
        local n_province_index = unpack_n_index(prov_index)
        if border_size < border_size_const then break end -- continue
        assert(absf(n_province_index) <= provinces_count)
        if n_province_index < 0 then break end -- across_water
        if unique_indices[absf(n_province_index)] ~= nil then break end -- continue

        unique_indices[absf(n_province_index)] = true
        table.insert(duchies[i], absf(n_province_index))
      until true
      end -- for k = 1, #neighbours do
    end -- for j = 1, count do
  until true
  end -- for i = 1, #duchies do

  do
    local duchies_provinces_count = 0
    for _,_ in pairs(unique_indices) do duchies_provinces_count = duchies_provinces_count + 1 end
    print("duchies provinces count " .. duchies_provinces_count)
  end

  -- добавляем остаток провинций в качестве отдельных герцогств
  local current_duchies_count = #duchies
  for province_index = 1, provinces_count do
    if unique_indices[province_index] == nil then
      table.insert(duchies, {province_index})
      unique_indices[province_index] = true
    end
  end

  do
    local duchies_provinces_count = 0
    for _,_ in pairs(unique_indices) do duchies_provinces_count = duchies_provinces_count + 1 end
    assert(duchies_provinces_count == provinces_count)
  end

  local province_duchy = utils.init_array(provinces_count, constants.uint32_max)
  for i = 1, #duchies do
    for j = 1, #duchies[i] do
      local p_index = duchies[i][j]
      province_duchy[p_index] = i
    end
  end

  -- отбираем у больших герцогств провинции в пользу малых (чаще всего это последние добавленные)
  for i = current_duchies_count+1, #duchies do
    assert(#duchies[i] == 1)

    local province_index = duchies[i][1]
    local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)
    local unique_indices = {} -- luacheck: ignore unique_indices
    for k = 1, #neighbours do
    repeat
      local prov_index, border_size = get_index_pair(neighbours_border_size[province_index][k])
      local n_province_index = unpack_n_index(prov_index);
      -- граница пролегает через воду
      if n_province_index < 0 then break end -- continue
      -- если граница между провинциями маленькая, то нам такая провинция ни к чему
      if border_size < border_size_const then break end -- continue
      local final_n_province_index = absf(n_province_index)
      local duchy_index = province_duchy[final_n_province_index]
      assert(duchy_index ~= constants.uint32_max)

      -- забираем только у 5 или 6-провинчатого соседа если такой имеется
      if #duchies[duchy_index] < 5 then break end -- continue
      if unique_indices[duchy_index] ~= nil then break end -- continue

      local size_before = #duchies[duchy_index]
      unique_indices[duchy_index] = true
      remove_from_array(duchies[duchy_index], final_n_province_index)
      assert(not array_contain(duchies[duchy_index], final_n_province_index))
      assert(#duchies[duchy_index] == size_before-1)

      table.insert(duchies[i], final_n_province_index)
      province_duchy[final_n_province_index] = i
    until true
    end -- for
  end -- for

  -- по прежнему могут остаться слишком маленькие герцогства
  -- в некоторых случаях это остров на одну провинцию
  -- в некоторых случайность
  -- такие герцогства можно соединить
  local duchies_count_stat_size = 7
  local duchies_count_stat = utils.init_array(duchies_count_stat_size, 0);
  local max_duchy = 0
  local min_duchy = 10
  local duchy_counter = 0
  for i = 1, #duchies do
  repeat
    if #duchies[i] == 0 then break end -- continue
    if #duchies[i] == 1 then duchies_count_stat[1] = duchies_count_stat[1] + 1 end
    if #duchies[i] == 2 then duchies_count_stat[2] = duchies_count_stat[2] + 1 end
    if #duchies[i] == 3 then duchies_count_stat[3] = duchies_count_stat[3] + 1 end
    if #duchies[i] == 4 then duchies_count_stat[4] = duchies_count_stat[4] + 1 end
    if #duchies[i] == 5 then duchies_count_stat[5] = duchies_count_stat[5] + 1 end
    if #duchies[i] == 6 then duchies_count_stat[6] = duchies_count_stat[6] + 1 end
    if #duchies[i] > 6  then duchies_count_stat[7] = duchies_count_stat[7] + 1 end

    duchy_counter = duchy_counter + 1
    max_duchy = maxf(max_duchy, #duchies[i])
    min_duchy = minf(min_duchy, #duchies[i])
  until true
  end

  print("duchies count  " .. duchy_counter)
  print("duchies size 1 " .. duchies_count_stat[1])
  print("duchies size 2 " .. duchies_count_stat[2])
  print("duchies size 3 " .. duchies_count_stat[3])
  print("duchies size 4 " .. duchies_count_stat[4])
  print("duchies size 5 " .. duchies_count_stat[5])
  print("duchies size 6 " .. duchies_count_stat[6])
  print("duchies size more 6 " .. duchies_count_stat[7])
  print("max duchy      " .. max_duchy)
  print("min duchy      " .. min_duchy)

  check_container_unique(duchies)

  -- однопровинчатые нужно передать минимальному соседу да и все
  for i = 1, #duchies do
  repeat
    if #duchies[i] == 0 then break end -- continue
    if #duchies[i] > 2 then break end -- continue

    local count = #duchies[i]
    for j = 1, count do
      local province_index = duchies[i][j];
      local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)
      local found = false
      for k = 1, #neighbours do
        local breakb = false
      repeat
        local prov_index, border_size = get_index_pair(neighbours_border_size[province_index][k])
        local n_province_index = unpack_n_index(prov_index)
        if n_province_index < 0 then break end -- continue
        if border_size < border_size_const then break end -- ???
        local n_duchy_index = province_duchy[absf(n_province_index)]

        if n_duchy_index == i then break end -- continue
        if #duchies[n_duchy_index] == 0 then break end -- continue

        local union_index = maxf(n_duchy_index, i)
        --local another = union_index == n_duchy_index and i or n_duchy_index
        local another = minf(n_duchy_index, i)
        assert(union_index ~= another)
        if #duchies[union_index] + #duchies[another] > 4 then break end -- continue

        append_table(duchies[union_index], duchies[another])
        for l = 1, #duchies[another] do
          local provice_index = duchies[another][l]
          province_duchy[provice_index] = union_index
        end

        duchies[another] = {}
        found = true
        breakb = true
      until true
        if breakb then break end
      end -- for

      if found then break end
    end -- for

  until true
  end -- for

  for i = 1, #duchies do
  repeat
    if #duchies[i] == 0 then break end -- continue
    if #duchies[i] ~= 1 then break end -- continue

    local province_index = duchies[i][1]
    local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)
    for k = 1, #neighbours do
      local breakb = false
    repeat
      local prov_index, _ = get_index_pair(neighbours_border_size[province_index][k])
      local n_province_index = unpack_n_index(prov_index)
      local n_duchy_index = province_duchy[absf(n_province_index)]

      if n_province_index < 0 then break end -- continue
      if n_duchy_index == i then break end -- continue
      if #duchies[n_duchy_index] == 0 then break end -- continue

      local union_index = maxf(n_duchy_index, i)
      --local another = union_index == n_duchy_index and i or n_duchy_index
      local another = minf(n_duchy_index, i)
      assert(union_index ~= another)
      if #duchies[union_index] + #duchies[another] > 6 then break end -- continue

      append_table(duchies[union_index], duchies[another])
      for l = 1, #duchies[another] do
        province_duchy[duchies[another][l]] = union_index
      end

      duchies[another] = {}
      breakb = true
    until true
      if breakb then break end
    end -- for
  until true
  end -- for

  check_container_unique(duchies)

  local original_duchies_size = #duchies
  -- do -- не понимаю почему это дает какую то ошибку
  --   local duchies_copy = {}
  --   for i = 1, #duchies do
  --     if #duchies[i] ~= 0 then
  --       table.insert(duchies_copy, duchies[i])
  --     end
  --   end
  --
  --   duchies = duchies_copy
  -- end

  duchies_count_stat = utils.init_array(duchies_count_stat_size, 0)
  max_duchy = 0
  min_duchy = 10
  duchy_counter = 0
  local check_duchies_count = 0
  for i = 1, #duchies do
  repeat
    if #duchies[i] == 0 then break end
    -- assert(#duchies[i] ~= 0)
    if #duchies[i] == 1 then duchies_count_stat[1] = duchies_count_stat[1] + 1 end
    if #duchies[i] == 2 then duchies_count_stat[2] = duchies_count_stat[2] + 1 end
    if #duchies[i] == 3 then duchies_count_stat[3] = duchies_count_stat[3] + 1 end
    if #duchies[i] == 4 then duchies_count_stat[4] = duchies_count_stat[4] + 1 end
    if #duchies[i] == 5 then duchies_count_stat[5] = duchies_count_stat[5] + 1 end
    if #duchies[i] == 6 then duchies_count_stat[6] = duchies_count_stat[6] + 1 end
    if #duchies[i] > 6  then duchies_count_stat[7] = duchies_count_stat[7] + 1 end

    check_duchies_count = check_duchies_count + #duchies[i]
    -- for j = 1, #duchies[i] do
    --   local index = duchies[i][j]
    --   --province_duchy[index] = duchy_counter
    --   province_duchy[index] = i
    -- end

    duchy_counter = duchy_counter + 1
    max_duchy = maxf(max_duchy, #duchies[i])
    min_duchy = minf(min_duchy, #duchies[i])
  until true
  end

  print("#duchies       " .. original_duchies_size)
  print("duchies count  " .. duchy_counter)
  print("duchies size 1 " .. duchies_count_stat[1])
  print("duchies size 2 " .. duchies_count_stat[2])
  print("duchies size 3 " .. duchies_count_stat[3])
  print("duchies size 4 " .. duchies_count_stat[4])
  print("duchies size 5 " .. duchies_count_stat[5])
  print("duchies size 6 " .. duchies_count_stat[6])
  print("duchies size more 6 " .. duchies_count_stat[7])
  print("max duchy      " .. max_duchy)
  print("min duchy      " .. min_duchy)

  do
    local size = #duchies
    local duchies_copy = utils.create_table(size, 0)
    local counter = 0
    for i = 1, size do
      local inner_size = #duchies[i]
      if inner_size > 0 then
        counter = counter + 1
        duchies_copy[counter] = duchies[i]
        for j = 1, inner_size do
          local index = duchies_copy[counter][j]
          province_duchy[index] = counter
        end
      end
    end

    duchies = duchies_copy
  end

  local prov_count = ctx.container:entities_count(types.entities.province)
  assert(check_duchies_count == prov_count)

  for i = 1, ctx.container:entities_count(types.entities.tile) do
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.duchie_index, constants.uint32_max)
  end

  for i = 1, #province_duchy do
    local duchy_index = province_duchy[i]
    local childs = ctx.container:get_childs(types.entities.province, i)
    for j = 1, #childs do
      local tile_index = childs[j]
      ctx.container:set_data_u32(types.entities.tile, tile_index, types.properties.tile.duchie_index, duchy_index)
    end
  end

  function_timer:checkpoint("duchies generated")

  -- в duchies есть пустые герцогства
  --assert(#duchies == duchy_counter)
  local duchies_neighbours = utils.init_array(#duchies, {})
  do
    local unique_border = utils.create_table(0, #duchies)
    for i = 1, tiles_count do
    repeat
      local duchy_index = ctx.container:get_data_u32(types.entities.tile, i, types.properties.tile.duchie_index)
      if duchy_index == constants.uint32_max then break end -- continue
      assert(#duchies[duchy_index] ~= 0)
      local tile = ctx.map:get_tile(i)
      for j = 1, tile.n_count do
      repeat
        local n_index = tile:get_neighbor_index(j)
        local n_duchy_index = ctx.container:get_data_u32(types.entities.tile, n_index, types.properties.tile.duchie_index)
        if n_duchy_index == constants.uint32_max then break end -- continue
        if duchy_index == n_duchy_index then break end -- continue
        assert(#duchies[n_duchy_index] ~= 0)

        local first_index = maxf(i, n_index)
        local second_index = minf(i, n_index)
        local key = make_index_pair(second_index, first_index)
        if unique_border[key] ~= nil then break end -- continue

        do
          local index = find_index(duchies_neighbours[duchy_index], function(data)
            local first,_ = get_index_pair(data)
            return first == n_duchy_index
          end)

          if index ~= -1 then
            local first, second = get_index_pair(duchies_neighbours[duchy_index][index])
            duchies_neighbours[duchy_index][index] = make_index_pair(first, second+1)
          else table.insert(duchies_neighbours[duchy_index], make_index_pair(n_duchy_index, 1)) end
        end

        do
          local index = find_index(duchies_neighbours[n_duchy_index], function(data)
            local first,_ = get_index_pair(data)
            return first == duchy_index
          end)

          if index ~= -1 then
            local first, second = get_index_pair(duchies_neighbours[n_duchy_index][index])
            duchies_neighbours[n_duchy_index][index] = make_index_pair(first, second+1)
          else table.insert(duchies_neighbours[n_duchy_index], make_index_pair(duchy_index, 1)) end
        end

        unique_border[key] = true
      until true
      end -- for
    until true
    end -- for

    for i = 1, #duchies_neighbours do
      table.sort(duchies_neighbours[i], function(first, second)
        local _, count1 = get_index_pair(first)
        local _, count2 = get_index_pair(second)
        return count1 > count2
      end)
    end
  end

  -- теперь нужно создать королевства
  -- проблема заключается в том что не существует какого то конвенционального числа герцогств в королевстве
  -- в цк2 королеских титулов 116, примерно 4 герцогства на каждое
  -- но разброс от 2 герцогства до 11, как быть
  -- там еще есть множество формальных королевских титулов
  -- но с этим еще более менее понятно

  -- я так понимаю, нужно сначала заспавнить половину (2/3? 3/4?) из всех королевств
  -- их заполнить до какого то предела (например 12 герцогств), а затем попытаться создать
  -- оставшееся количество титулов, все королевства с одним герцогством удалить и раздать существующим
  -- малые королевства соединить (непохо было бы с учетом длинны границы)
  -- герцогства по примерно такому же принципу более менее нормально генерируются

  local king_titles_count_max = duchy_counter / 4
  local king_titles_count = king_titles_count_max * (2.0 / 3.0)
  local unique_duchies = {}
  local king_queue = queue.new()
  local kingdoms = utils.init_array(king_titles_count, {})
  local duchy_kingdom = utils.init_array(duchy_counter, -1)

  local kingdom_func = function()
    assert(not king_queue:is_empty())
    while not king_queue:is_empty() do
    repeat
      local king_index, kingdom_duchy_index, current_n_index = table.unpack(king_queue:pop_left())

      if kingdom_duchy_index > #kingdoms[king_index] then break end -- continue
      local current_duchy_index = kingdoms[king_index][kingdom_duchy_index]

      assert(current_duchy_index <= #duchies_neighbours)

      -- нужно еще проверить случайного соседа
      -- нужно проверить соседей осортированных по длине границы герцогства (проверяю)
      -- по длине границы получаются в основном неплохие королевства
      local found = false
      for n_index = current_n_index, #duchies_neighbours[current_duchy_index] do
        local index, count = get_index_pair(duchies_neighbours[current_duchy_index][n_index])
        if count >= 6 and duchy_kingdom[index] == -1 then
          if current_n_index < #duchies_neighbours[current_duchy_index] then
            king_queue:push_right({king_index, kingdom_duchy_index, current_n_index+1})
          else
            king_queue:push_right({king_index, kingdom_duchy_index+1, 1})
          end

          assert(#duchies[index] ~= 0)
          found = true
          table.insert(kingdoms[king_index], index)
          assert(duchy_kingdom[index] == -1)
          duchy_kingdom[index] = king_index
        end
      end

      if not found then king_queue:push_right({king_index, kingdom_duchy_index+1, 1}) end
    until true
    end
  end

  for i = 1, king_titles_count do
    local duchy_index = -1
    local attempts = 0
    while duchy_index == -1 and attempts < 100 do
    repeat
      attempts = attempts + 1
      local rand_index = ctx.random:index(#duchies)
      --assert(#duchies[rand_index] ~= 0)
      if #duchies[rand_index] == 0 then break end -- continue
      if unique_duchies[rand_index] ~= nil then break end -- continue

      duchy_index = rand_index
    until true
    end

    assert(duchy_index ~= -1)

    unique_duchies[duchy_index] = true
    king_queue:push_right({i, 1, 1})
    table.insert(kingdoms[i], duchy_index)
    duchy_kingdom[duchy_index] = i

    for j = 1, #duchies_neighbours[duchy_index] do
      local index, _ = get_index_pair(duchies_neighbours[duchy_index][j])
      unique_duchies[index] = true
    end
  end

  kingdom_func()

  -- неразмеченные большие герцогства раздаем в новые королевства
  for i = 1, #duchy_kingdom do
  repeat
    if duchy_kingdom[i] ~= -1 then break end -- continue
    if #duchies[i] ~= 6 then break end -- continue

    local k_index = #kingdoms+1
    duchy_kingdom[i] = k_index
    kingdoms[k_index] = utils.create_table(10, 0)
    table.insert(kingdoms[k_index], i)
    king_queue:push_right({duchy_kingdom[i], 1, 1})
  until true
  end

  for i = 1, #duchy_kingdom do
  repeat
    if duchy_kingdom[i] ~= -1 then break end -- continue
    if #duchies[i] < 4 then break end -- continue
    if #duchies[i] == 0 then break end -- continue

    local found = false
    for j = 1, #duchies[i] do
      local province_index = duchies[i][j]
      assert(province_duchy[province_index] == i)

      local n = ctx.container:get_childs(types.entities.province_neighbors, province_index)
      for k = 1, #n do
      repeat
        local n_province_index = unpack_n_index(n[k])
        local n_duchy_index = province_duchy[absf(n_province_index)]
        if n_duchy_index == i then break end -- continue
        -- здесь ранее стоял 'i', но это не имело смысл, это условие мы проверили выше
        if duchy_kingdom[n_duchy_index] ~= -1 then break end -- continue
        if #duchies[n_duchy_index] == 0 then break end -- continue
        found = true
      until true
      end
    end

    if not found then break end

    local k_index = #kingdoms+1
    duchy_kingdom[i] = k_index
    kingdoms[k_index] = utils.create_table(10, 0)
    table.insert(kingdoms[k_index], i)
    king_queue:push_right({duchy_kingdom[i], 1, 1})
  until true
  end -- for

  if not king_queue:is_empty() then kingdom_func() end

  -- пустые королевства?

  do
    local max_kingdoms = 0
    local min_kingdoms = 235525
    local max_kingdoms_provinces = 0
    local min_kingdoms_provinces = 34363436
    local kingdoms2_count = 0
    local province_count = utils.init_array(#kingdoms, 0)
    local counter = 0
    for i = 1, #kingdoms do
      assert(#kingdoms[i] ~= 0)
      max_kingdoms = maxf(max_kingdoms, #kingdoms[i])
      min_kingdoms = minf(min_kingdoms, #kingdoms[i])

      for k = 1, #kingdoms[i] do
        local duchy_index = kingdoms[i][k]
        assert(#duchies[duchy_index] ~= 0)
        for _ = 1, #duchies[duchy_index] do
          province_count[i] = province_count[i] + 1
        end
      end

      counter = counter + province_count[i]
      max_kingdoms_provinces = maxf(max_kingdoms_provinces, province_count[i])
      min_kingdoms_provinces = minf(min_kingdoms_provinces, province_count[i])
      if #kingdoms[i] == 1 then kingdoms2_count = kingdoms2_count + 1 end
    end

    print("kingdoms_count  " .. #kingdoms)
    print("max_kingdoms    " .. max_kingdoms)
    print("min_kingdoms    " .. min_kingdoms)
    print("kingdoms1_count " .. kingdoms2_count)
    print("avg kingdoms count " .. #duchy_kingdom / #kingdoms)
    print("max_kingdoms_provinces " .. max_kingdoms_provinces)
    print("min_kingdoms_provinces " .. min_kingdoms_provinces)
    print("avg kingdoms provinces count " .. counter / #kingdoms)
  end -- do

  do
    local size = #kingdoms
    local kingdoms_copy = utils.create_table(size, 0)
    local counter = 0
    for i = 1, size do
      local inner_size = #kingdoms[i]
      if inner_size > 0 then
        counter = counter + 1
        kingdoms_copy[counter] = kingdoms[i]
        for j = 1, inner_size do
          local index = kingdoms_copy[counter][j]
          duchy_kingdom[index] = counter
        end
      end
    end

    kingdoms = kingdoms_copy
  end

  check_container_unique(kingdoms)

  local province_kingdom = utils.init_array(provinces_count, constants.uint32_max)
  for i = 1, #kingdoms do
    for j = 1, #kingdoms[i] do
      local duchy_index = kingdoms[i][j]
      assert(#duchies[duchy_index] ~= 0)
      for k = 1, #duchies[duchy_index] do
        local provice_index = duchies[duchy_index][k]
        province_kingdom[provice_index] = i;
      end
    end
  end

  for i = 1, tiles_count do
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.kingship_index, constants.uint32_max);
  end

  for i = 1, #province_kingdom do
    local kingdom_index = province_kingdom[i]
    local childs = ctx.container:get_childs(types.entities.province, i)
    for j = 1, #childs do
      local tile_index = childs[j]
      ctx.container:set_data_u32(types.entities.tile, tile_index, types.properties.tile.kingship_index, kingdom_index)
    end
  end

  function_timer:checkpoint("kingdoms generated")

  -- теперь получается более менее нормально, почти все королевские титулы правильной формы

  -- нужно сгенерить имперские титулы (в цк2 26 реальных имперских титулов)
  -- тот же принцип, находим соседей с протяженной границей, сортируем

  local kingdoms_neighbours = utils.init_array(#kingdoms, {});
  do
    local unique_border = {}
    for i = 1, tiles_count do
    repeat
      local kingdom_index = ctx.container:get_data_u32(types.entities.tile, i, types.properties.tile.kingship_index)
      if kingdom_index == constants.uint32_max then break end -- continue
      local tile = ctx.map:get_tile(i)
      for j = 1, tile.n_count do
      repeat
        local n_index = tile:get_neighbor_index(j)
        local n_kingdom_index = ctx.container:get_data_u32(types.entities.tile, n_index, types.properties.tile.kingship_index)
        if n_kingdom_index == constants.uint32_max then break end -- continue
        if kingdom_index == n_kingdom_index then break end -- continue

        local first_index = maxf(i, n_index)
        local second_index = minf(i, n_index)
        local key = make_index_pair(second_index, first_index)
        if unique_border[key] ~= nil then break end -- continue

        do
          local index = find_index(kingdoms_neighbours[kingdom_index], function(data)
            local index, _ = get_index_pair(data)
            return index == n_kingdom_index
          end)

          if index == -1 then table.insert(kingdoms_neighbours[kingdom_index], make_index_pair(n_kingdom_index, 1))
          else
            local id, count = get_index_pair(kingdoms_neighbours[kingdom_index][index])
            kingdoms_neighbours[kingdom_index][index] = make_index_pair(id, count+1)
          end
        end

        do
          local index = find_index(kingdoms_neighbours[n_kingdom_index], function(data)
            local index, _ = get_index_pair(data)
            return index == kingdom_index
          end)

          if index == -1 then table.insert(kingdoms_neighbours[n_kingdom_index], make_index_pair(kingdom_index, 1))
          else
            local id, count = get_index_pair(kingdoms_neighbours[n_kingdom_index][index])
            kingdoms_neighbours[n_kingdom_index][index] = make_index_pair(id, count+1)
          end
        end

        unique_border[key] = true
      until true
      end -- for
    until true
    end -- for

    for i = 1, #kingdoms_neighbours do
      table.sort(kingdoms_neighbours[i], function(first, second)
        local _, count1 = get_index_pair(first)
        local _, count2 = get_index_pair(second)
        return count1 > count2
      end)
    end
  end -- do

  local emperor_titles_count_max = #kingdoms / 4
  local emperor_titles_count = emperor_titles_count_max -- * (2.0/3.0)
  local emperor_queue = queue.new()
  local empires = utils.init_array(emperor_titles_count, {})
  local kingdom_empire = utils.init_array(#kingdoms, constants.uint32_max)

  local empire_func = function()
    assert(not emperor_queue:is_empty())
    while not emperor_queue:is_empty() do
    repeat
      local emperor_index, empire_kingdom_index, current_n_index = table.unpack(emperor_queue:pop_left())

      if empire_kingdom_index > #empires[emperor_index] then break end -- continue
      local current_kingdom_index = empires[emperor_index][empire_kingdom_index]

      assert(current_kingdom_index <= #kingdoms_neighbours)

      -- нужно еще проверить случайного соседа
      -- нужно проверить соседей осортированных по длине границы герцогства (проверяю)
      -- по длине границы получаются в основном неплохие королевства
      local found = false
      for n_index = current_n_index, #kingdoms_neighbours[current_kingdom_index] do
        local index, count = get_index_pair(kingdoms_neighbours[current_kingdom_index][n_index])
        if count >= 6 and kingdom_empire[index] == constants.uint32_max then
          if n_index < #kingdoms_neighbours[current_kingdom_index] then
            emperor_queue:push_right({emperor_index, empire_kingdom_index, n_index+1})
          else
            emperor_queue:push_right({emperor_index, empire_kingdom_index+1, 1})
          end

          assert(#kingdoms[index] ~= 0)
          found = true
          assert(kingdom_empire[index] == constants.uint32_max)
          table.insert(empires[emperor_index], index)
          kingdom_empire[index] = emperor_index
        end
      end

      if not found then emperor_queue:push_right({emperor_index, empire_kingdom_index+1, 1}) end
    until true
    end
  end

  do
    local unique_kingdoms = {}
    for i = 1, #empires do
      local attempts = 0
      local kingdom_index = -1
      while kingdom_index == -1 and attempts < 100 do
        attempts = attempts + 1
        local rand_index = ctx.random:index(#kingdoms)
        if unique_kingdoms[rand_index] == nil then
          kingdom_index = rand_index
        end
      end

      assert(kingdom_empire[kingdom_index] == constants.uint32_max)
      unique_kingdoms[kingdom_index] = true
      table.insert(empires[i], kingdom_index)
      kingdom_empire[kingdom_index] = i
      emperor_queue:push_right({i, 1, 1})

      for j = 1, #kingdoms_neighbours[kingdom_index] do
        local index, _ = get_index_pair(kingdoms_neighbours[kingdom_index][j])
        unique_kingdoms[index] = true
      end
    end
  end

  empire_func()

  check_container_unique(empires)

  -- у нас остаются империи с одним королевством, мы их либо добавляем соседям либо удаляем
  -- наверное империи должны быть минимум с 3-мя королевствами
  for i = 1, #empires do
  repeat
    if #empires[i] == 0 then break end -- continue
    if #empires[i] > 2 then break end -- continue
    local old_table = utils.create_table(10, 0)
    old_table, empires[i] = empires[i], old_table
    assert(#empires[i] == 0)
    local new_emp = -1
    for k = 1, #old_table do
      local kingdom_index = old_table[k]

      assert(kingdom_index >= 1 and kingdom_index <= #kingdoms_neighbours, "kingdom_index " .. kingdom_index)
      --local found = false
      for j = 1, #kingdoms_neighbours[kingdom_index] do
        local index, _ = get_index_pair(kingdoms_neighbours[kingdom_index][j])
        local emp = kingdom_empire[index]
        if emp ~= constants.uint32_max and #empires[emp] >= 3 and #empires[emp] < 10 then
          --for g = 1, #old_table do kingdom_empire[old_table[g]] = new_emp end
          --kingdom_empire[kingdom_index] = new_emp
          --append_table(empires[new_emp], old_table)
          --empires[i] = nil
          --empires[i] = utils.create_table(10, 0)
          --found = true
          new_emp = emp
        end
      end

      --if found then break end -- break (?)

      -- удаляем малую империю
      --empires[i][k] = true
      --remove_from_array(empires[i], kingdom_index)
      --kingdom_empire[kingdom_index] = constants.uint32_max
    end -- for

    if new_emp == -1 then
      for g = 1, #old_table do kingdom_empire[old_table[g]] = constants.uint32_max end
    else
      for g = 1, #old_table do kingdom_empire[old_table[g]] = new_emp end
      append_table(empires[new_emp], old_table)
    end
  until true
  end -- for

  for kingdom_index = 1, #kingdom_empire do
  repeat
    if kingdom_empire[kingdom_index] ~= constants.uint32_max then break end -- continue

    local found = 0
    for j = 1, #kingdoms_neighbours[kingdom_index] do
      local n_kingdom_index, _ = get_index_pair(kingdoms_neighbours[kingdom_index][j])
      if kingdom_empire[n_kingdom_index] == constants.uint32_max then

      found = found + 1
      end
    end

    if found < 2 then break end -- continue

    local new_emp = #empires+1
    empires[new_emp] = utils.create_table(10, 0)
    local size = 1
    empires[new_emp][size] = kingdom_index
    kingdom_empire[kingdom_index] = new_emp
    emperor_queue:push_right({new_emp, 1, 1})
    for j = 1, #kingdoms_neighbours[kingdom_index] do
      local n_kingdom_index, _ = get_index_pair(kingdoms_neighbours[kingdom_index][j])
      if kingdom_empire[n_kingdom_index] == constants.uint32_max then
        size = size + 1
        empires[new_emp][size] = n_kingdom_index
        kingdom_empire[n_kingdom_index] = new_emp
      end
    end
  until true
  end -- for

  check_container_unique(empires)

  if not emperor_queue:is_empty() then empire_func() end

  check_container_unique(empires)

  -- где то в этом месте у меня видимо портятся индексы
  -- do
  --   local empires_copy = utils.create_table(#empires, 0)
  --   for i = 1, #empires do
  --     if #empires[i] > 0 then
  --       table.insert(empires_copy, empires[i])
  --     end
  --   end
  --
  --   empires = empires_copy
  -- end

  do
    local max_empires = 0
    local min_empires = 235525
    local max_provinces_empires = 0
    local min_provinces_empires = 45366737
    local empires1_count = 0
    local empires11_count = 0
    local province_count = utils.init_array(#empires, 0)
    local counter = 0
    local e_counter = 0
    for i = 1, #empires do
    repeat
      --assert(#empires[i] ~= 0)
      if #empires[i] == 0 then break end -- continue

      for j = 1, #empires[i] do
        for k = 1, #kingdoms[j] do
          for _ = 1, #duchies[k] do
            province_count[i] = province_count[i] + 1
          end
        end
      end

      e_counter = e_counter + 1
      counter = counter + province_count[i]
      max_provinces_empires = maxf(max_provinces_empires, province_count[i])
      min_provinces_empires = minf(min_provinces_empires, province_count[i])
      max_empires = maxf(max_empires, #empires[i])
      min_empires = minf(min_empires, #empires[i])
      if #empires[i] == 3 then empires1_count = empires1_count + 1 end
      if #empires[i] == 11 then empires11_count = empires11_count + 1 end
    until true
    end -- for

    print("empires t count " .. #empires)
    print("empires_count   " .. e_counter)
    print("max_empires     " .. max_empires)
    print("min_empires     " .. min_empires)
    print("empires3_count  " .. empires1_count)
    print("empires11_count " .. empires11_count)
    print("avg empires count " .. #kingdom_empire / #empires)
    print("max_provinces_empires " .. max_provinces_empires)
    print("min_provinces_empires " .. min_provinces_empires)
    print("avg empires provinces count " .. counter / #empires)
  end -- do

  local province_empire = utils.init_array(provinces_count, constants.uint32_max)
  do
    local counter = 1
    for i = 1, #empires do
    repeat
      --assert(#empires ~= 0)
      --if #empires == 0 then break end -- continue

      for j = 1, #empires[i] do
        local kingdom_index = empires[i][j]
        --kingdom_empire[kingdom_index] = counter

        for k = 1, #kingdoms[kingdom_index] do
          local duchy_index = kingdoms[kingdom_index][k]
          for c = 1, #duchies[duchy_index] do
            local provice_index = duchies[duchy_index][c]
            province_empire[provice_index] = counter
          end
        end
      end

      counter = counter + 1
    until true
    end
  end

  for i = 1, tiles_count do
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.empire_index, constants.uint32_max)
  end

  for i = 1, #province_empire do
    local empire_index = province_empire[i]
    local childs = ctx.container:get_childs(types.entities.province, i)
    for j = 1, #childs do
      local tile_index = childs[j]
      local index = ctx.container:get_data_u32(types.entities.tile, tile_index, types.properties.tile.empire_index)
      assert(index == constants.uint32_max)
      ctx.container:set_data_u32(types.entities.tile, tile_index, types.properties.tile.empire_index, empire_index)
    end
  end

  -- чистка

  do
    local size = #empires
    local empires_copy = utils.create_table(size, 0)
    local counter = 0
    for i = 1, size do
      local inner_size = #empires[i]
      if inner_size > 0 then
        counter = counter + 1
        empires_copy[counter] = empires[i]
        for j = 1, inner_size do
          local kingdom_index = empires_copy[counter][j]
          kingdom_empire[kingdom_index] = counter
        end
      end
    end

    empires = empires_copy
  end

  function_timer:checkpoint("empires generated")

  -- выглядит с первого раза все более менее
  -- скорее всего было бы неплохо генерировать историю на основе титулов
  -- титулы - это отражение предыдущих и последующих достижений и амбиций

  -- как раздать титулы персонажам? нужно раздать покрайней мере несколько баронских титулов главе государства,
  -- один-два герцогства в зависимости от размера, королевство (размер > 20?) и империю (размер > 100?),
  -- желательно чтобы титулы находились все рядом друг с другом
  -- в цк2 на старте игры у всех императоров нет королевских титулов, все существующие королевские титулы независимы,
  -- у муслимов спавняться сильные вассалы занимающие несколько провинций с герцогским титулом,
  -- у католиков в разнобой сильные вассалы со слабыми, у остальных в основном (!) сильные вассалы
  -- нужно начать с раздачи титулов верхнего уровня

  -- титулы верхнего уровня не могут привести ко всем титулам нижнего уровня

  -- чутка исправил ошибку с генерацией титулов (их генерится больше чем нужно)
  -- кажется ошибка из-за неверного удаления пустых империй/королевств/герцогств
  -- нужно еще раз просмотреть что там

  -- по большому счету эти переменные нам не важны, они влияют только на размер контейнера генерации
  -- но при этом мы можем вылезать за пределы из-за того что не удаляем пустые империи/королевства/герцогства
  -- что делать? если использовать оригинальные размеры, то у нас будут явные пропуски в нумерации
  -- обязательно ли убирать пропуски? вряд ли, но потом если нам потребуются данные из контейнера,
  -- а они потребуются, нам будет сложно понять какой титул у нас создан, а какой нет, так что лучше уберу
  -- если не почистить то получается сложнее
  local empires_c = #empires
  local kingdoms_c = #kingdoms
  local duchies_c = #duchies
  --for i = 1, #empires  do empires_c  = empires_c  + bool_to_number(#empires[i] > 0)  end
  --for i = 1, #kingdoms do kingdoms_c = kingdoms_c + bool_to_number(#kingdoms[i] > 0) end
  --for i = 1, #duchies  do duchies_c  = duchies_c  + bool_to_number(#duchies[i] > 0)  end

  local titles_counter = empires_c + kingdoms_c + duchies_c + provinces_count
  local emp_offset = 0
  local king_offset = emp_offset + empires_c
  local duchy_offset = king_offset + kingdoms_c
  local baron_offset = duchy_offset + duchies_c
  ctx.container:clear_entities(types.entities.title)
  ctx.container:set_entity_count(types.entities.title, titles_counter)

  local_table.emp_offset = emp_offset
  local_table.king_offset = king_offset
  local_table.duchy_offset = duchy_offset
  local_table.baron_offset = baron_offset

  print("emp_offset   " .. emp_offset)
  print("king_offset  " .. king_offset)
  print("duchy_offset " .. duchy_offset)
  print("baron_offset " .. baron_offset)
  print("titles_count " .. titles_counter)

  for empire_index = 1, empires_c do
    ctx.container:set_data_u32(types.entities.title, emp_offset + empire_index, types.properties.title.parent, constants.uint32_max)
    ctx.container:set_data_u32(types.entities.title, emp_offset + empire_index, types.properties.title.owner, constants.uint32_max)
  end

  for kingdom_index = 1, kingdoms_c do
    ctx.container:set_data_u32(types.entities.title, king_offset + kingdom_index, types.properties.title.parent, constants.uint32_max)
    ctx.container:set_data_u32(types.entities.title, king_offset + kingdom_index, types.properties.title.owner, constants.uint32_max)
  end

  for duchy_index = 1, duchies_c do
    ctx.container:set_data_u32(types.entities.title, duchy_offset + duchy_index, types.properties.title.parent, constants.uint32_max)
    ctx.container:set_data_u32(types.entities.title, duchy_offset + duchy_index, types.properties.title.owner, constants.uint32_max)
  end

  for baron_index = 1, provinces_count do
    ctx.container:set_data_u32(types.entities.title, baron_offset + baron_index, types.properties.title.parent, constants.uint32_max)
    ctx.container:set_data_u32(types.entities.title, baron_offset + baron_index, types.properties.title.owner, constants.uint32_max)
  end

  local created_titles = 0
  local computed_titles_count = 0
  -- теперь когда мы не удаляем пустые империи
  local created_empires_titles_count = 0
  local created_kingdoms_titles_count = 0
  local created_duchies_titles_count = 0
  local created_barons_titles_count = 0

  local created_empires_titles_round_count = 0
  local created_kingdoms_titles_round_count = 0
  local created_duchies_titles_round_count = 0
  local created_barons_titles_round_count = 0

  local unique_kingdoms = utils.create_table(0, #kingdoms)
  local unique_duchies2 = utils.create_table(0, #duchies)
  local unique_barones = utils.create_table(0, provinces_count)

  -- начинаем создавать титулы
  for empire_index = 1, #empires do -- empires
  repeat
    if #empires[empire_index] == 0 then break end -- continue

    local final_empire_index = empire_index
    assert(final_empire_index > 0 and final_empire_index <= empires_c)
    local emp_id = "imperial" .. final_empire_index .. "_title"
    local emp_title = {
      id = emp_id,
      type = core.titulus.type.imperial,
      --heraldy = "shield_layer"
      -- строка означает id генератора
      -- но я еще могу сюда передать массив id слоев
      heraldy = "default_heraldy"
    }

    gen_title_color(emp_title, final_empire_index)
    utils.add_title(emp_title) -- local emp_dbg_index =
    created_titles = created_titles + 1
    created_empires_titles_count = created_empires_titles_count + 1
    created_empires_titles_round_count = created_empires_titles_round_count + 1

    -- по сути мы используем индекс
    -- имя нужно будет генерировать на основе культур и местности,
    -- я так понимаю нужно сначало сгенерировать какую то информацию о титулах,
    -- а потом собственно информацию о местности, возможно нужно придумать регион
    -- (например католический мир) в котором имена заспавним какие то католические
    -- регион распространения религии? что-то вроде

    for j = 1, #empires[empire_index] do -- kingdoms
      local kingdom_index = empires[empire_index][j]
      assert(kingdom_index > 0 and kingdom_index <= #kingdoms)
      assert(#kingdoms[kingdom_index] ~= 0)

      local final_kingdom_index = kingdom_index
      assert(final_kingdom_index > 0 and final_kingdom_index <= kingdoms_c)
      local parent = ctx.container:get_data_u32(types.entities.title, king_offset + final_kingdom_index, types.properties.title.parent) -- luacheck: ignore parent
      if parent ~= constants.uint32_max then print("kingdom " .. final_kingdom_index .. " empire " .. (parent - emp_offset)) end
      assert(parent == constants.uint32_max)

      assert(unique_kingdoms[kingdom_index] == nil)
      unique_kingdoms[kingdom_index] = true

      ctx.container:add_child(types.entities.title, emp_offset + final_empire_index, king_offset + final_kingdom_index)
      local king_id = "king" .. final_kingdom_index .. "_title"
      local king_title = {
        id = king_id,
        type = core.titulus.type.king,
        parent = emp_id,
        --heraldy = "shield_layer"
        heraldy = "default_heraldy"
      }
      gen_title_color(king_title, king_offset + final_kingdom_index);
      utils.add_title(king_title)
      created_titles = created_titles + 1
      created_kingdoms_titles_count = created_kingdoms_titles_count + 1
      created_kingdoms_titles_round_count = created_kingdoms_titles_round_count + 1

      ctx.container:set_data_u32(types.entities.title, king_offset + final_kingdom_index, types.properties.title.parent, emp_offset + final_empire_index)
      ctx.container:set_data_u32(types.entities.title, king_offset + final_kingdom_index, types.properties.title.owner, constants.uint32_max)

      for k = 1, #kingdoms[kingdom_index] do -- duchy
        local duchy_index = kingdoms[kingdom_index][k]
        assert(duchy_index > 0 and duchy_index <= #duchies)
        assert(#duchies[duchy_index] ~= 0)

        local final_duchy_index = duchy_index
        assert(final_duchy_index > 0 and final_duchy_index <= duchies_c)
        local parent = ctx.container:get_data_u32(types.entities.title, duchy_offset + final_duchy_index, types.properties.title.parent) -- luacheck: ignore parent
        if parent ~= constants.uint32_max then print("duchy " .. final_duchy_index .. " kingdom " .. (parent - king_offset)) end
        assert(parent == constants.uint32_max)

        assert(unique_duchies2[duchy_index] == nil)
        unique_duchies2[duchy_index] = true

        ctx.container:add_child(types.entities.title, king_offset + final_kingdom_index, duchy_offset + final_duchy_index)
        local duchy_id = "duke" .. final_duchy_index .. "_title"
        local duchy_title = {
          id = duchy_id,
          type = core.titulus.type.duke,
          parent = king_id,
          --heraldy = "shield_layer"
          heraldy = "default_heraldy"
        }

        gen_title_color(duchy_title, duchy_offset + final_duchy_index);
        utils.add_title(duchy_title)
        created_titles = created_titles + 1
        created_duchies_titles_count = created_duchies_titles_count + 1
        created_duchies_titles_round_count = created_duchies_titles_round_count + 1

        ctx.container:set_data_u32(types.entities.title, duchy_offset + final_duchy_index, types.properties.title.parent, king_offset + final_kingdom_index)
        ctx.container:set_data_u32(types.entities.title, duchy_offset + final_duchy_index, types.properties.title.owner, constants.uint32_max)

        for c = 1, #duchies[duchy_index] do -- barons
          local baron_index = duchies[duchy_index][c] -- по всей видимости я где то теряю провинцию, странно кажется я проверял
          assert(baron_index > 0 and baron_index <= provinces_count)

          -- в баронстве лежит неверный parent, почему? потому что #empires/#kingdoms/#duchies ~= empires_c/kingdoms_c/duchies_c
          -- теперь я использую created_X_titles_count для индексации, но индекс баронства по идее должен совпадать с индексом провинции
          local parent = ctx.container:get_data_u32(types.entities.title, baron_offset + baron_index, types.properties.title.parent) -- luacheck: ignore parent
          if parent ~= constants.uint32_max then print("baron " .. baron_index .. " duchy " .. (parent - duchy_offset)) end
          assert(parent == constants.uint32_max)

          assert(unique_barones[baron_index] == nil)
          unique_barones[baron_index] = true

          ctx.container:add_child(types.entities.title, duchy_offset + final_duchy_index, baron_offset + baron_index)
          local baron_id = "baron" .. baron_index .. "_title"
          local baron_title = {
            id = baron_id,
            type = core.titulus.type.baron,
            parent = duchy_id,
            --heraldy = "shield_layer",
            heraldy = "default_heraldy",
            province = baron_index
          }

          gen_title_color(baron_title, baron_offset + baron_index)
          utils.add_title(baron_title)
          created_titles = created_titles + 1
          created_barons_titles_count = created_barons_titles_count + 1
          created_barons_titles_round_count = created_barons_titles_round_count + 1
          -- при 5000 провинций получится около 6000 титулов (в общем минус оперативная память)

          ctx.container:set_data_u32(types.entities.title, baron_offset + baron_index, types.properties.title.parent, duchy_offset + final_duchy_index)
          ctx.container:set_data_u32(types.entities.title, baron_offset + baron_index, types.properties.title.owner, constants.uint32_max)
        end -- for baron
      end -- for duke
    end -- for king
  until true
  end -- for imperior

  print("empires/king/duke/baron titles count " .. created_titles)
  computed_titles_count = computed_titles_count + created_titles
  created_titles = 0
  print("round empires  titles count " .. created_empires_titles_round_count)
  print("round kingdoms titles count " .. created_kingdoms_titles_round_count)
  print("round duchies  titles count " .. created_duchies_titles_round_count)
  print("round barons   titles count " .. created_barons_titles_round_count)
  created_kingdoms_titles_round_count = 0
  created_duchies_titles_round_count = 0
  created_barons_titles_round_count = 0

  -- мне нужно поставить в соотвествие kingdom_index и final_kingdom_index
  for kingdom_index = 1, #kingdoms do
  repeat
    if #kingdoms[kingdom_index] == 0 then break end  -- continue

    local final_kingdom_index = kingdom_index
    assert(final_kingdom_index > 0 and final_kingdom_index <= kingdoms_c)
    local parent = ctx.container:get_data_u32(types.entities.title, king_offset + final_kingdom_index, types.properties.title.parent) -- luacheck: ignore parent
    if parent ~= constants.uint32_max then break end -- continue

    assert(unique_kingdoms[kingdom_index] == nil)
    unique_kingdoms[kingdom_index] = true

    local king_id = "king" .. final_kingdom_index .. "_title"
    local king_title = {
      id = king_id,
      type = core.titulus.type.king,
      --heraldy = "shield_layer"
      heraldy = "default_heraldy"
    }

    gen_title_color(king_title, king_offset + final_kingdom_index)
    utils.add_title(king_title)
    created_titles = created_titles + 1
    created_kingdoms_titles_count = created_kingdoms_titles_count + 1
    created_kingdoms_titles_round_count = created_kingdoms_titles_round_count + 1

    for i = 1, #kingdoms[kingdom_index] do
      local duchy_index = kingdoms[kingdom_index][i]
      assert(duchy_index > 0 and duchy_index <= #duchies)
      assert(#duchies[duchy_index] ~= 0)

      local final_duchy_index = duchy_index
      assert(final_duchy_index > 0 and final_duchy_index <= duchies_c)
      local parent = ctx.container:get_data_u32(types.entities.title, duchy_offset + final_duchy_index, types.properties.title.parent) -- luacheck: ignore parent
      if parent ~= constants.uint32_max then print("duchy " .. final_duchy_index .. " kingdom " .. (parent - king_offset)) end
      assert(parent == constants.uint32_max)

      assert(unique_duchies2[duchy_index] == nil)
      unique_duchies2[duchy_index] = true

      ctx.container:add_child(types.entities.title, king_offset + final_kingdom_index, duchy_offset + final_duchy_index)
      local duchy_id = "duke" .. duchy_index  .. "_title"
      local duchy_title = {
        id = duchy_id,
        type = core.titulus.type.duke,
        parent = king_id,
        --heraldy = "shield_layer"
        heraldy = "default_heraldy"
      }

      gen_title_color(duchy_title, duchy_offset + duchy_index)
      utils.add_title(duchy_title)
      created_titles = created_titles + 1
      created_duchies_titles_count = created_duchies_titles_count + 1
      created_duchies_titles_round_count = created_duchies_titles_round_count + 1

      ctx.container:set_data_u32(types.entities.title, duchy_offset + final_duchy_index, types.properties.title.parent, king_offset + final_kingdom_index)

      for c = 1, #duchies[duchy_index] do
        local baron_index = duchies[duchy_index][c]
        assert(baron_index > 0 and baron_index <= provinces_count)

        -- приходит уже созданный баронский титул, почему?
        local parent = ctx.container:get_data_u32(types.entities.title, baron_offset + baron_index, types.properties.title.parent) -- luacheck: ignore parent
        if parent ~= constants.uint32_max then print("baron " .. baron_index .. " duchy " .. (parent - duchy_offset) .. " container " .. duchy_index) end
        assert(parent == constants.uint32_max)

        assert(unique_barones[baron_index] == nil)
        unique_barones[baron_index] = true

        ctx.container:add_child(types.entities.title, duchy_offset + final_duchy_index, baron_offset + baron_index)
        local baron_id = "baron" .. baron_index .. "_title"
        local baron_title = {
          id = baron_id,
          type = core.titulus.type.baron,
          parent = duchy_id,
          heraldy = "default_heraldy",
          province = baron_index
        }

        gen_title_color(baron_title, baron_offset + baron_index)
        utils.add_title(baron_title)
        created_titles = created_titles + 1
        created_barons_titles_count = created_barons_titles_count + 1
        created_barons_titles_round_count = created_barons_titles_round_count + 1

        ctx.container:set_data_u32(types.entities.title, baron_offset + baron_index, types.properties.title.parent, duchy_offset + final_duchy_index)
      end -- for baron
    end -- for duke
  until true
  end -- for king

  print("king/duke/baron titles count " .. created_titles)
  computed_titles_count = computed_titles_count + created_titles
  created_titles = 0
  print("round kingdoms titles count " .. created_kingdoms_titles_round_count)
  print("round duchies  titles count " .. created_duchies_titles_round_count)
  print("round barons   titles count " .. created_barons_titles_round_count)
  created_duchies_titles_round_count = 0
  created_barons_titles_round_count = 0

  for duchy_index = 1, #duchies do
  repeat
    if #duchies[duchy_index] == 0 then break end  -- continue

    local final_duchy_index = duchy_index
    assert(final_duchy_index > 0 and final_duchy_index <= duchies_c)
    local parent = ctx.container:get_data_u32(types.entities.title, duchy_offset + final_duchy_index, types.properties.title.parent) -- luacheck: ignore parent
    if parent ~= constants.uint32_max then break end -- continue

    assert(unique_duchies2[duchy_index] == nil)
    unique_duchies2[duchy_index] = true

    local duchy_id = "duke" .. final_duchy_index .. "_title"
    local duchy_title = {
      id = duchy_id,
      type = core.titulus.type.duke,
      --heraldy = "shield_layer"
      heraldy = "default_heraldy"
    }

    gen_title_color(duchy_title, duchy_offset + final_duchy_index)
    utils.add_title(duchy_title)
    created_titles = created_titles + 1
    created_duchies_titles_count = created_duchies_titles_count + 1
    created_duchies_titles_round_count = created_duchies_titles_round_count + 1

    for c = 1, #duchies[duchy_index] do
      local baron_index = duchies[duchy_index][c]
      assert(baron_index > 0 and baron_index <= provinces_count)

      local parent = ctx.container:get_data_u32(types.entities.title, baron_offset + baron_index, types.properties.title.parent) -- luacheck: ignore parent
      assert(parent == constants.uint32_max)

      assert(unique_barones[baron_index] == nil)
      unique_barones[baron_index] = true

      ctx.container:add_child(types.entities.title, duchy_offset + final_duchy_index, baron_offset + baron_index)
      local baron_id = "baron" .. baron_index .. "_title"
      local baron_title = {
        id = baron_id,
        type = core.titulus.type.baron,
        parent = duchy_id,
        --heraldy = "shield_layer",
        heraldy = "default_heraldy",
        province = baron_index
      }

      gen_title_color(baron_title, baron_offset + baron_index)
      utils.add_title(baron_title)
      created_titles = created_titles + 1
      created_barons_titles_count = created_barons_titles_count + 1
      created_barons_titles_round_count = created_barons_titles_round_count + 1

      ctx.container:set_data_u32(types.entities.title, baron_offset + baron_index, types.properties.title.parent, duchy_offset + final_duchy_index)
    end
  until true
  end

  print("duke/baron titles count " .. created_titles)
  computed_titles_count = computed_titles_count + created_titles
  created_titles = 0
  print("round duchies  titles count " .. created_duchies_titles_round_count)
  print("round barons   titles count " .. created_barons_titles_round_count)
  created_barons_titles_round_count = 0

  for baron_index = 1, provinces_count do
  repeat
    local parent = ctx.container:get_data_u32(types.entities.title, baron_offset + baron_index, types.properties.title.parent)
    if parent ~= constants.uint32_max then break end -- continue

    assert(unique_barones[baron_index] == nil)
    unique_barones[baron_index] = true

    local baron_id = "baron" .. baron_index .. "_title"
    local baron_title = {
      id = baron_id,
      type = core.titulus.type.baron,
      --heraldy = "shield_layer",
      heraldy = "default_heraldy",
      province = baron_index
    }

    gen_title_color(baron_title, baron_offset + baron_index)
    -- было бы неплохо сразу в этом месте чекать валидность таблицы
    -- хотя раньше я думал что хорошей идеей будет сначала собрать все таблицы,
    -- а потом выдать пользователю все ошибки, а теперь я вижу использование памяти на все эти удовольствия
    -- нужно видимо описать загружаемые структуры для каждого типа,
    -- иначе слишком много потребляет памяти все это дело
    utils.add_title(baron_title)
    created_titles = created_titles + 1
    created_barons_titles_count = created_barons_titles_count + 1
    created_barons_titles_round_count = created_barons_titles_round_count + 1
  until true
  end

  print("baron titles count " .. created_titles)
  print("round barons   titles count " .. created_barons_titles_round_count)
  print("recomputed  empire titles count " .. created_empires_titles_count)
  print("            empire titles count " .. empires_c)
  print("recomputed kingdom titles count " .. created_kingdoms_titles_count)
  print("           kingdom titles count " .. kingdoms_c)
  print("recomputed   duchy titles count " .. created_duchies_titles_count)
  print("             duchy titles count " .. duchies_c)
  print("recomputed   baron titles count " .. created_barons_titles_count)
  print("             baron titles count " .. provinces_count)
  computed_titles_count = computed_titles_count + created_titles
  assert(computed_titles_count == titles_counter)
  assert(created_empires_titles_count + created_kingdoms_titles_count +
    created_duchies_titles_count + created_barons_titles_count == titles_counter)
  assert(created_empires_titles_count == empires_c)
  assert(created_kingdoms_titles_count == kingdoms_c)
  assert(created_duchies_titles_count == duchies_c)
  assert(created_barons_titles_count == provinces_count)

  -- нам нужно каким то образом где то еще эти титулы запомнить
  -- во первых для того чтобы нарисовать для них интерфейс и выделить их на карте
  -- во вторых для того чтобы с ними можно было взаимодействовать
  -- где их хранить? лучше всего в провинциях (это логично и меньше места занимает)
  -- для всех титулов необходимо еще сгенерировать эмблему
  -- отличный механизм сделан уже в цк2 https://ck2.paradoxwikis.com/Coats_of_arms_modding
  -- нужно только сильно переделать его в угоду рандомной генерации
  -- переделал, теперь слой заполняется в структуру 64 байта,
  -- для отрисовки одной геральдики используется несколько слоев

  -- для того чтобы хранить какие то такие данные не в тайлах
  -- нужно немного переделать функцию которая заполняет данными рендер
  -- пользователь должен смочь сделать функцию в луа с любыми данными

  -- тут есть одна проблема: как правильно выстроить иерархию данных, так чтобы было удобно заполнять данные
  -- титулы довольно удобно заполняются сверху вниз, но когда мы переходим к городам удобство заканчивается
  -- удобно при генерации городов сгенерировать и титул заодно (причем титул во многом будет "технический")
  -- (то есть там будут храниться названия городов)

  -- геральдика сильно переделана и теперь слои как контруктор будут хранится в отдельном буфере
  -- а последовательность слоев мы загружаем в каждом кадре (это от 500 до 2к геральдик)
  -- (количество скорее всего можно уменьшить)

  function_timer:finish()
end -- generate_titles

local function generate_characters(ctx, local_table)
  local function_timer = generator.timer_t.new("characters generation")

  local absf = math.abs

  -- для каждого государства мы создаем по персонажу
  -- затем внутри каждого государства мы должны распределить провинции на вассалов
  -- как раздать провинции? во первых нужно примерно прикинуть сколько будет провинций
  -- а это наверное будет зависеть от формы правления, а что с формой правления?
  -- это набор нескольких механизмов центральных в средневековой стране:
  -- 1. способ передачи власти
  -- 2. ответственность органов власти
  -- 3. разграничение полномочий
  -- первый пункт более менее простой - это очевидно нужно сделать
  -- ответственность? зависимость персонажа от чего то, перед советом (ну кстати изменяя набор механик мы можем подкорректировать ответственность и полномочия)
  -- разграничение полномочий? тут по идее должна быть механика совета
  -- тут есть две силы крупные монарх и совет (который состоит из вассалов)
  -- они долны уравновешиваться армиями
  -- племя - несколько провинции в децентрализованном государстве (но не всегда)
  -- чаще всего мы определяем племя по тех уровню, следующий вопрос что то такое тех уровень?
  -- тех уровень я полагаю очень похож на институты в еу4, то есть некоторое знание распространяется
  -- по провинциям и обозначает доступ к некоторым зданиям и бонусам (иногда его можно открыть)
  -- тех зависит от прошлых империй

  -- раздачу титулов нужно совместить с историей похоже

  -- можно раздать все титулы сначало одному персонажу, который в свою очередь раздаст
  -- титулы своим подельникам, как раздать титулы более высокого уровня?
  -- герцогские еще более менее понятно, титулы вообще тоже зависят от тех уровня
  -- думаю что нужно сначало тех уровень продумать

  local emp_offset = local_table.emp_offset
  local king_offset = local_table.king_offset
  local duchy_offset = local_table.duchy_offset
  local baron_offset = local_table.baron_offset
  local real_country_count = local_table.country_count
  print("real_country_count" .. real_country_count)
  assert(real_country_count ~= 0)

  local titles_count = ctx.container:entities_count(types.entities.title)
  local country_count = ctx.container:entities_count(types.entities.country)
  local provinces_count = ctx.container:entities_count(types.entities.province)
  local owners = utils.init_array(real_country_count, {})
  for i = 1, #owners do owners[i] = utils.create_table(0, 100) end
  local country_index_counter = 1
  for country_index = 1, country_count do -- некоторые страны пустые, почему то я не очистил их
  repeat
    local childs_count = ctx.container:get_childs_count(types.entities.country, country_index)
    --assert(#childs ~= 0)
    if childs_count == 0 then break end -- continue

    local real_country_index = country_index_counter
    country_index_counter = country_index_counter + 1

    --for _,province_index in ipairs(childs) do
    for i = 1, childs_count do
    repeat
      local province_index = ctx.container:get_child(types.entities.country, country_index, i)
      local baron_index = baron_offset + province_index
      local baron_val = owners[real_country_index][baron_index]
      owners[real_country_index][baron_index] = baron_val ~= nil and (baron_val+1) or 1

      local duchy_index = ctx.container:get_data_u32(types.entities.title, baron_index, types.properties.title.parent)
      if duchy_index == constants.uint32_max then break end -- continue
      local duchy_val = owners[real_country_index][duchy_index]
      owners[real_country_index][duchy_index] = duchy_val ~= nil and (duchy_val+1) or 1

      local king_index = ctx.container:get_data_u32(types.entities.title, duchy_index, types.properties.title.parent)
      if king_index == constants.uint32_max then break end -- continue
      local king_val = owners[real_country_index][king_index]
      owners[real_country_index][king_index] = king_val ~= nil and (king_val+1) or 1

      local emp_index = ctx.container:get_data_u32(types.entities.title, king_index, types.properties.title.parent)
      if emp_index == constants.uint32_max then break end -- continue
      local emp_val = owners[real_country_index][king_index]
      owners[real_country_index][emp_index] = emp_val ~= nil and (emp_val+1) or 1
    until true
    end

    if country_index == country_count then
      assert(real_country_index == real_country_count, "real index " .. real_country_index .. " real count " .. real_country_count)
    end
  until true
  end

  function_timer:checkpoint("Title owners")

  -- у меня остаются пустые страны после генерации

  -- так и что это нам дает? у нас теперь есть количество провинций принадлежащих титулам разных уровней
  -- так мы по идее можем определиться с титулом верхнего уровня
  -- я так понимаю нужно сделать тип в контейнере

  local computed_province_count = 0
  for country_index = 1, country_count do
    local childs_count = ctx.container:get_childs_count(types.entities.country, country_index)
    computed_province_count = computed_province_count + childs_count
  end
  print(computed_province_count)
  assert(computed_province_count == provinces_count)

  country_index_counter = 1
  local titles = utils.init_array(real_country_count, {})
  for country_index = 1, country_count do
  repeat
    local childs = ctx.container:get_childs(types.entities.country, country_index);
    if #childs == 0 then break end -- continue

    local real_country_index = country_index_counter
    country_index_counter = country_index_counter + 1
    local title_owned = owners[real_country_index]
    for j = baron_offset+1, titles_count do
    repeat
      if title_owned[j] == nil then break end -- continue
      local tmp = ctx.container:get_data_u32(types.entities.title, j, types.properties.title.owner)
      assert(tmp == constants.uint32_max)
      --local baron_index = j - baron_offset -- так у меня нет информации о том что это за титул
      table.insert(titles[real_country_index], j)
      ctx.container:set_data_u32(types.entities.title, j, types.properties.title.owner, real_country_index)
    until true
    end

    -- числа 20/70 на 5000 провок выглядят более менее прилично (выглядит прилично и для 1600 на более мелкой карте)
    -- нужно ли корректировать эти цифры для меньшего количество провок
    -- (2500 = 10/35)? если и нужно то видимо как то иначе, должен быть наверное какой то минимум
    -- сколько провок в среднем в титулах? может так?
    local king_start = 20
    local emp_start = 70

    if #childs < king_start then
      local max_prov = 0
      local index = -1
      for j = duchy_offset+1, baron_offset do
      repeat
        local duchy_index = j - duchy_offset
        local tmp = ctx.container:get_data_u32(types.entities.title, duchy_offset + duchy_index, types.properties.title.owner)
        if tmp ~= constants.uint32_max then break end -- continue

        if title_owned[j] ~= nil and max_prov < title_owned[j] then
          max_prov = title_owned[j]
          index = duchy_index
        end
      until true
      end

      -- один титул?
      ctx.container:set_data_u32(types.entities.title, duchy_offset + index, types.properties.title.owner, real_country_index)
      table.insert(titles[real_country_index], duchy_offset + index)

      break -- continue
    end -- if

    if #childs >= king_start and #childs < emp_start then -- это королевства? в цк2 византийская империя (2 старт) примерно 65 провинций
      local max_prov = 0
      local index = -1
      for j = king_offset+1, duchy_offset do
      repeat
        local king_index = j - king_offset
        local tmp = ctx.container:get_data_u32(types.entities.title, king_offset + king_index, types.properties.title.owner)
        if tmp ~= constants.uint32_max then break end -- continue

        if title_owned[j] ~= nil and max_prov < title_owned[j] then
          max_prov = title_owned[j]
          index = king_index
        end
      until true
      end

      -- один титул?
      ctx.container:set_data_u32(types.entities.title, king_offset + index, types.properties.title.owner, real_country_index)
      table.insert(titles[real_country_index], king_offset + index)
    end -- if

    if #childs >= emp_start then
      local max_prov = 0
      local index = -1
      for j = emp_offset+1, king_offset do
      repeat
        local emp_index = j - emp_offset
        local tmp = ctx.container:get_data_u32(types.entities.title, emp_offset + emp_index, types.properties.title.owner)
        if tmp ~= constants.uint32_max then break end -- continue

        if title_owned[j] ~= nil and max_prov < title_owned[j] then
          max_prov = title_owned[j]
          index = emp_index
        end
      until true
      end

      -- мы нашли индекс империи с которой у нас больше всего совпадений
      -- другое дело что исторически так все радужно с империями не было
      -- они часто разваливались и поэтому сильно меняли форму
      -- я должен создать титулы до генерации истории, а потом в истории
      -- раскидывать титулы (точнее искать максимальный титул для государства)
      -- но пока так

      ctx.container:set_data_u32(types.entities.title, emp_offset + index, types.properties.title.owner, real_country_index)
      table.insert(titles[real_country_index], emp_offset + index)
    end

    -- че с герцогствами? случайно их раскидать? выглядит идея еще ничего
    for j = duchy_offset+1, baron_offset do
    repeat
      if title_owned[j] == nil then break end -- continue
      local tmp = ctx.container:get_data_u32(types.entities.title, j, types.properties.title.owner)
      if tmp ~= constants.uint32_max then break end -- continue

      -- по идее наличие герцогского титула мало зависит от размера и скорее зависит от поворота истории
      -- поэтому нужен какой то случайный коэффициент
      -- коэффициент наверное будет зависеть от размера страны (?)
      -- в империях наиболее вероятно что будет сделан герцогский титул
      -- менее вероятно в королевствах
      -- в герцогствах должен один титул
      local base = 0.3
      local final = base + base * bool_to_number(#childs >= emp_start) -- 0.6 норм или нет?
      local prob = ctx.random:probability(final)
      if not prob then break end -- continue

      table.insert(titles[real_country_index], j)
      ctx.container:set_data_u32(types.entities.title, j, types.properties.title.owner, real_country_index)
    until true
    end

    -- хочется все бросить и сделать просто через создание конкретных объектов
    -- но я так сделать не могу =( сериализация полных состояний луа невозможна
    -- (в смысле я могу сохранить только конкретные данные (и кажется даже не ссылки))
    -- я могу попробовать запустить гарбадж коллектор, но скорее всего без особого успеха
    -- ко всему прочему мне бы выжать полный максимум производительности от проверки условий
    -- в том числе используя мультитрединг, это сложно используя только луа
    -- таким образом мне нужно правильно задать связи так чтобы при этом не сломался луа
    -- и комп от обработки 20к титулов и столько же персонажей (а может и больше)
    -- в луа 176 000 000 чисел (индексы) в таблице занимает 4гб,
    -- мне надо чтобы ~5000 провинций, ~12000 городов, ~15000 персонажей, ~15000 титулов,
    -- занимало желательно чтобы меньше 512 мб (ну мож гиг максимум)
    -- придется тогда некоторые таблицы сохранять (нужны указатели на родителей)
    -- мы можем положить индексы просто в ctx->container
    -- скорее всего нужно сделать обработку и индексов в парсере: так станет более менее полегче
    -- то есть вместо строки мы будем ожидать индекс или сделать и строку и индекс?

    -- наверное просто сохраню в таблице все титулы

    assert(#childs ~= 0)
    if country_index == country_count then assert(real_country_index == real_country_count, "real index " .. real_country_index .. " real count " .. real_country_count) end
  until true
  end

  function_timer:checkpoint("Goverment titles")

  -- как вообще в принципе раздать сгенерированные титулы сгенерированным персонажам?
  -- возможно даже сразу сгенерировать персонажей к титулам лучше чем генерировать сейчас отдельно

  -- теперь у нас есть как то раскиданные титулы государству, как раскидывать по персонажам?
  -- раскидывать нужно пока не кончатся провинции, отдавать в одни руки числом до герцогского (то есть от 1 до 6)
  -- (но 5-6 очень редко)
  for country_index = 1, real_country_count do
  --repeat
    -- сначало нужно посчитать что имеем
    local owned_titles = titles[country_index]
    assert(#owned_titles ~= 0)
    --if #owned_titles == 0 then break end -- continue

    local baron_titles = utils.create_table(0, #owned_titles)
    local duchy_titles = utils.create_table(20, 0)
    local king_titles = utils.create_table(10, 0)
    local emp_titles = utils.create_table(10, 0)
    local baron_titles_debug_counter = 0
    for _,title_index in ipairs(owned_titles) do
      if title_index > baron_offset then baron_titles[title_index] = true; baron_titles_debug_counter = baron_titles_debug_counter + 1
      elseif title_index > duchy_offset then table.insert(duchy_titles, title_index)
      elseif title_index > king_offset then table.insert(king_titles, title_index)
      elseif title_index > emp_offset then table.insert(emp_titles, title_index) end
    end

    assert(baron_titles_debug_counter ~= 0)
    assert(#king_titles + #emp_titles < 2)

    -- первым делаем господина, у господина должны быть все самые высокие титулы и покрайней мере одно герцогство
    local liege_index = -1 -- luacheck: ignore liege_index
    do
      local baron_titles_count = math.floor(ctx.random:closed(2, 6)) -- от 2 до 6 (более менее нормальное распределение)
      local counter = 0
      local choosen_baron_index = -1
      local final_titles = utils.create_table(20, 0)
      if #duchy_titles ~= 0 then
        append_table(final_titles, king_titles)
        append_table(final_titles, emp_titles)
        --king_titles = {}
        --emp_titles = {}

        -- герцогство выбираем случайно (герцогств может и не быть)
        local rand_index = ctx.random:index(#duchy_titles)
        local duchy_index = duchy_titles[rand_index]
        table.insert(final_titles, duchy_index)
        duchy_titles[rand_index] = duchy_titles[#duchy_titles]
        duchy_titles[#duchy_titles] = nil

        assert(duchy_index > duchy_offset and duchy_index <= baron_offset)

        -- собираем баронские титулы
        local duchy_childs = ctx.container:get_childs(types.entities.title, duchy_index)
        -- нам бы раздать случайные (?) соседние провки,

        local attemps = 0
        while choosen_baron_index == -1 and attemps < 100 do
        repeat
          attemps = attemps + 1
          local rand_child_index = ctx.random:index(#duchy_childs)
          local baron_title_index = duchy_childs[rand_child_index]
          local tmp = ctx.container:get_data_u32(types.entities.title, baron_title_index, types.properties.title.owner)
          if tmp ~= country_index then break end -- continue
          if baron_titles[baron_title_index] == nil then break end -- continue

          baron_titles[baron_title_index] = nil
          choosen_baron_index = baron_title_index
        until true
        end
      else
        local attemps = 0
        while choosen_baron_index == -1 and attemps < 100 do
          attemps = attemps + 1
          -- оригинально я тут брал бесконечно первый титул среди баронских
          -- это работало так как, когда беру титул, я удалю его из baron_titles
          -- хотя может быть лучше путь будет в цикле
          for baron_title_index,_ in pairs(baron_titles) do
            local tmp = ctx.container:get_data_u32(types.entities.title, baron_title_index, types.properties.title.owner)
            if tmp == country_index then
              baron_titles[baron_title_index] = nil
              choosen_baron_index = baron_title_index
              break
            end
          end
        end
      end

      assert(choosen_baron_index ~= -1, "Could not find proper baron title. Duchies count " .. #duchy_titles .. " baron count " .. baron_titles_debug_counter)
      table.insert(final_titles, choosen_baron_index)
      counter = counter + 1

      local last_title = choosen_baron_index
      while counter < baron_titles_count do
        local province_index = last_title - baron_offset -- по идее должно дать хороший луа индекс
        assert(province_index >= 1)
        local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)
        local index = 1
        while counter < baron_titles_count and index <= #neighbours do
        repeat
          local new_index = unpack_n_index(neighbours[index])
          index = index + 1
          local baron_index = absf(new_index) + baron_offset
          local tmp = ctx.container:get_data_u32(types.entities.title, baron_index, types.properties.title.owner)
          if tmp ~= country_index then break end -- continue
          if baron_titles[baron_index] == nil then break end -- continue

          table.insert(final_titles, baron_index)
          baron_titles[baron_index] = nil
          counter = counter + 1
        until true
        end

        -- если не добрали то че делаем
        -- у нас может быть ситуация когда у государства очень мало провинций
        -- тогда нужно выходить, че делать то
        assert(#final_titles ~= 0)
        if last_title == final_titles[#final_titles] then break end -- break
        last_title = final_titles[#final_titles]
      end -- while counter < baron_titles_count do

      local character_table = {
        family = {},
        stats = {
          money = 400
        },
        hero_stats = {},
        male = true,
        dead = false,
        titles = {}
      }

      for _,title_index in ipairs(final_titles) do
        if title_index > baron_offset then
          local str = "baron" .. (title_index - baron_offset) .. "_title"
          table.insert(character_table.titles, str)
          local city_id = "city" .. (title_index - baron_offset) .. "_title"
          table.insert(character_table.titles, city_id)
        elseif title_index > duchy_offset then
          local str = "duke" .. (title_index - duchy_offset) .. "_title"
          table.insert(character_table.titles, str)
        elseif title_index >= king_offset then
          local str = "king" .. (title_index - king_offset) .. "_title"
          table.insert(character_table.titles, str)
        elseif title_index >= emp_offset then
          local str = "imperial" .. (title_index - emp_offset) .. "_title"
          table.insert(character_table.titles, str)
        end
      end

      liege_index = utils.add_character(character_table)
    end

    assert(liege_index ~= -1)
    --function_timer:checkpoint("Country " .. country_index .. " owner generation")

    -- тут может быть что соседние вассалы разобрали все баронские титулы у текущего герцогства
    -- что делать в этом случае? 2 варианта: либо мы считаем что это герцогство не создано
    -- либо мы гарантируем каждому владельцу титула что у него будет баронство

    -- теперь делаем вассалов, у них либо герцогство либо несколько просто владений
    -- поэтому сначало раздаем герцогства, так то у этих челиков могут быть свои вассалы
    -- возможно нужно взять больше баронских титулов
    --local vassal_counter = 0
    while #duchy_titles ~= 0 do
    repeat
      local rand_index = ctx.random:index(#duchy_titles)
      local duchy_index = duchy_titles[rand_index]
      duchy_titles[rand_index] = duchy_titles[#duchy_titles]
      duchy_titles[#duchy_titles] = nil

      local final_titles = {}
      table.insert(final_titles, duchy_index)

      -- пока что наверное выберем первый вариант
      local baron_counter = 0
      local duchy_childs = ctx.container:get_childs(types.entities.title, duchy_index)
      for i = 1, #duchy_childs do
      repeat
        local index = duchy_childs[i]
        local tmp = ctx.container:get_data_u32(types.entities.title, index, types.properties.title.owner)
        if tmp ~= country_index then break end -- continue
        if baron_titles[index] == nil then break end -- continue

        baron_counter = baron_counter + 1
      until true
      end

      if baron_counter == 0 then break end -- continue

      -- собираем баронские титулы (нужно взять больше для того чтобы сделать вассалов вассала)
      local baron_titles_count = math.floor(ctx.random:closed(2, 6)) -- от 2 до 6 (более менее нормальное распределение)
      local counter = 0
      -- нам бы раздать случайные (?) соседние провки

      local choosen_baron_index = -1
      while choosen_baron_index == -1 do
      repeat
        local rand_index = ctx.random:index(#duchy_childs) -- luacheck: ignore rand_index
        local baron_title_index = duchy_childs[rand_index]
        local tmp = ctx.container:get_data_u32(types.entities.title, baron_title_index, types.properties.title.owner)
        if tmp ~= country_index then break end -- continue
        if baron_titles[baron_title_index] == nil then break end -- continue

        baron_titles[baron_title_index] = nil
        choosen_baron_index = baron_title_index
      until true
      end

      assert(choosen_baron_index ~= -1)
      table.insert(final_titles, choosen_baron_index)
      counter = counter + 1

      local last_title = choosen_baron_index
      --while counter < baron_titles_count do
        local province_index = last_title - baron_offset
        local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)
        local index = 1
        while counter < baron_titles_count and index <= #neighbours do
        repeat
          local new_index = unpack_n_index(neighbours[index])
          index = index + 1
          local baron_index = absf(new_index) + baron_offset
          local tmp = ctx.container:get_data_u32(types.entities.title, baron_index, types.properties.title.owner)
          if tmp ~= country_index then break end -- continue
          if baron_titles[baron_index] == nil then break end -- continue

          baron_titles[baron_index] = nil
          table.insert(final_titles, baron_index)
          counter = counter + 1
        until true
        end

        -- наверное здесь ограничимся тем что есть
--        assert(last_title ~= final_titles[#final_titles])
--        last_title = final_titles[#final_titles]
--      end

      local character_table = {
        family = {},
        stats = { money = 400 },
        hero_stats = {},
        male = true,
        dead = false,
        liege = liege_index,
        titles = {}
      }

      for _,title_index in ipairs(final_titles) do
        if title_index > baron_offset then
          local str = "baron" .. (title_index - baron_offset) .. "_title"
          table.insert(character_table.titles, str)
          local city_id = "city" .. (title_index - baron_offset) .. "_title"
          table.insert(character_table.titles, city_id)
        elseif title_index > duchy_offset then
          local str = "duke" .. (title_index - duchy_offset) .. "_title"
          table.insert(character_table.titles, str)
        else assert(false) end
      end

      utils.add_character(character_table)
      --vassal_counter = vassal_counter + 1
      --function_timer:checkpoint("Country " .. country_index .. " main vassal " .. vassal_counter .. " generation")
    until true
    end -- while #duchy_titles ~= 0 do

    -- могут остаться баронские титулы еще
    local next = next
    while next(baron_titles) ~= nil do
      local choosen_baron_index,_ = next(baron_titles)
      baron_titles[choosen_baron_index] = nil

      local baron_titles_count = ctx.random:closed(1, 6)
      local counter = 0
      local final_titles = { choosen_baron_index }
      counter = counter + 1

      local last_title = choosen_baron_index
      --while (counter < baron_titles_count) {
        local province_index = last_title - baron_offset
        local neighbours = ctx.container:get_childs(types.entities.province_neighbors, province_index)
        local index = 1
        while counter < baron_titles_count and index <= #neighbours do
        repeat
          local new_index = unpack_n_index(neighbours[index])
          index = index + 1
          local baron_index = absf(new_index) + baron_offset
          local tmp = ctx.container:get_data_u32(types.entities.title, baron_index, types.properties.title.owner)
          if tmp ~= country_index then break end -- continue
          if baron_titles[baron_index] == nil then break end -- continue

          baron_titles[baron_index] = nil
          table.insert(final_titles, baron_index)
          counter = counter + 1
        until true
        end

        -- наверное здесь ограничимся тем что есть
      --   assert(last_title ~= final_titles[#final_titles])
      --   last_title = final_titles[#final_titles]
      -- end

      local character_table = {
        family = {},
        stats = { money = 400 },
        hero_stats = {},
        male = true,
        dead = false,
        liege = liege_index,
        titles = {}
      }

      for _,title_index in ipairs(final_titles) do
        if title_index > baron_offset then
          local str = "baron" .. (title_index - baron_offset) .. "_title"
          table.insert(character_table.titles, str)
          local city_id = "city" .. (title_index - baron_offset) .. "_title"
          table.insert(character_table.titles, city_id)
        else assert(false) end
      end

      utils.add_character(character_table)
      --vassal_counter = vassal_counter + 1
      --function_timer:checkpoint("Country " .. country_index .. " remain vassal " .. vassal_counter .. " generation")
    end -- while next(baron_titles) ~= nil do

    --function_timer:checkpoint("Country " .. country_index .. " end. Country count " .. real_country_count)
  --until true
  end -- for country_index = 1, real_country_count do

  -- как то раскидали титулы по персонажам
  -- по идее мы закончили с генерацией (нужно еще наверное задать какие нибудь константы (например шанс смерти))
  -- теперь нужно сделать парсинг
  -- мы обходим все таблицы, валидируем их, парсим,
  -- после парсинга начинаем собирать все связи между данными в кучу
  -- удаляем старый луа стейт, создаем новый (хотя до сих пор не знаю нужен ли он)
  -- делаем всю необходимую работу для рендера (границы, стены),
  -- переходим непосредственно к игре
  -- требуется обойти всех персонажей и посчитать ход, причем часть хода можно сделать в мультитрединге
  -- (например посчитать инком, пересчитать статы, строительство и проч)
  -- где то здесь же идет ход игрока, отрисовка интерфейса
  -- вообще дизайн интерфейса будет той еще задачкой для меня

  -- как быть здесь? нужно получить id родительского титула

  -- как то так раздали титулы правителю и вассалам
  -- нужно сделать еще города

  function_timer:finish()
end -- generate_characters

local function generate_tech_level(ctx, local_table)
  local function_timer = generator.timer_t.new("tech level generation")

  -- что такое тех уровень?
  -- тех в средвековье определялся в основном прошлым
  -- то есть примерно в области западной римской империи тех чуть повыше
  -- восточная римская империя - высокий тех
  -- рядом с китаем высокий тех
  -- что такое тех уровень с технической стороны?
  -- я думаю что можно сделать что то похожее на институты в еу4
  -- хороший вопрос как их распределить?
  -- существует несколько техов например обработка железа дает возможность нанимать тяжелые отряды
  -- хотя по теху пока еще не понятно как лучше сделать
  -- в цк2 тех состоял из формы правления и культурных техов
  -- форма правления не давала ниче строить племенам, техи позволяли проводить некоторые законы
  -- таким образом техи должны зависить от формы правления, а форма правления должна зависеть от техов
  -- техи получить должно быть возможно без формы правления, причем техи можно получить не по порядку
  -- какие техи? должны быть техи "ключевые", они позволяют серьезно продвинуться по развитию
  -- например тех организованность позволяют получить какую то форму феодализма

  -- нужно решить несколько технических проблем
  -- как техи будут передавать свои бонусы? особые бонусы делать сложно
  -- особые бонусы это что?

  function_timer:finish()
end -- generate_tech_level

local function generate_cities(ctx, local_table) -- luacheck: ignore
  local function_timer = generator.timer_t.new("cities generation")

  -- do
  --   local building = { id = "test_building1", time = 4 }
  --   utils.add_building_type(building)
  -- end
  --
  -- do
  --   local building = { id = "test_building2", time = 4 }
  --   utils.add_building_type(building)
  -- end

  utils.load_troop_types("apates_quest/scripts/troops_config.lua")
  utils.load_building_types("apates_quest/scripts/building_type_config.lua")
  utils.load_city_types("apates_quest/scripts/city_types_config.lua")

  local provinces_creation_count = ctx.container:entities_count(types.entities.province)
  for i = 1, provinces_creation_count do
    local province_tiles_count = ctx.container:get_childs_count(types.entities.province, i)
    local rand_index = ctx.random:index(province_tiles_count)
    local rand_tile = ctx.container:get_child(types.entities.province, i, rand_index)
    -- id получаются совсем не информативными, они будут более информативными если генерировать имена
    -- в общем это проблема неединичной генерации, хочется использовать индексы, но они не информативные
    local city_id = "city" .. i .. "_title"
    local baron_id = "baron" .. i .. "_title"
    local city = { -- название города видимо будет храниться в титуле
      province = i,
      city_type = "city_type1",
      tile_index = rand_tile,
      title = city_id -- тут проще использовать индекс
    }
    utils.add_city(city)

    local city_title = {
      id = city_id,
      type = core.titulus.type.city,
      heraldy = "default_heraldy",
      -- родителя найдем тогда из провинции,
      -- не, лучше его указать
      parent = baron_id
    }

    gen_title_color(city_title, i)
    utils.add_title(city_title)

    --print("city " .. i .. " added")
  end

  -- тут же настраивался календарь, как его лучше всего сделать?
  -- настройка календаря - это стартовая дата, текущая дата, месяцы (названия + дни)
  -- размер недели? как указать названия? тут вот как раз таки мне нужна локализация

  calendar.set_week_days_count(7)
  calendar.set_start_date(865, 3, 25) -- останется наверное только это
  calendar.set_current_date(865, 3, 25)
  calendar.add_month_data("month.jan", 31) -- январь
  calendar.add_month_data("month.feb", 29)
  calendar.add_month_data("month.mar", 31)
  calendar.add_month_data("month.apr", 30)
  calendar.add_month_data("month.may", 31) -- май
  calendar.add_month_data("month.jun", 30)
  calendar.add_month_data("month.jul", 31)
  calendar.add_month_data("month.aug", 31)
  calendar.add_month_data("month.sep", 30) -- сентябрь
  calendar.add_month_data("month.oct", 31)
  calendar.add_month_data("month.nov", 30)
  calendar.add_month_data("month.dec", 31)

  -- что делать с локализацией? локализация в моем случае - это набор строк
  -- значащих одно и тоже в разных языках, локализация обычно представляет из себя
  -- список локалей, который представляют из себя словарь (map) строк
  -- другое дело что может быть несколько строк по id в зависимости от числительного
  -- честно говоря в моем случае мне скорее нужны просто "raw" строки
  -- строки мне еще нужно будет "компилировать" при получении

  --utils.add_localization_table() -- наверное будет только второй вариант
  utils.load_localization_table("apates_quest/scripts/localization_config.lua")

  -- было бы неплохо сделать какой нибудь лоад_ресурс_конфиг,
  -- чтобы не указывать здесь каждый раз новые строки с загрузкой чего нибудь нового
  utils.load_interactions("apates_quest/scripts/first_interaction.lua")
  utils.load_decisions("apates_quest/scripts/first_decision.lua")

  --error("generate_cities end")

  function_timer:finish()
end -- generate_cities

return {
  generate_provinces = generate_provinces,
  province_postprocessing = province_postprocessing,
  calculating_province_neighbors = calculating_province_neighbors,
  generate_countries = generate_countries,
  generate_cultures = generate_cultures,
  generate_religions = generate_religions,
  add_provinces_data = add_provinces_data,
  generate_heraldy = generate_heraldy,
  generate_titles = generate_titles,
  generate_characters = generate_characters,
  generate_cities = generate_cities
}
