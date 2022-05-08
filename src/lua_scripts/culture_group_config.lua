local default_culture_opinion = { character_opinion = -15, popular_opinion = -15 }

-- этого нужно сгенерировать штук 50, вообще я так понимаю мне нужен способ генерировать
-- локализацию, что вообще говоря ужс, я не могу пока что более менее понять что делать
-- с статическими данными
return {
  {
    id = "culture_group1",
    name_id = "culture_groups.culture_group1.name",
    description_id = "culture_groups.culture_group1.description",
    different_group = default_culture_opinion,
    different_cultures = default_culture_opinion,
    different_child_cultures = default_culture_opinion
  }
}
