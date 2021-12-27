#include "interaction.h"

#include "utils/utility.h"
#include "target_type.h"
#include "structures_header.h"

#include <iostream>

namespace devils_engine {
  namespace core {
    interaction::interaction() : input_count(0), options_count(0) {}
    
#define INPUT_CASE(type) case core::target_type::type: {      \
    in->input_array[current_index].first = std::string(id);     \
    in->input_array[current_index].second.number_type = script::number_type::object; \
    in->input_array[current_index].second.helper2 = static_cast<uint32_t>(core::type::s_type); \
    break;                                                    \
  }
    
//     void init_input_array(const sol::object &obj, core::interaction* in) {
//       assert(obj.get_type() == sol::type::table);
//       const auto input_t = obj.as<sol::table>();
//       in->input_count = 1;
//       for (const auto &pair : input_t) {
//         if (pair.second.get_type() != sol::type::number) continue;
//         
//         std::string id;
//         if (pair.first.get_type() == sol::type::string) {
//           id = pair.first.as<std::string>();
//         } else if (pair.first.get_type() == sol::type::number) {
//           id = "root";
//         }
//         
//         const uint32_t type = pair.second.as<uint32_t>();
//         const bool is_root = id == "root";
//         const size_t current_index = is_root ? 0 : in->input_count;
//         in->input_count += size_t(!is_root);
//         
//         if (is_root && !in->input_array[0].first.empty()) throw std::runtime_error("Root data is already setup");
//         
//         switch (type) {
//           INPUT_CASE(character)
//           INPUT_CASE(army)
//           INPUT_CASE(city)
//           INPUT_CASE(culture)
//           INPUT_CASE(dynasty)
//           INPUT_CASE(hero_troop)
//           INPUT_CASE(province)
//           INPUT_CASE(realm)
//           INPUT_CASE(religion)
//           INPUT_CASE(titulus)
//           
//           case core::target_type::boolean: {
//             if (is_root) throw std::runtime_error("Root node could not be boolean, number or string type");
//             in->input_array[current_index].first = id;
//             in->input_array[current_index].second.number_type = script::number_type::boolean;
//             break;
//           }
//           
//           case core::target_type::number: {
//             if (is_root) throw std::runtime_error("Root node could not be boolean, number or string type");
//             in->input_array[current_index].first = id;
//             in->input_array[current_index].second.number_type = script::number_type::number;
//             break;
//           }
//           
//           case core::target_type::string: {
//             if (is_root) throw std::runtime_error("Root node could not be boolean, number or string type");
//             in->input_array[current_index].first = id;
//             in->input_array[current_index].second.number_type = script::number_type::string;
//             break;
//           }
//           
//           default: throw std::runtime_error("Bad input target type");
//         }
//         
//         assert(in->input_count <= 16);
//       }
//     }
    
    bool validate_interaction(const size_t &index, const sol::table &table) {
      UNUSED_VARIABLE(index);
      size_t counter = 0;
      std::string_view id;
      if (const auto proxy = table["id"]; proxy.valid() && proxy.get_type() == sol::type::string) {
        id = proxy.get<std::string_view>();
      } else { PRINT("Interaction must have an id"); ++counter; return false; }
      
      // это не особ полезно теперь
//       enum core::interaction::type t;
//       if (const auto proxy = table["type"]; proxy.valid() && proxy.get_type() == sol::type::number) {
//         const auto index = proxy.get<uint32_t>();
//         if (index >= static_cast<uint32_t>(core::decision::type::count)) throw std::runtime_error("Bad decision " + std::string(id) + " type");
//         t = static_cast<enum core::decision::type>(index);
//       } else { PRINT("Decision " + std::string(id) + " must have a type"); ++counter; return false; }
      
      if (const auto proxy = table["name"]; !(proxy.valid() && (proxy.get_type() == sol::type::string || proxy.get_type() == sol::type::table))) {
        PRINT("Interaction " + std::string(id) + " must have a name"); ++counter;
      }
      
      if (const auto proxy = table["description"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid description"); ++counter; }
      }
      
      // что с инпутом? инпут должен быть
      if (const auto proxy = table["input"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Interaction " + std::string(id) + " must have an expected input table"); ++counter;
      }
      
      // должен ли потентиал присутствовать всегда?
      if (const auto proxy = table["potential"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Interaction " + std::string(id) + " must have a potential check script"); ++counter;
      }
      
      if (const auto proxy = table["condition"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Interaction " + std::string(id) + " must have a condition check script"); ++counter;
      }
      
      if (const auto proxy = table["auto_accept"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid auto_accept check script"); ++counter; }
      }
      
      if (const auto proxy = table["immediate"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid immediate effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_accept"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid on_accept effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_auto_accept"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid on_auto_accept effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_decline"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid on_decline effect script"); ++counter; }
      }
      
      if (const auto proxy = table["pre_auto_accept"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid pre_auto_accept effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_blocked_effect"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid on_blocked_effect effect script"); ++counter; }
      }
      
      if (const auto proxy = table["ai_will_do"]; !(proxy.valid() && (proxy.get_type() == sol::type::number || proxy.get_type() == sol::type::table))) {
        PRINT("Interaction " + std::string(id) + " must have an ai_will_do number script"); ++counter;
      }
      
      if (const auto proxy = table["ai_accept"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid ai_accept number script"); ++counter; }
      }
      
      if (const auto proxy = table["ai_frequency"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid ai_frequency number script"); ++counter; }
      }
      
      if (const auto proxy = table["ai_potential"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid ai_potential number script"); ++counter; }
      }
      
      if (const auto proxy = table["send_options"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid array send_options scripts"); ++counter; }
        else {
          const auto t = proxy.get<sol::table>();
          for (const auto &pair : t) {
            if (pair.second.get_type() != sol::type::table) continue;
            
            const auto opt = pair.second.as<sol::table>();
            std::string_view opt_id;
            if (const auto proxy = opt["id"]; proxy.valid() && proxy.get_type() == sol::type::string) {
              opt_id = proxy.get<std::string_view>();
            } else { PRINT("Interaction " + std::string(id) + " option must have an id"); ++counter; return false; }
            
            if (const auto proxy = opt["name"]; !(proxy.valid() && (proxy.get_type() == sol::type::string || proxy.get_type() == sol::type::table))) {
              PRINT("Interaction " + std::string(id) + " option " + std::string(opt_id) + " must have a name"); ++counter;
            }
            
            if (const auto proxy = opt["description"]; proxy.valid()) {
              if (proxy.get_type() != sol::type::string && proxy.get_type() != sol::type::table) { 
                PRINT("Interaction " + std::string(id) + " option " + std::string(opt_id) + " must have a valid description"); ++counter; 
              }
            }
            
            // должен ли потентиал присутствовать всегда?
            if (const auto proxy = opt["potential"]; proxy.valid()) {
              if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " option " + std::string(opt_id) + " must have a potential check script"); ++counter; }
            }
            
            if (const auto proxy = opt["condition"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
              PRINT("Interaction " + std::string(id) + " option " + std::string(opt_id) + " must have a condition check script"); ++counter;
            }
            
            if (const auto proxy = opt["effect"]; proxy.valid()) {
              if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " option " + std::string(opt_id) + " must have a valid effect script"); ++counter; }
            }
            
            if (const auto proxy = opt["ai_will_do"]; !(proxy.valid() && (proxy.get_type() == sol::type::number || proxy.get_type() == sol::type::table))) {
              PRINT("Interaction " + std::string(id) + " option " + std::string(opt_id) + " must have an ai_will_do number script"); ++counter;
            }
          }
        }
      }
      
      return counter == 0;
    }
    
    void parse_interaction(core::interaction* interaction, const sol::table &table) {
      interaction->id = table["id"];
      
//       if (const auto proxy = table["type"]; proxy.valid() && proxy.get_type() == sol::type::number) {
//         const uint32_t data = proxy.get<uint32_t>();
//         if (data >= static_cast<uint32_t>(core::decision::type::count)) throw std::runtime_error("Bad decision type");
//         decision->type = static_cast<enum core::decision::type>(data);
//       }
      
      script::input_data inter_input;
      inter_input.current = inter_input.root = script::object::type_bit::character;
      // все интеракции идут от персонажа, передаются кому то другому
      // эти другие передаются в контексте, можно ли их проверить? хороший вопрос
      
      script::create_string(inter_input, &interaction->name_script,        table["name"]);
      script::create_string(inter_input, &interaction->description_script, table["description"]);
      
      script::create_condition(inter_input, &interaction->potential,   table["potential"]);
      script::create_condition(inter_input, &interaction->condition,   table["conditions"]);
      script::create_condition(inter_input, &interaction->auto_accept, table["auto_accept"]);
      script::create_condition(inter_input, &interaction->ai_potential, table["ai_potential"]); // это разве кондишен?
      
      script::create_effect(inter_input, &interaction->immediate,         table["immediate"]);
      script::create_effect(inter_input, &interaction->on_accept,         table["on_accept"]);
      script::create_effect(inter_input, &interaction->on_auto_accept,    table["on_auto_accept"]);
      script::create_effect(inter_input, &interaction->on_decline,        table["on_decline"]);
      script::create_effect(inter_input, &interaction->pre_auto_accept,   table["pre_auto_accept"]);
      script::create_effect(inter_input, &interaction->on_blocked_effect, table["on_blocked_effect"]);
      
      script::create_number(inter_input, &interaction->ai_accept,    table["ai_accept"]);
      script::create_number(inter_input, &interaction->ai_will_do,   table["ai_will_do"]);
      script::create_number(inter_input, &interaction->ai_frequency, table["ai_frequency"]);
      //script::create_number(inter_input, &interaction->ai_potential, table["ai_potential"]);
      
      if (!interaction->name_script.valid()) throw std::runtime_error("Interaction " + interaction->id + " must have name script");
      if (!interaction->potential.valid())   throw std::runtime_error("Interaction " + interaction->id + " must have potential script");
      if (!interaction->condition.valid())   throw std::runtime_error("Interaction " + interaction->id + " must have condition script");
      if (!interaction->ai_will_do.valid())  throw std::runtime_error("Interaction " + interaction->id + " must have ai_will_do script");
      
      if (const auto proxy = table["send_options"]; proxy.valid()) {
        size_t counter = 0;
        const auto t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (pair.second.get_type() != sol::type::table) continue;
          if (counter >= core::interaction::max_options_count) 
            throw std::runtime_error("Too many interaction options, maximum is " + std::to_string(core::interaction::max_options_count));
          
          const auto opt = pair.second.as<sol::table>();
          interaction->send_options[interaction->options_count].id = opt["id"];
          script::create_string(   inter_input, &interaction->send_options[counter].name_script,        opt["name"]);
          script::create_string(   inter_input, &interaction->send_options[counter].description_script, opt["description"]);
          script::create_condition(inter_input, &interaction->send_options[counter].potential,          opt["potential"]);
          script::create_condition(inter_input, &interaction->send_options[counter].condition,          opt["condition"]);
          script::create_effect(   inter_input, &interaction->send_options[counter].effect,             opt["effect"]);
          script::create_number(   inter_input, &interaction->send_options[counter].ai_will_do,         opt["ai_will_do"]);
          
          if (!interaction->send_options[counter].name_script.valid()) 
            throw std::runtime_error("Interaction " + interaction->id + " send option " + std::to_string(counter) + " must have a name script");
          
          if (!interaction->send_options[counter].condition.valid()) 
            throw std::runtime_error("Interaction " + interaction->id + " send option " + std::to_string(counter) + " must have a condition script");
          
          if (!interaction->send_options[counter].ai_will_do.valid()) 
            throw std::runtime_error("Interaction " + interaction->id + " send option " + std::to_string(counter) + " must have a ai_will_do script");
          
          ++counter;
        }
        
        interaction->options_count = counter;
      }
    }
  }
}
