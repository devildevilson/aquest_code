--luacheck: no max line length
--luacheck: ignore local_table

-- показатели скорости супер плачевные, лучше разрешить здесь подключение либ через реквайр
-- иначе получается абсолютный ужас по производительности, в релизном билде все выглядит гораздо лучше
-- прежде всего проблемы создают методы из с++, там большой оверхед, но в релизном билде убирается
-- и цифры становятся терпимыми
-- prepare step took 704688 mcs
-- seting up generator took 76015 mcs
-- plate datas generating took 62980 mcs
-- plates generation took 3448251 mcs
-- boundary edges computation took 1100641 mcs
-- plate boundary stresses computation took 1687772 mcs
-- plate boundary distances computation took 14403939 mcs
-- vertex elevation computation took 11473439 mcs
-- tile elevation bluring took 4748286 mcs
-- normalizing values took 691789 mcs
-- tiles heat computation took 1982395 mcs
-- tile distances computation took 2850360 mcs
-- tiles moisture computation took 1366660 mcs
-- biomes creation took 2008528 mcs
-- все равно конечно довольно долго + почему то контейнер создается супер долго

-- тут нам похуй что мы добавляем сюда
local maf = require("apates_quest.scripts.maf")
local types = require("apates_quest.scripts.entities_types_table")
local queue = require("apates_quest.scripts.queue")
local biomes_config = require("apates_quest.scripts.biomes_config")

-- 64бит луа позволяет нам делать так
local function make_index_pair(index1, index2)
  return (index1 << 32) | index2
end

local function get_index_pair(index_pair)
  return (index_pair >> 32) & constants.uint32_max, index_pair & constants.uint32_max
end

local function compute_boundary_edges(ctx, local_table)
  local function_timer = generator.timer_t.new("boundary edges computation")
  --collectgarbage("collect")
  local maxf = math.max
  local minf = math.min

  local tiles_count = ctx.map:tiles_count()
  local pairs_set = utils.create_table(0, tiles_count)

  --local plates_count = local_table.final_plates_count

  local container = ctx.container
  utils.each_tile_neighbor(ctx, function(tile_index, neighbor_index)
    local current_plate_index       = container:get_data_u32(types.entities.tile, tile_index, types.properties.tile.plate_index)
    local neighbor_tile_plate_index = container:get_data_u32(types.entities.tile, neighbor_index, types.properties.tile.plate_index)
    if current_plate_index ~= neighbor_tile_plate_index then
      local max = maxf(tile_index, neighbor_index)
      local min = minf(tile_index, neighbor_index)
      local key = make_index_pair(min, max)
      pairs_set[key] = true
    end
  end)

  -- for i = 1, tiles_count do
  --   local current_tile_index = i
  --   local current_plate_index = ctx.container:get_data_u32(types.entities.tile, current_tile_index, types.properties.tile.plate_index)
  --   assert(current_plate_index <= plates_count)
  --   local tile = ctx.map:get_tile(current_tile_index)
  --   for j = 1, tile.n_count do
  --     local neighbor_tile_index = tile:get_neighbor_index(j)
  --     assert(current_tile_index ~= neighbor_tile_index)
  --
  --     local neighbor_tile_plate_index = ctx.container:get_data_u32(types.entities.tile, neighbor_tile_index, types.properties.tile.plate_index)
  --     assert(neighbor_tile_plate_index <= plates_count)
  --     if current_plate_index ~= neighbor_tile_plate_index then
  --       local max = maxf(current_tile_index, neighbor_tile_index)
  --       local min = minf(current_tile_index, neighbor_tile_index)
  --       assert(max < constants.uint32_max)
  --       assert(min < constants.uint32_max)
  --       local key = make_index_pair(min, max)
  --       -- входит ли число в основную часть числа с плавающей точкой
  --       -- хотя луа 5.3 по идее оперирует интами как основным типом
  --       -- (ну то есть арифметические операции и хранит он целочисленный тип как целочисленный)
  --       assert(key < constants.max_safe_integer)
  --       pairs_set[key] = true
  --     end
  --   end
  -- end

  -- найдет ли pairs все объекты в таблице? даже те что лежат в массиве таблицы? да
  local pairs_counter = utils.count(pairs_set)
  --for _, _ in pairs(pairs_set) do pairs_counter = pairs_counter + 1 end
  --local pairs_counter_mem = pairs_counter
  --for i, v in ipairs(pairs_set) do pairs_counter = pairs_counter + 1 end
  -- assert(pairs_counter_mem == pairs_counter)

  print("edges count " .. pairs_counter)
  --print("edges count " .. pairs_counter_mem)
  assert(pairs_counter ~= 0)

  ctx.container:set_entity_count(types.entities.edge, pairs_counter)
  local edge_counter = 1
  for k, _ in pairs(pairs_set) do
    local first, second = get_index_pair(k)

    -- ctx.container:add_child(types.entities.edge, edge_counter, first)
    -- ctx.container:add_child(types.entities.edge, edge_counter, second)
    ctx.container:set_data_u32(types.entities.edge, edge_counter, types.properties.edge.first_tile,  first)
    ctx.container:set_data_u32(types.entities.edge, edge_counter, types.properties.edge.second_tile, second)

    edge_counter = edge_counter + 1
  end

  --pairs_set = nil
  function_timer:finish()
end -- compute_boundary_edges

local function project(vector, normal)
  local num = normal:dot(normal)
  if num < constants.epsilon then return maf.vector(0.0, 0.0, 0.0) end

  local num2 = vector:dot(normal)
  return normal * (num2 / num)
end

local function calculate_axial_rotation(axis, rate, pos)
  local dir = maf.vector(0.0, 0.0, 0.0)
  axis:cross(pos, dir)
  local length = dir:length()
  if math.abs(length) < constants.epsilon then return maf.vector(0.0, 0.0, 0.0) end

  local p = project(pos, axis)
  local diff = pos - p
  local position_axis_distance = diff:length()
  return (dir / length) * (rate * position_axis_distance);
end

local function compute_plate_boundary_stress(ctx, local_table)
  local function_timer = generator.timer_t.new("plate boundary stresses computation")
  --collectgarbage("collect")

  local size = ctx.container:entities_count(types.entities.edge)
  local boundary_stresses = utils.init_array(size, {})
  local tiles_count = ctx.map:tiles_count()
  local plates_count = local_table.final_plates_count
  for i = 1, size do
    local first_tile  = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.first_tile)
    local second_tile = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.second_tile)
    local first_plate  = ctx.container:get_data_u32(types.entities.tile, first_tile, types.properties.tile.plate_index)
    local second_plate = ctx.container:get_data_u32(types.entities.tile, second_tile, types.properties.tile.plate_index)
    assert(first_tile <= tiles_count)
    assert(second_tile <= tiles_count)
    assert(first_plate <= plates_count)
    assert(second_plate <= plates_count)
    assert(first_plate ~= second_plate)
    local drift_axis1 = maf.vector(ctx.container:get_data_vec3(types.entities.plate, first_plate,  types.properties.plate.drift_axis))
    local drift_axis2 = maf.vector(ctx.container:get_data_vec3(types.entities.plate, second_plate, types.properties.plate.drift_axis))
    local spin_axis1  = maf.vector(ctx.container:get_data_vec3(types.entities.plate, first_plate,  types.properties.plate.spin_axis))
    local spin_axis2  = maf.vector(ctx.container:get_data_vec3(types.entities.plate, second_plate, types.properties.plate.spin_axis))
    local drift_rate1 = ctx.container:get_data_f32(types.entities.plate, first_plate,  types.properties.plate.drift_rate)
    local drift_rate2 = ctx.container:get_data_f32(types.entities.plate, second_plate, types.properties.plate.drift_rate)
    local spin_rate1  = ctx.container:get_data_f32(types.entities.plate, first_plate,  types.properties.plate.spin_rate)
    local spin_rate2  = ctx.container:get_data_f32(types.entities.plate, second_plate, types.properties.plate.spin_rate)

    local first_tile_data  = ctx.map:get_tile(first_tile)
    local second_tile_data = ctx.map:get_tile(second_tile)
    local first_tile_pos  = maf.vector(ctx.map:get_point(first_tile_data.center))
    local second_tile_pos = maf.vector(ctx.map:get_point(second_tile_data.center))

    local boundary_position = (first_tile_pos + second_tile_pos) / 2.0
    boundary_position:normalize()
    local boundary_normal = second_tile_pos - first_tile_pos
    boundary_normal:normalize()
    local boundary_vector = boundary_normal:cross(boundary_position, maf.vector()) -- так создается новая переменная boundary_vector
    boundary_vector:normalize()

    local plate_movement0 = calculate_axial_rotation(drift_axis1, drift_rate1, boundary_position) + calculate_axial_rotation(spin_axis1, spin_rate1, boundary_position)
    local plate_movement1 = calculate_axial_rotation(drift_axis2, drift_rate2, boundary_position) + calculate_axial_rotation(spin_axis2, spin_rate2, boundary_position)
--  local plate_movement0 = prop1.drift_axis * prop1.drift_rate + calculate_axial_rotation(prop1.spin_axis, prop1.spin_rate, boundary_position)
--  local plate_movement1 = prop2.drift_axis * prop2.drift_rate + calculate_axial_rotation(prop2.spin_axis, prop2.spin_rate, boundary_position)

    local relative_movement = plate_movement1 - plate_movement0
    local pressure_vector = project(relative_movement, boundary_normal)
    local pressure = pressure_vector:length()
    pressure = pressure_vector:dot(boundary_normal) < 0.0 and -pressure or pressure
    local shear_vector = project(relative_movement, boundary_vector)
    local shear = shear_vector:length()

    boundary_stresses[i] = {
      pressure = 2.0 / (1.0 + math.exp(-pressure * 33.33)) - 1, -- unused
      shear = 2.0 / (1.0 + math.exp(-shear * 33.33)) - 1,       -- unused
      plate_movement0 = plate_movement0,
      plate_movement1 = plate_movement1
    }
  end

  for i = 1, #boundary_stresses do
    ctx.container:set_data_vec3(types.entities.edge, i, types.properties.edge.plate0_movement, boundary_stresses[i].plate_movement0:unpack())
    ctx.container:set_data_vec3(types.entities.edge, i, types.properties.edge.plate1_movement, boundary_stresses[i].plate_movement1:unpack())
  end

  function_timer:finish()
end -- compute_plate_boundary_stress

local function assign_distance_field(ctx, seeds, stops, distances)
  --local edges_count = ctx.container:entities_count(types.entities.edge)
  --local index_queue = utils.create_table(ctx.map:tiles_count(), 0)
  utils.int_random_queue(ctx.random:num(), #seeds, function(index, queue_push)
    local edge_index = index
    if not seeds[edge_index] then return end

    local first_tile  = ctx.container:get_data_u32(types.entities.edge, edge_index, types.properties.edge.first_tile)
    local second_tile = ctx.container:get_data_u32(types.entities.edge, edge_index, types.properties.edge.second_tile)
    queue_push(first_tile)
    queue_push(second_tile)
    distances[first_tile] = utils.create_pair_u32f32(edge_index, 0)
    distances[second_tile] = utils.create_pair_u32f32(edge_index, 0)
  end, function(data, queue_push)
    local tile = ctx.map:get_tile(data)
    local current_plate_idx = ctx.container:get_data_u32(types.entities.tile, data, types.properties.tile.plate_index)
    local current_edge, dist = utils.unpack_pair_u32f32(distances[data])

    for i = 1, tile.n_count do
      local n_index = tile:get_neighbor_index(i) -- tile index
      local n_plate_idx = ctx.container:get_data_u32(types.entities.tile, n_index, types.properties.tile.plate_index)
      if current_plate_idx == n_plate_idx then
        -- edge index
        local _, n_dist = utils.unpack_pair_u32f32(distances[n_index])
        if n_dist == 100000.0 and not stops[n_index] then
          distances[n_index] = utils.create_pair_u32f32(current_edge, dist + 1.0)
          queue_push(n_index)
        end
      end
    end

  end)

  -- for k,v in pairs(seeds) do
  --   assert(k <= edges_count)
  --   local first_tile  = ctx.container:get_data_u32(types.entities.edge, k, types.properties.edge.first_tile)
  --   local second_tile = ctx.container:get_data_u32(types.entities.edge, k, types.properties.edge.second_tile)
  --   table.insert(index_queue, first_tile)
  --   table.insert(index_queue, second_tile)
  --   distances[first_tile] = utils.create_pair_u32f32(k, 0)
  --   distances[second_tile] = utils.create_pair_u32f32(k, 0)
  -- end
  --
  -- while #index_queue ~= 0 do
  --   local rand_index = ctx.random:index(#index_queue)
  --   local current_tile = index_queue[rand_index]
  --   index_queue[rand_index] = index_queue[#index_queue]
  --   index_queue[#index_queue] = nil
  --
  --   local tile = ctx.map:get_tile(current_tile)
  --   local current_plate_idx = ctx.container:get_data_u32(types.entities.tile, current_tile, types.properties.tile.plate_index)
  --   local current_edge, dist = utils.unpack_pair_u32f32(distances[current_tile])
  --   for i = 1, tile.n_count do
  --     local n_index = tile:get_neighbor_index(i)
  --     local n_plate_idx = ctx.container:get_data_u32(types.entities.tile, n_index, types.properties.tile.plate_index)
  --     if current_plate_idx == n_plate_idx then
  --       local id, n_dist = utils.unpack_pair_u32f32(distances[n_index])
  --       if n_dist == 100000.0 and not stops[n_index] then
  --         distances[n_index] = utils.create_pair_u32f32(current_edge, dist + 1.0)
  --         table.insert(index_queue, n_index)
  --       end
  --     end
  --   end
  -- end
end

local function compute_plate_boundary_distances(ctx, local_table)
  local function_timer = generator.timer_t.new("plate boundary distances computation")
  --collectgarbage("collect")

  -- найдем дальности от тайла к границам плиты
  local tiles_count = ctx.map:tiles_count()
  local edges_count = ctx.container:entities_count(types.entities.edge)
  --local plates_count = ctx.container:entities_count(types.entities.plate)
  --local tile_index_queue = queue.new()
  local unique_tiles = utils.init_array(tiles_count, false) -- игнорируем тайлы на границе
  local edge_index_dist = utils.init_array(tiles_count, 1.0)

  assert(edges_count ~= 0)

  utils.int_queue(edges_count, function(index, queue_push)
    local first_tile  = ctx.container:get_data_u32(types.entities.edge, index, types.properties.edge.first_tile)
    local second_tile = ctx.container:get_data_u32(types.entities.edge, index, types.properties.edge.second_tile)

    assert(first_tile  <= tiles_count)
    assert(second_tile <= tiles_count)

    queue_push(first_tile)
    queue_push(second_tile)
    unique_tiles[first_tile]  = true
    unique_tiles[second_tile] = true

    local tile1 = ctx.map:get_tile(first_tile)
    local tile2 = ctx.map:get_tile(second_tile)

    local point1 = maf.vector(ctx.map:get_point(tile1.center))
    local point2 = maf.vector(ctx.map:get_point(tile2.center))

    local dist = point1:distance(point2) / 2
    edge_index_dist[first_tile]  = utils.create_pair_u32f32(index, dist)
    edge_index_dist[second_tile] = utils.create_pair_u32f32(index, dist)
  end, function(data, queue_push)
    local tile = ctx.map:get_tile(data)
    local point1 = maf.vector(ctx.map:get_point(tile.center))
    for i = 1, tile.n_count do
      local n_index = tile:get_neighbor_index(i)
      if not unique_tiles[n_index] then
        local n_tile = ctx.map:get_tile(n_index)
        local point2 = maf.vector(ctx.map:get_point(n_tile.center))

        local edge_index, dist = utils.unpack_pair_u32f32(edge_index_dist[data])
        assert(edge_index <= edges_count)
        -- должно быть деление на 2 как в предыдущем? нет,
        -- тут мы находим растояние до ближайшей границы для каждого тайла
        local new_dist = point1:distance(point2) + dist
        local _, dist2 = utils.unpack_pair_u32f32(edge_index_dist[n_index])
        if dist2 > new_dist then
          edge_index_dist[n_index] = utils.create_pair_u32f32(edge_index, new_dist)
          queue_push(n_index)
        end
      end
    end
  end)

  -- for i = 1, edges_count do
  --   local first_tile  = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.first_tile)
  --   local second_tile = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.second_tile)
  --
  --   assert(first_tile  <= tiles_count)
  --   assert(second_tile <= tiles_count)
  --
  --   tile_index_queue:push_right(first_tile)
  --   tile_index_queue:push_right(second_tile)
  --   unique_tiles[first_tile]  = true
  --   unique_tiles[second_tile] = true
  --
  --   local tile1 = ctx.map:get_tile(first_tile)
  --   local tile2 = ctx.map:get_tile(second_tile)
  --
  --   local point1 = maf.vector(ctx.map:get_point(tile1.center))
  --   local point2 = maf.vector(ctx.map:get_point(tile2.center))
  --
  --   local dist = point1:distance(point2) / 2
  --   edge_index_dist[first_tile]  = utils.create_pair_u32f32(i, dist)
  --   edge_index_dist[second_tile] = utils.create_pair_u32f32(i, dist)
  -- end
  --
  -- function_timer:checkpoint("initial queue creation")
  --
  -- while not tile_index_queue:is_empty() do
  --   local index = tile_index_queue:pop_left()
  --   local tile = ctx.map:get_tile(index)
  --   local point1 = maf.vector(ctx.map:get_point(tile.center))
  --   for i = 1, tile.n_count do
  --     local n_index = tile:get_neighbor_index(i)
  --     if not unique_tiles[n_index] then
  --       local n_tile = ctx.map:get_tile(n_index)
  --       local point2 = maf.vector(ctx.map:get_point(n_tile.center))
  --
  --       local edge_index, dist = utils.unpack_pair_u32f32(edge_index_dist[index])
  --       assert(edge_index <= edges_count)
  --       -- должно быть деление на 2 как в предыдущем? нет,
  --       -- тут мы находим растояние до ближайшей границы для каждого тайла
  --       local new_dist = point1:distance(point2) + dist
  --       local edge_index2, dist2 = utils.unpack_pair_u32f32(edge_index_dist[n_index])
  --       if dist2 > new_dist then
  --         edge_index_dist[n_index] = utils.create_pair_u32f32(edge_index, new_dist)
  --         tile_index_queue:push_right(n_index)
  --       end
  --     end
  --   end
  -- end -- while

  --tile_index_queue = nil
  unique_tiles = nil

  for i = 1, #edge_index_dist do
    local edge_index, dist = utils.unpack_pair_u32f32(edge_index_dist[i])
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.edge_index, edge_index)
    ctx.container:set_data_f32(types.entities.tile, i, types.properties.tile.edge_dist,  dist)
  end
  edge_index_dist = nil

  function_timer:checkpoint("tile distances is setted")

  local mountains  = utils.init_array(edges_count, false)
  local oceans     = utils.init_array(edges_count, false)
  local coastlines = utils.init_array(edges_count, false)
  for i = 1, edges_count do
    local first_tile  = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.first_tile)
    local second_tile = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.second_tile)
    local plate_index0 = ctx.container:get_data_u32(types.entities.tile, first_tile, types.properties.tile.plate_index)
    local plate_index1 = ctx.container:get_data_u32(types.entities.tile, second_tile, types.properties.tile.plate_index)

    local data0_oceanic = ctx.container:get_data_u32(types.entities.plate, plate_index0, types.properties.plate.oceanic)
    local data1_oceanic = ctx.container:get_data_u32(types.entities.plate, plate_index1, types.properties.plate.oceanic)

    if data0_oceanic == 0 then data0_oceanic = false else data0_oceanic = true end
    if data1_oceanic == 0 then data1_oceanic = false else data1_oceanic = true end

    assert(type(data0_oceanic) == "boolean")
    assert(type(data1_oceanic) == "boolean")

    -- необходимо какое-то ограничение или коэффициент
    -- иначе получается плохо
    -- у нас задача: раскидать границы по эффектам столкновения и расхождения плит
    -- в зависимости от типа взаимодействия на границе мы получаем разные типы поверхностей

    local plate_movement0 = maf.vector(ctx.container:get_data_vec3(types.entities.edge, i, types.properties.edge.plate0_movement))
    local plate_movement1 = maf.vector(ctx.container:get_data_vec3(types.entities.edge, i, types.properties.edge.plate1_movement))

    local length0 = plate_movement0:length()
    local length1 = plate_movement1:length()

    local dir0 = length0 < constants.epsilon and maf.vector(0.0, 0.0, 0.0) or plate_movement0 / length0
    local dir1 = length1 < constants.epsilon and maf.vector(0.0, 0.0, 0.0) or plate_movement1 / length1

    local first_tile_data  = ctx.map:get_tile(first_tile)
    local second_tile_data = ctx.map:get_tile(second_tile)
    local first_tile_pos  = maf.vector(ctx.map:get_point(first_tile_data.center))
    local second_tile_pos = maf.vector(ctx.map:get_point(second_tile_data.center))

    --local boundary_position = (second_tile_pos + first_tile_pos) / 2.0f
    local boundary_normal = second_tile_pos - first_tile_pos
    boundary_normal:normalize()

    local dot0 = dir0:dot(boundary_normal)
    local dot1 = dir1:dot(-boundary_normal)

    local collided = dot0 > 0.3 and dot1 > 0.3
    local opposite_dir = dot0 < -0.3 and dot1 < -0.3
    if data0_oceanic and data1_oceanic then

      if collided then         mountains[i] = true
      elseif opposite_dir then oceans[i] = true
      else                     oceans[i] = true end

    elseif not data0_oceanic and not data1_oceanic then

      if collided then         mountains[i] = true
      elseif opposite_dir then oceans[i] = true
      else                     coastlines[i] = true end

    else

      if collided then         mountains[i] = true
      elseif opposite_dir then oceans[i] = true
      else                     coastlines[i] = true end

    end
  end -- for

  function_timer:checkpoint("making plate forces")

  local mountain_dist  = utils.init_array(tiles_count, utils.create_pair_u32f32(constants.uint32_max, 100000.0))
  local ocean_dist     = utils.init_array(tiles_count, utils.create_pair_u32f32(constants.uint32_max, 100000.0))
  local coastline_dist = utils.init_array(tiles_count, utils.create_pair_u32f32(constants.uint32_max, 100000.0))
  do -- sanity
    local a, b = utils.unpack_pair_u32f32(coastline_dist[1])
    assert(a == constants.uint32_max)
    assert(b == 100000.0)
  end

  local stops = utils.create_table(0, edges_count)
  local mountains_count = 0
  local oceans_count = 0
  local coastlines_count = 0
  assert(#mountains ~= 0)
  for i = 1, #mountains do
    if mountains[i] then
      local first_tile  = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.first_tile)
      local second_tile = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.second_tile)
      stops[first_tile] = true
      stops[second_tile] = true
      mountains_count = mountains_count + 1
    end
  end

  assert(mountains_count ~= 0)

  for i,v in ipairs(oceans) do
    if v then
      local first_tile  = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.first_tile)
      local second_tile = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.second_tile)
      stops[first_tile] = true
      stops[second_tile] = true
      oceans_count = oceans_count + 1
    end
  end

  assert(oceans_count ~= 0)

  for i,v in ipairs(coastlines) do
    if v then
      local first_tile  = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.first_tile)
      local second_tile = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.second_tile)
      stops[first_tile] = true
      stops[second_tile] = true
      coastlines_count = coastlines_count + 1
    end
  end

  assert(coastlines_count ~= 0)

  local ocean_stops = utils.create_table(0, oceans_count)
  local coastline_stops = utils.create_table(0, coastlines_count)

  for i,v in ipairs(oceans) do
    if v then
      local first_tile  = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.first_tile)
      local second_tile = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.second_tile)
      ocean_stops[first_tile] = true
      ocean_stops[second_tile] = true
    end
  end

  for i,v in ipairs(coastlines) do
    if v then
      local first_tile  = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.first_tile)
      local second_tile = ctx.container:get_data_u32(types.entities.edge, i, types.properties.edge.second_tile)
      coastline_stops[first_tile] = true
      coastline_stops[second_tile] = true
    end
  end

  function_timer:checkpoint("making stop condition")

  -- по умолчанию у меня сделаны вычисления по плитам
  -- но кажется можно и так, мне просто нужно проверить относятся ли два тайла к одной плите
  assign_distance_field(ctx, mountains, ocean_stops, mountain_dist)
  assign_distance_field(ctx, oceans, coastline_stops, ocean_dist)
  assign_distance_field(ctx, coastlines, stops, coastline_dist)

  -- do -- примерно в 10 раз быстрее без оптимизаций (боже луа)
  --   local type_indices = {
  --     edge_type = types.entities.edge,
  --     edge_property_first_index = types.properties.edge.first_tile,
  --     edge_property_second_index = types.properties.edge.second_tile,
  --     tile_type = types.entities.tile,
  --     tile_property_plate_index = types.properties.tile.plate_index
  --   }
  --   utils.assign_distance_field(ctx, type_indices, mountains, ocean_stops, mountain_dist)
  --   utils.assign_distance_field(ctx, type_indices, oceans, coastline_stops, ocean_dist)
  --   utils.assign_distance_field(ctx, type_indices, coastlines, stops, coastline_dist)
  -- end

  function_timer:checkpoint("making distance fields")

  for i = 1, tiles_count do
    local edge_index, dist = utils.unpack_pair_u32f32(mountain_dist[i])
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.mountain_index, edge_index)
    ctx.container:set_data_f32(types.entities.tile, i, types.properties.tile.mountain_dist,  dist)
  end

  for i = 1, tiles_count do
    local edge_index, dist = utils.unpack_pair_u32f32(ocean_dist[i])
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.ocean_index, edge_index)
    ctx.container:set_data_f32(types.entities.tile, i, types.properties.tile.ocean_dist,  dist)
  end

  for i = 1, tiles_count do
    local edge_index, dist = utils.unpack_pair_u32f32(coastline_dist[i])
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.coastline_index, edge_index)
    ctx.container:set_data_f32(types.entities.tile, i, types.properties.tile.coastline_dist,  dist)
  end

  -- for sanity (luacheck cannot ignore it)
  -- mountains = nil
  -- oceans = nil
  -- coastlines = nil
  -- ocean_stops = nil
  -- coastline_stops = nil
  -- stops = nil
  -- mountain_dist = nil
  -- ocean_dist = nil
  -- coastline_dist = nil

  function_timer:finish()
end -- compute_plate_boundary_distances

local function mix_val(x, y, val) return x + val * (y - x) end
local function clamp(low, n, high) return math.min(math.max(n, low), high) end

local function calculate_vertex_elevation(ctx, local_table)
  local function_timer = generator.timer_t.new("vertex elevation computation")
  --collectgarbage("collect")
  local maxf = math.max
  local minf = math.min
  local absf = math.abs

  ctx.noise:set_noise_type(utils.noiser.noise_type.NoiseType_Perlin)

  local noise_multiplier_local = local_table.userdata.noise_multiplier
  local tiles_count = ctx.map:tiles_count()
  local tile_elevation = utils.init_array(tiles_count, -10)
  for i = 1, tiles_count do
    local accum_elevation = 0.0 -- luacheck: ignore accum_elevation
    local elevations_count = 0
    local plate_index = ctx.container:get_data_u32(types.entities.tile, i, types.properties.tile.plate_index)
    local tile_index = i
    local tile = ctx.map:get_tile(tile_index)
    local tile_point = maf.vector(ctx.map:get_point(tile.center))

    local plate_elevation = ctx.container:get_data_f32(types.entities.plate, plate_index, types.properties.plate.base_elevation)
    elevations_count = elevations_count + 1

    local a_index = ctx.container:get_data_u32(types.entities.tile, i, types.properties.tile.mountain_index)
    local a_dist  = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.mountain_dist)
    local b_index = ctx.container:get_data_u32(types.entities.tile, i, types.properties.tile.ocean_index)
    local b_dist  = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.ocean_dist)
    local c_index = ctx.container:get_data_u32(types.entities.tile, i, types.properties.tile.coastline_index)
    local c_dist  = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.coastline_dist)

    a_dist = a_dist + constants.epsilon
    b_dist = b_dist + constants.epsilon
    c_dist = c_dist + constants.epsilon

    local max_k =  1.0
    local min_k = -1.0
    local a_k = max_k
    local b_k = min_k
    do
      -- больший коэффициент горы, меньший воды
      if a_index ~= constants.uint32_max then
        local edge_index = a_index
        local first_tile  = ctx.container:get_data_u32(types.entities.edge, edge_index, types.properties.edge.first_tile)
        local second_tile = ctx.container:get_data_u32(types.entities.edge, edge_index, types.properties.edge.second_tile)
        local plate0 = ctx.container:get_data_u32(types.entities.tile, first_tile,  types.properties.tile.plate_index)
        local plate1 = ctx.container:get_data_u32(types.entities.tile, second_tile, types.properties.tile.plate_index)
        local opposing_plate_index = plate_index == plate0 and plate1 or plate0

        local opposing_plate_elevation = ctx.container:get_data_f32(types.entities.plate, opposing_plate_index, types.properties.plate.base_elevation)
        local boundary_elevation = maxf(opposing_plate_elevation, plate_elevation)

        -- к boundary_elevation нужно прибавить какую то силу
        -- максимум который мы можем прибавить это 0.5f
        -- как его расчитать, мы можем взять доты как на предыдущих шагах
        -- но доты нужно как то компенсировать силой

        -- необходимо какое-то ограничение или коэффициент
        -- иначе получается плохо
        -- у нас задача: раскидать границы по эффектам столкновения и расхождения плит
        -- в зависимости от типа взаимодействия на границе мы получаем разные типы поверхностей

        local plate_movement0 = maf.vector(ctx.container:get_data_vec3(types.entities.edge, edge_index, types.properties.edge.plate0_movement))
        local plate_movement1 = maf.vector(ctx.container:get_data_vec3(types.entities.edge, edge_index, types.properties.edge.plate1_movement))

        local length0 = plate_movement0:length()
        local length1 = plate_movement1:length()

        local dir0 = length0 < constants.epsilon and maf.vector(0.0, 0.0, 0.0) or plate_movement0 / length0
        local dir1 = length1 < constants.epsilon and maf.vector(0.0, 0.0, 0.0) or plate_movement1 / length1

        local first_tile_data = ctx.map:get_tile(first_tile)
        local second_tile_data = ctx.map:get_tile(second_tile);
        local first_tile_pos  = maf.vector(ctx.map:get_point(first_tile_data.center))
        local second_tile_pos = maf.vector(ctx.map:get_point(second_tile_data.center))

        --local boundary_position = (second_tile_pos + first_tile_pos) / 2.0f
        local boundary_normal = second_tile_pos - first_tile_pos
        boundary_normal:normalize()

        local dot0 = dir0:dot(boundary_normal)
        local dot1 = dir1:dot(-boundary_normal)

        local dot_k = (dot0 + dot1) / 2.0
        local final_k = boundary_elevation < 0.0 and maxf(-boundary_elevation * (1.0 + dot_k) * 0.8, -boundary_elevation) or mix_val(-0.2, 0.4, dot_k)

        a_k = boundary_elevation + final_k;

        assert(a_k >= 0.0 and a_k <= 1.0)
      end

      -- наоборот
      if b_index ~= constants.uint32_max then
        local edge_index = b_index
        local first_tile  = ctx.container:get_data_u32(types.entities.edge, edge_index, types.properties.edge.first_tile)
        local second_tile = ctx.container:get_data_u32(types.entities.edge, edge_index, types.properties.edge.second_tile)
        local plate0 = ctx.container:get_data_u32(types.entities.tile, first_tile,  types.properties.tile.plate_index)
        local plate1 = ctx.container:get_data_u32(types.entities.tile, second_tile, types.properties.tile.plate_index)
        local opposing_plate_index = plate_index == plate0 and plate1 or plate0

        local opposing_plate_elevation = ctx.container:get_data_f32(types.entities.plate, opposing_plate_index, types.properties.plate.base_elevation)
        local boundary_elevation = minf(opposing_plate_elevation, plate_elevation)

        -- к boundary_elevation нужно прибавить какую то силу
        -- максимум который мы можем прибавить это 0.5f
        -- как его расчитать, мы можем взять доты как на предыдущих шагах
        -- но доты нужно как то компенсировать силой

        -- необходимо какое-то ограничение или коэффициент
        -- иначе получается плохо
        -- у нас задача: раскидать границы по эффектам столкновения и расхождения плит
        -- в зависимости от типа взаимодействия на границе мы получаем разные типы поверхностей

        local plate_movement0 = maf.vector(ctx.container:get_data_vec3(types.entities.edge, edge_index, types.properties.edge.plate0_movement))
        local plate_movement1 = maf.vector(ctx.container:get_data_vec3(types.entities.edge, edge_index, types.properties.edge.plate1_movement))

        local length0 = plate_movement0:length()
        local length1 = plate_movement1:length()

        local dir0 = length0 < constants.epsilon and maf.vector(0.0, 0.0, 0.0) or plate_movement0 / length0
        local dir1 = length1 < constants.epsilon and maf.vector(0.0, 0.0, 0.0) or plate_movement1 / length1

        local first_tile_data = ctx.map:get_tile(first_tile)
        local second_tile_data = ctx.map:get_tile(second_tile);
        local first_tile_pos  = maf.vector(ctx.map:get_point(first_tile_data.center))
        local second_tile_pos = maf.vector(ctx.map:get_point(second_tile_data.center))

        --local boundary_position = (second_tile_pos + first_tile_pos) / 2.0f
        local boundary_normal = second_tile_pos - first_tile_pos
        boundary_normal:normalize()

        local dot0 = dir0:dot(boundary_normal)
        local dot1 = dir1:dot(-boundary_normal)

        local dot_k = 1.0 - absf(dot0 + dot1) / 2.0 -- нужно увеличить К воды
        local final_k = boundary_elevation > 0.0 and minf(-boundary_elevation * (1.0 + dot_k) * 0.8, -boundary_elevation) or mix_val(-0.2, 0.05, dot_k)

        b_k = boundary_elevation + final_k;

        assert(b_k >= -1.0 and b_k <= 0.0)
      end
    end

    -- тут нужно учесть текущий подъем везде
    if a_index == constants.uint32_max and b_index == constants.uint32_max then
      local edge_index = c_index
      --print("edge_index " .. edge_index)
      local first_tile  = ctx.container:get_data_u32(types.entities.edge, edge_index, types.properties.edge.first_tile)
      local second_tile = ctx.container:get_data_u32(types.entities.edge, edge_index, types.properties.edge.second_tile)
      local plate0 = ctx.container:get_data_u32(types.entities.tile, first_tile,  types.properties.tile.plate_index)
      local plate1 = ctx.container:get_data_u32(types.entities.tile, second_tile, types.properties.tile.plate_index)
      local opposing_plate_index = plate_index == plate0 and plate1 or plate0
      local opposing_plate_elevation = ctx.container:get_data_f32(types.entities.plate, opposing_plate_index, types.properties.plate.base_elevation)
      accum_elevation = (opposing_plate_elevation + plate_elevation) / 2.0
    else
      accum_elevation = (a_k/a_dist + b_k/b_dist) / (1.0/a_dist + 1.0/b_dist + 1.0/c_dist)
    end

    -- при каких интересно значениях будет больше 1
    accum_elevation = accum_elevation + noise_multiplier_local * ctx.noise:get_noise(tile_point.x, tile_point.y, tile_point.z)
    --accum_elevation = (accum_elevation - (-0.9)) / (1.1 - (-0.9))
    accum_elevation = clamp(accum_elevation, -1.0, 1.0)

    assert(accum_elevation == accum_elevation)
    assert(accum_elevation >= -1.0 and accum_elevation <= 1.0)

    assert(elevations_count ~= 0);

    tile_elevation[i] = accum_elevation / elevations_count
  end

  for i = 1, #tile_elevation do
    ctx.container:set_data_f32(types.entities.tile, i, types.properties.tile.elevation, tile_elevation[i])
    --ctx.map:set_tile_height(i, tile_elevation[i])
  end

  ctx.map:set_tiles_height(ctx.container, types.properties.tile.elevation)
  function_timer:finish()
end -- calculate_vertex_elevation

local function blur_tile_elevation(ctx, local_table)
  local function_timer = generator.timer_t.new("tile elevation bluring")
  --collectgarbage("collect")

  local tiles_count = ctx.map:tiles_count()

  local new_elevations = utils.init_array(tiles_count, 0.0)

  local new_old_ratio_local = local_table.userdata.blur_ratio
  local water_ground_ratio_local = local_table.userdata.blur_water_ratio
  local iterations_count = local_table.userdata.blur_iterations_count

  for _ = 1, iterations_count do
    for i = 1, tiles_count do
      new_elevations[i] = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.elevation)
    end

    for i = 1, tiles_count do
      local tile = ctx.map:get_tile(i)
      local old_elevation = new_elevations[i]

      local accum_elevation = 0.0 -- luacheck: ignore accum_elevation
      local accum_water = 0.0
      local accum_ground = 0.0
      for j = 1, tile.n_count do
        local n_index = tile:get_neighbor_index(j)
        --local n_old_elevation = ctx.container:get_data_f32(types.entities.tile, n_index, types.properties.tile.elevation)
        local n_old_elevation = new_elevations[n_index]
        --accum_elevation = accum_elevation + n_old_elevation
        if n_old_elevation <= 0.0 then accum_water = accum_water + n_old_elevation
        else accum_ground = accum_ground + n_old_elevation end
      end
      --accum_elevation += old_elevation;
      accum_water = accum_water * water_ground_ratio_local
      accum_ground = accum_ground * (2.0 - water_ground_ratio_local)
      accum_elevation = (accum_water + accum_ground) / tile.n_count

      local sum_k = new_old_ratio_local
      --new_elevations[i] = old_elevation * sum_k + accum_elevation * (1.0 - sum_k)
      local elev = old_elevation * sum_k + accum_elevation * (1.0 - sum_k)
      ctx.container:set_data_f32(types.entities.tile, i, types.properties.tile.elevation, elev)
    end
  end

  for i = 1, tiles_count do
    ctx.container:set_data_f32(types.entities.tile, i, types.properties.tile.elevation, new_elevations[i])
  end

  ctx.map:set_tiles_height(ctx.container, types.properties.tile.elevation)

  local water_counter = 0
  for i = 1, tiles_count do
    local h = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.elevation)
    --ctx.map:set_tile_height(i, h)
    if h < 0.0 then water_counter = water_counter + 1 end
  end

  print("oceanic tiles after recompute " .. water_counter)
  print("oceanic tiles k               " .. (water_counter / ctx.map:tiles_count()))
  function_timer:finish()
end -- blur_tile_elevation

local function normalize_fractional_values(ctx, local_table, entity_id, property_id)
  local function_timer = generator.timer_t.new("normalizing values")
  local maxf = math.max
  local minf = math.min
  local absf = math.abs

  local minimum =  100000
  local maximum = -100000
  local entity_count = ctx.container:entities_count(entity_id)
  --local new_data = utils.init_array(entity_count, 0)
  for i = 1, entity_count do
    local data = ctx.container:get_data_f32(entity_id, i, property_id)
    minimum = minf(minimum, data)
    maximum = maxf(maximum, data)
  end

  -- по идее нужно использовать диапазон от нуля до максимума, знак запоминать
  local minimum_abs = absf(minimum)
  for i = 1, entity_count do
    local data = ctx.container:get_data_f32(entity_id, i, property_id)
    if data >= 0.0 then data = (data - 0.0) / (maximum - 0.0)
    else
      local data_abs = absf(data)
      data_abs = (data_abs - 0.0) / (minimum_abs - 0.0)
      data = -data_abs
    end

    ctx.container:set_data_f32(entity_id, i, property_id, data)
  end

  function_timer:finish()
end

local function normalize_tile_elevation(ctx, local_table)
  normalize_fractional_values(ctx, local_table, types.entities.tile, types.properties.tile.elevation)
  -- тут можно по слоям распределить высоты, мне нужно сделать воду ровной поверхностью
  ctx.map:set_tiles_height(ctx.container, types.properties.tile.elevation)
end

local function mapper(value, smin, smax, dmin, dmax)
  return ((value - smin) / (smax - smin)) * (dmax - dmin) + dmin
end

local function compute_tile_heat(ctx, local_table)
  local function_timer = generator.timer_t.new("tiles heat computation")
  local maxf = math.max
  local minf = math.min
  local absf = math.abs
  --update_noise_seed(ctx)

  local tiles_count = ctx.map:tiles_count()
  local tile_heat = utils.init_array(tiles_count, 0.0)

  local noise_k = 0.1

  local min_value =  1.0
  local max_value = -1.0
  for i = 1, tiles_count do
    local tile = ctx.map:get_tile(i)
    local x,y,z = ctx.map:get_point(tile.center)
    -- скорее всего нужно увеличить вклад шума
    tile_heat[i] = ctx.noise:get_noise(x, y, z)
    min_value = minf(min_value, tile_heat[i])
    max_value = maxf(max_value, tile_heat[i])
  end

  local min_value_abs = absf(min_value)
  for i = 1, tiles_count do
    if tile_heat[i] >= 0.0 then tile_heat[i] = (tile_heat[i] - 0.0) / (max_value - 0.0)
    else
      local abs_val = absf(tile_heat[i])
      tile_heat[i] = -((abs_val - 0.0) / (min_value_abs - 0.0))
    end

    tile_heat[i] = noise_k * tile_heat[i]
  end

  for i = 1, tiles_count do
    local height = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.elevation)

    local tile = ctx.map:get_tile(i)
    local point = maf.vector(ctx.map:get_point(tile.center))
    point:normalize()
    local h_part = mix_val(-0.9, 0.2, 1.0 - height)
    local h_k = h_part;
    local y_part = 1.0 - absf(point.y)
    local final_y = y_part + h_k * y_part
    local k = clamp(final_y, 0.0, 1.0)

    tile_heat[i] = tile_heat[i] + k
    tile_heat[i] = mapper(tile_heat[i], -0.1, 1.1, 0.0, 1.0)
  end

  for i = 1, tiles_count do
    ctx.container:set_data_f32(types.entities.tile, i, types.properties.tile.heat, tile_heat[i])
  end

  function_timer:finish()
end

local function compute_tile_distances(ctx, local_table, predicate, index_property_id, dist_property_id)
  local function_timer = generator.timer_t.new("tile distances computation")
  local tiles_count = ctx.map:tiles_count()
  local ground_distance = utils.init_array(tiles_count, make_index_pair(constants.uint32_max, constants.uint32_max))
  local index_queue = queue.new()

  for i = 1, tiles_count do
    if predicate(ctx, local_table, i) then
      index_queue:push_right(i)
      ground_distance[i] = make_index_pair(i, 0)
    end
  end

  while not index_queue:is_empty() do
    local current_tile = index_queue:pop_left()

    local index, dist = get_index_pair(ground_distance[current_tile])
    local tile = ctx.map:get_tile(current_tile)
    for i = 1, tile.n_count do
      local n_index = tile:get_neighbor_index(i)
      local _, n_dist = get_index_pair(ground_distance[n_index])
      if n_dist == constants.uint32_max then
        ground_distance[n_index] = make_index_pair(index, dist + 1)
        index_queue:push_right(n_index)
      end
    end
  end

  for i = 1, tiles_count do
    local index, dist = get_index_pair(ground_distance[i])
    ctx.container:set_data_u32(types.entities.tile, i, index_property_id, index)
    ctx.container:set_data_u32(types.entities.tile, i, dist_property_id,  dist)
  end

  function_timer:finish()
end

local function compute_tile_water_distances(ctx, local_table)
  compute_tile_distances(ctx, local_table, function(context, local_table, index)
    local h = context.container:get_data_f32(types.entities.tile, index, types.properties.tile.elevation)
    return h < 0.0
  end, types.properties.tile.water_index, types.properties.tile.water_dist)
end

local function compute_tile_moisture(ctx, local_table)
  local function_timer = generator.timer_t.new("tiles moisture computation")
  local maxf = math.max
  local minf = math.min
  --local absf = math.abs
  --update_noise_seed(ctx);

  local tiles_count = ctx.map:tiles_count()
  local wetness = utils.init_array(tiles_count, 0.0)
  local max_val = -1.0
  local min_val =  1.0
  for i = 1, tiles_count do
    local tile = ctx.map:get_tile(i)
    local x,y,z = ctx.map:get_point(tile.center)
    local k = ctx.noise:get_noise(x, y, z)
    wetness[i] = k
    max_val = maxf(max_val, wetness[i])
    min_val = minf(min_val, wetness[i])
  end

  for i = 1, tiles_count do
    wetness[i] = (wetness[i] - min_val) / (max_val - min_val)
    --local height = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.elevation)
    --local heat   = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.heat)
    wetness[i] = wetness[i] * wetness[i] --  * heat
    --local dist_to_water = local_table["tiles"][i]["water_distance"]["dist"];
    local dist_to_water = ctx.container:get_data_u32(types.entities.tile, i, types.properties.tile.water_dist)
    if dist_to_water > 0 then
      local dist_k = 1.0 - (dist_to_water > 3 and 3 or dist_to_water) / 3.0
      wetness[i] = minf(wetness[i] + wetness[i] * dist_k, 1.0)
    end
  end

  for i = 1, tiles_count do
    ctx.container:set_data_f32(types.entities.tile, i, types.properties.tile.moisture, wetness[i])
  end

  function_timer:finish()
end

local function calcutate_biome(elevation, temperature, wetness)
  if elevation < 0.0 then -- вода
    if temperature < 0.3 then return "biome_ocean_glacier"
    else return "biome_ocean" end
  elseif elevation < 0.6 then -- обычная территория
    if temperature > 0.75 then

      if wetness > 0.75 then return "biome_rain_forest"
      elseif wetness > 0.5 then return "biome_rain_forest"
      elseif wetness > 0.25 then return "biome_grassland"
      else return "biome_desert" end

    elseif temperature > 0.5 then

      if wetness > 0.75 then return "biome_rain_forest"
      elseif wetness > 0.5 then return "biome_deciduous_forest"
      elseif wetness > 0.25 then return "biome_grassland"
      else return "biome_desert" end

    elseif temperature > 0.25 then

      if wetness > 0.75 then return "biome_deciduous_forest"
      elseif wetness > 0.5 then return "biome_conifer_forest"
      elseif wetness > 0.25 then return "biome_grassland"
      else return "biome_desert" end

    elseif temperature > 0.15 then
      return "biome_tundra"
    else
      return "biome_land_glacier"
    end
  else -- горы
    if temperature > 0.5 then return "biome_mountain"
    else return "biome_snowy_mountain" end
  end
end

local function setup_table(t, _)
  for i = 1, #biomes_config do
    --local index = seasons:add_biome(biomes_config[i])
    local index = i

    if t[biomes_config[i].id] ~= nil then
      error("Biome id collision " .. biomes_config[i].id)
    end

    t[biomes_config[i].id] = {t = biomes_config[i], index = index}
  end
end

local function create_biomes(ctx, local_table)
  local function_timer = generator.timer_t.new("biomes creation")
  -- так вот мы пришли к самому сложному на данный момент
  -- биомы в генераторе я задаю напрямую, а мне нужно пихать таблицу с биомами
  -- ко всему прочему было бы нелохо отделть таблицу от генератора чтобы
  -- иметь возможность что то исправить без перегенерации всей карты
  -- тут нужно придумать какой то механизм... я думал о том чтобы пихать туда строку, по этой
  -- строке искать луа скрипт, который вернет массив таблиц, но тут возникает вопрос:
  -- сколько этот скрипт должен возвращать? что этот скрипт должен получить?
  -- должен ли этот скрипт иметь возможность работать с файловой системой?
  -- (думаю, что только через require, который должен быть сильно модифицирован)
  utils.load_images("apates_quest/scripts/images_config.lua") -- добавляет скрипт, который вернет пачку таблиц
  utils.load_biomes("apates_quest/scripts/biomes_config.lua") -- вместо add_biome мы можем сделать load_biomes
  -- в эту функцию будем передавать путь до луа скрипта на загрузку

  -- теперь биомы у меня грузятся из конфига

  local biome_table = {}
  setup_table(biome_table, ctx.seasons)
  local season_index = ctx.seasons:allocate_season()

  local tiles_count = ctx.map:tiles_count()
  for i = 1, tiles_count do
    local elevation = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.elevation)
    local temperature = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.heat)
    local wetness = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.moisture)
    local biome_id = calcutate_biome(elevation, temperature, wetness)
    assert(biome_id ~= nil)

    local biome_data = biome_table[biome_id]
    if biome_data == nil then error("Could not find biome " .. biome_id) end
    ctx.seasons:set_tile_biome(season_index, i, biome_data.index)
    ctx.container:set_data_u32(types.entities.tile, i, types.properties.tile.color, biome_data.t.data.color)
  end

  local maxf = math.max
  local minf = math.min
  local layers_count = 20
  local layer_height = 1.0 / layers_count
  for i = 1, tiles_count do
    local h = ctx.container:get_data_f32(types.entities.tile, i, types.properties.tile.elevation)
    local layer = maxf(minf(h / layer_height, layers_count), 0) -- -layers_count
    layer = h < 0.0 and -1 or layer
    local final_h = layer * layer_height
    ctx.container:set_data_f32(types.entities.tile, i, types.properties.tile.elevation, final_h)
  end

  ctx.map:set_tiles_height(ctx.container, types.properties.tile.elevation)

  -- сразу цвета задать?
  ctx.map:set_tiles_color(ctx.container, types.properties.tile.color)
  -- local tiles_count = ctx.map:tiles_count()
  -- for i = 1, tiles_count do
  --   local id = ctx.map:get_tile_biome(i)
  --   local biome_data = biome_table[id]
  --   if biome_data == nil then error("Could not find biome " .. id) end
  --   ctx.map:set_tile_color(i, biome_data.color)
  -- end

  function_timer:finish()
end

-- как задать биом тайла? как в битве?
-- биом задается в сезонах, сезоны - это что? массив байтов для каждого тайла по 64 раза
-- 500к * 64 * строку - это смерть по памяти, в этом случае: либо смена сезона
-- будет происходить по коллбеку в котором мы собственно обойдем все 500к тайлов да зададим новые биомы
-- либо смена сезона будет происходить по таблице как я хотел сделать раньше
-- в чем проблема 1): информация о тепле и влажности и возможно других вещах будет недоступна в игре
-- в чем проблема 2): у меня нет понимания в каком порядке будут загружены биомы, а значит я не могу
-- взять индексы биомов
-- можно зафорсить здесь функцию add_biome, она дает валидный индекс, но тогда каждое изменение биомов
-- потребует перегенерацию мира
-- каждый дополнительный тип информации для тайла требует 2мб памяти
-- биомы по провинциям? возможно
-- запретить добавлять биомы после заполения сезонов?

return {
  compute_boundary_edges = compute_boundary_edges,
  compute_plate_boundary_stress = compute_plate_boundary_stress,
  compute_plate_boundary_distances = compute_plate_boundary_distances,
  calculate_vertex_elevation = calculate_vertex_elevation,
  blur_tile_elevation = blur_tile_elevation,
  normalize_tile_elevation = normalize_tile_elevation,
  compute_tile_heat = compute_tile_heat,
  compute_tile_water_distances = compute_tile_water_distances,
  compute_tile_moisture = compute_tile_moisture,
  create_biomes = create_biomes
}
