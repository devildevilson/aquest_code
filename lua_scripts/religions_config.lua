-- вообще я бы хотел чтобы обращение не было такой легкой механикой
-- как в крузаках (тип отправил священника, он там все поделал, и через некоторое время провинция обратилась)
-- тут как обычно нужно добавить элемент политики, почему вообще стоит такая большая необходимость
-- в обращении всех подряд у религий? у муслимов это было связано просто с тем что все враги вокруг
-- и их надо уничтожить в любом случае, но у христиан какой то такой потребности не стояло
-- у христиан была римская империя которая просто объявила государственную религию,
-- а дальше фактически сами правители обращались, для того чтобы получить важные связи среди крупных
-- христианских государств, то есть у нас тут два варианта: либо выжигать либо примерно таже механика
-- что и в крузаках, как выжигать? тут сложно что то адекватное придумать это скорее всего нужно будет реализовывать
-- через огромное количество дополнительных механик, а это супер запарно так что придется видимо делать как в крузаках

local righteous = {
  character_opinion = 0,
  popular_opinion = 0,
  intermarriage = true,
  title_usurpation = true,
  holy_wars = false
}

local astray = {
  character_opinion = -10,
  popular_opinion = -15,
  intermarriage = true,
  title_usurpation = true,
  holy_wars = false
}

local hostile = {
  character_opinion = -20,
  popular_opinion = -30,
  intermarriage = true,
  title_usurpation = false,
  holy_wars = true
}

local evil = {
  character_opinion = -30,
  popular_opinion = -45,
  intermarriage = false,
  title_usurpation = false,
  holy_wars = true
}

-- прежде чем религии определять нужно определить религиозные группы, группа должна быть у любой религии
return {
  {
    id = "religion1",
    name_id = "religions.rel1.name",
    description_id = "religions.rel1.description",
    --parent = "parent_religion1", -- может не быть
    group = "religion_group1", -- группа должна существовать в любом случае
    aggression = 0.5, -- с агрессией пока что ничего не понятно
    crusade_name_id = "religions.rel1.crusade",
    holy_order_names_table_id = "religions.rel1.orders", -- указатель на таблицу
    scripture_name_id = "religions.rel1.scripture",
    good_gods_table_id = "religions.rel1.gods",
    evil_gods_table_id = "religions.rel1.evil_gods",
    high_god_name_id = "religions.rel1.crusade",
    piety_name_id = "religions.rel1.piety",
    priest_title_name_id = "religions.rel1.priest_title",
    reserved_male_names_table_id = "religions.rel1.male_names",
    reserved_female_names_table_id = "religions.rel1.female_names",
    -- цвет, картинка,
    bonuses = {}, -- тип стата, собственно бонус
    opinion_bonuses = {}, -- ???
    -- тут переопределяются данные из религиозной группы или из родителькой религии
    different_group = evil,
    different_religions = evil,
    different_faiths = hostile
  }
}
