#ifndef CORE_STRUCTURES_H
#define CORE_STRUCTURES_H

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <array>
#include <atomic>
#include "character.h"
#include "render/shared_structures.h"
#include "data_parser.h"
#include "utils/bit_field.h"
#include "utils/traits_modifier_attribs.h"
#include "utils/string_bank.h"
#include "declare_structures.h"

// на каждый чих нужно придумать иконки

namespace devils_engine {
  namespace ai {
    struct path_container;
  }
  
  namespace core {
    const size_t max_conditions_count = 64;
    const size_t max_effects_count = 64;
    const size_t max_options_count = 8;
    const size_t max_troops_count = 20;
    
    struct tile_template {
      
    };
    
    struct tile {
      static const structure s_type = structure::tile;
      
      // возможно стоит выделить тайл и некоторые характеристики в нем
      // например для того чтобы описать правильно режим отображения карты
      float height; // вообще то это уже задано
      uint32_t province;
      uint32_t city;
      uint32_t struct_index;
      
      // темплейт
      tile();
    };
    
    // если персонаж владеет титулом баронским этой провинции - он владеет и столицей провинции? (как в цк2)
    // скорее всего да
    struct province {
      static const structure s_type = structure::province;
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      static const size_t cities_max_game_count = 10;
      // static attrib* attributes;
      
      // название (или название по столице провинции?)
      // удобно и герб тоже по столице провинции дать (нет, титул для баронства существует)
      titulus* title; // не будет меняться, но нужно заполнить титул (или его заполнять в другом месте?)
      std::vector<uint32_t> tiles;
      std::vector<uint32_t> neighbours;
      // какие техи распространились в этой провинции
      // что то вроде std::vector<bool> ?
      // локальные характеристики (принадлежность провинции?)
      // несколько "городов" (могут быть и замки и епископства, больше типов?) (как минимум должны быть епископства)
      // какой максимум поселений? 7 в цк2, у меня скорее всего меньше (3-4?)
      uint32_t cities_max_count; // в зависимости от размера провинции
      uint32_t cities_count;
      std::array<city*, cities_max_game_count> cities;
      
      modificators_container<modificators_container_size> modificators; // по идее их меньше чем треитов
      events_container<events_container_size> events; // должно хватить
      flags_container<flags_container_size> flags; // флагов довольно много
      
      province();
    };
    
    // постройка в городе 
    struct building_type {
      static const structure s_type = structure::building_type;
      static const size_t maximum_prev_buildings = 8;
      static const size_t maximum_limit_buildings = 8;
      static const size_t maximum_stat_modifiers = 16;
      static const size_t maximum_unit_stat_modifiers = 16;
      
      std::string id;
      size_t name_id;
      size_t desc_id;
      std::vector<utils::functions_container::operation> potential;
      std::vector<utils::functions_container::operation> conditions;
      // предыдущие постройки (достаточно ли?) (с другой стороны вряд ли имеет смысл ограничивать)
      std::array<const building_type*, maximum_prev_buildings> prev_buildings;
      // нельзя построить если существуют ...
      std::array<const building_type*, maximum_limit_buildings> limit_buildings;
      // то что заменяет (индекс? заменяет предыдущий уровень?)
      const building_type* replaced;
      // улучшение (если построено предыдущее здание, то открывается это)
      const building_type* upgrades_from;
      // стоимость постройки (может использовать разные ресурсы) (нужно наверное денюшку вытащить в характеристики персонажа)
      // скорость строительства (недели)
      size_t time;
      // как ии поймет что нужно строить это здание? (ии все же наверное будет работать по функции луа)
      // при каких условиях здание будет построено перед началом игры (нужно ли?) (должна быть возможность указать что построено а что нет в генераторе)
      // вижен? (дает ли вижен провинции для игрока построившего это) (это для дополнительных построек вроде торговых постов в цк2)
      // во что конвертируется (?) (нужно ли делать переход от одного типа города к другому?)
      // модификаторы (в цк2 могло дать скорость иследований, в моем случае нужно придумать какой то иной способ)
      std::array<stat_modifier, maximum_stat_modifiers> mods;
      std::array<unit_stat_modifier, maximum_unit_stat_modifiers> unit_mods; // модификаторы героев по типу
      // здания наверное могу давать какие то эвенты с какой то периодичностью
      
      float money_cost;
      float authority_cost;
      float esteem_cost;
      float influence_cost;
      
      building_type();
    };
    
    struct city_type {
      static const structure s_type = structure::city_type;
      static const size_t maximum_buildings = 128;
      std::string id;
      // вообще по идее мы можем здесь не ограничивать количество зданий, нет это нужно для непосредственно города
      std::array<const building_type*, maximum_buildings> buildings;
      // нужно указать что этот город это епископство для механики религии
      // тут я так понимаю может потребоваться несколько флагов
      // дефолтные статы?
      std::array<stat_container, city_stats::count> stats;
      // локализация (название типа)
      size_t name_id;
      size_t desc_id;
      // графика
      // как перейти от тайла к городу? нужно хранить какую то информацию в тайле, мы можем использовать 
      // 1 32bit int для того чтобы хранить индекс биома и индекс того что стоит на тайле
      // 
      render::image_t city_image_top;
      render::image_t city_image_face;
      render::image_t city_icon;
      float scale;
      
      city_type();
    };
    
    // на карте еще было бы неплохо разместить что то вроде данжей (или например хижину ведьмы)
    // я полагаю необходимо разделить эти вещи
    struct city {
      static const structure s_type = structure::city;
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      static const size_t bit_field_size = city_type::maximum_buildings / SIZE_WIDTH;
      static_assert(bit_field_size == 2);
      
      size_t name_index;
      // во время загрузки титул мы можем заполнить по провинции
      struct province* province; // по идее можно смело ставить const
      struct titulus* title; // герб
      const city_type* type;
      utils::bit_field<bit_field_size> available_buildings;
      utils::bit_field<bit_field_size> complited_buildings;
      size_t start_building;
      uint32_t building_index;
      uint32_t tile_index;
      // характеристики (текущие характеристики города, база + со всех зданий) (характеристики довольно ограничены, нужно ли дать возможность их модифицировать?)
      std::array<stat_container, city_stats::count> current_stats;
      
//       modificators_container<modificators_container_size> modificators; // по идее их меньше чем треитов
//       events_container<events_container_size> events; // эвенты и флаги хранятся в титуле
//       flags_container<flags_container_size> flags;
      phmap::flat_hash_map<const modificator*, size_t> modificators;
      
      city();
      
      // нужно ли тут проверять limit_buildings и prev_buildings? где то это делать нужно в любом случае, и я думаю что это быстрый способ собрать available_buildings 
      // то есть если лимит выполняется или прев не выполняется, то это здание не появляется в постройках, более сложные случаи уходят в conditions
      bool check_build(character* c, const uint32_t &building_index) const;
      // функция непосредственно строительства (отнимаем у игрока деньги)
      bool start_build(character* c, const uint32_t &building_index);
      void advance_building();
    };
    
    // треит только для персонажа?
    struct trait {
      static const structure s_type = structure::trait;
      static const size_t max_stat_modifiers_count = 16;
      
      struct numeric_attribs {
        uint8_t birth;
        uint8_t inherit_chance;
        uint8_t both_parent_inherit_chance;
        uint8_t dummy; // тут что?
      };
      
      std::string id;
      size_t name_str;
      size_t description_str;
      // каждые сколько нибудь детей получают этот трейт
      // каста? для некоторых религий
      // шанс унаследовать от родителя и шанс унаследовать от обоих родителей
      // религиозная ветвь (по идее лучше бы сделать отдельно религией?)
      // еще неплохо было бы сделать какие нибудь констреинты, то есть можно/нельзя получить этот трейт если есть другой
      struct numeric_attribs numeric_attribs;
      render::image_t icon;
      utils::bit_field_32<1> attribs;
      //stat_bonus bonuses[16]; 
      std::array<stat_modifier, max_stat_modifiers_count> bonuses; // нужно еще сгенерировать описание для этого
      
      // надеюсь можно будет передать функцию в луа (но кажется нельзя ссылаться на inline функцию)
      trait();
      inline bool get_attrib(const uint32_t &index) const { return attribs.get(index); }
      inline void set_attrib(const uint32_t &index, const bool value) { attribs.set(index, value); }
    };
    
    // разделить треиты и модификаторы чтобы удалить модификаторы после смерти?
    struct modificator {
      static const structure s_type = structure::modificator;
      static const size_t max_stat_modifiers_count = 16;
      
      std::string id;
      size_t name_id;
      size_t description_id;
      size_t time;
      render::image_t icon;
      utils::bit_field_32<1> attribs;
      //stat_bonus bonuses[16]; // нужно еще сгенерировать описание для этого
      std::array<stat_modifier, max_stat_modifiers_count> bonuses; // нужно еще сгенерировать описание для этого
      // бонус к отношениям
      
      // надеюсь можно будет передать функцию в луа (но кажется нельзя ссылаться на inline функцию)
      modificator();
      inline bool get_attrib(const uint32_t &index) const { return attribs.get(index); }
      inline void set_attrib(const uint32_t &index, const bool value) { attribs.set(index, value); }
    };
    
    struct troop_type {
      static const structure s_type = structure::troop_type;
      std::string id;
      size_t name_id;
      size_t description_id;
      // сильные и слабые стороны?
      std::array<stat_container, troop_stats::count> stats;
      // нужны иконки и отображение в бою (2д графика с 8 гранями)
      render::image_t card; // карточка для отображения в контейнере интерфейса армии
      // отображение в бою можно сделать через состояния
      // скилы? было бы неплохо что нибудь такое добавить
      
      troop_type();
    };
    
    // отряды поди должны отдельно существовать (?), мы либо их в городе держим как подкреп
    // либо нанимаем, на мой взгляд и в том и в другом случае нужен указатель
    struct troop {
      const troop_type* type;
      struct character* character; // это может быть отряд полководца
      // характеристики (здоровье, количество в отряде, атака, урон и прочее)
      std::array<stat_container, troop_stats::count> moded_stats; // название нужно сменить
      std::array<stat_container, troop_stats::count> current_stats; //  изменяются только в бою?
      
      troop();
    };
    
    enum class path_finding_state {
      idle,
      get_task,
      finding_path,
      stop,
      stopped,
    };
    
    // по идее что у армии что у героев (что у всех остальных объектов движения)
    // данные поиска пути вообще не особенно будут отличаться,
    // поэтому лучше все это дело унифицировать, причем эта же структура
    // по идее должна работать и битве и в столкновении
    struct path_finding_data {
      std::atomic<ai::path_container*> path;
      size_t path_size;
      size_t current_path;
      std::atomic<uint32_t> start_tile;
      std::atomic<uint32_t> end_tile;
      std::atomic<size_t> path_task;
    };
    
    // армия создается только когда мы собираем войска
    // при роспуске эту структуру удаляем
    // тут нужно указать еще позицию на карте конкретную, для анимации
    // тут еще должно быть хранилище для пути в армии + манипуляция этим путем
    // отряды можно передать, нападение на другую армию, нападение на город, 
    // зайти в город? засада? быстрый марш? пока сделаем базовые штуки
    // 
    struct army { // а как сделана принадлежность армий? пока что вообще ни как
      static const structure s_type = structure::army;
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      
      // отряды + полководцы (главный полководец должен быть всегда первым)
      std::array<troop, max_troops_count> troops; // возьмем в качестве отправной точки 20 юнитов
      uint32_t troops_count;
      // позиция (просто указатель на тайл?)
      uint32_t tile_index;
      // характеристики армии? передвижение, атака, защита (?), все характеристики для отрядов
      std::array<stat_container, army_stats::count> computed_stats; // 
      std::array<stat_container, army_stats::count> current_stats;
      // какие то спутники? (спутники по идее у персонажа)
      // графика (иконка и отображение на карте)
      //render::image_t map_img; // на карте это нужно отображать с флагом
      float current_pos;
      
      // путь? размер пути, пройденный путь, причем эти данные должны добавиться атомарно
      // если мы тут добавим атомарный флаг с состоянием - мы легко решим проблему остановки задачи 
      // нет не легко, нужно тогда еще свою очередь с заданиями тащить, короче тут сложно и непонятно
      // атомарный указатель на атомарную переменную? если есть указатель значит есть задача
      // да но при этом не известно когда она запустится, по идее у нас фифо очередь по этому можно подождать
      // я так подозреваю что в других играх весь поиск делает один поток (что скорее тупо чем практично)
      std::atomic<ai::path_container*> path;
      size_t path_size;
      size_t current_path;
      std::atomic<uint32_t> start_tile;
      std::atomic<uint32_t> end_tile;
      uint32_t army_gpu_slot;
      std::atomic<size_t> path_task;
      
      phmap::flat_hash_map<const modificator*, size_t> modificators;
      phmap::flat_hash_map<const event*, event_container> events;
      phmap::flat_hash_map<std::string_view, size_t> flags;
      //modificators_container<modificators_container_size> modificators; // по идее их меньше чем треитов
      //events_container<events_container_size> events; // должно хватить
      //flags_container<flags_container_size> flags; // флагов довольно много
      
      army();
      ~army();
      
      void set_pos(const glm::vec3 &pos);
      glm::vec3 get_pos() const;
      void set_img(const render::image_t &img);
      render::image_t get_img() const; // куда пихнуть геральдику? так не хочется еще заводить переменные для армий
    };
    
    // нужно описать тип, по нему мы будем нанимать себе в отряд 
    struct party_member_type {
      
    };
    
    // возможно будет частью персонажа
    // нужно у персонажа сделать отдельную геройскую секцию
    // героями будут ну очень малое количество людей по сравнению с общим
    // но при этом несколько геройских вещей должны быть доступны и обычным челикам
    struct hero {
      struct character* character;
      std::array<stat_container, hero_stats::count> moded_stats;
      std::array<stat_container, hero_stats::count> current_stats;
      // графика, как отображать персов в интерфейсе? (нужно хотя бы интерфейс сделать, хаха) думаю что карточки персонажей как раз то что нужно
      
      hero();
    };
    
    // это герой размещенный на карте (по идее мы должны создать эту структуру для каждого героя)
    // объявил себя героем - получи структуру, анимации передвижения я так понимаю будут только у игрока, 
    // следовательно наверное мне не нужно делать много специальных вещей для этого, компы наверное сразу будут прыгать
    // к последней точке, с другой стороны было бы неплохо сделать возможность почекать как был сделан ход
    struct hero_troop {
      static const structure s_type = structure::hero_troop;
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      // как определить это число я пока не понимаю
      // слишком много слотов делать не нужно иначе получится слишком много микроменеджмента
      // число слотов должно быть около 7 (как в героях)
      static const size_t max_game_party_size = 8; // думаю что 8 - это то что нужно, входит ли непосредсвенно лидер партии в это число? скорее всего да
      // нужно определиться с интеллектом, по идее интеллект влияет на силу заклинаний, но я бы хотел чтобы еще по нему увеличивалось количество людей в партии
      // думаю что можно сделать так: средние значения взрослого здорового человека это по 10 каждой характеристики
      // и если одна характиристика достигает 20 то персонаж может объявить себя героем
      // характеристика 40 - это максимум,                (может быть все же 15)
      // соответственно 10 интеллекста это размер пати 3, 12 (13?) - размер пати 4, 20 - 5, 25 - 6, 30 - 7, 35 - 8
      // пусть строго каждые пять интеллекта +1 к размеру пати, для того чтобы объявить себя героем с 20 интеллектом
      // персонаж должен быть практикующим магом
      
      // отряд (отряд у героя состоит из персонажей, некоторые из них сгенерированные (копейщик, нанятый в городе))
      uint32_t party_size;
      // видимо у персонажей должен быть по умолчанию средний интеллект (какой?)
      // и по ходу дела персонаж может либо улучшить его либо ухудшить
      // таким образом уменьшив или увеличив размер партии
      uint32_t max_party_size; // я бы хотел добавить возможность прокачать характеристику "размер отряда героя"
      std::array<hero, max_game_party_size> party;
      
      std::atomic<size_t> path_task;
      std::atomic<ai::path_container*> path; // нужно как то сказать что путь уже можно использовать? атомики более менее норм решение
      size_t path_size;
      size_t current_path;
      uint32_t start_tile;
      uint32_t end_tile;
      // переменные для анимации: нужна наверное одна float переменная для конкретной позиции (анимации только для игрока?) 
      float current_pos;
      // позиция
      uint32_t tile_index;
      uint32_t army_gpu_slot; // это обязательный аттрибут любой армии или героя, нужно где то сделать поиск в обратную сторону
      // статы
      
      phmap::flat_hash_map<const modificator*, size_t> modificators;
      phmap::flat_hash_map<const event*, event_container> events;
      phmap::flat_hash_map<std::string_view, size_t> flags;
//       modificators_container<modificators_container_size> modificators;
//       events_container<events_container_size> events;
//       flags_container<flags_container_size> flags;
      
      hero_troop();
      ~hero_troop();
    };
    
    // (например призвать ко двору нового аристократа, по аналогии с цк2)
    // тут какая то полугенерация, есть основные действия которые не изменяются, 
    // и есть действия сгенерированные становятся доступны после каких нибудь событий в игре
    // нужно проверить как в цк2 (реконкиста помоему одно из таких действий)
    // (нет, реконкиста привязана к культуре)
    // решения можно практически полностью описать в json (по аналогии с цк или европой)
    // есть блок контролирующий появление решения, блок условий и блок эффектов
    // соответственно мы можем задать множество функций на проверку всех этих условий
    // это позволит нам сгенерировать классные описания 
    // может быть еще вещью которая появляется по правой кнопке на персонаже или городе (тип decision)
    // в таких решениях целей может быть несколько (2, +1 в некоторых случаях)
    struct decision {
      static const structure s_type = structure::decision;
      std::string id;
      size_t name_id;
      size_t description_id;
      uint32_t target;
      std::vector<utils::functions_container::operation> potential;
      std::vector<utils::functions_container::operation> conditions;
      std::vector<utils::functions_container::operation> effects;
      
      decision();
    };
    
    struct religion_group {
      static const structure s_type = structure::religion_group;
      
    };
    
    // могут быть как сгенерироваными так и нет
    struct religion {
      static const structure s_type = structure::religion;
      std::string id;
      size_t name_str;
      // описание (?) (возможно частично должно быть сгенерировано например на основе прав)
      size_t description_str;
      const religion_group* group;
      const religion* parent;
      const religion* reformed;
      // с кем можно жениться?
      // сколько жен или наложниц? (нужно ли?)
      // мультипликатор к короткому правлению
      // агрессивность ии
      double aggression;
      // элемент одежды главы (головной убор и сама одежда)
      // сколько ресурсов дает за отсутствие войны
      // название священного похода
      size_t crusade_str;
      // название священного текста
      size_t scripture_str;
      // имена богов и злых богов
      // имя главного бога
      size_t high_god_str;
      // название благочестия
      size_t piety_str;
      // титул священников
      size_t priest_title_str;
      uint32_t opinion_stat_index; // все персонажи этой религии, нужно ли делать отношения отдельных групп персонажей
      render::image_t image;
      // цвет?
      // права (ограничения механик последователей данной религии)
      utils::bit_field_32<utils::religion_mechanics::bit_container_size> mechanics;
      
      // модификаторы к персонажу и юнитам
      // модификаторы если на своей земле
      
      religion();
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
    };
    
    // возможно будет представлен только в качестве id
    // нет скорее всего нужно сделать как отдельную сущность
    struct culture {
      static const structure s_type = structure::culture;
      static const size_t max_stat_modifiers_count = 16;
      
      std::string id;
      size_t name_id;
      // возможно несколько механик (например использование рек кораблями, как в цк2)
      // в цк2 культуры давали отряд, здание, эвенты, решения (decision), 
      // типы правления (китайская администрация), законы наследования, гендерный законы, тактики
      // культурные имена
      utils::localization::string_bank* name_bank;
      // патронимы (специальные префиксы или постфиксы)
      // династические префиксы
      // шансы что назовут в честь деда
      // родительская культура
      const culture* parent; // культурная группа?
      // модификаторы
      // модификаторы персонажа
      std::array<stat_modifier, max_stat_modifiers_count> attrib; // модификаторы юнитов?
      utils::bit_field_32<utils::culture_mechanics::bit_container_size> mechanics;
      
      culture();
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
    };
    
    struct law {
      static const structure s_type = structure::law;
      static const size_t max_stat_modifiers_count = 16;
      static const size_t max_mechanics_modifiers_count = 16;
      
      std::string id;
      size_t name_id; // у некоторых сущностей имена должны быть определены заранее (ну то есть мы наверное можем сгенерировать закон, но пока нет)
      size_t description_id;
      std::array<stat_modifier, max_stat_modifiers_count> attrib;                        // закон тоже может изменить аттрибуты
      std::array<utils::mechanics_modifier, max_mechanics_modifiers_count> modificators; // применяем изменения к механикам государства (тут видимо нужно делать больше размер)
      std::vector<utils::functions_container::operation> potential;
      std::vector<utils::functions_container::operation> conditions;
      // но закон еще меняет механику, механика будет привязана к функции луа (не будет наверное)
      // источники говорят что феодализм существовал в китае рядом с чиновниками
      // и по умолчанию государство раздавало земли крестьянам и чиновникам,
      // скопив много земель богатые китайцы видимо превращались в феодалов,
      // тут у нас явно какой то закон о запрете наследования земли
      // скорее всего законы будут заданы здесь
      // есть ряд механик которые может делать игрок, например забирать титулы
      // нужно чтобы именно эти механики у игрока забирались и передавались некоему органу 
      // (совет, а если могущественный то парламент)
      // описал очень много механик связанных с законами
      // закон меняет механику лишь однажды, для того чтобы отменить закон нужно принять предыдущий закон
      // аттрибуты перевычисляются каждый ход (?) 
      
      law();
    };
    
    // нужно определиться с принципом: все что не запрещено - разрешено или все что не разрешено - запрещено
    // права != законы, права прежде всего должны как то ограничивать механики
    // например наследовать могут только мужчины, значит из некоей выборки родственников нужно выделить только мужчин
    // старше там скольки то лет, по идее здесь нужно будет только ограничить выборку, что мы можем сделать в луа
    // то есть достаточно по идее 64 (?) бит для запоминания, что можно а что нет
    // но вообще у нас есть еще законы (причем по категориям), и мехиники могут меняться и по законам
    // возможно "права" останутся только у религий, которые в этом плане будут статичными
    struct right {
      
    };
    
    // сгенерированный эвент? скорее нет, чем да
    struct event {
      static const structure s_type = structure::event;
      
      struct option {
        size_t name_id; // наверное и названия нужно скомпилировать
        size_t desc_id;
        std::vector<utils::functions_container::operation> conditions; // наверное можно использовать статический массив
        std::vector<utils::functions_container::operation> effects; // как сгенерить описание?
        
        option();
      };
      
      std::string id;
      size_t name_id; // индекс строки должен состоять из индекса банка и индекса собственно строки
      size_t description_id;
      enum utils::target_data::type target;
      render::image_t image;
      size_t mtth; // среднее время возникновения, должны быть модификаторы для этого 
      //std::function<bool(const target_data&)> potential;
      uint32_t options_count;
      std::vector<utils::functions_container::operation> conditions; // средняя длинна скорее всего не привышает 16 (может и меньше) 
      std::array<option, max_options_count> options;
      
      event();
      
      void fire(character* c);
    };
  }
}

// нужно еще придумать что делать с локализацией
// нужны файлы локализации, где мы будем брать строки по айдишнику
// есть банк строк id = en если у нас локализация en
// строки нужно еще парсить

// описывать решения в json
// в блоке allow указаны условия выполнения решения, в effect - то что произойдет
// они легко парсятся в словесное описание
// мне нужно определить вообще все возможные функции которые могут потребоваться в таких описаниях
// например добавить немного монет, либо мы можем определить луа функцию (но что делать с описанием?)
// для того чтобы это сработало нужно очень быстро обходить все "target" (все титулы, все города)
// и сверять все свойства, делать это каждый ход гораздо проще чем каждый кадр, но при этом
// лучше конечно чтобы я мог сбалансировать нагрузку на ход, как это делать? запускать проверку 
// по частям (500 в этом ходу 500 в следующем и проч), я должен описать интерфейс так чтобы 
// как можно сильнее сократить количество проверок, эффективно парсить требуемое по условиям
// (мне кажется парсинг легче в луа? но невозможно сделать адекватный мультитрединг в этом случае)
// в цк2 все сущности строго определены, все действия строго определены, и моддер просто 
// составлял в текстовом файле последовательность условий, и игра их пыталась исполнить

// как мы будем делать обход? чекаем тип, обходим вектор, проверяем каждого персонажа,
// но мне можно не проверять уже стриггеренные эвенты, я так понимаю что тут нужно проверять 
// мапу (?) для того чтобы проверки не делать, нет, нужно создать несколько векторов 
// с парочкой данных для каждого таргета (двумерный массив? так или иначе нужно как то запомнить когда первый раз триггернули)
// мапу заводить для каждого персонажа? нам в любом случае нужен механизм для 
// запоминания флага (тега), мы можем просто рядом запомнить время флага
// это нам поможет с эвентами, что с решениями? решения нужно разбить по типам
// одни типы пойдут в окно решений, другие по правой кнопке будут открываться,
// нужно придумать фильтр, нужно как то ограничить дальность взаимодействий персонажей,
// возможно даже ограничить взаимодействие с чужими вассалами
// некоторые решения должны быть написаны от руки (например женитьба и какие то такие вещи)
// причем наверное нужно сделать отдельно дипломатию, чтобы автоматизировать некоторые вещи
// вообщем нужно придумать серьезные ограничения

// 

#endif
