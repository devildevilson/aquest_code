function generator_beginner(ctx, table) 
  local abc = ctx.noise.get_noise(1, 2, 3)
  local tiles_count = ctx.map.tiles_count()
  local rand_index = ctx.random.index(124)
  local elevation = ctx.container.get_data_float(entities.tile, 12, properties.tile.elevation)
end
