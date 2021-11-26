#ifndef DEVILS_ENGINE_CORE_STATS_H
#define DEVILS_ENGINE_CORE_STATS_H

#define CONCAT(a, b) a##b
#define PENALTY_STAT(stat) CONCAT(stat, _penalty)

// хп и количество людей в отряде это тоже самое?
// тотал вар говорит что нет, пока битву не сделаю не пойму

// возможно лучше переделать на одну макрофункцию все? зачем разные? существует undef который легко покрывает мои нужды

#define BASE_TROOP_STATS_LIST STAT_FUNC(troop_size) /* текущий размер? */ \
  STAT_FUNC(morale)               \
  STAT_FUNC(armor)                \
  STAT_FUNC(siege_armor)          \
  STAT_FUNC(magic_resistance)     \
  STAT_FUNC(fire_resistance)      \
  STAT_FUNC(charge)               \
  /* нужно наверное добавить еще какой тип (что то вроде темной магии) для которой будет очень мало резистов */ \
  /* этим может быть и fire damage (или уменьшать его от резиста к магии? огненные стрелы это не магия) */ \
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
  STAT_FUNC(consumption)          \
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
  STAT_FUNC(health_per_damage_level) /* с уменьшением здоровья уменьшается и урон */ \
  STAT_FUNC(salary) /* денюшка за ход (?) */ \
  STAT_FUNC(recovery) /* восстановление хп, каждый ход восстанавливаемся почуть чуть в зависимости от снабжения? */              

#define SHARED_TROOP_HERO_STATS_LIST STAT_FUNC(max_hp) /* текущее хп? */ \
  STAT_FUNC(initiative) /* для того чтобы это хорошо работало нужна шкала с инициативой как в пятых героях */             \
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
  /* могут улучшиться по эвентам, невидимы игроку */ \
  STAT_FUNC(assassinate_chance_factor)  \
  STAT_FUNC(arrest_chance_factor)       \
  STAT_FUNC(plot_power_factor)          \
  STAT_FUNC(murder_plot_power_factor)   \
  STAT_FUNC(defensive_plot_power_factor)\
  STAT_FUNC(plot_discovery_chance)   \
  /* статы для того чтобы сделать ии немного разным */ \
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
  
// авторитет, уважение и влияние: расходники в эвентах и решениях
#define RESOURCE_STATS_LIST STAT_FUNC(money) \
  STAT_FUNC(authority) \
  STAT_FUNC(esteem)    \
  STAT_FUNC(influence)
  
// у нас еще может быть прибавка с разных источников
// есть прибавка для сюзерена (прибавляет владельцу, его сюзерену и далее по иерархии)
// может ли быть у прибавки для сюзерена фактор? хороший вопрос, но скорее нет чем да
// прибавка к авторитету, уважению и влиянию каждый ход (приходит в основном от треитов)
#define RESOURCE_INCOME_STATS_LIST STAT_FUNC(tax_income) \
  STAT_FUNC(trade_income)                \
  STAT_FUNC(authority_income)            \
  STAT_FUNC(esteem_income)               \
  STAT_FUNC(influence_income)            
  
#define LIEGE_RESOURCE_INCOME_STATS_LIST STAT_FUNC(liege_authority_income)      \
  STAT_FUNC(liege_esteem_income)         \
  STAT_FUNC(liege_influence_income) 
  
// город, феодал, церковь, нужно ли разделение на город/республику, скорее всего нет
// увеличение или уменьшение прибавки, супер мощный бафф/дебафф
#define RESOURCE_INCOME_FACTOR_STATS_LIST STAT_FUNC(money_income_factor)         \
  STAT_FUNC(tax_income_factor)           \
  STAT_FUNC(trade_income_factor)         \
  STAT_FUNC(authority_income_factor) /* модификация для всех владельцев титулов в этой провинции */    \
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
  STAT_FUNC(max_population_factor) /* может ли государство модифицировать максимальное количество жителей? например тех? */   \
  STAT_FUNC(global_revolt_risk) /* риск восстания в государстве */      \
  STAT_FUNC(vassal_limit)
  
#define BUILD_FACTOR_STATS_LIST STAT_FUNC(build_cost_factor)           \
  STAT_FUNC(build_time_factor)           

#define SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST STAT_FUNC(culture_flex) /* толерантность к другим культурам или скорость принятия, это я так полагаю для ии */ \
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
// модификатор для восстановления юнитов ???
#define ARMY_FACTOR_STATS_LIST STAT_FUNC(provision_recovery_factor) \
  STAT_FUNC(morale_recovery_factor) /* полководец может улучшить ситуацию с восстановлением, восстановление перекрывает небоевые потери */   \
  STAT_FUNC(casualties_factor) /* прибавка к модификатору потерь */    \
  STAT_FUNC(speed_factor)              \
  STAT_FUNC(vision_factor)             
  
// модификатор необходимой провизии армии (складывается из всех провизий отрядов)
// передвижение тоже должно быть ресурсом
#define ARMY_RESOURCE_STATS_LIST STAT_FUNC(provision) /* некая переменная которая должна обозначать богатсво провинции и возможность кормить конкретную армию */ \
  
  //STAT_FUNC(morale) /* мораль как ресурс это что? такая штука явно присутствовала в цк как мне повторить? */

#define BASE_HERO_TROOP_STATS_LIST STAT_FUNC(discipline)
  
#define SHARED_HERO_TROOP_ARMY_STATS_LIST STAT_FUNC(speed) /* скорость на карте */ \
  STAT_FUNC(vision) /* видимость армии (не думаю что нужно делать больше 2-3 тайлов) */

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
  
#define UNIQUE_RESOURCES_LIST               \
  RESOURCE_STATS_LIST                       \
  CITY_RESOURCE_STATS_LIST                  \
  ARMY_RESOURCE_STATS_LIST                  \
  
#define UNIQUE_STATS_RESOURCES_LIST \
  UNIQUE_STATS_LIST     \
  UNIQUE_RESOURCES_LIST \
  
#define STATS_OFFSET_LIST \
  STAT_OFFSET_FUNC(character_stats) \
  STAT_OFFSET_FUNC(realm_stats) \
  STAT_OFFSET_FUNC(province_stats) \
  STAT_OFFSET_FUNC(city_stats) \
  STAT_OFFSET_FUNC(army_stats) \
  STAT_OFFSET_FUNC(hero_troop_stats) \
  STAT_OFFSET_FUNC(troop_stats) \
  STAT_OFFSET_FUNC(hero_stats) \
  STAT_OFFSET_FUNC(character_resources) \
  STAT_OFFSET_FUNC(realm_resources) \
  STAT_OFFSET_FUNC(city_resources) \
  STAT_OFFSET_FUNC(army_resources)
  
#define STAT_TYPES_LIST \
  STAT_TYPES_FUNC(invalid) \
  STAT_TYPES_FUNC(character_stat) \
  STAT_TYPES_FUNC(realm_stat) \
  STAT_TYPES_FUNC(province_stat) \
  STAT_TYPES_FUNC(city_stat) \
  STAT_TYPES_FUNC(army_stat) \
  STAT_TYPES_FUNC(hero_troop_stat) \
  STAT_TYPES_FUNC(troop_stat) \
  STAT_TYPES_FUNC(hero_stat) \
  STAT_TYPES_FUNC(character_resource) \
  STAT_TYPES_FUNC(realm_resource) \
  STAT_TYPES_FUNC(city_resource) \
  STAT_TYPES_FUNC(army_resource)
  
  
#define OPINION_MODIFIER_TYPES_LIST \
  OPINION_MODIFIER_TYPE_FUNC(invalid) \
  OPINION_MODIFIER_TYPE_FUNC(general_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(vassal_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(religious_vassal_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(powerful_vassal_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(direct_vassal_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(fellow_vassal_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(free_invest_vassal_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(liege_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(same_religion_liege_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(same_parent_religion_liege_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(same_religion_group_liege_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(different_religion_liege_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(different_parent_religion_liege_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(different_religion_group_liege_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(independent_ruler_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(opinion_of_male_rulers) \
  OPINION_MODIFIER_TYPE_FUNC(opinion_of_female_rulers) \
  OPINION_MODIFIER_TYPE_FUNC(council_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(tribunal_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(clergy_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(courtier_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(guest_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(courtier_and_guest_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(prisoner_opinion) \
  \
  OPINION_MODIFIER_TYPE_FUNC(twin_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(dynasty_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(male_dynasty_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(female_dynasty_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(child_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(oldest_child_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(youngest_child_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(spouse_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(close_relative_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(player_heir_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(child_except_player_heir_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(eligible_child_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(eligible_child_except_player_heir_opinion) \
  \
  OPINION_MODIFIER_TYPE_FUNC(male_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(female_opinion) \
  \
  OPINION_MODIFIER_TYPE_FUNC(ambition_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(sex_appeal_opinion) \
  \
  OPINION_MODIFIER_TYPE_FUNC(same_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(same_opinion_if_same_religion) \
  OPINION_MODIFIER_TYPE_FUNC(opposite_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(same_culture_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(same_culture_opinion_if_same_religion) \
  \
  OPINION_MODIFIER_TYPE_FUNC(same_religion_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(same_parent_religion_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(same_religion_group_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(different_religion_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(different_parent_religion_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(different_religion_group_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(infidel_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(church_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(rel_head_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(realm_priest_opinion) \
  \
  /* дальше идут бонусы к конкретным вещам или типам */ \
  OPINION_MODIFIER_TYPE_FUNC(character_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(realm_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(culture_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(culture_group_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(religion_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(religion_group_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(city_type_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(holding_type_opinion) \
  OPINION_MODIFIER_TYPE_FUNC(other_dynasty_opinion)

  
// ко всем статам еще нужны иконки, названия, описания
// по всей видимости должны быть четкие разделения на статы фракции -> персонажа -> провинции -> города -> здания
namespace devils_engine {
  namespace core {
    struct troop_type;
    
    namespace stat_type {
      enum values {
#define STAT_TYPES_FUNC(val) val,
        STAT_TYPES_LIST
#undef STAT_TYPES_FUNC
        
        count
      };
    }
    
    namespace stat_value_type {
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
    // вряд ли нужно давать героям возможность сбежать
    // вместо этого нужно сделать удачу и мораль
    namespace hero_stats {
      enum values {
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
    // интрига? не помешало бы что то подобное, 
    // хотя с этим частично могут справиться и другие статы
    namespace character_stats {
      enum values {
#define STAT_FUNC(val) val,
#define CHARACTER_PENALTY_STAT_FUNC(val) val##_penalty,
        CHARACTER_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
        
        count
      };
      
      const stat_value_type::values types[] = {
        // пенальти
        stat_value_type::int_t,
        stat_value_type::int_t,
        stat_value_type::int_t,
        stat_value_type::float_t, // health
        stat_value_type::float_t, // fertility
        stat_value_type::int_t,
        stat_value_type::int_t,
        stat_value_type::int_t,
        
        // базовые статы
        stat_value_type::int_t,
        stat_value_type::int_t,
        stat_value_type::int_t,
        stat_value_type::float_t, // health
        stat_value_type::float_t, // fertility
        stat_value_type::int_t,
        stat_value_type::int_t,
        stat_value_type::int_t,
        
        stat_value_type::int_t,  // demesne_size
        
        stat_value_type::float_t,
        stat_value_type::float_t,
        stat_value_type::float_t,
        stat_value_type::float_t,
        stat_value_type::float_t,
        stat_value_type::float_t, // plot_discovery_chance
        
        stat_value_type::int_t,
        stat_value_type::int_t,
        stat_value_type::int_t,
        stat_value_type::int_t,
        stat_value_type::int_t,   // ai_ambition
        
        // провинция/город/персонаж статы
        stat_value_type::float_t,
        stat_value_type::float_t,
        stat_value_type::int_t,
        
        // доход
        stat_value_type::float_t, // tax
        stat_value_type::float_t, // trade
        stat_value_type::float_t,
        stat_value_type::float_t,
        stat_value_type::float_t,
        
        // множитель дохода
        stat_value_type::float_t, // money (общий множитель)
        stat_value_type::float_t, // tax
        stat_value_type::float_t, // trade
        stat_value_type::float_t,
        stat_value_type::float_t,
        stat_value_type::float_t,
        
        // множитель строительства
        stat_value_type::float_t,
        stat_value_type::float_t,
        
        // множители для армии
        stat_value_type::float_t,
        stat_value_type::float_t,
        stat_value_type::float_t,
        stat_value_type::float_t,
        stat_value_type::float_t,
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
    
    // этими статами меняем приток денег от города например
    // хотя тут наверное не статы а модификаторы
    namespace realm_stats {
      enum values {
        // нужно добавить еще глобальные характеристики для торговли
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
        // может быть нужно добавить температуру провинции? или какой то такой аттрибут чтобы контролировать провизию
        
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
#define STAT_FUNC(val) val,
        HERO_TROOP_STATS_LIST
#undef STAT_FUNC
        
        count
      };
    }
    
    // для этих значений свойства определены
    // нужно вот что сделать, задать максимальное количество разных типов
    // в цк3 опинион модификатор действительно отдельно, в скрипте можно указать
    // от кого к кому 
    // сколько всего типов владений? какие формы правления?
    // формы правления напрямую у меня нет, но есть разные права у акторов в государстве
    // думаю что отношения должны быть скорее с конкретными акторами и их целями
    // то есть должны быть некие объединения групп акторов по каким то целям
    // и вот по этим целям раздавать модификаторы к отношениям
    namespace opinion_modifiers {
      enum values {
#define OPINION_MODIFIER_TYPE_FUNC(val) val,
        OPINION_MODIFIER_TYPES_LIST
#undef OPINION_MODIFIER_TYPE_FUNC
        
        count,
//         max_count = 128
        opinion_modifier_type = general_opinion,
        entity_type_opinion_modifier = character_opinion
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
        character_resources = hero_stats + hero_stats::count,
        realm_resources = character_resources + character_resources::count,
        city_resources = realm_resources + realm_resources::count,
        army_resources = city_resources + city_resources::count,
        
        count = army_resources + army_resources::count
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
