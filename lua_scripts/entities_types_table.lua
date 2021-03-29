-- тут бы тогда тоже сделать счетчик с единицы
local counter_var = 1
local function inc_counter()
  local cur = counter_var
  counter_var = counter_var + 1
  return cur
end

local function reset_inc_counter()
  counter_var = 1
  local cur = counter_var
  counter_var = counter_var + 1
  return cur
end

local entities = {
  tile               = reset_inc_counter(), -- 1
  plate              = inc_counter(),
  edge               = inc_counter(),
  province           = inc_counter(),
  province_neighbors = inc_counter(),
  culture            = inc_counter(), -- 6
  country            = inc_counter(),
  title              = inc_counter()
}

local properties = {
  tile = {
    color            = reset_inc_counter(), -- 1
    plate_index      = inc_counter(),
    edge_index       = inc_counter(),
    edge_dist        = inc_counter(),
    mountain_index   = inc_counter(),
    mountain_dist    = inc_counter(), -- 6
    ocean_index      = inc_counter(),
    ocean_dist       = inc_counter(),
    coastline_index  = inc_counter(),
    coastline_dist   = inc_counter(),
    water_index      = inc_counter(), -- 11
    water_dist       = inc_counter(),
    elevation        = inc_counter(),
    heat             = inc_counter(),
    moisture         = inc_counter(),
    biome            = inc_counter(), -- 16
    province_index   = inc_counter(),
    culture_id       = inc_counter(),
    country_index    = inc_counter(),
    duchie_index     = inc_counter(),
    kingship_index   = inc_counter(), -- 21
    empire_index     = inc_counter(),

    count = inc_counter()-1 -- 22
  },
  plate = {
    drift_axis     = reset_inc_counter(), -- 1
    drift_axis1    = inc_counter(),
    drift_axis2    = inc_counter(),
    drift_rate     = inc_counter(),
    spin_axis      = inc_counter(),
    spin_axis1     = inc_counter(), -- 6
    spin_axis2     = inc_counter(),
    spin_rate      = inc_counter(),
    base_elevation = inc_counter(),
    oceanic        = inc_counter(),
    count          = inc_counter()-1 -- 10
  },
  edge = {
    first_tile       = reset_inc_counter(), -- 1
    second_tile      = inc_counter(),
    plate0_movement  = inc_counter(),
    plate0_movement1 = inc_counter(),
    plate0_movement2 = inc_counter(),
    plate1_movement  = inc_counter(), -- 6
    plate2_movement1 = inc_counter(),
    plate3_movement2 = inc_counter(),
    count            = inc_counter()-1 -- 8
  },
  culture = { count = 0 },
  province = {
    country_index = reset_inc_counter(), -- 1
    title_index   = inc_counter(),
    count         = inc_counter()-1 -- 2
  },
  title = {
    parent = reset_inc_counter(), -- 1
    owner  = inc_counter(),
    count  = inc_counter()-1 -- 2
  }
}

return {entities = entities, properties = properties}
