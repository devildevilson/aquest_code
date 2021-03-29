-- конфиг таблиц геральдик, нужно ли какое то ограничение на количество геральдик?
-- каждый слой занимает 64 байта, разных геральдик должно быть около 11-12к
-- 64 байта на 15к примерно равно 1 мб, другое дело что слоев скорее всего будет 5-6 в среднем
-- на каждый титул + при текущем дизайне сложно создать переиспользуемые слои

return {
  {
    id = "shield_layer",
    stencil = "heraldy_shield1", -- image
    next_layer = "lion_layer",
    colors = { -- this layer would be background shield with 0.7 red color
      utils.make_color(0.0, 0.0, 0.0, 1.0), -- r color component
      utils.make_color(0.0, 0.0, 0.0, 1.0), -- g color component
      utils.make_color(0.0, 0.0, 0.0, 1.0), -- b color component
      utils.make_color(0.7, 0.0, 0.0, 1.0)  -- 1-(r+g+b) color component
    },
    coords = {0.0, 0.0, 1.0, 1.0}, -- image location
    tex_coords = {0.0, 0.0, 1.0, 1.0}, -- image scale
    discard_layer = false,
    continue_layer = false
  },
  {
    id = "shield_player_layer",
    stencil = "heraldy_shield1",
    next_layer = "lion_layer",
    colors = {
      utils.make_color(0.0, 0.0, 0.0, 1.0),
      utils.make_color(0.0, 0.0, 0.0, 1.0),
      utils.make_color(0.0, 0.0, 0.0, 1.0),
      utils.make_color(0.0, 0.7, 0.0, 1.0)
    },
    coords = {0.0, 0.0, 1.0, 1.0},
    tex_coords = {0.0, 0.0, 1.0, 1.0},
    discard_layer = false,
    continue_layer = false
  },
  {
    id = "lion_layer",
    stencil = "heraldy_lion1",
    next_layer = "quarterly_layer",
    colors = { -- color is not important in this layer
      utils.make_color(0.0, 0.0, 0.0, 1.0),
      utils.make_color(0.0, 0.0, 0.0, 1.0),
      utils.make_color(0.0, 0.0, 0.0, 1.0),
      utils.make_color(0.0, 0.7, 0.0, 1.0)
    },
    coords = {0.15, 0.1, 0.85, 0.8}, -- lion is smaller than shield
    tex_coords = {0.0, 0.0, 1.0, 1.0},
    discard_layer = true, -- color is discarded outside lion
    continue_layer = false
  },
  {
    id = "quarterly_layer",
    stencil = "heraldy_quarterly",
    next_layer = nil, -- last heraldy layer
    colors = { -- this stencil consists of 4 colors, which have the chess layout
      utils.make_color(0.8, 0.6, 0.0, 1.0),
      utils.make_color(0.9, 0.9, 0.0, 1.0),
      utils.make_color(0.9, 0.9, 0.0, 1.0),
      utils.make_color(0.8, 0.6, 0.0, 1.0)
    },
    coords = {0.15, 0.1, 0.85, 0.8},
    tex_coords = {0.0, 0.0, 0.2, 0.2}, -- lion consists of small images (any layer can refer to this layer)
    discard_layer = false,
    continue_layer = false -- stop discarding color
  },
}
