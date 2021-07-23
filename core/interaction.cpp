#include "interaction.h"
#include "utils/utility.h"
#include "target_type.h"
#include "structures_header.h"

namespace devils_engine {
  namespace core {
    interaction::interaction() : input_count(0) {}
    
#define INPUT_CASE(type) case core::target_type::type: {      \
    in->input_array[current_index].first = std::string(id);     \
    in->input_array[current_index].second.number_type = script::number_type::object; \
    in->input_array[current_index].second.helper2 = static_cast<uint32_t>(core::type::s_type); \
    break;                                                    \
  }
    
    void init_input_array(const sol::object &obj, core::interaction* in) {
      assert(obj.get_type() == sol::type::table);
      const auto input_t = obj.as<sol::table>();
      in->input_count = 1;
      for (const auto &pair : input_t) {
        if (pair.second.get_type() != sol::type::number) continue;
        
        std::string id;
        if (pair.first.get_type() == sol::type::string) {
          id = pair.first.as<std::string>();
        } else if (pair.first.get_type() == sol::type::number) {
          id = "root";
        }
        
        const uint32_t type = pair.second.as<uint32_t>();
        const bool is_root = id == "root";
        const size_t current_index = is_root ? 0 : in->input_count;
        in->input_count += size_t(!is_root);
        
        if (is_root && !in->input_array[0].first.empty()) throw std::runtime_error("Root data is already setup");
        
        switch (type) {
          INPUT_CASE(character)
          INPUT_CASE(army)
          INPUT_CASE(city)
          INPUT_CASE(culture)
          INPUT_CASE(dynasty)
          INPUT_CASE(hero_troop)
          INPUT_CASE(province)
          INPUT_CASE(realm)
          INPUT_CASE(religion)
          INPUT_CASE(titulus)
          
          case core::target_type::boolean: {
            if (is_root) throw std::runtime_error("Root node could not be boolean, number or string type");
            in->input_array[current_index].first = id;
            in->input_array[current_index].second.number_type = script::number_type::boolean;
            break;
          }
          
          case core::target_type::number: {
            if (is_root) throw std::runtime_error("Root node could not be boolean, number or string type");
            in->input_array[current_index].first = id;
            in->input_array[current_index].second.number_type = script::number_type::number;
            break;
          }
          
          case core::target_type::string: {
            if (is_root) throw std::runtime_error("Root node could not be boolean, number or string type");
            in->input_array[current_index].first = id;
            in->input_array[current_index].second.number_type = script::number_type::string;
            break;
          }
          
          default: throw std::runtime_error("Bad input target type");
        }
        
        assert(in->input_count <= 16);
      }
    }
    
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
      
      core::init_input_array(table["input"], interaction);
      const uint32_t root_type = interaction->input_array[0].second.helper2;
      script::init_string_from_script(root_type, table["name"], &interaction->name_script);
      if (const auto proxy = table["description"]; proxy.valid()) script::init_string_from_script(root_type, proxy, &interaction->description_script);
      script::init_condition(root_type, table["potential"], &interaction->potential);
      script::init_condition(root_type, table["conditions"], &interaction->condition);
      if (const auto proxy = table["auto_accept"]; proxy.valid()) script::init_condition(root_type, proxy, &interaction->auto_accept);
      if (const auto proxy = table["immediate"]; proxy.valid()) script::init_action(root_type, proxy, &interaction->immediate);
      if (const auto proxy = table["on_accept"]; proxy.valid()) script::init_action(root_type, proxy, &interaction->on_accept);
      if (const auto proxy = table["on_auto_accept"]; proxy.valid()) script::init_action(root_type, proxy, &interaction->on_auto_accept);
      if (const auto proxy = table["on_decline"]; proxy.valid()) script::init_action(root_type, proxy, &interaction->on_decline);
      if (const auto proxy = table["pre_auto_accept"]; proxy.valid()) script::init_action(root_type, proxy, &interaction->pre_auto_accept);
      if (const auto proxy = table["on_blocked_effect"]; proxy.valid()) script::init_action(root_type, proxy, &interaction->on_blocked_effect);
      if (const auto proxy = table["ai_accept"]; proxy.valid()) script::init_number_from_script(root_type, proxy, &interaction->ai_accept);
      script::init_number_from_script(root_type, table["ai_will_do"], &interaction->ai_will_do);
      if (const auto proxy = table["ai_frequency"]; proxy.valid()) script::init_number_from_script(root_type, proxy, &interaction->ai_frequency);
      if (const auto proxy = table["ai_potential"]; proxy.valid()) script::init_number_from_script(root_type, proxy, &interaction->ai_potential);
      
      if (const auto proxy = table["send_options"]; proxy.valid()) {
        const auto t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (pair.second.get_type() != sol::type::table) continue;
          
          const auto opt = pair.second.as<sol::table>();
          interaction->send_options.emplace_back();
          interaction->send_options.back().id = opt["id"];
          script::init_string_from_script(root_type, opt["name"], &interaction->name_script);
          if (const auto proxy = opt["description"]; proxy.valid()) script::init_string_from_script(root_type, proxy, &interaction->description_script);
          if (const auto proxy = opt["potential"]; proxy.valid()) script::init_condition(root_type, proxy, &interaction->send_options.back().potential);
          script::init_condition(root_type, opt["condition"], &interaction->send_options.back().condition);
          // думаю что эффект должен быть в любом случае
          if (const auto proxy = opt["effect"]; proxy.valid()) script::init_action(root_type, proxy, &interaction->send_options.back().effect);
          script::init_number_from_script(root_type, opt["ai_will_do"], &interaction->send_options.back().ai_will_do);
        }
      }
    }
  }
}
