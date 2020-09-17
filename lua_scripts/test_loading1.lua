-- как определить какие картинки мне где нужны?
-- вообще в игре могут пригодиться почти все изображения
-- но например экраны загрузки не нужны почти нигде кроме загрузки
-- анимационные состояния не нужны нигде кроме боев (и то разные в разных)
-- текстуры биомов могут пригодиться в боях

-- изображения экранов загрузки и главного меню, текстурки биомов + объекты биомов, треиты и модификаторы,
-- гербы (точнее то из чего собираются), персонажи (много разных изображений частей лица (+ возможно задник)),
-- карточки отрядов и героев, состояния анимации персонажей в битвах,
-- игровые иконки (хотя это и треиты тоже), постройки на карте (города, данжи и проч)
-- получается примерно так: "common", "biome", "icon", "heraldy", "face", "troop" ("hero"), "state", "architecture"
-- эти данные будут использоваться везде
-- "icon"    - игровые иконки (треиты, модификаторы, иконки статов и прочее прочее), скейлятся до 32х32 автоматически
-- "heraldy" - геральдические изображения, тоже должны скейлиться до маленьких значений (32х32?), так же должны быть особых цветов
-- "face"    - различные элементы лиц, скейлятся наверное до 128, где эти изображения нужны?
-- "card"    - карточки героев и отрядов, скейлить? мне еще потребуется их уместить в интерфейсе
-- эти данные мы подгружаем при переходе состояния игры
-- "common"  - используем для экранов загрузки и меню
-- "state"   - анимационные состояния в битвах, нужны только в битвах
-- "encounter_biome" - текстурки тайлов, текстурки красивостей?, текстурки препядствий (возможно только для геройской битвы)
-- "battle_biome" - тайлы для непосредственно варгейма
-- "biome"   - текстурка земли + текстурки объектов (или это другое?),
-- "architecture" - данжи и строения на карте, могут ли потребоваться в битвах? (какие то тайловые усилители для отрядов?)

-- в героях уникальные поля битв не по тайлам а нарисованные, возможно имеет смысл будет сделать так же в будущем
-- мне же желательно определить заранее описание биомов, текстурок с красивостями, текстурок препядствий
-- наверное все же нужно разделить

-- вообще конечно лучше сериализовать все в игре и поновой загружать все при переходах от состоянию к состоянию
-- то есть нужно определить структуры которые мне нужны в разных игровых состояниях
-- и сериализовать их? мне нужно сохранить только данные карты практически как сейв
-- оставить только некоторые данные (например описание персонажа и отряда, титулы персонажа (наверное только главный))
-- все остальное можно удалить, а после битвы запомнить результаты битвы, удалить карту и заного загрузить все данные
-- все это должно идти вместе с интерфейсом

function load1()
  local load_table = {}
  load_table.id = "test_img"
  load_table.path = "textures/img/img1.png"
  load_table.atlas_data = {
    width = 32,
    height = 32,
    rows = 1,
    columns = 5,
    count = 5
  }
  load_table.scale = {
    width = 128,
    height = 128,
    filter = "linear"
  }
  load_table.type = "common"
  return load_table

  local img = get_image("test_img.1.uv")
end
