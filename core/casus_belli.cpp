#include "casus_belli.h"

#include "utils/utility.h"
#include "utils/magic_enum_header.h"
#include "declare_structures.h"
#include "script/object.h"

#include <iostream>

namespace devils_engine {
  namespace core {
    casus_belli::casus_belli() :
      battle_warscore_mult(1.0f),
      infamy_modifier(1.0f),
      ticking_war_score_multiplier(1.0f),
      att_ticking_war_score_multiplier(1.0f),
      def_ticking_war_score_multiplier(1.0f),
      max_defender_occupation_score(50.0f),
      max_attacker_occupation_score(50.0f),
      max_defender_battle_score(50.0f),
      max_attacker_battle_score(50.0f)
    {}
    
    bool casus_belli::can_use(const realm*, const realm*, const titulus*) const { return false; }
    
    float casus_belli::authority_cost(const realm*, const realm*, const titulus*) const { return 0.0f; }
    
    float casus_belli::esteem_cost(const realm*, const realm*, const titulus*) const { return 0.0f; }
    
    float casus_belli::influence_cost(const realm*, const realm*, const titulus*) const { return 0.0f; }
    
    float casus_belli::money_cost(const realm*, const realm*, const titulus*) const { return 0.0f; }
    
    float casus_belli::ai_probability(const realm*, const realm*, const titulus*) const { return 0.0f; }
    
    std::string_view casus_belli::name(const realm*, const realm*, const titulus*) const { return "invalid"; }
    
    std::string_view casus_belli::war_name(const realm*, const realm*, const titulus*) const { return "invalid"; }
    
#define NUMBER_VARIABLE(name) if (const auto proxy = table[#name]; proxy.valid()) { \
        if (proxy.get_type() != sol::type::number) { PRINT("Casus belli " + std::string(id) + " must have a valid " #name " number"); ++counter; } \
      }
      
#define NUMBER_INIT(name) if (const auto proxy = table[#name]; proxy.valid()) casus_belli->name = proxy.get<double>();
    
    bool validate_casus_belli(const size_t &index, const sol::table &table) {
      UNUSED_VARIABLE(index);
      size_t counter = 0;
      std::string_view id;
      if (const auto proxy = table["id"]; proxy.valid() && proxy.get_type() == sol::type::string) {
        id = proxy.get<std::string_view>();
      } else { PRINT("Casus belli must have an id"); ++counter; return false; }
      
      // это не особ полезно теперь
//       enum core::interaction::type t;
//       if (const auto proxy = table["type"]; proxy.valid() && proxy.get_type() == sol::type::number) {
//         const auto index = proxy.get<uint32_t>();
//         if (index >= static_cast<uint32_t>(core::decision::type::count)) throw std::runtime_error("Bad decision " + std::string(id) + " type");
//         t = static_cast<enum core::decision::type>(index);
//       } else { PRINT("Decision " + std::string(id) + " must have a type"); ++counter; return false; }
      
      if (const auto proxy = table["name"]; !(proxy.valid() && (proxy.get_type() == sol::type::string || proxy.get_type() == sol::type::table))) {
        PRINT("Casus belli " + std::string(id) + " must have a name"); ++counter;
      }
      
      if (const auto proxy = table["war_name"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string && proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid war name"); ++counter; }
      }
      
      if (const auto proxy = table["can_use"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Casus belli " + std::string(id) + " must have a can_use check script"); ++counter;
      }
      
      if (const auto proxy = table["is_valid"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Casus belli " + std::string(id) + " must have a is_valid check script"); ++counter;
      }
      
      if (const auto proxy = table["ai_will_do"]; !(proxy.valid() && (proxy.get_type() == sol::type::number || proxy.get_type() == sol::type::table))) {
        PRINT("Casus belli " + std::string(id) + " must have an ai_will_do number script"); ++counter;
      }
      
      if (const auto proxy = table["on_add"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_add effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_add_post"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_add_post effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_success"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_success effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_success_post"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_success_post effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_fail"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_fail effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_fail_post"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_fail_post effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_reverse_demand"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_reverse_demand effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_reverse_demand_post"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_reverse_demand_post effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_attacker_leader_death"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_attacker_leader_death effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_defender_leader_death"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_defender_leader_death effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_thirdparty_death"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_thirdparty_death effect script"); ++counter; }
      }
      
      if (const auto proxy = table["on_invalidation"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Casus belli " + std::string(id) + " must have a valid on_invalidation effect script"); ++counter; }
      }
      
      NUMBER_VARIABLE(authority_cost)
      NUMBER_VARIABLE(esteem_cost)
      NUMBER_VARIABLE(influence_cost)
      NUMBER_VARIABLE(money_cost)
      NUMBER_VARIABLE(battle_warscore_mult)
      NUMBER_VARIABLE(infamy_modifier)
      NUMBER_VARIABLE(ticking_war_score_multiplier)
      NUMBER_VARIABLE(att_ticking_war_score_multiplier)
      NUMBER_VARIABLE(def_ticking_war_score_multiplier)
      NUMBER_VARIABLE(max_defender_occupation_score)
      NUMBER_VARIABLE(max_attacker_occupation_score)
      NUMBER_VARIABLE(max_defender_battle_score)
      NUMBER_VARIABLE(max_attacker_battle_score)
      NUMBER_VARIABLE(truce_turns)
      
      for (size_t i = 0; i < core::casus_belli::count; ++i) {
        const auto enum_name = magic_enum::enum_name(static_cast<core::casus_belli::flags>(i));
        if (const auto proxy = table[enum_name]; proxy.valid()) {
          if (proxy.get_type() != sol::type::boolean) { PRINT("Casus belli " + std::string(id) + " must have a valid " + std::string(enum_name) + " flag"); ++counter; }
        }
      }
      
      return counter == 0;
    }
    
    void parse_casus_belli(core::casus_belli* casus_belli, const sol::table &table) {
      casus_belli->id = table["id"];
      script::input_data cb_input;
      cb_input.current = script::object::type_bit::character;
      cb_input.root = script::object::type_bit::character;
      
      script::input_data war_input;
      war_input.current = script::object::type_bit::war;
      war_input.root = script::object::type_bit::war;
      
      // если в луа это не определено, то что? игнорируем? надо просто добавить методы валидации скрипта и потом проверить что нужно
      script::create_string(   cb_input, &casus_belli->name_script,           table["name"]);
      script::create_condition(cb_input, &casus_belli->can_use_script,        table["can_use"]);
      script::create_number(   cb_input, &casus_belli->ai_will_do,            table["ai_will_do"]);
      
      script::create_number(   cb_input, &casus_belli->authority_cost_script, table["authority_cost"]);
      script::create_number(   cb_input, &casus_belli->esteem_cost_script,    table["esteem_cost"]);
      script::create_number(   cb_input, &casus_belli->influence_cost_script, table["influence_cost"]);
      script::create_number(   cb_input, &casus_belli->money_cost_script,     table["money_cost"]);
      
      script::create_string(   war_input, &casus_belli->war_name_script, table["war_name"]);
      script::create_condition(war_input, &casus_belli->is_valid,        table["is_valid"]);
      
      script::create_effect(war_input, &casus_belli->on_add,                   table["on_add"]);
      script::create_effect(war_input, &casus_belli->on_add_post,              table["on_add_post"]);
      script::create_effect(war_input, &casus_belli->on_success,               table["on_success"]);
      script::create_effect(war_input, &casus_belli->on_success_post,          table["on_success_post"]);
      script::create_effect(war_input, &casus_belli->on_fail,                  table["on_fail"]);
      script::create_effect(war_input, &casus_belli->on_fail_post,             table["on_fail_post"]);
      script::create_effect(war_input, &casus_belli->on_reverse_demand,        table["on_reverse_demand"]);
      script::create_effect(war_input, &casus_belli->on_reverse_demand_post,   table["on_reverse_demand_post"]);
      script::create_effect(war_input, &casus_belli->on_attacker_leader_death, table["on_attacker_leader_death"]);
      script::create_effect(war_input, &casus_belli->on_defender_leader_death, table["on_defender_leader_death"]);
      script::create_effect(war_input, &casus_belli->on_thirdparty_death,      table["on_thirdparty_death"]);
      script::create_effect(war_input, &casus_belli->on_invalidation,          table["on_invalidation"]);
      
      if (!casus_belli->name_script.valid()) throw std::runtime_error("Casus belli " + casus_belli->id + " must have a name script");
      if (!casus_belli->ai_will_do.valid()) throw std::runtime_error("Casus belli " + casus_belli->id + " must have a ai_will_do script");
      if (!casus_belli->is_valid.valid()) throw std::runtime_error("Casus belli " + casus_belli->id + " must have a is_valid script");
      if (!casus_belli->can_use_script.valid()) throw std::runtime_error("Casus belli " + casus_belli->id + " must have a can_use script");
      
//       NUMBER_INIT(authority_cost)
//       NUMBER_INIT(esteem_cost)
//       NUMBER_INIT(influence_cost)
//       NUMBER_INIT(money_cost)
      NUMBER_INIT(battle_warscore_mult)
      NUMBER_INIT(infamy_modifier)
      NUMBER_INIT(ticking_war_score_multiplier)
      NUMBER_INIT(att_ticking_war_score_multiplier)
      NUMBER_INIT(def_ticking_war_score_multiplier)
      NUMBER_INIT(max_defender_occupation_score)
      NUMBER_INIT(max_attacker_occupation_score)
      NUMBER_INIT(max_defender_battle_score)
      NUMBER_INIT(max_attacker_battle_score)
      
      if (const auto proxy = table["truce_turns"]; proxy.valid()) casus_belli->truce_turns = proxy.get<size_t>();
      
      for (size_t i = 0; i < core::casus_belli::count; ++i) {
        const auto enum_name = magic_enum::enum_name(static_cast<core::casus_belli::flags>(i));
        if (const auto proxy = table[enum_name]; proxy.valid()) {
          const bool val = proxy.get<bool>();
          casus_belli->flags_field.set(i, val);
        }
      }
      
      
    }
  }
}
