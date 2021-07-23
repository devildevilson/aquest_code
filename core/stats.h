#ifndef STATS_H
#define STATS_H

#define CONCAT(a, b) a##b
#define PENALTY_STAT(stat) CONCAT(stat, _penalty)

// хп и количество людей в отряде это тоже самое?
// тотал вар говорит что нет, пока битву не сделаю не пойму

// возможно лучше переделать на одну макрофункцию все? зачем разные? существует undef который легко покрывает мои нужды

#define BASE_TROOP_STATS_LIST STAT_FUNC(troop_size) \
  STAT_FUNC(morale)               \
  STAT_FUNC(armor)                \
  STAT_FUNC(siege_armor)          \
  STAT_FUNC(magic_resistance)     \
  STAT_FUNC(fire_resistance)      \
  STAT_FUNC(charge)               \
  STAT_FUNC(morale_damage)        \
  STAT_FUNC(melee_damage)         \
  STAT_FUNC(melee_armor_piercing) \
  STAT_FUNC(melee_magic)          \
  STAT_FUNC(melee_fire)           \
  STAT_FUNC(melee_siege)          \
  STAT_FUNC(range_damage)         \
  STAT_FUNC(range_armor_piercing) \
  STAT_FUNC(range_magic)          \
  STAT_FUNC(range_fire)           \
  STAT_FUNC(range_siege)          \
  STAT_FUNC(accuracy)             \
  STAT_FUNC(reloading)            \
  STAT_FUNC(maintenance)          \
  STAT_FUNC(provision)            \
  STAT_FUNC(reinforce)
  
#define TROOP_FACTOR_STATS_LIST STAT_FUNC(maintenance_factor)          \
  STAT_FUNC(provision_factor)            \
  STAT_FUNC(reinforce_factor)
  
  // пенальти? как сделать проклятие или дебаффы?
  // это статы в геройской битве, там они не должны сильно меняться
  // хотя вообще то нет, пенальти нужно для для того чтобы посчитать резист
  // стат - (резист + пенальти)
#define BASE_HERO_STATS_LIST STAT_FUNC(min_damage)              \
  STAT_FUNC(max_damage)              \
  STAT_FUNC(health_per_damage_level) \
  STAT_FUNC(salary)                  \
  STAT_FUNC(recovery)               

#define SHARED_TROOP_HERO_STATS_LIST STAT_FUNC(max_hp) \
  STAT_FUNC(initiative)              \
  STAT_FUNC(melee_attack)            \
  STAT_FUNC(melee_defence)           \
  STAT_FUNC(range_attack)            \
  STAT_FUNC(range_defence)           \
  STAT_FUNC(ammo)                    
  
#define HERO_FACTOR_STATS_LIST STAT_FUNC(salary_factor)             \
  STAT_FUNC(recovery_factor)               

#define BASE_CHARACTER_STATS_LIST STAT_FUNC(military) \
  STAT_FUNC(managment)               \
  STAT_FUNC(diplomacy)               \
  STAT_FUNC(health)                  \
  STAT_FUNC(fertility)               \
  STAT_FUNC(strength)                \
  STAT_FUNC(agility)                 \
  STAT_FUNC(intellect)               \
  STAT_FUNC(demesne_size)            \
  STAT_FUNC(assassinate_chance_factor)  \
  STAT_FUNC(arrest_chance_factor)       \
  STAT_FUNC(plot_power_factor)          \
  STAT_FUNC(murder_plot_power_factor)   \
  STAT_FUNC(defensive_plot_power_factor)\
  STAT_FUNC(plot_discovery_chance)   \
  STAT_FUNC(ai_rationality)          \
  STAT_FUNC(ai_zeal)                 \
  STAT_FUNC(ai_greed)                \
  STAT_FUNC(ai_honor)                \
  STAT_FUNC(ai_ambition)             
  
#define CHARACTER_PENALTY_STATS_LIST CHARACTER_PENALTY_STAT_FUNC(military) \
  CHARACTER_PENALTY_STAT_FUNC(managment) \
  CHARACTER_PENALTY_STAT_FUNC(diplomacy) \
  CHARACTER_PENALTY_STAT_FUNC(health)    \
  CHARACTER_PENALTY_STAT_FUNC(fertility) \
  CHARACTER_PENALTY_STAT_FUNC(strength)  \
  CHARACTER_PENALTY_STAT_FUNC(agility)   \
  CHARACTER_PENALTY_STAT_FUNC(intellect)
  
#define RESOURCE_STATS_LIST STAT_FUNC(money) \
  STAT_FUNC(authority) \
  STAT_FUNC(esteem)    \
  STAT_FUNC(influence)
  
// у нас еще может быть прибавка с разных источников
// есть прибавка для сюзерена (прибавляет владельцу, его сюзерену и далее по иерархии)
// может ли быть у прибавки для сюзерена фактор? хороший вопрос, но скорее нет чем да
#define RESOURCE_INCOME_STATS_LIST STAT_FUNC(tax_income) \
  STAT_FUNC(trade_income)                \
  STAT_FUNC(authority_income)            \
  STAT_FUNC(esteem_income)               \
  STAT_FUNC(influence_income)            
  
#define LIEGE_RESOURCE_INCOME_STATS_LIST STAT_FUNC(liege_authority_income)      \
  STAT_FUNC(liege_esteem_income)         \
  STAT_FUNC(liege_influence_income) 
  
// город, феодал, церковь, нужно ли разделение на город/республику, скорее всего нет
#define RESOURCE_INCOME_FACTOR_STATS_LIST STAT_FUNC(money_income_factor)         \
  STAT_FUNC(tax_income_factor)           \
  STAT_FUNC(trade_income_factor)         \
  STAT_FUNC(authority_income_factor)     \
  STAT_FUNC(esteem_income_factor)        \
  STAT_FUNC(influence_income_factor)     
  
#define VASSAL_RESOURCE_INCOME_FACTOR_STATS_LIST STAT_FUNC(republic_tax_income_factor)  \
  STAT_FUNC(feodal_tax_income_factor)    \
  STAT_FUNC(theocracy_tax_income_factor)
  
//   STAT_FUNC(city_tax_income_factor)
//   STAT_FUNC(castle_tax_income_factor)
//   STAT_FUNC(abbey_tax_income_factor)
  
// модификаторы статов армии и популяции

// такая беда: у нас вполне могут быть ресурсы государства отдельные от персонажа
// даже не так, ресурсы нужно перенести в realm, а также инком,
// но у персонажа могут быть модификаторы инкома, также как и у realm'а
#define BASE_REALM_STATS_LIST STAT_FUNC(hero_recovery_factor)        \
  STAT_FUNC(hero_discipline)          \
  STAT_FUNC(max_population_factor)    \
  STAT_FUNC(global_revolt_risk)       \
  STAT_FUNC(vassal_limit)
  
#define BUILD_FACTOR_STATS_LIST STAT_FUNC(build_cost_factor)           \
  STAT_FUNC(build_time_factor)           

#define SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST STAT_FUNC(culture_flex)                 \
  STAT_FUNC(religion_flex)                \
  STAT_FUNC(short_reign_length)   
  
#define SHARED_PROVINCE_CITY_REALM_STATS_LIST   STAT_FUNC(tech_spread_factor)          \
  STAT_FUNC(population_growth_factor) \
  STAT_FUNC(revolt_risk_factor)          

#define BASE_PROVINCE_STATS_LIST STAT_FUNC(local_revolt_risk)            \
  STAT_FUNC(disease_defence)              \
  STAT_FUNC(attrition)                    \
  STAT_FUNC(attrition_factor)
  
#define BASE_CITY_STATS_LIST STAT_FUNC(population_growth)        \
  STAT_FUNC(max_population)              \
  STAT_FUNC(trade_power)                 \
  STAT_FUNC(production)                  
  
#define CITY_RESOURCE_STATS_LIST STAT_FUNC(population)

// дисциплина отсутствует в цк, в европке она немного влияла на почти все статы
// нужна ли она мне? вряд ли
// мораль кстати расчитывается как среднее среди морали юнитов, а может быть и нет (мораль командования)
#define BASE_ARMY_STATS_LIST STAT_FUNC(max_provision)      \
  STAT_FUNC(provision_decrease) \
  STAT_FUNC(army_maintenance)        \
  STAT_FUNC(max_morale)         \
  STAT_FUNC(morale_recovery)    \
  STAT_FUNC(casualties)
  
// с дисциплиной пока что ничего не понятно
//   STAT_FUNC(discipline)
  
#define SHARED_PROVINCE_CITY_STATS_LIST STAT_FUNC(provision_production) \
  STAT_FUNC(provision_production_factor) \
  STAT_FUNC(winter_provision_production) 

// эти вещи могут быть глобальными (могут быть у государства)
#define ARMY_FACTOR_STATS_LIST STAT_FUNC(provision_recovery_factor) \
  STAT_FUNC(morale_recovery_factor)    \
  STAT_FUNC(casualties_factor)         \
  STAT_FUNC(speed_factor)              \
  STAT_FUNC(vision_factor)             
  
#define ARMY_RESOURCE_STATS_LIST STAT_FUNC(provision) \
  STAT_FUNC(morale)

#define BASE_HERO_TROOP_STATS_LIST STAT_FUNC(discipline)
  
#define SHARED_HERO_TROOP_ARMY_STATS_LIST STAT_FUNC(speed) \
  STAT_FUNC(vision) 

#define CHARACTER_STATS_LIST CHARACTER_PENALTY_STATS_LIST \
  BASE_CHARACTER_STATS_LIST                 \
  SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST \
  RESOURCE_INCOME_STATS_LIST                \
  RESOURCE_INCOME_FACTOR_STATS_LIST         \
  BUILD_FACTOR_STATS_LIST                   \
  ARMY_FACTOR_STATS_LIST 
  
#define REALM_STATS_LIST BASE_REALM_STATS_LIST \
  RESOURCE_INCOME_STATS_LIST                \
  RESOURCE_INCOME_FACTOR_STATS_LIST         \
  VASSAL_RESOURCE_INCOME_FACTOR_STATS_LIST  \
  SHARED_PROVINCE_CITY_REALM_STATS_LIST     \
  BUILD_FACTOR_STATS_LIST                   \
  ARMY_FACTOR_STATS_LIST                    \
  HERO_FACTOR_STATS_LIST
  
// армии тут нет, но вот на отряд поди может сказаться
#define PROVINCE_STATS_LIST BASE_PROVINCE_STATS_LIST \
  SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST \
  SHARED_PROVINCE_CITY_REALM_STATS_LIST     \
  SHARED_PROVINCE_CITY_STATS_LIST           \
  RESOURCE_INCOME_STATS_LIST                \
  RESOURCE_INCOME_FACTOR_STATS_LIST         \
  BUILD_FACTOR_STATS_LIST                   \
  TROOP_FACTOR_STATS_LIST
  
#define CITY_STATS_LIST BASE_CITY_STATS_LIST \
  SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST \
  SHARED_PROVINCE_CITY_REALM_STATS_LIST     \
  SHARED_PROVINCE_CITY_STATS_LIST           \
  RESOURCE_INCOME_STATS_LIST                \
  RESOURCE_INCOME_FACTOR_STATS_LIST         \
  LIEGE_RESOURCE_INCOME_STATS_LIST          \
  BUILD_FACTOR_STATS_LIST                   \
  TROOP_FACTOR_STATS_LIST
  
#define ARMY_STATS_LIST BASE_ARMY_STATS_LIST \
  SHARED_HERO_TROOP_ARMY_STATS_LIST \
  ARMY_FACTOR_STATS_LIST  \
  TROOP_FACTOR_STATS_LIST
  
#define TROOP_STATS_LIST BASE_TROOP_STATS_LIST \
  TROOP_FACTOR_STATS_LIST \
  SHARED_HERO_TROOP_ARMY_STATS_LIST \
  SHARED_TROOP_HERO_STATS_LIST
  
#define HERO_TROOP_STATS_LIST BASE_HERO_TROOP_STATS_LIST \
  SHARED_HERO_TROOP_ARMY_STATS_LIST \
  HERO_FACTOR_STATS_LIST

#define HERO_STATS_LIST BASE_HERO_STATS_LIST \
  HERO_FACTOR_STATS_LIST \
  SHARED_HERO_TROOP_ARMY_STATS_LIST \
  SHARED_TROOP_HERO_STATS_LIST
  
#define UNIQUE_STATS_LIST CHARACTER_PENALTY_STATS_LIST \
  BASE_CHARACTER_STATS_LIST                 \
  BASE_REALM_STATS_LIST                     \
  BASE_PROVINCE_STATS_LIST                  \
  BASE_CITY_STATS_LIST                      \
  BASE_ARMY_STATS_LIST                      \
  BASE_TROOP_STATS_LIST                     \
  BASE_HERO_STATS_LIST                      \
  BASE_HERO_TROOP_STATS_LIST                \
  HERO_FACTOR_STATS_LIST                    \
  TROOP_FACTOR_STATS_LIST                   \
  SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST \
  SHARED_PROVINCE_CITY_REALM_STATS_LIST     \
  SHARED_PROVINCE_CITY_STATS_LIST           \
  SHARED_HERO_TROOP_ARMY_STATS_LIST         \
  SHARED_TROOP_HERO_STATS_LIST              \
  RESOURCE_INCOME_STATS_LIST                \
  RESOURCE_INCOME_FACTOR_STATS_LIST         \
  LIEGE_RESOURCE_INCOME_STATS_LIST          \
  VASSAL_RESOURCE_INCOME_FACTOR_STATS_LIST  \
  BUILD_FACTOR_STATS_LIST                   \
  ARMY_FACTOR_STATS_LIST                    \
  RESOURCE_STATS_LIST
  
  // тут немного сложнее
// BASE_HERO_TROOP_STATS_LIST
// BASE_HERO_STATS_LIST
// HERO_FACTOR_STATS_LIST
  
// ко всем статам еще нужны иконки, названия, описания
// по всей видимости должны быть четкие разделения на статы фракции -> персонажа -> провинции -> города -> здания
namespace devils_engine {
  namespace core {
    struct troop_type;
    
    enum class unit_type {
      invalid,
      troop,
      hero,
      hero_troop,
      character,
      realm,
      city,
      province
    };
    
    namespace stat_type {
      enum values {
        int_t,
        uint_t,
        float_t,
        count
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
        
#define STAT_FUNC(val) val,
        
        TROOP_STATS_LIST
        
#undef STAT_FUNC
        
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
    // наверное да только за какой то ресурс (например так может уменьшаться влияние) кому даются какие статы?
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
        
#define STAT_FUNC(val) val,
        
        HERO_STATS_LIST
        
#undef STAT_FUNC
        
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
//         PENALTY_STAT(military)
//         PENALTY_STAT(managment)
//         PENALTY_STAT(diplomacy)
//         PENALTY_STAT(health)
//         PENALTY_STAT(fertility)
//         PENALTY_STAT(strength)
//         PENALTY_STAT(agility)
//         PENALTY_STAT(intellect)
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
//         income, // инком может быть из других источников (должность) инком от налогов расчитывается во фракции
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
        //CONCAT(character_, authority_increase_mod)
        
        // мнение о персонаже, думаю что нужно все мнения собрать в отдельную кучу

#define STAT_FUNC(val) val,
#define CHARACTER_PENALTY_STAT_FUNC(val) val##_penalty,
        
        CHARACTER_STATS_LIST
        
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
        
        count
      };
      
      const stat_type::values types[] = {
        // пенальти
        stat_type::int_t,
        stat_type::int_t,
        stat_type::int_t,
        stat_type::float_t, // health
        stat_type::float_t, // fertility
        stat_type::int_t,
        stat_type::int_t,
        stat_type::int_t,
        
        // базовые статы
        stat_type::int_t,
        stat_type::int_t,
        stat_type::int_t,
        stat_type::float_t, // health
        stat_type::float_t, // fertility
        stat_type::int_t,
        stat_type::int_t,
        stat_type::int_t,
        
        stat_type::int_t,  // demesne_size
        
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
        
        // провинция/город/персонаж статы
        stat_type::float_t,
        stat_type::float_t,
        stat_type::int_t,
        
        // доход
        stat_type::float_t, // tax
        stat_type::float_t, // trade
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        
        // множитель дохода
        stat_type::float_t, // money (общий множитель)
        stat_type::float_t, // tax
        stat_type::float_t, // trade
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        
        // множитель строительства
        stat_type::float_t,
        stat_type::float_t,
        
        // множители для армии
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        stat_type::float_t,
        // забыл че удалил в армии =(
      };
      
      static_assert(sizeof(types)/sizeof(types[0]) == count);
    }
    
    namespace character_resources {
      enum values {
#define STAT_FUNC(val) val,
        RESOURCE_STATS_LIST
#undef STAT_FUNC
        count
      };
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
        
#define STAT_FUNC(val) val,
        
        REALM_STATS_LIST
        
#undef STAT_FUNC
        
        count
      };
    }
    
    namespace realm_resources {
      enum values {
#define STAT_FUNC(val) val,
        RESOURCE_STATS_LIST
#undef STAT_FUNC
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
        
#define STAT_FUNC(val) val,
        PROVINCE_STATS_LIST
#undef STAT_FUNC
        
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
        
#define STAT_FUNC(val) val,
        
        CITY_STATS_LIST
        
#undef STAT_FUNC
        
        count
      };
    }
    
    namespace city_resources {
      enum values {
#define STAT_FUNC(val) val,
        CITY_RESOURCE_STATS_LIST
#undef STAT_FUNC
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
        
#define STAT_FUNC(val) val,
        ARMY_STATS_LIST
#undef STAT_FUNC
        
        count
      };
    }
    
    namespace army_resources {
      enum values {
#define STAT_FUNC(val) val,
        ARMY_RESOURCE_STATS_LIST
#undef STAT_FUNC
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
        
#define STAT_FUNC(val) val,
        HERO_TROOP_STATS_LIST
#undef STAT_FUNC
        
        count
      };
    }
    
    // оффсеты? они нужны чтобы правильно заполнить луа таблицы, а вот в самой игре они нужны?
    namespace offsets {
      enum values {
        character_stats = 0,
        realm_stats = character_stats + character_stats::count,
        province_stats = realm_stats + realm_stats::count,
        city_stats = province_stats + province_stats::count,
        army_stats = city_stats + city_stats::count,
        hero_troop_stats = army_stats + army_stats::count,
        troop_stats = hero_troop_stats + hero_troop_stats::count,
        hero_stats = troop_stats + troop_stats::count,
        
        count = hero_stats + hero_stats::count
      };
    }
    
    // что делать войсками? я хочу чтобы тип войска можно было бы определить
    // но при этом в городах нужно сделать здания которые дают тех или иных юнитов
    // то есть тип войска и здания должны быть заданы, у зданий должен быть аттрибут
    // с типом войск который он дает, нужно сначало создать войска а потом 
    // заполнить это поле, как хранить? хранить будем в типе здания,
    // в городе лишь обходим здания и собираем войска
  }
}

#endif
