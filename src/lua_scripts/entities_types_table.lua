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
  tile                 = reset_inc_counter(), -- 1
  plate                = inc_counter(),
  edge                 = inc_counter(),
  province             = inc_counter(),
  province_neighbors   = inc_counter(),
  culture              = inc_counter(), -- 6
  country              = inc_counter(),
  title                = inc_counter(),
  air_current_outflows = inc_counter(),
  air_whorls           = inc_counter()
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
    air_current      = inc_counter(),
    air_speed        = inc_counter(),
    heat             = inc_counter(), -- 16
    moisture         = inc_counter(),
    biome            = inc_counter(),
    province_index   = inc_counter(),
    culture_id       = inc_counter(),
    country_index    = inc_counter(), -- 21
    duchie_index     = inc_counter(),
    kingship_index   = inc_counter(),
    empire_index     = inc_counter(),

    count = inc_counter()-1 -- 24
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
  province = {
    country_index = reset_inc_counter(), -- 1
    title_index   = inc_counter(),
    culture_id    = inc_counter(),
    religion_id   = inc_counter(),
    count         = inc_counter()-1 -- 4
  },
  province_neighbors = { count = 0 },
  culture = { count = 0 },
  country = { count = 0 },
  title = {
    parent = reset_inc_counter(), -- 1
    owner  = inc_counter(),
    count  = inc_counter()-1 -- 2
  },
  air_current_outflows = {
    outflow0  = reset_inc_counter(), -- 1
    outflow01 = inc_counter(),
    outflow02 = inc_counter(),
    outflow1  = inc_counter(),
    outflow11 = inc_counter(),
    outflow12 = inc_counter(), -- 6
    outflow2  = inc_counter(),
    outflow21 = inc_counter(),
    outflow22 = inc_counter(),
    outflow3  = inc_counter(),
    outflow31 = inc_counter(), -- 11
    outflow32 = inc_counter(),
    outflow4  = inc_counter(),
    outflow41 = inc_counter(),
    outflow42 = inc_counter(),
    outflow5  = inc_counter(), -- 16
    outflow51 = inc_counter(),
    outflow52 = inc_counter(),
    count = inc_counter()-1 -- 18
  },
  air_whorls = {
    tile     = reset_inc_counter(), -- 1
    strength = inc_counter(),
    radius   = inc_counter(),
    count    = inc_counter()-1, -- 3
  }
}

return {entities = entities, properties = properties}
