#include "objects_selector.h"

#include <stdexcept>
#include "utils/globals.h"
#include "utils/systems.h"
#include "core_context.h"

// я забыл что у меня нет доступа к армиям из контекста
// зато у меня должен быть доступ к армии из персонажа
// тогда по идее у нас как бы путеществуют по карте персонажи скорее 
// чем армии или отряды, нужно наверное придерживаться такой
// архитектуры, 

namespace devils_engine {
  namespace utils {
    objects_selector::unit::unit() : type(invalid), index(UINT32_MAX) {}
    objects_selector::objects_selector() : count(0) {}
//     core::character* objects_selector::get(const uint32_t &index) const {
//       if (index >= count) throw std::runtime_error("Wrong selection index");
//       if (units[index].type != unit::type::character) return nullptr;
//       auto ctx = global::get<systems::map_t>()->core_context;
//       return ctx->get_character(units[index].index);
//     }

    bool objects_selector::has(const uint32_t &index) const {
      if (index >= count) throw std::runtime_error("Wrong selection index");
      return units[index].type != unit::type::invalid;
    }

    core::army* objects_selector::get_army(const uint32_t &index) const {
      if (index >= count) throw std::runtime_error("Wrong selection index");
      if (units[index].type != unit::type::army) return nullptr;
      auto ctx = global::get<systems::map_t>()->core_context;
      auto c = ctx->get_character(units[index].index); // можем сделать так
      return c->army;
    }
    
    core::hero_troop* objects_selector::get_troop(const uint32_t &index) const {
      if (index >= count) throw std::runtime_error("Wrong selection index");
      if (units[index].type != unit::type::hero) return nullptr;
      auto ctx = global::get<systems::map_t>()->core_context;
      auto c = ctx->get_character(units[index].index); // можем сделать так
      return c->troop;
    }
    
    void objects_selector::add(const enum unit::type type, const uint32_t &index) {
      const uint32_t current_index = count;
      if (current_index >= maximum_selection) return;
      ++count;
      units[current_index].type = type;
      units[current_index].index = index;
    }
    
    void objects_selector::clear() {
      for (uint32_t i = 0; i < maximum_selection; ++i) {
        units[i].type = unit::type::invalid;
        units[i].index = UINT32_MAX;
      }
      
      count = 0;
    }
    
    void objects_selector::copy(std::vector<unit> &copy_selector) const {
      copy_selector.clear();
      if (count == 0) return;
      
      copy_selector.resize(count);
      for (uint32_t i = 0; i < count; ++i) {
        copy_selector[i] = units[i];
      }
    }
    
    std::vector<objects_selector::unit> objects_selector::copy() const {
      std::vector<objects_selector::unit> copy_selector(count);
      for (uint32_t i = 0; i < count; ++i) {
        copy_selector[i] = units[i];
      }
      
      return copy_selector;
    }
  }
}
