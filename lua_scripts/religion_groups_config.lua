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

-- в цк3 еще можно указать что группа подвержена ересям (что это конкретно означает?)
-- мне тоже нужно сделать механику с ересями, что такое ересь по большому счету?
-- вообще говоря ересь это новое верование с которым не согласна текущая церковь (по идее)
-- следовательно если у церкви есть много ресурсов, то она может попытаться объявить
-- те или иные вещи ересью, то есть тут вопрос что может религиозный институт
-- + ко всему должны быть эвенты которые опрокидывают текущую религию
return {
  {
    id = "religion_group1",
    name_id = "religions.group1.name",
    description_id = "religions.group1.name",
    different_group = evil,
    different_religion = evil,
    different_faith = hostile
  }
}
