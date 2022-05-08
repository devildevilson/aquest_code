#ifndef DEVILS_ENGINE_CORE_HERO_TYPE_H
#define DEVILS_ENGINE_CORE_HERO_TYPE_H

#include <string>
#include "stats.h"
#include "utils/structures_utils.h"
#include "character.h"

// специализация и вторичный скилл могут быть не просто наращиванием какой то характеристики (хотя это полезные скиллы)
// а могут быть еще некой механикой: например улучшение пирушки (лучше проводим, больше эвентов, дешевле, больше остаточный бонус)
// надо видимо расписать доконца какие бонусы я хочу и посмотреть что можно сделать
// придется сделать какой то список механик, хот я думаю что надо сначала придумать пару-тройку скиллов и спеков
// так будет очевиднее что нужно делать

namespace devils_engine {
  namespace core {
    struct complex_bonus {
      core::character_stats::values ref_stat; // текущий или база? скорее всего база
      core::character_stats::values end_stat;
      float start_value;
      float limit_value;
      float add_value;
      float mul_value;
      float initial_add;
      float initial_mul;
      
//       inline void compute(character* c) {
//         const float ref = c->stats.get(ref_stat);
//         const float val = initial_add + initial_mul * (std::min(std::max(ref - start_value, 0.0f) * mul_value, limit_value) + add_value);
//         c->current_stats.add(end_stat, val);
//       }
    };
    
    struct secondary_skill {
      std::string id;
      std::string name_id;
      std::string description_id;
      
      // в документах
      // вообще конечно возможно имеет смысл рассмотреть вариант со скриптом
      // что мне вообще может там потребоваться? парочка функций с формулой?
      // мин/макс, сравнение, вообще наверное полная иерархия операций будет полезна
    };
    
    struct specialization {
      std::string id;
      std::string name_id;
      std::string description_id;
      
      // что тут? здесь должна быть механика наращивания какой то характеристики от уровня
      // будет ли у меня уровень? не уверен, наращивание тогда можно сделать от характеристик
      // к сожалению определенное количество специализаций влияют на заклинания/скиллы только в битве
      // возможно мне потребуется что то нестандартное, наращивание также может быть по двум характеристикам
      // может быть как маскимум двух наращиваний
      
      float start_value;
      float limit_value;
      float add_value;
      float mul_value;
      float initial_add;
      float initial_mul;
      
      // эта формула практически покрывает все кейсы наращивания характеристик
      // val = initial_add + initial_mul * (min(max(stat - start_value, 0) * mul_value, limit_value) + add_value)
      // либо использовать все же скрипт, что определяет скрипт?
    };
    
    // должно ли в типе быть например тип оружия которым мы пользуемся? вряд ли, хотя если типов не очень много то может еще ничего
    // в типе наверняка используется графика, а значит тип определяет визуально героя, а это в свою очередь влияет на анимацию
    // и ожидания от навыков, было бы неплохо чтобы условный паладин получал только какие то более паладинские навыки 
    // а условный варвар - более варварские навыки + у нас визуально тип должен зависеть от развития государства
    // тип героя используется при проверках выдачи спеков и вторичек, имеет смысл здесь задать какую то таблицу шансов
    // которую я использую в скриптах для понимания кому что выдать, как к ней обращаться и как она должна выглядеть?
    // мапа id:number? возможно нужно подойти иначе, держать таблицу в луа и подгружать ее при загрузке эвентов
    // мы можем таблицу держать в конфигах создания типа героя и оттуда подгружать в скрипт - возможно наилучшее решение
    // 
    struct hero_type {
      static const size_t max_stat_modifiers_count = 16;
      static const size_t max_opinion_modifiers_count = 16;
      
      std::string id;
      std::string name_id;
      std::string description_id;
      
      // в случае геройского типа эти статы - дополнение к существующим статам, возможно имеет смысл оформить как модификаторы?
      // да лучше как модификаторы, но геройские статы это другое (статы в битве)
      utils::stats_container<hero_stats::values> stats;
      std::array<stat_bonus, max_stat_modifiers_count> bonuses; // бонусы к статам персонажа
      std::array<opinion_modifier, max_opinion_modifiers_count> opinion_mods;
      
      // таблица получения вторичных скиллов или даже не так
      // вторичные скиллы мы будем получать периодически по эвентам
      // хотя таблица все равно полезна
      
      // графика, тип героя в общем нужен только для графики
      // какая графика? отображение в партии, возможно особая картинка в инвентаре
      // в боях графику нужную будем подгружать по id
      //std::string encounter_graphics_id;
      
      hero_type();
    };
  }
}

#endif
