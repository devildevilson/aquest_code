local default_culture_opinion = { character_opinion = -15, popular_opinion = -15 }

-- как задаются механики? нам нужно перечислить название механики
-- мы по идее можем перечислить строки или можем сделать строки ключами?
-- лучше наверное ключами задать: так можно указать все особенности эксплицитно
local features = {
  horde = false,
  used_for_random = false,
  allow_in_ruler_designer = false,
  dukes_called_kings = false,
  baron_titles_hidden = false,
  count_titles_hidden = false,
  founder_named_dynasties = false,
  dynasty_title_names = false,
  disinherit_from_blinding = false,
  allow_looting = false,
  seafarer = false,
  dynasty_name_first = false,
  feminist = false,

  has_master_gender = true,
  has_lord_gender = true,
  has_king_gender = true,
  has_emperor_gender = true,
  has_hero_gender = true,
  has_wizard_gender = true,
  has_duke_gender = true,
  has_count_gender = true,
  has_heir_gender = true,
  has_prince_gender = true,
  has_baron_gender = true
}

-- и мне чего то такого нужно сделать штук 200
return {
  {
    id = "culture1",
    name_id = "cultures.culture1.name",
    description_id = "cultures.culture1.description",
    names_table_id = "cultures.english.character_names",
    -- пока с трудом понимаю как работают патронимы, видимо будет гораздо более трудная задача
    patronims_table_id = "cultures.culture1.patronim_table",
    additional_table_id = "cultures.culture1.additional_table",
    grandparent_name_chance = 0.2,
    group = "culture_group1",
    --parent = "",
    --image = "culture1_icon", -- должна ли у культур быть иконка? вряд ли
    color = utils.make_color(0.1, 0.1, 0.1, 1.0),
    bonuses = {},
    features = features,
    different_groups = default_culture_opinion,
    different_cultures = default_culture_opinion,
    different_child_cultures = default_culture_opinion
  }

}
