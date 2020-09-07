#include "build_subsystem.h"
#include "bin/core_structures.h"

namespace devils_engine {
  namespace ai {
    build_subsystem::build_subsystem() {
      attribs.set(attrib_threadsave_check, true);
      attribs.set(attrib_threadsave_process, true);
      attribs.set(attrib_not_dead_characters, true);
      attribs.set(attrib_playable_characters, true);
    }
    
    uint32_t build_subsystem::check(core::character* c) const {
      // что мы проверяем? если мы сейчас находимся в войне, то привлекательность строительства падает
      // если у нас мало денег, то мы в принципе не способны что-либо построить 
      // возможно цель этого персонажа несовместима со строительством
      // и возможно сам характер персонажа не позволяет ничего особенно строить
      
      // должны быть какие то игровые переменные которые позволят быстро ограничить строительство
      // например минимальная сумма денег 
      if (c->current_stats[core::character_stats::money].fval < 200.0f) return SUB_SYSTEM_SKIP;
      
      // получаем новое случайное число примерно так
      const auto current_state = c->rng_state;
      c->rng_state = utils::rng::next(c->rng_state);
      const double val = utils::rng::normalize(utils::rng::value(current_state));
      
      // какая вероятность?
      const double build_probability = 1.0 / 70.0; // должно вызываться в среднем раз в 70 ходов
      if (val >= build_probability) return SUB_SYSTEM_SKIP;
      
      // мы можем вернуть тип того что хотим сейчас построить
      return 0;
    }
    
    void build_subsystem::process(core::character* c, const uint32_t &data) {
      // здесь сложнее вычисления
      // нужно обойти все титулы и в каких городах можно что построить
      // и либо начать строительство либо не найти подходящее здание
      
      std::vector<core::city*> cities;
      auto current_title = c->factions[core::character::self]->titles;
      while (current_title != nullptr) {
        if (current_title->type == core::titulus::type::city) cities.push_back(current_title->get_city());
        current_title = current_title->next;
      }
      
      std::vector<double[core::city_type::maximum_buildings]> probabilities(cities.size());
      memset(probabilities.data(), 0, sizeof(double) * core::city_type::maximum_buildings * probabilities.size());
      
      const float money = c->current_stats[core::character_stats::money].fval;
      
      // вот у нас есть список городов, доступность зданий мы можем считать здесь
      // с другой стороны доступность возможно нужно пересчитать отдельно, для каждого города можно задать переменную которая будет отслеживать изменение остояния провинции
      // в каком городе будем строить? чаще всего нас интересует лишь столица/не столица
      // больше нас интересуют конкретные здания (комп должен стремиться сначало делать дешевые войска и легкие деньги)
      for (size_t k = 0; k < cities.size(); ++k) {
        auto city = cities[k];
        if (city->start_building == SIZE_MAX) continue;
        
        auto city_type = city->type;
        for (size_t i = 0; i < core::city_type::maximum_buildings && city_type->buildings[i] != nullptr; ++i) {
          if (city->complited_buildings.get(i)) continue;
          if (!city->available_buildings.get(i)) continue;
          
          // вот здание которое мы можем построить
          // нам нужно как то понять что мы будем строить именно это здание
          // это вероятность, как она расчитывается? вероятность это число [0,1)
          // для каждого доступного здания нужно расчитать некую вероятность
          // если денег у нас немного (меньше чем цена здания * 2)
          // то мы стремимся построить дешевые здания
          // если денег у нас много (больше чем цена здания * 2)
          // то мы хотим строить более дорогие здания
          // вероятность строительства дорогого здания с достаточным количеством денег больше чем дешевого с достаточным
          
          auto building = city_type->buildings[i];
          float cost = 0.0f;
          for (size_t j = 0; j < core::building_type::maximum_stat_modifiers; ++j) {
            if (building->mods[j].type != core::unit_type::character) continue;
            if (building->mods[j].stat != core::character_stats::money) continue; // думаю что нужно вытащить стоимость
            cost = building->mods[j].mod.fval;
            break;
          }
          
          float money_k = cost / money;
          float inv_k = 1.0f - money_k;
          float fmk = money_k < 0.5f ? (0.5f - (inv_k - 0.5f)) + 0.5f : inv_k + 0.07f;
          const float final_money_k = glm::mix(0.1f, 0.9f, fmk); // это число еще модифицирует коэффициент здания + нужно учесть характер персонажа
          
          probabilities[k][i] = final_money_k / 2.0f;
        }
        
        // нужно просмотреть здания во всех городах
      }
      
      bool found = false;
      for (size_t k = 0; k < cities.size(); ++k) {
        for (size_t i = 0; i < core::city_type::maximum_buildings; ++i) {
          if (probabilities[k][i] < EPSILON) continue;
          const auto current_state = c->rng_state;
          c->rng_state = utils::rng::next(c->rng_state);
          const double val = utils::rng::normalize(utils::rng::value(current_state));
          
          if (val < probabilities[k][i]) {
            if (cities[k]->start_build(c, i)) { // здесь мы отнимаем денюжку
              found = true;
              break;
            }
          }
        }
        
        //if (found) break; // наверное просмотрим все до конца
      }
      
      // хорошо если в некотрые моменты комп не будет строить вообще, но при этом он должен тратить денюжки
      // когда отнимать денюжки? пытаться построить одно здание или пытаться построить больше?
      // 
      
      // теперь понятно что эта функция должна запускаться не часто
    }
  }
}
