-- конфиг картинок, устроен так же как и конфиг биомов (см. biomes_config.lua)
-- в таких конфигах нам пригодится знать какие моды мы загрузили
-- по прежнему картинки требуется грузить и выгружать было бы неплохо сделать
-- "типы" картинок, такие что мы можем определить какое качество нам необходимо
-- + понять когда че удалить, в общем так можно сделать только если строго задать типы
-- нужно запретить использование картинок других типов в неподходящих местах
-- что с сэмплерами делать? нужно либо от них отказаться, либо просто сделать две опции
-- и неарест по умолчанию

local valid_mod = core.is_loaded("apates_quest")
if not valid_mod then
  error("Modification apates_quest is needed")
end

return {
  {
    id = "hex_grass1",
    path = "apates_quest/textures/biomes/hex_grass4.png",
    type = 0,
    sampler = 1
  },
  {
    id = "hex_grass2",
    path = "apates_quest/textures/biomes/hex_grass5.png",
    type = 0,
    sampler = 1
  },
  {
    id = "hex_water",
    path = "apates_quest/textures/biomes/hex_water.png",
    type = 0,
    sampler = 1
  },
  {
    id = "hex_snow",
    path = "apates_quest/textures/biomes/hex_snow.png",
    type = 0,
    sampler = 1
  },
  {
    id = "hex_cold_water",
    path = "apates_quest/textures/biomes/hex_cold_water.png",
    type = 0,
    sampler = 1
  },
  {
    id = "hex_desert",
    path = "apates_quest/textures/biomes/hex_desert.png",
    type = 0,
    sampler = 1
  },
  {
    id = "rain_tree",
    path = "apates_quest/textures/biomes/upper_palm.png",
    scale = { width = 32, height = 32, filter = 1 },
    type = 0,
    sampler = 1
  },
  {
    id = "deciduous_tree",
    path = "apates_quest/textures/biomes/upper_tree5.png",
    scale = { width = 32, height = 32, filter = 1 },
    type = 0,
    sampler = 1
  },
  {
    id = "coniferous_tree",
    path = "apates_quest/textures/biomes/upper_spruce2.png",
    scale = { width = 32, height = 32, filter = 1 },
    type = 0,
    sampler = 1
  },
  {
    id = "hero_img",
    path = "apates_quest/textures/armies_and_heroes/KnightHorseback7_128_nearest.png",
    type = 0,
    sampler = 1
  },
  -- геральдика
  {
    id = "heraldy_shield1",
    path = "apates_quest/textures/heraldy/shield_straight.png",
    type = 0,
    sampler = 0
  },
  {
    id = "heraldy_quarterly",
    path = "apates_quest/textures/heraldy/quarterly.png",
    type = 0,
    sampler = 1
  },
  {
    id = "heraldy_lion1",
    path = "apates_quest/textures/heraldy/royal_lion.png",
    type = 0,
    sampler = 0
  },
  -- города
  {
    id = "castle_top",
    path = "apates_quest/textures/structures/castle_top.png",
    type = 0,
    sampler = 1
  },
  {
    id = "castle_face",
    path = "apates_quest/textures/structures/castle_face.png",
    type = 0,
    sampler = 1
  },
}
