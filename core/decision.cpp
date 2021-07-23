#include "decision.h"

#include "utils/utility.h"
#include "script/script_block_functions.h"
#include "structures_header.h"
#include "target_type.h"

namespace devils_engine {
  namespace core {
    const structure decision::s_type;
    //, potential_count(0), potential_array(nullptr), condition_count(0), condition_array(nullptr), effect_count(0), effect_array(nullptr) 
    decision::decision() : type(type::minor), input_count(0) {}
    decision::~decision() {
//       delete [] potential_array;
//       delete [] condition_array;
//       delete [] effect_array;
    }
    
    bool decision::check_shown_condition(script::context* ctx) const {
      // осталось решить что с рнд, рнд нужно каждый раз приводить в дефолтное состояние
      // 
//       script::random_state rnd { 5235545 };
//       
//       script::context ctx{
//         {},
//         {},
//         &rnd,
//         nullptr
//       };
//       
//       ctx.array_data.emplace_back(root);
//       ctx.array_data.emplace_back(helper);
      
      const script::target_t t = check_input(ctx);
      //return script::condition(root, ctx, potential_count, potential_array) == TRUE_BLOCK;
      return script::condition(t, ctx, 1, &potential) == TRUE_BLOCK;
    }
    
    bool decision::check_condition(script::context* ctx) const {
      const script::target_t t = check_input(ctx);
      //return script::condition(t, ctx, condition_count, condition_array) == TRUE_BLOCK;
      return script::condition(t, ctx, 1, &condition) == TRUE_BLOCK;
    }
    
    void decision::iterate_potential(script::context* ctx) const {
      const script::target_t t = check_input(ctx);
      assert(ctx->itr_func != nullptr);
      script::condition(t, ctx, 1, &potential);
    }
    
    void decision::iterate_conditions(script::context* ctx) const {
      const script::target_t t = check_input(ctx);
      assert(ctx->itr_func != nullptr);
      script::condition(t, ctx, 1, &condition);
    }
    
    void decision::iterate_actions(script::context* ctx) const {
      const script::target_t t = check_input(ctx);
      assert(ctx->itr_func != nullptr);
      script::action(t, ctx, 1, &effect);
    }
    
    bool decision::run(script::context* ctx) const {
      const script::target_t t = check_input(ctx);
      const bool check = script::condition(t, ctx, 1, &condition) == TRUE_BLOCK;
      if (!check) return false;
      
      //script::action(t, ctx, effect_count, effect_array);
      script::action(t, ctx, 1, &effect);
      return true;
    }
    
    std::string_view decision::get_name(script::context* ctx) const {
      const script::target_t t = check_input(ctx);
      return script::get_string_from_script(t, ctx, &name_script);
    }
    
    std::string_view decision::get_description(script::context* ctx) const {
      const script::target_t t = check_input(ctx);
      return script::get_string_from_script(t, ctx, &description_script);
    }
    
    bool decision::check_ai(script::context* ctx) const {
      UNUSED_VARIABLE(ctx);
      // проверка ии, она пишется отдельным скриптом
      return false;
    }
    
    bool decision::run_ai(script::context* ctx) const {
      UNUSED_VARIABLE(ctx);
      // я думал что запуск для ии будет чем то отличаться, но вряд ли
      return false;
    }
    
    script::target_t decision::check_input(script::context* ctx) const {
      // тут наверное нужно проверить входные данные, как это сделать?
      // входные данные у нас записаны в инпут, и наверное нужно просто проверить типы?
      // то есть
      if (ctx->root.type != input_array[0].second.helper2) throw std::runtime_error("Input root is wrong type");
      for (size_t i = 1; i < input_count; ++i) {
        auto itr = ctx->map_data.find(input_array[i].first);
        if (itr == ctx->map_data.end()) throw std::runtime_error("Could not find " + input_array[i].first + " data");
        const bool check = 
          itr->second.command_type == input_array[i].second.command_type && 
          itr->second.number_type == input_array[i].second.number_type && 
          itr->second.helper2 == input_array[i].second.helper2;
          
        if (!check) throw std::runtime_error("Bad " + std::to_string(i+1) + " decision argument");
      }
      
      return ctx->root;
    }
    
#define INPUT_CASE(type) case core::target_type::type: {      \
    d->input_array[current_index].first = std::string(id);     \
    d->input_array[current_index].second.number_type = script::number_type::object; \
    d->input_array[current_index].second.helper2 = static_cast<uint32_t>(core::type::s_type); \
    break;                                                    \
  }
    
    void init_input_array(const sol::object &obj, core::decision* d) {
      assert(obj.get_type() == sol::type::table);
      const auto input_t = obj.as<sol::table>();
      d->input_count = 1;
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
        const size_t current_index = is_root ? 0 : d->input_count;
        d->input_count += size_t(!is_root);
        
        if (is_root && !d->input_array[0].first.empty()) throw std::runtime_error("Root data is already setup");
        
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
            d->input_array[current_index].first = id;
            d->input_array[current_index].second.number_type = script::number_type::boolean;
            break;
          }
          
          case core::target_type::number: {
            if (is_root) throw std::runtime_error("Root node could not be boolean, number or string type");
            d->input_array[current_index].first = id;
            d->input_array[current_index].second.number_type = script::number_type::number;
            break;
          }
          
          case core::target_type::string: {
            if (is_root) throw std::runtime_error("Root node could not be boolean, number or string type");
            d->input_array[current_index].first = id;
            d->input_array[current_index].second.number_type = script::number_type::string;
            break;
          }
          
          default: throw std::runtime_error("Bad input target type");
        }
        
        assert(d->input_count <= 16);
      }
    }
    
    bool validate_decision(const size_t &index, const sol::table &table) {
      UNUSED_VARIABLE(index);
      size_t counter = 0;
      std::string_view id;
      if (const auto proxy = table["id"]; proxy.valid() && proxy.get_type() == sol::type::string) {
        id = proxy.get<std::string_view>();
      } else { PRINT("Decision must have an id"); ++counter; return false; }
      
      // это не особ полезно теперь
      enum core::decision::type t;
      if (const auto proxy = table["type"]; proxy.valid() && proxy.get_type() == sol::type::number) {
        const auto index = proxy.get<uint32_t>();
        if (index >= static_cast<uint32_t>(core::decision::type::count)) throw std::runtime_error("Bad decision " + std::string(id) + " type");
        t = static_cast<enum core::decision::type>(index);
      } else { PRINT("Decision " + std::string(id) + " must have a type"); ++counter; return false; }
      
      if (const auto proxy = table["name"]; !(proxy.valid() && (proxy.get_type() == sol::type::string || proxy.get_type() == sol::type::table))) {
        PRINT("Decision " + std::string(id) + " must have a name"); ++counter;
      }
      
      if (const auto proxy = table["description"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string && proxy.get_type() != sol::type::table) { PRINT("Decision " + std::string(id) + " must have a valid description"); ++counter; }
      }
      
      // что с инпутом? инпут должен быть
      if (const auto proxy = table["input"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have an expected input table"); ++counter;
      }
      
      // должен ли потентиал присутствовать всегда?
      if (const auto proxy = table["potential"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have a potential check script"); ++counter;
      }
      
      if (const auto proxy = table["condition"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have a condition check script"); ++counter;
      }
      
      if (const auto proxy = table["effect"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have an effect script"); ++counter;
      }
      
      if (const auto proxy = table["ai_will_do"]; !(proxy.valid() && (proxy.get_type() == sol::type::number || proxy.get_type() == sol::type::table))) {
        PRINT("Decision " + std::string(id) + " must have a valid ai_will_do number script"); ++counter;
      }
      
      return counter == 0;
    }
    
    void parse_decision(core::decision* decision, const sol::table &table) {
      decision->id = table["id"];
      
      if (const auto proxy = table["type"]; proxy.valid() && proxy.get_type() == sol::type::number) {
        const uint32_t data = proxy.get<uint32_t>();
        if (data >= static_cast<uint32_t>(core::decision::type::count)) throw std::runtime_error("Bad decision type");
        decision->type = static_cast<enum core::decision::type>(data);
      }
      
      core::init_input_array(table["input"], decision);
      const uint32_t root_type = decision->input_array[0].second.helper2;
      script::init_string_from_script(root_type, table["name"], &decision->name_script);
      if (const auto proxy = table["description"]; proxy.valid()) script::init_string_from_script(root_type, proxy, &decision->description_script);
      script::init_condition(root_type, table["potential"], &decision->potential);
      script::init_condition(root_type, table["conditions"], &decision->condition);
      script::init_action(root_type, table["effects"], &decision->effect);
      script::init_number_from_script(root_type, table["ai_will_do"], &decision->ai_will_do);
    }
  }
}
