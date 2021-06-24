#ifndef STATS_H
#define STATS_H

#include <cstdint>
#include "render/shared_structures.h"
#include "utils/magic_enum_header.h"
#include "utils/bit_field.h"

#define CONCAT(a, b) a##b
#define PENALTY_STAT(stat) CONCAT(stat, _penalty)

#define TROOP_STATS_LIST TROOP_STAT_FUNC(troop_size), \
  TROOP_STAT_FUNC(max_hp),               \
  TROOP_STAT_FUNC(morale),               \
  TROOP_STAT_FUNC(armor),                \
  TROOP_STAT_FUNC(siege_armor),          \
  TROOP_STAT_FUNC(speed),                \
  TROOP_STAT_FUNC(initiative),           \
  TROOP_STAT_FUNC(melee_attack),         \
  TROOP_STAT_FUNC(melee_defence),        \
  TROOP_STAT_FUNC(range_attack),         \
  TROOP_STAT_FUNC(range_defence),        \
  TROOP_STAT_FUNC(charge),               \
  TROOP_STAT_FUNC(morale_damage),        \
  TROOP_STAT_FUNC(melee_damage),         \
  TROOP_STAT_FUNC(melee_armor_piercing), \
  TROOP_STAT_FUNC(melee_magic),          \
  TROOP_STAT_FUNC(melee_fire),           \
  TROOP_STAT_FUNC(melee_siege),          \
  TROOP_STAT_FUNC(range_damage),         \
  TROOP_STAT_FUNC(range_armor_piercing), \
  TROOP_STAT_FUNC(range_magic),          \
  TROOP_STAT_FUNC(range_fire),           \
  TROOP_STAT_FUNC(range_siege),          \
  TROOP_STAT_FUNC(accuracy),             \
  TROOP_STAT_FUNC(reloading),            \
  TROOP_STAT_FUNC(ammo),                 \
  TROOP_STAT_FUNC(maintenance),          \
  TROOP_STAT_FUNC(provision),            \
  TROOP_STAT_FUNC(recovery),

#define HERO_STATS_LIST HERO_STAT_FUNC(max_hp), \
  HERO_STAT_FUNC(speed),                   \
  HERO_STAT_FUNC(initiative),              \
  HERO_STAT_FUNC(melee_attack),            \
  HERO_STAT_FUNC(melee_defence),           \
  HERO_STAT_FUNC(range_attack),            \
  HERO_STAT_FUNC(range_defence),           \
  HERO_STAT_FUNC(min_damage),              \
  HERO_STAT_FUNC(max_damage),              \
  HERO_STAT_FUNC(ammo),                    \
  HERO_STAT_FUNC(health_per_damage_level), \
  HERO_STAT_FUNC(recovery),               

#define CHARACTER_STATS_LIST CHARACTER_STAT_FUNC(military), \
  CHARACTER_STAT_FUNC(managment),               \
  CHARACTER_STAT_FUNC(diplomacy),               \
  CHARACTER_STAT_FUNC(health),                  \
  CHARACTER_STAT_FUNC(fertility),               \
  CHARACTER_STAT_FUNC(strength),                \
  CHARACTER_STAT_FUNC(agility),                 \
  CHARACTER_STAT_FUNC(intellect),               \
  CHARACTER_STAT_FUNC(PENALTY_STAT(military)),  \
  CHARACTER_STAT_FUNC(PENALTY_STAT(managment)), \
  CHARACTER_STAT_FUNC(PENALTY_STAT(diplomacy)), \
  CHARACTER_STAT_FUNC(PENALTY_STAT(health)),    \
  CHARACTER_STAT_FUNC(PENALTY_STAT(fertility)), \
  CHARACTER_STAT_FUNC(PENALTY_STAT(strength)),  \
  CHARACTER_STAT_FUNC(PENALTY_STAT(agility)),   \
  CHARACTER_STAT_FUNC(PENALTY_STAT(intellect)), \
  CHARACTER_STAT_FUNC(demesne_size),            \
  CHARACTER_STAT_FUNC(culture_flex),            \
  CHARACTER_STAT_FUNC(religion_flex),           \
  CHARACTER_STAT_FUNC(assassinate_chance_mod),  \
  CHARACTER_STAT_FUNC(arrest_chance_mod),       \
  CHARACTER_STAT_FUNC(plot_power_mod),          \
  CHARACTER_STAT_FUNC(murder_plot_power_mod),   \
  CHARACTER_STAT_FUNC(defensive_plot_power_mod),\
  CHARACTER_STAT_FUNC(plot_discovery_chance),   \
  CHARACTER_STAT_FUNC(ai_rationality),          \
  CHARACTER_STAT_FUNC(ai_zeal),                 \
  CHARACTER_STAT_FUNC(ai_greed),                \
  CHARACTER_STAT_FUNC(ai_honor),                \
  CHARACTER_STAT_FUNC(ai_ambition),             \
  CHARACTER_STAT_FUNC(authority_income_mod),    \
  CHARACTER_STAT_FUNC(esteem_income_mod),       \
  CHARACTER_STAT_FUNC(influence_income_mod),    \
  CHARACTER_STAT_FUNC(income_mod),              \
  CHARACTER_STAT_FUNC(authority_income),        \
  CHARACTER_STAT_FUNC(esteem_income),           \
  CHARACTER_STAT_FUNC(influence_income),        \
  CHARACTER_STAT_FUNC(income),                  \
  CHARACTER_STAT_FUNC(authority),               \
  CHARACTER_STAT_FUNC(esteem),                  \
  CHARACTER_STAT_FUNC(influence),               \
  CHARACTER_STAT_FUNC(money),

#define REALM_STATS_LIST REALM_STAT_FUNC(tax_income_mod), \
  REALM_STAT_FUNC(trade_income_mod),         \
  REALM_STAT_FUNC(provision_mod),            \
  REALM_STAT_FUNC(army_recovery_mod),        \
  REALM_STAT_FUNC(army_morale_recovery_mod), \
  REALM_STAT_FUNC(army_discipline),          \
  REALM_STAT_FUNC(army_maintenance_mod),     \
  REALM_STAT_FUNC(army_morale),              \
  REALM_STAT_FUNC(casualties_mod),           \
  REALM_STAT_FUNC(army_speed),               \
  REALM_STAT_FUNC(army_vision),              \
  REALM_STAT_FUNC(hero_recovery_mod),        \
  REALM_STAT_FUNC(hero_discipline),          \
  REALM_STAT_FUNC(build_cost_mod),           \
  REALM_STAT_FUNC(build_time_mod),           \
  REALM_STAT_FUNC(population_growth_mod),    \
  REALM_STAT_FUNC(max_population_mod),       \
  REALM_STAT_FUNC(tech_spread_mod),          \
  REALM_STAT_FUNC(global_revolt_risk),       \
  REALM_STAT_FUNC(revolt_risk_mod),          \
  REALM_STAT_FUNC(vassal_limit),
  
#define PROVINCE_STATS_LIST PROVINCE_STAT_FUNC(tax_income_mod), \
  PROVINCE_STAT_FUNC(provision_mod),                \
  PROVINCE_STAT_FUNC(army_recovery_mod),            \
  PROVINCE_STAT_FUNC(army_morale_recovery_mod),     \
  PROVINCE_STAT_FUNC(army_discipline),              \
  PROVINCE_STAT_FUNC(army_maintenance_mod),         \
  PROVINCE_STAT_FUNC(army_morale),                  \
  PROVINCE_STAT_FUNC(casualties_mod),               \
  PROVINCE_STAT_FUNC(army_speed),                   \
  PROVINCE_STAT_FUNC(army_vision),                  \
  PROVINCE_STAT_FUNC(authority_increase_mod),       \
  PROVINCE_STAT_FUNC(esteem_increase_mod),          \
  PROVINCE_STAT_FUNC(influence_increase_mod),       \
  PROVINCE_STAT_FUNC(authority_increase_liege_mod), \
  PROVINCE_STAT_FUNC(esteem_increase_liege_mod),    \
  PROVINCE_STAT_FUNC(influence_increase_liege_mod), \
  PROVINCE_STAT_FUNC(local_revolt_risk),            \
  PROVINCE_STAT_FUNC(disease_defence),              \
  PROVINCE_STAT_FUNC(culture_flex),                 \
  PROVINCE_STAT_FUNC(religion_flex),                \
  PROVINCE_STAT_FUNC(short_reign_length),           \
  PROVINCE_STAT_FUNC(attrition),                    \
  PROVINCE_STAT_FUNC(max_attrition),
  
#define CITY_STATS_LIST CITY_STAT_FUNC(tax_income), \
  CITY_STAT_FUNC(tax_income_mod),           \
  CITY_STAT_FUNC(authority_increase),       \
  CITY_STAT_FUNC(esteem_increase),          \
  CITY_STAT_FUNC(influence_increase),       \
  CITY_STAT_FUNC(authority_increase_liege), \
  CITY_STAT_FUNC(esteem_increase_liege),    \
  CITY_STAT_FUNC(influence_increase_liege), \
  CITY_STAT_FUNC(build_cost_mod),           \
  CITY_STAT_FUNC(build_time_mod),           \
  CITY_STAT_FUNC(population),               \
  CITY_STAT_FUNC(population_growth),        \
  CITY_STAT_FUNC(max_population),           \
  CITY_STAT_FUNC(tech_spread),              \
  CITY_STAT_FUNC(demesne_size),             \
  CITY_STAT_FUNC(trade_power),              \
  CITY_STAT_FUNC(production),               \
  CITY_STAT_FUNC(provision),                \
  CITY_STAT_FUNC(winter_provision),         \
  CITY_STAT_FUNC(provision_mod),            \
  CITY_STAT_FUNC(army_recovery_mod),        \
  CITY_STAT_FUNC(army_morale_recovery_mod), \
  CITY_STAT_FUNC(army_discipline),          \
  CITY_STAT_FUNC(army_maintenance_mod),     \
  CITY_STAT_FUNC(army_morale),              \
  CITY_STAT_FUNC(casualties_mod),           \
  CITY_STAT_FUNC(army_speed),               \
  CITY_STAT_FUNC(army_vision),

#define ARMY_STATS_LIST ARMY_STAT_FUNC(speed), \
  ARMY_STAT_FUNC(vision),          \
  ARMY_STAT_FUNC(provision_mod),   \
  ARMY_STAT_FUNC(maintenance_mod), \
  ARMY_STAT_FUNC(morale_mod),      \
  ARMY_STAT_FUNC(morale_recovery), \
  ARMY_STAT_FUNC(discipline),      \
  ARMY_STAT_FUNC(recovery_mod),    \
  ARMY_STAT_FUNC(casualties_mod),
  
#define HERO_TROOP_STATS_LIST HERO_TROOP_STAT_FUNC(speed), \
  HERO_TROOP_STAT_FUNC(vision), \
  HERO_TROOP_STAT_FUNC(discipline), \
  HERO_TROOP_STAT_FUNC(recovery_mod),
  
// ко всем статам еще нужны иконки, названия, описания
// по всей видимости должны быть четкие разделения на статы фракции -> персонажа -> провинции -> города -> здания
namespace devils_engine {
  namespace core {
    struct troop_type;
    
    enum class unit_type {
      invalid,
      troop,
      hero,
      character,
      faction,
      city,
      province
    };
    
    namespace stat_type {
      enum values {
        uint_t,
        int_t,
        float_t
      };
    }
    
    // типы значений? по идее тут все значения int
    namespace troop_stats { // не подходит для геройских битв
      enum values {
//         current_troop_size, 
//         current_hp,
//         current_morale,
//         troop_size, // текущий размер?
//         max_hp,     // текущее хп?
//         morale,
//         armor,
//         siege_armor, // защита против урона по зданиям
//         speed,
//         initiative,
//         melee_attack,
//         melee_defence,
//         range_attack,
//         range_defence,
//         charge,
//         // нужно наверное добавить еще какой тип (что то вроде темной магии) для которой будет очень мало резистов
//         // этим может быть и fire damage (или уменьшать его от резиста к магии? огненные стрелы это не магия)
//         morale_damage,
//         melee_damage, 
//         melee_armor_piercing,
//         melee_magic,
//         melee_fire,
//         melee_siege,
//         range_damage,
//         range_armor_piercing,
//         range_magic,
//         range_fire,
//         range_siege,
//         accuracy,
//         reloading,
//         ammo,
//         // небоевые статы
//         maintenance, // денюшка за ход (?)
//         provision,   // некая переменная которая должна обозначать богатсво провинции и возможность кормить конкретную армию
//         recovery,    // восстановление хп, каждый ход восстанавливаемся почуть чуть в зависимости от снабжения?
        
#define TROOP_STAT_FUNC(val) val
        
        TROOP_STATS_LIST
        
#undef TROOP_STAT_FUNC
        
        count
      };
    }
    
    // здесь по идее все инты
    // в баннер саге используется такой стат как виллповер, позволяет делать какие то экстра вещи (дальше бегать например)
    // не думаю что мне нужен виллповер
    // у юнитов должны быть еще какие то пассивки (например всегда отвечать, атака на все тайлы вокруг и проч)
    // или активки, может ли здание давать эти статы? при посещении (так лучше не делать) или владельцу и его пати?
    // всем пати под знаменами владельца? (то есть у нас может быть партия при дворе, поэтому получает бонус)
    // какие то партии мы можем отправить ко двору, например, сюзерена и получать бонус от него, но контролировать партию
    // так можно сделать, соответственно такие здания должны быть редки, можем ли мы отправить героя ко двору вассала?
    // наверное да только за какой то ресурс (например так может уменьшаться влияние), кому даются какие статы?
    // у нас видимо должно быть разделение на типы героев в этом случае 
    // (типы героев вообще вещь полезная: стартовые статы героя, пассивки, активки, условия генерации и прочее)
    namespace hero_stats {
      enum values {
//         current_hp,
//         max_hp, // текущее хп?
//         // вряд ли нужно давать героям возможность сбежать
//         // вместо этого нужно сделать удачу и мораль
//         //leadership,
//         speed,
//         initiative,  // для того чтобы это хорошо работало нужна шкала с инициативой как в пятых героях
//         melee_attack,
//         melee_defence,
//         range_attack,
//         range_defence,
//         min_damage, // доп урон?
//         max_damage,
//         ammo,
//         health_per_damage_level, // с уменьшением здоровья уменьшается и урон
//         // небоевые статы
//         recovery,
//         // зарплата?
        
#define HERO_STAT_FUNC(val) val
        
        HERO_STATS_LIST
        
#undef HERO_STAT_FUNC
        
        count
      };
    }
    
    // я подумал что можно сделать несколько статов: престиж, благочестие, авторитет
    // нет, лучше сделать вот что: авторитет, уважение и влияние
    // характеристики которые мы расходуем по разному в разных эвентах
    // (то есть они не привязаны напрямую к военному делу или церкви)
    // возможно отношение церкви нужно передать через другую переменную
    // эти статы не наследуются (есть трейты которые могут унаследоваться)
    // наследуются только титулы и деньги + кровь 
    // модификаторы отношений к дворянству и церкови?
    // как пересчитывать статы? нужны ли дефолтные статы? 
    // выяснил что в цк2 показаны дефолтные статы + показано какой трейт как воздействует
    // 
    namespace character_stats {
      enum values {
//         // статы правителя
//         military, // пенальти для каждого скила?
//         managment,
//         diplomacy,
//         // интрига? не помешало бы что то подобное, 
//         // хотя с этим частично могут справиться и другие статы
//         health,
//         fertility,
//         // геройские статы
//         strength,
//         agility,
//         intellect,
//         
//         PENALTY_STAT(military),
//         PENALTY_STAT(managment),
//         PENALTY_STAT(diplomacy),
//         PENALTY_STAT(health),
//         PENALTY_STAT(fertility),
//         PENALTY_STAT(strength),
//         PENALTY_STAT(agility),
//         PENALTY_STAT(intellect),
//         
//         demesne_size,
//         culture_flex,  // толерантность к другим культурам, это я так полагаю для ии
//         religion_flex,
//         // могут улучшиться по эвентам, невидимы игроку
//         assassinate_chance_mod,
//         arrest_chance_mod,
//         plot_power_mod,
//         murder_plot_power_mod,
//         defensive_plot_power_mod,
//         plot_discovery_chance,
//         // статы для того чтобы сделать ии немного разным
//         ai_rationality,
//         ai_zeal,
//         ai_greed,
//         ai_honor,
//         ai_ambition,
//         
//         // увеличение или уменьшение прибавки, супер мощный бафф/дебафф
//         authority_increase_mod,
//         esteem_increase_mod,
//         influence_increase_mod,
//         income_mod,
//         
//         // прибавка к авторитету, уважению и влиянию каждый ход (приходит в основном от треитов)
//         authority_increase,
//         esteem_increase,
//         influence_increase,
//         income, // инком может быть из других источников (должность), инком от налогов расчитывается во фракции
//         
//         // авторитет, уважение и влияние: расходники в эвентах и решениях
//         authority,
//         esteem,
//         influence,
//         money,
        
        // в зданиях можно указать тип объекта, стат которого мы увеличиваем (то есть character_modificators и проч)
        // и тогда не придется отделять здесь фракционные статы от статов персонажа по названиям
        // с другой стороны можно отделить и тогда не придется разделять character_modificators и faction_modificators
        // нет, почти для всех модификаторов лучше отделить, в цк2 названия модификаторов не пересекаются
        // статы персонажа и статы фракции можно совместить, другое дело что какие то модификаторы наследуются и статы фракции персонажам без нее вобщем то не нужны
        //CONCAT(character_, authority_increase_mod),
        
        // мнение о персонаже, думаю что нужно все мнения собрать в отдельную кучу
        
#define CHARACTER_STAT_FUNC(val) val
        
        CHARACTER_STATS_LIST
        
#undef CHARACTER_STAT_FUNC
        
        count,
        resources_start = authority, // все что не ресурс можно свободно использовать в треитах
      };
      
      const stat_type::values types[] = {
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::float_t, // health
        stat_type::float_t, // fertility
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::uint_t,
        
        // пенальти
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::float_t, // health
        stat_type::float_t, // fertility
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::uint_t,
        
        stat_type::uint_t,  // demesne_size
        stat_type::float_t,
        stat_type::float_t,
        
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t, // plot_discovery_chance
        
        stat_type::int_t,
        stat_type::int_t,
        stat_type::int_t,
        stat_type::int_t,
        stat_type::int_t,   // ai_ambition
        
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t, // income_mod
        
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t, // income
        
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t, // money
      };
      
      static_assert(sizeof(types)/sizeof(types[0]) == count);
    }
    
    // для этих значений свойства определены
    namespace opinion_stats {
      enum values {
        ambition_opinion,
        vassal_opinion,
        sex_appeal_opinion,
        same_opinion,
        same_opinion_if_same_religion,
        opposite_opinion,
        liege_opinion,
        general_opinion,
        twin_opinion,
        dynasty_opinion,
        male_dynasty_opinion,
        female_dynasty_opinion,
        child_opinion,
        oldest_child_opinion,
        youngest_child_opinion,
        spouse_opinion,
        male_opinion,
        female_opinion,
        same_religion_opinion,
        infidel_opinion,
        church_opinion,
        feudal_opinion,
        rel_head_opinion,
        free_invest_vassal_opinion,
        clan_sentiment,
        count,
        max_count = 128
        
//         temple_opinion,             // отношения к конкретным типам владений
//         temple_all_opinion,         // мне нужно будет добавить отдельно
//         city_opinion,               // так же как и отношения конкретных религий
//         castle_opinion,
//         tribal_opinion,
//         unreformed_tribal_opinion,
      };
    }
    
    // этими статами меняем приток денег от города например
    // хотя тут наверное не статы а модификаторы
    namespace realm_stats {
      enum values {
//         tax_income_mod,
//         trade_income_mod,
//         
//         provision_mod,
//         
//         army_recovery_mod, // модификатор для восстановления юнитов
//         army_morale_recovery_mod,
//         army_discipline,   // все армии из этого города стартуют с такой прибавкой дисциплины
//         army_maintenance_mod,
//         army_morale,
//         casualties_mod,    // прибавка к модификатору потерь
//         army_speed,
//         army_vision,
//         
//         hero_recovery_mod, // модификатор восстановления героя в принципе (например между противникамим в данже)
//         hero_discipline,   // модификатор дисциплины для всех героев под этими знаменами
// //         authority_increase_mod, // положим в персонажа
// //         esteem_increase_mod,
// //         influence_increase_mod,
// //         church_opinion,         // прибавка к отношению церкви
// //         vassal_opinion,         // прибавка к отношению вассалов
//         build_cost_mod,
//         build_time_mod,
//         population_growth_mod,
//         max_population_mod,     // может ли государство модифицировать максимальное количество жителей? например тех?
//         tech_spread_mod,
//         global_revolt_risk,     // риск восстания в государстве
//         revolt_risk_mod,
//         
//         vassal_limit,           //
        
        // глобальные характеристики для торговли
        
#define REALM_STAT_FUNC(val) val
        
        REALM_STATS_LIST
        
#undef REALM_STAT_FUNC
        
        count
      };
    }
    
    namespace province_stats {
      enum values {
//         tax_income_mod,
//         provision_mod,
//         
//         army_recovery_mod, // модификатор для восстановления юнитов
//         army_morale_recovery_mod,
//         army_discipline,   // все армии из этого города стартуют с такой прибавкой дисциплины
//         army_maintenance_mod,
//         army_morale,
//         casualties_mod,    // прибавка к модификатору потерь
//         army_speed,
//         army_vision,
//         
//         authority_increase_mod, // модификация для всех владельцев титулов в этой провинции
//         esteem_increase_mod,    // модификация для владельца или для господина?
//         influence_increase_mod,
//         authority_increase_liege_mod,
//         esteem_increase_liege_mod,    
//         influence_increase_liege_mod,
//         
//         local_revolt_risk,
//         disease_defence,
//         culture_flex,
//         religion_flex,
//         
//         short_reign_length, // нужно ли?
//         attrition, // наверное небоевые потери будем расчитывать по всей провинции
//         max_attrition,
//         
//         // торговля
        
#define PROVINCE_STAT_FUNC(val) val
        PROVINCE_STATS_LIST
#undef PROVINCE_STAT_FUNC
        
        count
      };
    }
    
    // доход, добавление ресурсов (например, авторитет и может быть какой то физический ресурс)
    // торговля: существуют торговые ноды, в них входят провинции, 
    // в провинциях города производят продукцию и торговое влияние, продукция умножается на цену товара,
    // это формирует доход нода, от торгового влияния зависит перераспределение денег в ноде,
    // также торговое влияние позволяет перенаправлять часть дохода (увеличивая его) в другой нод,
    // и так пока доход не будет собран в каком либо ноде
    namespace city_stats {
      enum values {
//         tax_income,
//         tax_income_mod,
//         
//         authority_increase, // владельцу
//         esteem_increase,
//         influence_increase,
//         authority_increase_liege, // владельцу и господину? наверное только господину
//         esteem_increase_liege,
//         influence_increase_liege, // нужны тут модификаторы?
// //         church_opinion, // мнение отдельно модифицируется
// //         vassal_opinion,
//         // бонусы для всех войск из этого города
//         build_cost_mod,
//         build_time_mod,
//         // население, нужно придумать хорошую механику централизации
//         population,
//         population_growth,
//         max_population,
//         tech_spread,
//         demesne_size, // размер личных владений
//         // торговля
//         trade_power, // в чью пользу идет профит от торговли в ноде
//         production,  // сколько производится продукции в городе (значение продукции умножается на цену товара) (это как раз дает непосредственно доход в год)
//         // в европке только два стата для торговли по сути
//         // остальное это куча различных модификаторов для этих статов в разных условиях
//         
//         // армейские статы
//         provision,         // сколько отрядов может прокормить армия
//         winter_provision,  // возможно нужно контролировать модификатором
//         provision_mod,
//         army_recovery_mod, // модификатор для восстановления юнитов
//         army_morale_recovery_mod,
//         army_discipline,   // все армии из этого города стартуют с такой прибавкой дисциплины
//         army_maintenance_mod,
//         army_morale,
//         casualties_mod,    // прибавка к модификатору потерь
//         army_speed,
//         army_vision,
//         
//         // геройские статы
// //         hero_speed, // непонятно как герою дать этот стат в городе, герой если "отдыхает" то скорее всего находится при дворе какого то правителя
// //         hero_vision,
// //         hero_recovery_mod, // глобальный модификатор
// //         hero_discipline,   // глобальный модификатор
//         
//         // можно еще добавить несколько аттрибутов с окончанием _penalty, для того чтобы моделировать ущерб от болезни (который не может быть вылечен больше чем заболел)
//         
//         // может быть нужно добавить температуру провинции? или какой то такой аттрибут чтобы контролировать провизию
        
#define CITY_STAT_FUNC(val) val
        
        CITY_STATS_LIST
        
#undef CITY_STAT_FUNC
        
        count
      };
    }
    
    // статы так или иначе влияющие на всю армию и на каждого юнита в армии
    // дальность хода - это скорость? скорее всего
    // что такое мод? это конкретное значение или множитель? где конкретное значение?
    namespace army_stats {
      enum values {
//         speed,           // скорость на карте
//         vision,          // видимость армии (не думаю что нужно делать больше 2-3 тайлов)
//         provision_mod,   // модификатор необходимой провизии армии (складывается из всех провизий отрядов)
//         maintenance_mod,
//         morale_mod,      // модификатор морали - наверное должен быть изменяемым по эвентам
//         morale_recovery, // восстановление морали после битвы, в процентах?
//         discipline,      // расхлябанность армии, на что влияет? некоторые статы должны от этого зависеть (умение атаки/защиты, урон по морали, инициатива)
//         recovery_mod,    // полководец может улучшить ситуацию с восстановлением, восстановление перекрывает небоевые потери
//         casualties_mod,  // полководец может чутка уменьшить урон от небоевых потерь
//         // 
        
#define ARMY_STAT_FUNC(val) val
        ARMY_STATS_LIST
#undef ARMY_STAT_FUNC
        
        count
      };
    }
    
    namespace hero_troop_stats {
      enum values {
//         speed,           // скорость на карте
//         vision,          // видимость геройской партии (не думаю что нужно делать больше 2-3 тайлов)
// //         maintenance_mod,
//         discipline,      // расхлябанность отряда героя, на что влияет? некоторые статы должны от этого зависеть (умение атаки/защиты, урон по морали, инициатива)
//         recovery_mod,    // восстановление геройской партии
        
#define HERO_TROOP_STAT_FUNC(val) val
        HERO_TROOP_STATS_LIST
#undef HERO_TROOP_STAT_FUNC
        
        count
      };
    }
    
    // оффсеты? они нужны чтобы правильно заполнить луа таблицы, а вот в самой игре они нужны?
    namespace offsets {
      enum values {
        character_stats = 0,
        faction_stats = character_stats + character_stats::count,
        province_stats = faction_stats + realm_stats::count,
        city_stats = province_stats + province_stats::count,
        army_stats = city_stats + city_stats::count,
        hero_troop_stats = army_stats + army_stats::count,
        troop_stats = hero_troop_stats + hero_troop_stats::count,
        hero_stats = troop_stats + troop_stats::count,
        
        count = hero_stats + hero_stats::count
      };
    }
    
    union stat_container {
      uint32_t uval;
      int32_t ival;
      float fval;
    };
    
    struct stat_data {
      enum flags {
        show_as_percent, // стат от нуля до единицы
        is_good,         // положительный стат рисуется зеленым
        is_monthly,      // ежемесячный
        is_hidden,       // скрытый от игрока стат
        show_value,      // используется для того чтобы не показывать значение для какой то переменной
        count
      };
      
      static constexpr const uint32_t bit_container_size = count / UINT32_WIDTH + 1;
      
      size_t name;
      size_t description;
      render::image_t img;
      utils::bit_field_32<bit_container_size> flag_bits;
      uint32_t max_decimals;
      // минимальные максимальные значения?
    };
    
    struct stat_data_container {
      stat_data troop_stats_data[troop_stats::count];
      stat_data hero_stats_data[hero_stats::count];
      stat_data character_stats_data[character_stats::count];
      stat_data opinion_stats_data[opinion_stats::max_count];
      stat_data faction_stats_data[realm_stats::count];
      stat_data province_stats_data[province_stats::count];
      stat_data city_stats_data[city_stats::count];
      stat_data army_stats_data[army_stats::count];
      stat_data hero_troop_stats_data[hero_troop_stats::count];
    };
    
    struct bonus {
      float raw_add;
      float raw_mul;
      float fin_add;
      float fin_mul;
      
      inline bonus() : raw_add(0.0f), raw_mul(0.0f), fin_add(0.0f), fin_mul(0.0f) {}
    };
    
    struct stat_bonus {
      unit_type type;
      uint32_t stat;
      struct bonus bonus;
      
      inline stat_bonus() : type(unit_type::invalid), stat(UINT32_MAX) {}
    };
    
    struct stat_modifier {
      unit_type type;
      uint32_t stat;
      stat_container mod;
      
      inline stat_modifier() : type(unit_type::invalid), stat(UINT32_MAX), mod{0} {}
    };
    
    struct unit_stat_modifier {
      const troop_type* type;
      uint32_t stat;
      stat_container mod;
      
      inline unit_stat_modifier() : type(nullptr), stat(UINT32_MAX), mod{0} {}
    };
    
    // что делать войсками? я хочу чтобы тип войска можно было бы определить
    // но при этом в городах нужно сделать здания которые дают тех или иных юнитов
    // то есть тип войска и здания должны быть заданы, у зданий должен быть аттрибут
    // с типом войск который он дает, нужно сначало создать войска а потом 
    // заполнить это поле, как хранить? хранить будем в типе здания,
    // в городе лишь обходим здания и собираем войска
  }
}

#endif
