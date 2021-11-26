-- предположим что я хочу отдельный конфиг для некоторых типов данных, например для описания биомов
-- какие биомы я могу сгенерировать, а какие то необходимо задать через конфиг
-- в чем прикол, по идее этот конфиг должен вернуть список таблиц с валидным описанием биомов
-- а в сериализации мы просто запомним путь до этого файла, и видимо будем пытаться запускать его
-- каждый раз при загрузке, тут вот какая проблема возможна: файл поменяется и может отвалиться
-- загрузка после обновления, тут бы какую проверку на дурака, тип хранить хешсуммы файлов?
-- но при этом я лично буду часто менять эти файлы, поэтому нужно по крайней мере не выдавать
-- ФАТАЛ ЭРРОР в этом случае, но ворнинги не помешают, что еще?
-- здесь явно не должно быть возможности "сгенерировать" данные, то есть нужно почистить все
-- матх.рандом, я не должен иметь возможность считывать какой нибудь /dev/random, да и вообще
-- что то за пределами папки с игрой, но работать с файлами поди нужно оставить возможность,
-- с файлами но не с файловой системой, записи на диск тоже не должно быть, остальные вещи?
-- почему бы и нет, строки, утф8, локализация, арифметика, принт, ассерт, таблицы и прочее

-- в этои файле мы ОБЯЗАНЫ учесть изменения файлов приходящих к нам из require или io.open
-- loadfile, dofile, load здесь не нужны, работа с файлами через io
-- в io нужно убрать почти все функции
--local valuable_module = require("apates_quest/module")

-- local function load_biomes(seasons)
--   -- мы можем прямо в сезонах валидировать и создать биом
--   -- тогда мы вернем индекс
--   local b_index = seasons:add_biome({})
--   -- затем нам нужен сезон в который мы как раз должны положить биом
--   local s_index = seasons:allocate_season()
--   -- кладем биомы в сезоны, но так мы можем сделать только при генерации
--   -- какой другой вариант? писать какие то костыли? лучше наверное оставить так
--   for i = 1, tiles_count do
--     seasons:set_biome(i, s_index, b_index)
--   end
-- end

-- я вот что подумал, биомы можно разнообразить
-- объекты на биомах могут обладать анимацией
-- анимация - 3 текстурки которые мы тут указываем, для чего она нам?
-- можем оформить берег (анимация разбивающихся волн), оформить воронку в океане
-- является ли это анимацией текстурирования гекса? скорее всего
-- анимация на обычных тайлах? (возможно какое нибудь особое здание)
-- + на некоторых тайлах можно поставить источник частиц (например дымит вулкан)

-- будет тогда 3 типа: 3 объекта, 3 фрейма анимации у объектов, 3 фрейма анимации у тайла

local biome_rain_forest = {
  id = "biome_rain_forest",
  name_id = "biomes.biome_rain_forest.name",
  description_id = "biomes.biome_rain_forest.desc",
  base_speed = 1.0,
  data = {
    color = utils.make_color(0.0, 0.7, 0.2, 1.0),
    density = 9.0,
    objects = { -- rendering data, maximum 3
      { texture = "rain_tree", min_scale = 0.3, max_scale = 1.0, probability = 1.0 }
    }
  }
}

local biome_ocean = {
  id = "biome_ocean",
  name_id = "biomes.biome_ocean.name",
  description_id = "biomes.biome_ocean.desc",
  attributes = { "water", },
  base_speed = 1.0,
  data = { color = utils.make_color(0.2, 0.2, 0.8, 1.0) }
}

local biome_ocean_glacier = {
  id = "biome_ocean_glacier",
  name_id = "biomes.biome_ocean_glacier.name",
  description_id = "biomes.biome_ocean_glacier.desc",
  attributes = { "winter", },
  base_speed = 0.75,
  data = { color = utils.make_color(0.8, 0.8, 1.0, 1.0) }
}

local biome_desert = {
  id = "biome_desert",
  name_id = "biomes.biome_desert.name",
  description_id = "biomes.biome_desert.desc",
  attributes = { "wasteland", },
  base_speed = 1.0,
  data = { color = utils.make_color(0.914, 0.914, 0.2, 1.0) }
}

local biome_rocky = {
  id = "biome_rocky",
  name_id = "biomes.biome_rocky.name",
  description_id = "biomes.biome_rocky.desc",
  base_speed = 1.0,
  data = { color = utils.make_color(1.0, 0.2, 0.2, 1.0) }
}

local biome_plains = {
  id = "biome_plains",
  name_id = "biomes.biome_plains.name",
  description_id = "biomes.biome_plains.desc",
  base_speed = 1.0,
  data = { color = utils.make_color(0.553, 0.769, 0.208, 1.0) }
}

local biome_swamp = {
  id = "biome_swamp",
  name_id = "biomes.biome_swamp.name",
  description_id = "biomes.biome_swamp.desc",
  base_speed = 0.75,
  data = { color = utils.make_color(0.0, 1.0, 0.0, 1.0) }
}

local biome_grassland = {
  id = "biome_grassland",
  name_id = "biomes.biome_grassland.name",
  description_id = "biomes.biome_grassland.desc",
  base_speed = 1.0,
  data = { color = utils.make_color(0.553, 0.769, 0.208, 1.0) }
}

local biome_deciduous_forest = {
  id = "biome_deciduous_forest",
  name_id = "biomes.biome_deciduous_forest.name",
  description_id = "biomes.biome_deciduous_forest.desc",
  base_speed = 1.0,
  data = {
    color = utils.make_color(0.0, 0.8, 0.0, 1.0),
    density = 8.0,
    objects = {
      { texture = "deciduous_tree", min_scale = 0.3, max_scale = 1.0, probability = 1.0 }
    }
  }
}

-- тундра может быть и летняя и зимняя (не особо много вариаций)
local biome_tundra = {
  id = "biome_tundra",
  name_id = "biomes.biome_tundra.name",
  description_id = "biomes.biome_tundra.desc",
  base_speed = 1.0,
  data = { color = utils.make_color(0.6, 0.6, 0.6, 1.0) }
}

local biome_land_glacier = {
  id = "biome_land_glacier",
  name_id = "biomes.biome_land_glacier.name",
  description_id = "biomes.biome_land_glacier.desc",
  attributes = { "winter", },
  base_speed = 0.75,
  data = { color = utils.make_color(0.6, 0.6, 0.6, 1.0) }
}

local biome_conifer_forest = {
  id = "biome_conifer_forest",
  name_id = "biomes.biome_conifer_forest.name",
  description_id = "biomes.biome_conifer_forest.desc",
  base_speed = 1.0,
  data = {
    color = utils.make_color(0.0, 0.6, 0.0, 1.0),
    density = 12.0,
    objects = {
      { texture = "coniferous_tree", min_scale = 0.3, max_scale = 1.0, probability = 1.0 }
    }
  }
}

local biome_mountain = {
  id = "biome_mountain",
  name_id = "biomes.biome_mountain.name",
  description_id = "biomes.biome_mountain.desc",
  attributes = { "not_passable" },
  base_speed = 1.0,
  data = { color = utils.make_color(0.2, 0.2, 0.2, 1.0) }
}

local biome_snowy_mountain = {
  id = "biome_snowy_mountain",
  name_id = "biomes.biome_snowy_mountain.name",
  description_id = "biomes.biome_snowy_mountain.desc",
  attributes = { "not_passable" },
  base_speed = 1.0,
  data = { color = utils.make_color(0.914, 0.988, 1.0, 1.0) }
}

-- возвращаем список таблиц, в таблицах теперь не нужен id (???)
-- нужен, + к нему зададим имя, описание, аттрибуты, базовую скорость
-- биомы должны быть строго в последовательном массиве !!! возможно нужно включить проверку последовательности данных
return { -- maximum 255 biomes
  biome_ocean,
  biome_ocean_glacier,
  biome_desert,
  biome_rain_forest,
  biome_rocky,
  biome_plains,
  biome_swamp,
  biome_grassland,
  biome_deciduous_forest,
  biome_tundra,
  biome_land_glacier,
  biome_conifer_forest,
  biome_mountain,
  biome_snowy_mountain
}
