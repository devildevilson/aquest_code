-- конфиг таблиц геральдик, нужно ли какое то ограничение на количество геральдик?
-- каждый слой занимает 64 байта, разных геральдик должно быть около 11-12к
-- 64 байта на 15к примерно равно 1 мб, другое дело что слоев скорее всего будет 5-6 в среднем
-- на каждый титул + при текущем дизайне сложно создать переиспользуемые слои
-- геральдики нужно еще генерировать, генерировать в этом случае мы можем просто
-- последовательности id, которые затем преобразуем в индексы,
-- геральдику видимо нужно будет еще подцепить... где? в интерфейсе? нет, это глупо
-- делать отдельный стейт не хочется (хотя он мне скорее всего потребуется)

-- local function west_europe_heraldy_generator(seed)
--   local num = utils.pnrg64(seed)
-- end

-- в слотах массива таблицы должны хранится данные о слоях,
-- нет смысла делать дополнительную вложенность, и не буду тогда,
-- table.unpack должен сделать то что мне нужно
local heraldies_table = {
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
    tex_coords = {0.0, 0.0, 0.2, 0.2}, -- lion consists of small images
    discard_layer = false,
    continue_layer = false -- stop discarding color
  },
}

-- для генерации по сути нужно решить сколько раундов всяких красивостей пихнуть на щит
-- функции принимают на вход seed, по этому сиду мы можем погенерировать списки id слоев
local function default_heraldy(_)
  return {"shield_layer", "lion_layer", "quarterly_layer"}
end

local function default_player_heraldy(_)
  return {"shield_player_layer", "lion_layer", "quarterly_layer"}
end

heraldies_table.default_heraldy = default_heraldy
heraldies_table.default_player_heraldy = default_player_heraldy

return heraldies_table
