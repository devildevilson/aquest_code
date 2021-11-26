#ifndef SYSTEMS_LOADING_DEFINES_H
#define SYSTEMS_LOADING_DEFINES_H

#define LOADING_MAIN_MENU_FUNC_LIST

#define LOADING_SAVED_WORLD_FUNC_LIST \
  LOADING_FUNC(deserialize_world) \
  LOADING_FUNC(preparation) \
  LOADING_FUNC(creating_earth) \
  LOADING_FUNC(loading_images) \
  LOADING_FUNC(creating_entities) \
  LOADING_FUNC(connecting_game_data) \
  LOADING_FUNC(preparing_biomes) \
  LOADING_FUNC(creating_borders) \
  LOADING_FUNC(make_player)

#define LOADING_GENERATED_MAP_FUNC_LIST \
  LOADING_FUNC(starting) \
  LOADING_FUNC(serializing_world) \
  LOADING_FUNC(preparation) \
  LOADING_FUNC(creating_earth) \
  LOADING_FUNC(loading_images) \
  LOADING_FUNC(creating_entities) \
  LOADING_FUNC(connecting_game_data) \
  LOADING_FUNC(preparing_biomes) \
  LOADING_FUNC(creating_borders) \
  LOADING_FUNC(checking_world) \
  LOADING_FUNC(make_player) \
  LOADING_FUNC(ending)

#define LOADING_SAVE_GAME_FUNC_LIST

#define LOADING_GENERATOR_FUNC_LIST \
  LOADING_FUNC(gen_prepare)

#define LOADING_BATTLE_FUNC_LIST

#define LOADING_ENCOUNTER_FUNC_LIST
  

// энтити парсятся теперь при создании
// LOADING_FUNC(parsing_entities)

#endif
