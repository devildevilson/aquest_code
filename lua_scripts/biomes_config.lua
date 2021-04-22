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

-- возвращаем список таблиц, в таблицах теперь не нужен id (???)
return {
  {
    id = "biome_ocean", -- не забыть id, нужно ли к id добавлять "неймспейс" apates_quest/ ?
    color = utils.make_color(0.2, 0.2, 0.8, 1.0),
    -- на ворлд мапе наверное не нужно делать вид с бок и вид сверху для объектов биома
    -- вполне можно сделать наверное 3 объекта, но тут большие проблемы с кешем графических процессоров
    -- + я не уверен что это полезно, вряд ли их нужно разглядывать вообще
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.1, max_scale2 = 0.3,
    density = 9.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_ocean_glacier",
    color = utils.make_color(0.8, 0.8, 1.0, 1.0),
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_desert",
    color = utils.make_color(0.914, 0.914, 0.2, 1.0),
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_rain_forest",
    color = utils.make_color(0.0, 0.7, 0.2, 1.0),
    object1 = nil, object2 = "rain_tree",
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.3, max_scale2 = 1.0,
    density = 9.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_rocky",
    color = utils.make_color(1.0, 0.2, 0.2, 1.0),
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_plains",
    color = utils.make_color(0.553, 0.769, 0.208, 1.0),
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_swamp",
    color = utils.make_color(0.0, 1.0, 0.0, 1.0),
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_grassland",
    color = utils.make_color(0.553, 0.769, 0.208, 1.0),
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_deciduous_forest",
    color = utils.make_color(0.0, 0.8, 0.0, 1.0),
    object1 = nil, object2 = "deciduous_tree",
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_tundra",
    color = utils.make_color(0.6, 0.6, 0.6, 1.0),
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_land_glacier",
    color = utils.make_color(0.914, 0.988, 1.0, 1.0),
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_conifer_forest",
    color = utils.make_color(0.0, 0.6, 0.0, 1.0),
    object1 = nil, object2 = "coniferous_tree",
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_mountain",
    color = utils.make_color(0.2, 0.2, 0.2, 1.0),
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  },
  {
    id = "biome_snowy_mountain",
    color = utils.make_color(0.914, 0.988, 1.0, 1.0),
    object1 = nil, object2 = nil,
    min_scale1 = 0.0, max_scale1 = 0.0,
    min_scale2 = 0.0, max_scale2 = 0.0,
    density = 0.0,
    height1 = 0.0, height2 = 0.0
  }
}
