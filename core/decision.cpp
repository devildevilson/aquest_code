#include "decision.h"

#include "utils/utility.h"
#include "structures_header.h"
#include "target_type.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "bin/game_time.h"

#include <iostream>
#include <limits>

namespace devils_engine {
  namespace core {
    const structure decision::s_type;
    //, potential_count(0), potential_array(nullptr), condition_count(0), condition_array(nullptr), effect_count(0), effect_array(nullptr) 
    decision::decision() : ai_goal(false), major(false) {}
    decision::~decision() {
//       delete [] potential_array;
//       delete [] condition_array;
//       delete [] effect_array;
    }
    
    struct remember_context1 {
      script::context* ctx;
      script::context mem;
      
      remember_context1(script::context* ctx) : ctx(ctx), mem(*ctx) {
        const size_t current_turn = global::get<systems::core_t>()->game_calendar->current_turn();
        ctx->current_turn = current_turn;
      }
      
      ~remember_context1() { *ctx = std::move(mem); }
    };
    
    struct make_function1 {
      script::context* ctx;
      
      make_function1(script::context* ctx, const sol::function &func) {
        ctx->draw_function = [func] (const script::draw_data* data) -> bool {
          const auto ret = func(data);
          CHECK_ERROR_THROW(ret);
          return ret.get_type() == sol::type::boolean ? ret.get<bool>() : true;
        };
      }
      
      ~make_function1() { ctx->draw_function = nullptr; }
    };
    
    compiled_decision::compiled_decision(const decision* d, const script::context &ctx) : d(d), ctx(ctx), used(false) {}
    std::string_view compiled_decision::get_name() {
      remember_context1 rc(&ctx);
      return d->name_script.compute(&ctx);
    }
    
    std::string_view compiled_decision::get_description() {
      remember_context1 rc(&ctx);
      return d->description_script.compute(&ctx);
    }
    
    std::string_view compiled_decision::get_confirm_text() {
      remember_context1 rc(&ctx);
      return d->confirm_text.compute(&ctx);
    }
    
    bool compiled_decision::ai_potential() {
      const size_t current_turn = global::get<systems::core_t>()->game_calendar->current_turn();
      ctx.current_turn = current_turn;
      return d->ai_potential.compute(&ctx);
    }
    
    bool compiled_decision::potential() {
      const size_t current_turn = global::get<systems::core_t>()->game_calendar->current_turn();
      ctx.current_turn = current_turn;
      return d->potential.compute(&ctx);
    }
    
    bool compiled_decision::condition() {
      remember_context1 rc(&ctx);
      return d->condition.compute(&ctx);
    }
    
    double compiled_decision::ai_will_do() {
      remember_context1 rc(&ctx);
      return d->ai_will_do.compute(&ctx);
    }
    
    double compiled_decision::ai_check_frequency() {
      remember_context1 rc(&ctx);
      return d->ai_will_do.compute(&ctx);
    }
    
    bool compiled_decision::run() {
      remember_context1 rc(&ctx);
      
      if (used) throw std::runtime_error("Trying to make same decision twice, id: " + d->id);
      
      const bool potential = d->potential.compute(&ctx);
      if (!potential) return false;
      
      const bool condition = d->condition.compute(&ctx);
      if (!condition) return false;
      
      // стоимость
      if (d->money_cost.valid()) {
        const double cost = d->money_cost.compute(&ctx);
        auto c = ctx.root.get<core::character*>();
        const double cur = c->resources.get(core::character_resources::money);
        if (cur < cost) return false;
        c->resources.add(core::character_resources::money, -cost);
      }
      
      if (d->authority_cost.valid()) {
        const double cost = d->authority_cost.compute(&ctx);
        auto c = ctx.root.get<core::character*>();
        const double cur = c->resources.get(core::character_resources::authority);
        if (cur < cost) return false;
        c->resources.add(core::character_resources::authority, -cost);
      }
      
      if (d->esteem_cost.valid()) {
        const double cost = d->esteem_cost.compute(&ctx);
        auto c = ctx.root.get<core::character*>();
        const double cur = c->resources.get(core::character_resources::esteem);
        if (cur < cost) return false;
        c->resources.add(core::character_resources::esteem, -cost);
      }
      
      if (d->influence_cost.valid()) {
        const double cost = d->influence_cost.compute(&ctx);
        auto c = ctx.root.get<core::character*>();
        const double cur = c->resources.get(core::character_resources::influence);
        if (cur < cost) return false;
        c->resources.add(core::character_resources::influence, -cost);
      }
      
      if (d->effect.valid()) d->effect.compute(&ctx);
      
      used = true;
      return true;
    }
    
    double compiled_decision::money_cost() {
      remember_context1 rc(&ctx);
      
      const double dNAN = std::numeric_limits<double>::quiet_NaN();
      
      const bool potential = d->potential.compute(&ctx);
      if (!potential) return dNAN;
      
      const bool condition = d->condition.compute(&ctx);
      if (!condition) return dNAN;
      
      if (!d->money_cost.valid()) return 0.0;
      return d->money_cost.compute(&ctx);
    }
    
    double compiled_decision::authority_cost() {
      remember_context1 rc(&ctx);
      
      const double dNAN = std::numeric_limits<double>::quiet_NaN();

      const bool potential = d->potential.compute(&ctx);
      if (!potential) return dNAN;
      
      const bool condition = d->condition.compute(&ctx);
      if (!condition) return dNAN;
      
      if (!d->authority_cost.valid()) return 0.0;
      return d->authority_cost.compute(&ctx);
    }
    
    double compiled_decision::esteem_cost() {
      remember_context1 rc(&ctx);
      
      const double dNAN = std::numeric_limits<double>::quiet_NaN();
      
      const bool potential = d->potential.compute(&ctx);
      if (!potential) return dNAN;
      
      const bool condition = d->condition.compute(&ctx);
      if (!condition) return dNAN;
      
      if (!d->esteem_cost.valid()) return 0.0;
      return d->esteem_cost.compute(&ctx);
    }
    
    double compiled_decision::influence_cost() {
      remember_context1 rc(&ctx);
      
      const double dNAN = std::numeric_limits<double>::quiet_NaN();

      const bool potential = d->potential.compute(&ctx);
      if (!potential) return dNAN;
      
      const bool condition = d->condition.compute(&ctx);
      if (!condition) return dNAN;
      
      if (!d->influence_cost.valid()) return 0.0;
      return d->influence_cost.compute(&ctx);
    }
    
    void compiled_decision::draw_name(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "name");
      d->name_script.draw(&ctx);
    }
    
    void compiled_decision::draw_description(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "description");
      d->description_script.draw(&ctx);
    }
    
    void compiled_decision::draw_confirm_text(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "confirm_text");
      d->confirm_text.draw(&ctx);
    }
    
    void compiled_decision::draw_ai_potential(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "ai_potential");
      d->ai_potential.draw(&ctx);
    }
    
    void compiled_decision::draw_potential(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "potential");
      d->potential.draw(&ctx);
    }
    
    void compiled_decision::draw_condition(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "condition");
      d->condition.draw(&ctx);
    }
    
    void compiled_decision::draw_ai_will_do(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "ai_will_do");
      d->ai_will_do.draw(&ctx);
    }
    
    void compiled_decision::draw_ai_check_frequency(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "ai_check_frequency");
      d->ai_check_frequency.draw(&ctx);
    }
    
    void compiled_decision::draw_effect(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "effect");
      d->effect.draw(&ctx);
    }
    
    void compiled_decision::draw_money_cost(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "money_cost");
      d->money_cost.draw(&ctx);
    }
    
    void compiled_decision::draw_authority_cost(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "authority_cost");
      d->authority_cost.draw(&ctx);
    }
    
    void compiled_decision::draw_esteem_cost(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "esteem_cost");
      d->esteem_cost.draw(&ctx);
    }
    
    void compiled_decision::draw_influence_cost(const sol::function &func) {
      remember_context1 rc(&ctx);
      make_function1 mf(&ctx, func);
      
      ctx.set_data(d->id, "influence_cost");
      d->influence_cost.draw(&ctx);
    }
    
    bool validate_decision(const size_t &index, const sol::table &table) {
      UNUSED_VARIABLE(index);
      size_t counter = 0;
      std::string_view id;
      if (const auto proxy = table["id"]; proxy.valid() && proxy.get_type() == sol::type::string) {
        id = proxy.get<std::string_view>();
      } else { PRINT("Decision must have an id"); ++counter; return false; }
      
      if (const auto proxy = table["name"]; !(proxy.valid() && (proxy.get_type() == sol::type::string || proxy.get_type() == sol::type::table))) {
        PRINT("Decision " + std::string(id) + " must have a name"); ++counter;
      }
      
      if (const auto proxy = table["description"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string && proxy.get_type() != sol::type::table) { PRINT("Decision " + std::string(id) + " must have a valid description"); ++counter; }
      }
      
      if (const auto proxy = table["confirm_text"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string && proxy.get_type() != sol::type::table) { PRINT("Decision " + std::string(id) + " must have a valid confirm_text"); ++counter; }
      }
      
      // является ли обязательным?
      if (const auto proxy = table["ai_potential"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have a ai_potential check script"); ++counter;
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
      
      if (const auto proxy = table["ai_will_do"]; !proxy.valid() || (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have a valid ai_will_do number script"); ++counter;
      }
      
      if (const auto proxy = table["ai_check_frequency"]; !proxy.valid() || (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have a valid ai_check_frequency number script"); ++counter;
      }
      
      if (const auto proxy = table["money_cost"]; !proxy.valid() || (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have a valid money_cost number script"); ++counter;
      }
      
      if (const auto proxy = table["authority_cost"]; !proxy.valid() || (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have a valid authority_cost number script"); ++counter;
      }
      
      if (const auto proxy = table["esteem_cost"]; !proxy.valid() || (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have a valid esteem_cost number script"); ++counter;
      }
      
      if (const auto proxy = table["influence_cost"]; !proxy.valid() || (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have a valid influence_cost number script"); ++counter;
      }
      
      if (const auto proxy = table["cooldown"]; !proxy.valid() || (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table)) {
        PRINT("Decision " + std::string(id) + " must have a valid cooldown number script"); ++counter;
      }
      
      return counter == 0;
    }
    
    void parse_decision(core::decision* decision, const sol::table &table) {
      decision->id = table["id"];
      
      script::input_data inter_input;
      inter_input.current = inter_input.root = script::object::type_bit::character;
      
      script::create_string(inter_input, &decision->name_script, table["name"]);
      script::create_string(inter_input, &decision->description_script, table["description"]);
      script::create_string(inter_input, &decision->confirm_text, table["confirm_text"]);
      
      script::create_condition(inter_input, &decision->ai_potential, table["ai_potential"]);
      script::create_condition(inter_input, &decision->potential, table["potential"]);
      script::create_condition(inter_input, &decision->condition, table["condition"]);
      script::create_effect(inter_input, &decision->effect, table["effect"]);
      script::create_number(inter_input, &decision->ai_will_do, table["ai_will_do"]);
      script::create_number(inter_input, &decision->ai_check_frequency, table["ai_check_frequency"]);
      
      script::create_number(inter_input, &decision->money_cost, table["money_cost"]);
      script::create_number(inter_input, &decision->authority_cost, table["authority_cost"]);
      script::create_number(inter_input, &decision->esteem_cost, table["esteem_cost"]);
      script::create_number(inter_input, &decision->influence_cost, table["influence_cost"]);
      
      script::create_number(inter_input, &decision->cooldown, table["cooldown"]);
      
      if (!decision->name_script.valid()) throw std::runtime_error("Decision " + decision->id + " must have name script");
      if (!decision->potential.valid())   throw std::runtime_error("Decision " + decision->id + " must have potential script");
      if (!decision->condition.valid())   throw std::runtime_error("Decision " + decision->id + " must have condition script");
      if (!decision->effect.valid())      throw std::runtime_error("Decision " + decision->id + " must have effect script");
      if (!decision->ai_will_do.valid())  throw std::runtime_error("Decision " + decision->id + " must have ai_will_do script");
    }
  }
}
