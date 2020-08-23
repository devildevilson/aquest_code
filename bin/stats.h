#ifndef STATS_H
#define STATS_H

#include <cstdint>
#include "render/shared_structures.h"
#include <magic_enum.hpp>
#include "utils/bit_field.h"

// ко всем статам еще нужны иконки, названия, описания
namespace devils_engine {
  namespace core {
    enum class unit_type {
      troop,
      hero,
      character,
      faction,
      city,
      province,
      invalid
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
//         current_troop_size, // это особые статы: они не будут в обычном смысле появляться в карточке персонажа
//         current_hp,
//         current_morale,
        troop_size, // текущий размер?
        max_hp,     // текущее хп?
        morale,
        armor,
        siege_armor, // защита против урона по зданиям
        speed,
        initiative,
        melee_attack,
        melee_defence,
        range_attack,
        range_defence,
        charge,
        // нужно наверное добавить еще какой тип (что то вроде темной магии) для которой будет очень мало резистов
        // этим может быть и fire damage (или уменьшать его от резиста к магии? огненные стрелы это не магия)
        morale_damage,
        melee_damage, 
        melee_armor_piercing,
        melee_magic,
        melee_fire,
        melee_siege,
        range_damage,
        range_armor_piercing,
        range_magic,
        range_fire,
        range_siege,
        accuracy,
        reloading,
        ammo,
        // небоевые статы
        maintenance, // денюшка за ход (?)
        provision,   // некая переменная которая должна обозначать богатсво провинции и возможность кормить конкретную армию
        recovery,    // восстановление хп, каждый ход восстанавливаемся почуть чуть в зависимости от снабжения?
        
        count
      };
    }
    
    // здесь по идее все инты
    // в баннер саге используется такой стат как виллповер, позволяет делать какие то экстра вещи (дальше бегать например)
    // не думаю что мне нужен виллповер
    // у юнитов должны быть еще какие то пассивки (например всегда отвечать, атака на все тайлы вокруг и проч)
    // или активки
    namespace hero_stats {
      enum values {
//         current_hp,
        max_hp, // текущее хп?
        // вряд ли нужно давать героям возможность сбежать
        // вместо этого нужно сделать удачу и мораль
        //leadership,
        speed,
        initiative,  // для того чтобы это хорошо работало нужна шкала с инициативой как в пятых героях
        melee_attack,
        melee_defence,
        range_attack,
        range_defence,
        min_damage, // доп урон?
        max_damage,
        ammo,
        health_per_damage_level, // с уменьшением здоровья уменьшается и урон
        // небоевые статы
        recovery,
        // зарплата?
        
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
    namespace character_stats {
      enum values {
        // статы правителя
        military, // пенальти для каждого скила?
        managment,
        diplomacy,
        // интрига? не помешало бы что то подобное, 
        // хотя с этим частично могут справиться и другие статы
        health,
        fertility,
        // авторитет, уважение и влияние: расходники в эвентах и решениях
        authority,
        esteem,
        influence,
        // прибавка к авторитету, уважению и влиянию каждый ход
        authority_increase_mod,
        esteem_increase_mod,
        influence_increase_mod,
        // геройские статы
        strength,
        agility,
        intellect,
        
        demesne_size,
        culture_flex,  // толерантность к другим культурам, это я так полагаю для ии
        religion_flex,
        // могут улучшиться по эвентам, невидимы игроку
        assassinate_chance_mod,
        arrest_chance_mod,
        plot_power_mod,
        murder_plot_power_mod,
        defensive_plot_power_mod,
        plot_discovery_chance,
        // статы для того чтобы сделать ии немного разным
        ai_rationality,
        ai_zeal,
        ai_greed,
        ai_honor,
        ai_ambition,
        
        // мнение о персонаже, думаю что нужно все мнения собрать в отдельную кучу
        
        count
      };
      
      const stat_type::values types[] = {
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::uint_t,
        stat_type::float_t,
        stat_type::float_t,
        
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        
        stat_type::int_t,
        stat_type::int_t,
        stat_type::int_t,
        stat_type::int_t,
        stat_type::int_t,
      };
      
      static_assert(sizeof(types)/sizeof(stat_type::values) == count-1);
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
    namespace faction_stats {
      enum values {
        tax_income_mod,
        provision_mod,
        army_recovery_mod,
        hero_recovery_mod,
        morale_recovery_mod,
        authority_increase_mod,
        esteem_increase_mod,
        influence_increase_mod,
//         church_opinion,         // прибавка к отношению церкви
//         vassal_opinion,         // прибавка к отношению вассалов
        build_cost_mod,
        build_time_mod,
        population_growth_mod,
        max_population_mod,     // может ли государство модифицировать максимальное количество жителей?
        tech_spread_mod,
        global_revolt_risk,     // риск восстания в государстве
        revolt_risk_mod,
        
        // глобальные характеристики для торговли
        
        count
      };
    }
    
    namespace province_stats {
      enum values {
        tax_income_mod,
        provision_mod,
        army_recovery_mod,
        hero_recovery_mod,
        morale_recovery_mod,
        authority_increase_mod, // модификация для всех владельцев титулов в этой провинции
        esteem_increase_mod,
        influence_increase_mod,
        local_revolt_risk,
        disease_defence,
        culture_flex,
        religion_flex,
        short_reign_length, // нужно ли?
        attrition, // наверное небоевые потери будем расчитывать по всей провинции
        max_attrition,
        
        // торговля
        
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
        tax_income,
        tax_income_mod,
        provision,
        winter_provision,
        provision_mod,
        army_recovery_mod,
        hero_recovery_mod,
        morale_recovery_mod,
        authority_increase, // владельцу
        esteem_increase,
        influence_increase,
        authority_increase_liege, // владельцу и господину? наверное только господину
        esteem_increase_liege,
        influence_increase_liege,
        church_opinion,
        vassal_opinion,
        // бонусы для всех войск из этого города
        build_cost_mod,
        build_time_mod,
        // население, нужно придумать хорошую механику централизации
        population,
        population_growth,
        max_population,
        tech_spread,
        demesne_size, // размер личных владений
        // торговля
        trade_power, // в чью пользу идет профит от торговли в ноде
        production,  // сколько производится продукции в городе (значение продукции умножается на цену товара) (это как раз дает непосредственно доход в год)
        // в европке только два стата для торговли по сути
        // остальное это куча различных модификаторов для этих статов в разных условиях
        
        // можно еще добавить несколько аттрибутов с окончанием _penalty, для того чтобы моделировать ущерб от болезни (который не может быть вылечен больше чем заболел)
        
        count
      };
    }
    
    // статы так или иначе влияющие на всю армию и на каждого юнита в армии
    namespace army_stats {
      enum values {
        speed,           // скорость на карте
        vision,          // видимость армии (не думаю что нужно делать больше 2-3 тайлов)
        provision_mod,   // модификатор необходимой провизии армии (складывается из всех провизий отрядов)
        maintenance_mod,
        morale_mod,      // модификатор морали - наверное должен быть изменяемым по эвентам
        morale_recovery, // восстановление морали после битвы, в процентах?
        discipline,      // расхлябанность армии, на что влияет? некоторые статы должны от этого зависеть (умение атаки/защиты, урон по морали, инициатива)
        recovery_mod,    // полководец может улучшить ситуацию с восстановлением, восстановление перекрывает небоевые потери
        casualties_mod,  // полководец может чутка уменьшить урон от небоевых потерь
        // 
        
        count
      };
    }
    
    namespace hero_troop_stats {
      enum values {
        speed,           // скорость на карте
        vision,          // видимость геройской партии (не думаю что нужно делать больше 2-3 тайлов)
//         maintenance_mod,
        discipline,      // расхлябанность отряда героя, на что влияет? некоторые статы должны от этого зависеть (умение атаки/защиты, урон по морали, инициатива)
        recovery_mod,    // восстановление геройской партии
        
        count
      };
    }
    
    union stat_container {
      uint32_t uval;
      int32_t ival;
      float fval;
    };
    
    struct stat_data {
      enum flags {
        show_as_percent,
        is_good,
        is_monthly,
        is_hidden,
        show_value,
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
      stat_data faction_stats_data[faction_stats::count];
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
