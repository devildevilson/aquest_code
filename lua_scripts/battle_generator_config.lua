-- конфиг для генерации выглядит примерно вот так,

return {
  preparation_function = {
    hint = "getting map info",
    path = "battle_map_begin.lua",
    name = "reading_map_info",
    string = [[
      return function (args)
        -- blah blah blah
      end
    ]]
  },
  -- battle_over_function = {
  --   hint = "battle over",
  --   path = "battle_map_begin.lua",
  --   name = "reading_map_info"
  -- },
  generator_functions = {
    {
      hint = "preparation",
      path = "battle_map_begin.lua",
      name = "begin"
    },
    {
      hint = "creating biomes",
      path = "battle_map_begin.lua",
      name = "make_biomes"
    },
  }
}
