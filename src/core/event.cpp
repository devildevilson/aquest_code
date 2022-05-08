#include "event.h"

#include "script/system.h"
#include "utils/globals.h"
#include "utils/systems.h"

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
      
      auto sys = global::get<systems::map_t>()->script_system.get();
      
#define MAKE_SCRIPT_CONDITION(name) event->name = sys->create_condition<script::object>(table[#name], #name);
#define MAKE_SCRIPT_EFFECT(name) event->name = sys->create_effect<script::object>(table[#name], #name);
#define MAKE_SCRIPT_NUMBER(name) event->name = sys->create_number<script::object>(table[#name], #name);
      
//       // какой у эвента инпут? к сожалению скорее всего неизвестно
//       script::input_data input;
//       input.current = script::object::type_bit::all_objects;
//       input.prev = 0;

      event->name_script = sys->create_string<script::object>(table["name"], "name");
      event->description_script = sys->create_string<script::object>(table["description"], "description");
      
      MAKE_SCRIPT_CONDITION(potential)
      
      event->mtth_script = sys->create_number<script::object>(table["mtth"], "mtth");
      
      if (!event->name_script.valid()) throw std::runtime_error("Event " + event->id + " must have a name script");
      if (!event->potential.valid()) throw std::runtime_error("Event " + event->id + " must have a potential script");
      // нужен ли мттх?
      //if (!event->mtth_script.valid()) throw std::runtime_error("Event " + event->id + " must have a name script");
      
#undef MAKE_SCRIPT_CONDITION
#undef MAKE_SCRIPT_EFFECT
#undef MAKE_SCRIPT_NUMBER
      
      
#define MAKE_SCRIPT_CONDITION(name) event->options[counter].name = sys->create_condition<script::object>(option_table[#name], #name);
#define MAKE_SCRIPT_EFFECT(name) event->options[counter].name = sys->create_effect<script::object>(option_table[#name], #name);
#define MAKE_SCRIPT_NUMBER(name) event->options[counter].name = sys->create_number<script::object>(option_table[#name], #name);
      
      size_t counter = 0;
      const auto options = table["options"].get<sol::table>();
      for (const auto &pair : options) {
        if (pair.second.get_type() != sol::type::table) continue;
        
        if (counter >= core::event::max_options_count) throw std::runtime_error("Too many options for event " + event->id);
        
        const auto option_table = pair.second.as<sol::table>();
        
        event->options[counter].name_script = sys->create_string<script::object>(option_table["name"], "name");
        event->options[counter].description_script = sys->create_string<script::object>(option_table["description"], "description");
        MAKE_SCRIPT_CONDITION(condition)
        MAKE_SCRIPT_EFFECT(effect)
        MAKE_SCRIPT_NUMBER(ai_weight)
        
        if (!event->options[counter].name_script.valid()) throw std::runtime_error("Event " + event->id + " option " + std::to_string(counter+1) + " must have a name script");
        if (!event->options[counter].condition.valid()) throw std::runtime_error("Event " + event->id + " option " + std::to_string(counter+1) + " must have a condition script");
        if (!event->options[counter].effect.valid()) throw std::runtime_error("Event " + event->id + " option " + std::to_string(counter+1) + " must have a effect script");
        if (!event->options[counter].ai_weight.valid()) throw std::runtime_error("Event " + event->id + " option " + std::to_string(counter+1) + " must have a ai_weight script");
        
        ++counter;
      }
      
#undef MAKE_SCRIPT_CONDITION
#undef MAKE_SCRIPT_EFFECT
#undef MAKE_SCRIPT_NUMBER
    }
  }
}
