local potential_wife = {
  { -- небольшая оптимизация, эти простые функции пойдут первыми
    is_married = false,
    is_adult = true,
  },
  is_male = { NAND = { "root.is_male" } }, -- пол должен различаться
  --is_male = false,
  OR = { -- нужно исправить, чтобы был ранний выход из функций
    -- religion = { -- это другой блок в принципе, а значит тут опять вернется AND
    --   allow_mariages_with = "root.religion"
    -- },
    has_same_religion_as = "context:root", -- context:root и root эквивалентные записи
    -- {
    --   condition = {},
    --   -- если есть блок condition, то этот блок выполняется как AND, нужно ли это поменять? думаю что да
    --   -- то есть блок без специальных указаний наследует свойство блока выше
    -- }
  },
  -- дипломатическое расстояние нужно проверить

  -- нужно проверить отношение к root у главы семьи (или тот кто принимает решение за этого персонажа)
  -- это скорее всего отец, но может быть и кто то другой, а может быть и не совсем глава семьи,
  -- но это скорее всего ближайший прямой предок владелец титула, как его найти
  -- нужно пройтись по главам семьи, они определяются типом брака (патрилинейный или матрилинейный)
  -- как понять какой брак заключен? нужно где-то флаг поставить

  --  какие еще проверки?
}

return potential_wife