#ifndef CORE_STRUCTURES_H
#define CORE_STRUCTURES_H

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <array>
#include "character.h"
#include "render/shared_structures.h"
#include "data_parser.h"
#include "utils/bit_field.h"
#include "utils/traits_modifier_attribs.h"
#include "utils/realm_mechanics.h"
#include "utils/string_bank.h"

// на каждый чих нужно придумать иконки

namespace devils_engine {
  namespace core {
    const size_t max_conditions_count = 64;
    const size_t max_effects_count = 64;
    const size_t max_options_count = 8;
    const size_t max_troops_count = 20;
    // как определить это число я пока не понимаю
    // слишком много слотов делать не нужно иначе получится слишком много микроменеджмента
    // число слотов должно быть около 7 (как в героях)
    const size_t max_game_party_size = 8; // думаю что 8 - это то что нужно, входит ли непосредсвенно лидер партии в это число? скорее всего да
    // нужно определиться с интеллектом, по идее интеллект влияет на силу заклинаний, но я бы хотел чтобы еще по нему увеличивалось количество людей в партии
    // думаю что можно сделать так: средние значения взрослого здорового человека это по 10 каждой характеристики
    // и если одна характиристика достигает 20 то персонаж может объявить себя героем
    // характеристика 40 - это максимум,                (может быть все же 15)
    // соответственно 10 интеллекста это размер пати 3, 12 (13?) - размер пати 4, 20 - 5, 25 - 6, 30 - 7, 35 - 8
    // пусть строго каждые пять интеллекта +1 к размеру пати, для того чтобы объявить себя героем с 20 интеллектом
    // персонаж должен быть практикующим магом
    
    struct tile;
    struct province;
    struct city;
    struct faction;
    struct army;
    struct hero;
    struct decision;
    struct religion;
    struct culture;
    struct law;
    struct right;
    struct dynasty;
    struct event;
    struct troop;
    struct party_member;
    
    struct tile_template {
      
    };
    
    struct tile {
      // возможно стоит выделить тайл и некоторые характеристики в нем
      // например для того чтобы описать правильно режим отображения карты
    };
    
    // если персонаж владеет титулом баронским этой провинции - он владеет и столицей провинции? (как в цк2)
    // скорее всего да
    struct province {
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      // static attrib* attributes;
      
      // название (или название по столице провинции?)
      // удобно и герб тоже по столице провинции дать
      titulus* title;
      std::vector<uint32_t> tiles;
      std::vector<uint32_t> neighbours;
      // какие техи распространились в этой провинции
      // что то вроде std::vector<bool> ?
      // локальные характеристики (принадлежность провинции?)
      // несколько "городов" (могут быть и замки и епископства, больше типов?) (как минимум должны быть епископства)
      // какой максимум поселений? 7 в цк2, у меня скорее всего меньше (3-4?)
      uint32_t cities_max_count; // в зависимости от размера провинции
      uint32_t cities_count;
      city* cities[10];
      
      modificators_container<modificators_container_size> modificators; // по идее их меньше чем треитов
      events_container<events_container_size> events; // должно хватить
      flags_container<flags_container_size> flags; // флагов довольно много
    };
    
    // постройка в городе 
    struct building_type {
      static const size_t maximum_prev_buildings = 8;
      static const size_t maximum_limit_buildings = 8;
      
      std::string id;
      // название
      // описание
      // доступность (проверки условий)
      // возможность
      // предыдущие постройки (можно ли какой то максимум определить?)
      const building_type* prev_buildings[maximum_prev_buildings];
      // нельзя построить если существуют ... (максимум?)
      const building_type* limit_buildings[maximum_limit_buildings];
      // то что заменяет (индекс? заменяет предыдущий уровень?)
      const building_type* replaced;
      // улучшение (если построено предыдущее здание, то открывается это)
      const building_type* upgrades_from;
      // стоимость постройки (может использовать разные ресурсы)
      // скорость строительства (недели)
      size_t time;
      // как ии поймет что нужно строить это здание? (ии все же наверное будет работать по функции луа)
      // при каких условиях здание будет построено перед началом игры (нужно ли?) (должна быть возможность указать что построено а что нет в генераторе)
      // вижен? (дает ли вижен провинции для игрока построившего это)
      // во что конвертируется (?) (нужно ли делать переход от одного типа города к другому?)
      // модификаторы (в цк2 могло дать скорость иследований, в моем случае нужно придумать какой то иной способ)
      // здания наверное могу давать какие то эвенты с какой то периодичностью
    };
    
    struct city_type {
      static const size_t maximum_buildings = 128;
      std::string id;
      const building_type* buildings[maximum_buildings];
      // графика
    };
    
    // на карте еще было бы неплохо разместить что то вроде данжей (или например хижину ведьмы)
    // я полагаю необходимо разделить эти вещи
    struct city {
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      static const size_t bit_field_size = city_type::maximum_buildings / SIZE_WIDTH;
      static_assert(bit_field_size == 2);
      
      uint32_t name_index;
      struct province* province;
      struct titulus* title; // герб
      const city_type* type;
      utils::bit_field<bit_field_size> available_buildings;
      utils::bit_field<bit_field_size> complited_buildings;
      size_t start_building;
      uint32_t building_index;
      // характеристики (текущие характеристики города, база + со всех зданий) (характеристики довольно ограничены, нужно ли дать возможность их модифицировать?)
      
      modificators_container<modificators_container_size> modificators; // по идее их меньше чем треитов
      events_container<events_container_size> events; // должно хватить
      flags_container<flags_container_size> flags; // флагов довольно много
    };
    
    struct trait {
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
      render::image_t icon;
      utils::bit_field_32<1> attribs;
      stat_bonus bonuses[16]; // нужно еще сгенерировать описание для этого
      
      // надеюсь можно будет передать функцию в луа (но кажется нельзя ссылаться на inline функцию)
//       inline bool is_hero() const { return attribs.get(utils::trait::is_hero); }
//       inline bool is_agnatic() const { return attribs.get(utils::trait::is_agnatic); }
//       inline bool is_enatic() const { return attribs.get(utils::trait::is_enatic); }
//       inline bool cannot_inherit() const { return attribs.get(utils::trait::cannot_inherit); }
//       inline bool cannot_marry() const { return attribs.get(utils::trait::cannot_marry); }
//       inline bool educational() const { return attribs.get(utils::trait::educational); }
//       inline bool is_hidden() const { return attribs.get(utils::trait::is_hidden); }
//       inline bool prevent_death_from_age() const { return attribs.get(utils::trait::prevent_death_from_age); }
//       inline bool prevent_death_from_disease() const { return attribs.get(utils::trait::prevent_death_from_disease); }
//       inline bool prevent_death_from_magic_disease() const { return attribs.get(utils::trait::prevent_death_from_magic_disease); }
//       inline bool inbred_trait() const { return attribs.get(utils::trait::inbred_trait); }
//       inline bool incapacitating() const { return attribs.get(utils::trait::incapacitating); } // требуется регент + не может наследовать, насколько я понимаю этот эффект мы можем получить с помощью других свойств
//       inline bool is_epidemic() const { return attribs.get(utils::trait::is_epidemic); }
//       inline bool is_health() const { return attribs.get(utils::trait::is_health); }
//       inline bool is_disease() const { return attribs.get(utils::trait::is_disease); }
//       inline bool is_magic_disease() const { return attribs.get(utils::trait::is_magic_disease); }
//       inline bool is_leadership() const { return attribs.get(utils::trait::is_leadership); }
//       inline bool is_childhood() const { return attribs.get(utils::trait::is_childhood); }
//       inline bool is_lifestyle() const { return attribs.get(utils::trait::is_lifestyle); }
//       inline bool is_personality() const { return attribs.get(utils::trait::is_personality); }
//       inline bool is_priest() const { return attribs.get(utils::trait::is_priest); } // этот трейт делает перса священником, могут выпадать эвенты связанные с этим
//       inline bool is_on_adventure() const { return attribs.get(utils::trait::is_on_adventure); } // требуется регент
//       inline bool can_get_on_born() const { return attribs.get(utils::trait::can_get_on_born); } // этот трейт можем получить при рождении
//       inline bool rebel_inherited() const { return attribs.get(utils::trait::rebel_inherited); } // если восстающий персонаж мертв, то этот трейт переходит к любому наследнику
//       inline bool is_religious() const { return attribs.get(utils::trait::is_religious); }
//       inline bool same_trait_visibility() const { return attribs.get(utils::trait::same_trait_visibility); }
//       inline bool hidden_from_others() const { return attribs.get(utils::trait::hidden_from_others); }
//       inline bool is_vice() const { return attribs.get(utils::trait::is_vice); }
//       inline bool is_virtue() const { return attribs.get(utils::trait::is_virtue); }
//       inline bool succession_gfx() const { return attribs.get(utils::trait::succession_gfx); }
//       inline bool is_symptom() const { return attribs.get(utils::trait::is_symptom); }
//       //inline bool prevent_decadence() const { return attribs.get(3); } // возможно механика decadence будет присутствовать в каком то виде
//       inline void set_is_hero(const bool value) { attribs.set(0, value); }
//       inline void set_is_major(const bool value) { attribs.set(1, value); }
      inline bool get_attrib(const uint32_t &index) const { return attribs.get(index); }
      inline void set_attrib(const uint32_t &index, const bool value) { attribs.set(index, value); }
    };
    
    // разделить треиты и модификаторы чтобы удалить модификаторы после смерти?
    struct modificator {
      std::string id;
      size_t name;
      size_t description;
      size_t time;
      render::image_t icon;
      utils::bit_field_32<1> attribs;
      stat_bonus bonuses[16]; // нужно еще сгенерировать описание для этого
      
      // надеюсь можно будет передать функцию в луа (но кажется нельзя ссылаться на inline функцию)
      inline bool get_attrib(const uint32_t &index) const { return attribs.get(index); }
      inline void set_attrib(const uint32_t &index, const bool value) { attribs.set(index, value); }
    };
    
    struct faction {
      // глобальные характеристики
      stat_data stats[faction_stats::count];
      struct character* character;
      // принятые законы (законы по категориям, причем первые две категории всегда про наследование)
      // права жителей фракции (скорее всего едины с законами) (состоят из прав религии и локальных прав, что то еще?) (более менее сделано)
      // всякие технические вещи
      utils::bit_field_32<utils::realm_mechanics::bit_container_size> mechanics; // это по сути и будут нашими законами
      // конкретные законы и их группировка задается в структуре
      
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
    };
    
    struct troop_type {
      std::string id;
      // название
      // описание
      // сильные и слабые стороны?
      stat_container stats[troop_stats::count];
      // нужны иконки и отображение в бою (2д графика с 8 гранями)
      // скилы? было бы неплохо что нибудь такое добавить
    };
    
    struct troop {
      const troop_type* type;
      struct character* character; // это может быть отряд полководца
      // характеристики (здоровье, количество в отряде, атака, урон и прочее)
      stat_container moded_stats[troop_stats::count];
      stat_container current_stats[troop_stats::count]; //  изменяются только в бою?
    };
    
    // армия создается только когда мы собираем войска
    // при роспуске эту структуру удаляем
    struct army {
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      
      // отряды + полководцы (главный полководец должен быть всегда первым)
      uint32_t troops_count;
      std::array<troop, max_troops_count> troops; // возьмем в качестве отправной точки 20 юнитов
      // позиция (просто указатель на тайл?)
      uint32_t tile_index;
      // характеристики армии? передвижение, атака, защита (?), все характеристики для отрядов
      // какие то спутники? (спутники по идее у персонажа)
      // графика (иконка и отображение на карте)
      
      modificators_container<modificators_container_size> modificators; // по идее их меньше чем треитов
      events_container<events_container_size> events; // должно хватить
      flags_container<flags_container_size> flags; // флагов довольно много
    };
    
    // нужно описать тип, по нему мы будем нанимать себе в отряд 
    struct party_member_type {
      
    };
    
    struct hero {
      struct character* character;
      stat_container moded_stats[troop_stats::count];
      stat_container current_stats[hero_stats::count];
      // графика
    };
    
    // это герой размещенный на карте (по идее мы должны создать эту структуру для каждого героя)
    // объявил себя героем - получи структуру
    struct hero_troop {
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      
      // отряд (отряд у героя состоит из персонажей, некоторые из них сгенерированные (копейщик, нанятый в городе))
      uint32_t party_size;
      // видимо у персонажей должен быть по умолчанию средний интеллект (какой?)
      // и по ходу дела персонаж может либо улучшить его либо ухудшить
      // таким образом уменьшив или увеличив размер партии
      uint32_t max_party_size; // я бы хотел добавить возможность прокачать характеристику "размер отряда героя"
      std::array<hero, max_game_party_size> party;
      // позиция
      uint32_t tile_index;
      
      modificators_container<modificators_container_size> modificators;
      events_container<events_container_size> events;
      flags_container<flags_container_size> flags;
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
      std::string id;
      size_t name_id;
      size_t description_id;
      uint32_t target;
      std::vector<utils::functions_container::operation> potential;
      std::vector<utils::functions_container::operation> conditions;
      std::vector<utils::functions_container::operation> effects;
    };
    
    struct religion_group {
      
    };
    
    // могут быть как сгенерироваными так и нет
    struct religion {
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
      
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
    };
    
    // возможно будет представлен только в качестве id
    // нет скорее всего нужно сделать как отдельную сущность
    struct culture {
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
      const culture* parent;
      // модификаторы
      // модификаторы персонажа
      stat_modifier attrib[16];
      utils::bit_field_32<utils::culture_mechanics::bit_container_size> mechanics;
      
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
    };
    
    struct law {
      std::string id;
      size_t name_id; // у некоторых сущностей имена должны быть определены заранее (ну то есть мы наверное можем сгенерировать закон, но пока нет)
      size_t description_id;
      stat_modifier attrib[16];                   // закон тоже может изменить аттрибуты
      utils::mechanics_modifier modificators[16]; // применяем изменения к механикам государства
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
      struct option {
        size_t name_id; // наверное и названия нужно скомпилировать
        size_t desc_id;
        std::vector<utils::functions_container::operation> conditions; // наверное можно использовать статический массив
        std::vector<utils::functions_container::operation> effects; // как сгенерить описание?
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
      
      
    };
    
    struct context {
      // мемори пулы для всех структур
      // какие то объекты должны возвращаться константными
      // 
      
      std::vector<province*> provinces;
      std::vector<city*> cities;
      std::vector<building_type*> building_types;
      std::vector<faction*> factions;
      std::vector<troop*> troops; // тип отряда
      std::vector<army*> armies;
      std::vector<hero*> party_members; // тип члена партии
      std::vector<hero_troop*> hero_troops;
      std::vector<decision*> decisions; // 
      std::vector<religion*> religions; // какие то ограничения от религий
      std::vector<culture*> cultures; // культуры поди тоже что то должны добавить
      std::vector<law*> laws; // ограничения и механики от законов
      std::vector<dynasty*> dynasties; // это по сути хранилища для персонажей
      std::vector<event*> events;
      
      // для создания всех структур нужно придумать луа таблицы
      // не всех, все что относится к карте (тайл, провинция, провинция, город, фракция, религия, культура, династии)
      // нужно создавать вручную, для некоторых вещей нужно прописать характеристики в конфиге
      // некоторые вещи будут заданы более жестко, вот что я подумал, 
      // нужно по возможности вообще весь конфиг сделать во время генерации карты
      // так на мой взгляд проще писать загрузку модов (нужно сделать только загрузку генератора)
      // некоторые данные скорее всего не получиться загрузить в генераторе 
      // (изображения, нужно осадить модеров которые попытаются слишком много всего загрузить, аудио)
      // для тайлов нужно просто какой то темплейт создать, так и для всего остального можно сделать темплейт
      // мне нужно будет это сделать для аттрибутов, или лучше сделать аттрибуты в конфиге
      // не, можно и так, нужно разделить темплейт на собственно аттрибуты и аттрибуты генерации, последние удалить после генерации
      // нужно ли как то ограничивать темплейт характеристик? на самом деле сначало бы сделать константные характеристики
      // еще как сделать режимы отображения карты
      std::pair<province*, uint32_t> create_province();
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
