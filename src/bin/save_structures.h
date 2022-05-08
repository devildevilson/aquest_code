#ifndef SAVE_STRUCTURES_H
#define SAVE_STRUCTURES_H

#include <cstdint>
#include <cstddef>

//#ifdef DE_STRUCTURES_SERIALIZATION
#include "cista/cista.h"
//#endif

#include "core_structures.h"

// какая задача? написать аналоги основных структур для сериализации
// 

namespace devils_engine {
  namespace serialization {
//#ifdef DE_STRUCTURES_SERIALIZATION
    constexpr const auto MODE = cista::mode::WITH_VERSION | cista::mode::WITH_INTEGRITY; // opt. versioning + check sum
    namespace data = cista::offset;
//#endif
    
    // вообще чтобы все это дело не переписывать я могу по идее задать парочку defin'ов
    // скорее всего не потребуется переводить каждый тип в тип подходящий цисте
    // сделать нужно только несколько типов (ниже)
    struct province {
      uint32_t cities_max_count;
      uint32_t cities_count;
      data::array<size_t, core::province::cities_max_game_count> cities;
      
      // обычные статы нормально встают в цисту (хотя если их много?)
      // остается только решить че делать с модификаторами, эвентами и флагами
      // что делать с изображениями? изображения важно загрузить в том же порядке
      // в котором они и были, битовые поля? скорее всего ничего особенного
      // у нас есть указатель на имя в банке, скорее всего он останестя прежним
      // атомарные переменные наверное нужно переделать в обычные (?)
      // многие из структур бессмыслено хранить в сейв файле, там должны быть:
      // конкретные армии, конкретные партии, конкретные персонажи, фракции,
      // конкретный отряд в армии, титулы (нужно указать владение в персонаже, и из него брать инфу при загрузке)
      // конкретный город (точнее небольшая часть информации о городе, так как остальное пойдет из других мест),
      // добавятся еще наверное состояния данжей, и похоже что в сохранении все
      // было бы неплохо хранить какие то текущие изображения в виде строк
      
      // в провинции кстати нужно хранить построенные города
    };
    
    struct city {
      // строительство + статы + тип?
      size_t type; // индекс типа, по идее загрузка должна быть всегда выдавать один и тот же порядок
      data::array<size_t, core::city::bit_field_size> available_buildings_bits;
      data::array<size_t, core::city::bit_field_size> complited_buildings_bits;
      data::array<uint32_t, core::city_stats::count> current_stats;
    };
    
    struct troop {
      // статы + возможный полководец + тип?
      size_t type;
      size_t character;
      data::array<uint32_t, core::troop_stats::count> moded_stats;  // побитово сохраняем все эти вещи
      data::array<uint32_t, core::troop_stats::count> current_stats;
    };
    
    struct path_finding_data {
      data::vector<uint32_t> path;
      size_t path_size;
      size_t current_path;
      uint32_t start_tile;
      uint32_t end_tile;
      size_t path_task;
    };
    
    struct event_container {
      size_t time; // время (в ходах) когда флаг был добавлен
      data::array<data::pair<size_t, size_t>, 8> event_stack; // размер стека получаем по валидным указателям
    };
    
    struct army {
      using events_container_t = data::vector<data::pair<size_t, event_container>>;
      
      // статы + отряды + текущие характеристики + путь
      struct path_finding_data path_finding_data;
      data::array<troop, core::max_troops_count> troops; // возьмем в качестве отправной точки 20 юнитов
      uint32_t troops_count;
      uint32_t tile_index;
      data::array<uint32_t, core::army_stats::count> computed_stats;
      data::array<uint32_t, core::army_stats::count> current_stats;
      float current_pos;
      
      // тут по идее нужно будет еще сохранить какие то данные из гпу
      // текстурка кстати + модификаторы + эвенты + флаги
      
      data::vector<data::pair<size_t, size_t>> modificators;
      events_container_t events; // тут по идее нужно хранить индексы с типом 
      data::vector<data::pair<data::string, size_t>> flags;
    };
    
    struct hero_troop {
      using events_container_t = data::vector<data::pair<size_t, event_container>>;
      
      // статы + персонажи в партии + путь
      struct path_finding_data path_finding_data;
      
      uint32_t party_size;
      uint32_t max_party_size;
      data::array<size_t, core::hero_troop::max_game_party_size> party;
      uint32_t tile_index;
      
      data::vector<data::pair<size_t, size_t>> modificators;
      events_container_t events;
      data::vector<data::pair<data::string, size_t>> flags;
    };
    
    // скорее всего не нужно, так как данные об этом мы получим из персонажей
    // хотя как не нужно: эвенты которые должны вот вот произойти + флаги
    //struct titulus {};
    
    struct character {
      using events_container_t = data::vector<data::pair<size_t, event_container>>;
      
      // тут много всего: нужно запомнить все взаимоотношения персонажей +
      // статы + двор + отношения и принадлежность к фракциям + 
      // культура + религии + состояние генератора + династия
      
      struct family {
        size_t real_parents[2];
        size_t parents[2];
        size_t children;
        size_t next_sibling;
        size_t prev_sibling;
        size_t consort;
        size_t previous_consorts;
        size_t owner;
        size_t concubines;
        size_t blood_dynasty;
        size_t dynasty;
      };
      
      struct relations {
        data::array<size_t, core::character::relations::max_game_friends> friends;
        data::array<size_t, core::character::relations::max_game_rivals> rivals;
        data::array<size_t, core::character::relations::max_game_lovers> lovers;
      };
      
      static_assert(sizeof(family) == sizeof(core::character::family));
      static_assert(sizeof(relations) == sizeof(core::character::relations));
      
      data::array<uint32_t, core::character_stats::count> stats;
      data::array<uint32_t, core::character_stats::count> current_stats;
      data::array<uint32_t, core::hero_stats::count> hero_stats;
      data::array<uint32_t, core::hero_stats::count> current_hero_stats;
      uint32_t name_number;
      
      int64_t born_day;
      int64_t death_day;
      size_t name_str;
      size_t nickname_str;
      size_t suzerain;
      size_t imprisoner;
      size_t next_prisoner;
      size_t prev_prisoner;
      size_t next_courtier;
      size_t prev_courtier;
      
      size_t troop;
      size_t army;
      
      struct family family;
      struct relations relations;
      size_t culture;
      size_t religion;
      size_t hidden_religion;
      data::array<size_t, core::character::faction_type_count> factions;
      
      size_t data;
      utils::rng::state rng_state;
      
      data::vector<size_t> traits;
      data::vector<data::pair<size_t, size_t>> modificators;
      events_container_t events;
      data::vector<data::pair<data::string, size_t>> flags;
    };
    
    struct dynasty {
      // по ходу дела в игре должны появляться новые династии
      // значит династии тоже нужно сохранять, что в них?
      // герб + имя + id (?) + механики династии (?) +
      // текущие статы династии (может быть какие нибудь модификаторы за крутую династию) +
      // возможно какой то стейт
      data::string id;
    };
    
    // фракции то поди нужно разделить на легальные (глава, суд, совет) и нелегальные фракции (восстания ?)
    struct faction {
      // статы фракции + титулы + законы + заключенные + отношения между фракциями
      // тут же должна быть дипломатия
      size_t leader;
      size_t heir;

      size_t liege;
      size_t state;
      size_t council;
      size_t tribunal;
      
      size_t vassals;
      size_t next_vassal;
      size_t prev_vassal;
      
      size_t titles;
      size_t main_title;
      
      size_t courtiers;
      size_t prisoners;
      
      data::array<uint32_t, core::realm_stats::count> stats;
      data::array<uint32_t, utils::realm_mechanics::bit_container_size> laws_container;
    };
    
    struct calendar {
      // текущий ход
    };
    
    // кажется я перечислил все структуры + добавятся какие то глобальные состояния (состояния генератора)
    // модификаторы присутствуют во многих структурах, поэтому наверное их отдельно
    // 
    
    // как то так выглядит конкретный сейв, здесь еще ожидает пара изменений
    // его нужно поместить в папку с миром, 
    struct save_game_data {
      data::vector<province> provinces;
      data::vector<city> cities;
      data::vector<troop> troops;
      data::vector<army> armies;
      data::vector<hero_troop> hero_troops;
      data::vector<character> characters;
      data::vector<dynasty> dynasties;
      data::vector<faction> faction;
    };
  }
}

#endif
