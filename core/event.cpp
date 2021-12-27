#include "event.h"

#include <iostream>

namespace devils_engine {
  namespace core {
    const structure event::s_type;
    event::event() : 
      image{GPU_UINT_MAX},  
      options_count(0) 
    {}
    
    event::option::option() {}
    
#define VALIDATE_VALUE_EXISTANCE(table, key) if (const auto proxy = table[key]; !proxy.valid()) { std::cout << "Event " << id << " must have a " key " script" << "\n"; ++counter; }
    
    bool validate_event(const size_t &, const sol::table &table) {
      // как валидировать?
      // просто проверить чтобы были именно таблицы, да вот тоже нет, могут быть и обычные объекты
      // нужны проверки что хотя бы что то есть по этим ключам
      const auto id = table["id"].get<std::string>();
      size_t counter = 0;
      VALIDATE_VALUE_EXISTANCE(table, "name")
      VALIDATE_VALUE_EXISTANCE(table, "potential")
      
      if (const auto proxy = table["options"]; !proxy.valid() || proxy.get_type() != sol::type::table) { std::cout << "Event " << id << " must have at least 1 option" << "\n"; ++counter; }
      else {
        const auto options = table["options"].get<sol::table>();
        for (const auto &pair : options) {
          if (pair.second.get_type() != sol::type::table) continue;
          
          const auto option_table = pair.second.as<sol::table>();
          VALIDATE_VALUE_EXISTANCE(option_table, "name")
          VALIDATE_VALUE_EXISTANCE(option_table, "condition")
          VALIDATE_VALUE_EXISTANCE(option_table, "effect")
          VALIDATE_VALUE_EXISTANCE(option_table, "ai_weight")
        }
      }
      
      return counter == 0;
    }
    
    void parse_event(core::event* event, const sol::table &table) {
      event->id = table["id"];
      
      // какой у эвента инпут? к сожалению скорее всего неизвестно
      script::input_data input;
      input.current = script::object::type_bit::all_objects;
      input.prev = 0;
      
      script::create_string(input, &event->name_script, table["name"]);
      script::create_string(input, &event->description_script, table["description"]);
      script::create_condition(input, &event->potential, table["potential"]);
      script::create_number(input, &event->mtth_script, table["mtth"]);
      
      if (!event->name_script.valid()) throw std::runtime_error("Event " + event->id + " must have a name script");
      if (!event->potential.valid()) throw std::runtime_error("Event " + event->id + " must have a potential script");
      // нужен ли мттх?
      //if (!event->mtth_script.valid()) throw std::runtime_error("Event " + event->id + " must have a name script");
      
      size_t counter = 0;
      const auto options = table["options"].get<sol::table>();
      for (const auto &pair : options) {
        if (pair.second.get_type() != sol::type::table) continue;
        
        if (counter >= core::event::max_options_count) throw std::runtime_error("Too many options for event " + event->id);
        
        const auto option_table = pair.second.as<sol::table>();
        
        script::create_string(input, &event->options[counter].name_script, option_table["name"]);
        script::create_string(input, &event->options[counter].description_script, option_table["description"]);
        script::create_condition(input, &event->options[counter].condition, table["condition"]);
        script::create_effect(input, &event->options[counter].effects, table["effect"]);
        script::create_number(input, &event->options[counter].ai_weight, table["ai_weight"]);
        
        if (!event->options[counter].name_script.valid()) throw std::runtime_error("Event " + event->id + " option " + std::to_string(counter+1) + " must have a name script");
        if (!event->options[counter].condition.valid()) throw std::runtime_error("Event " + event->id + " option " + std::to_string(counter+1) + " must have a condition script");
        if (!event->options[counter].effects.valid()) throw std::runtime_error("Event " + event->id + " option " + std::to_string(counter+1) + " must have a effect script");
        if (!event->options[counter].ai_weight.valid()) throw std::runtime_error("Event " + event->id + " option " + std::to_string(counter+1) + " must have a ai_weight script");
        
        ++counter;
      }
    }
  }
}
