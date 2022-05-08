-- конфиг генератора выглядит как: список файлов луа и описание функции из них

return {
  scripts = {
    "apates_quest/scripts/gen_part1_functions.lua",
    "apates_quest/scripts/gen_part2_functions.lua",
    "apates_quest/scripts/gen_part3_functions.lua"
  },
  steps = {
    {
      name = "Tectonic plates generator",
      functions = {
        "setup_generator",
        "generate_plates",
        "generate_plate_datas"
      }
    },
    {
      name = "Biomes generator",
      functions = {
        "compute_boundary_edges",
        "compute_plate_boundary_stress",
        "compute_plate_boundary_distances",
        "calculate_vertex_elevation",
        "blur_tile_elevation",
        "normalize_tile_elevation",
        "compute_tile_heat",
        "compute_tile_water_distances",
        "compute_tile_moisture",
        "create_biomes"
      }
    },
    {
      name = "Countries generator",
      functions = {
        "generate_provinces",
        "province_postprocessing",
        "calculating_province_neighbors",
        "generate_countries",
        "generate_cultures",
        "generate_religions",
        "add_provinces_data",
        "generate_heraldy",
        "generate_titles",
        "generate_characters",
        "generate_cities"
      }
    }
  }
}
