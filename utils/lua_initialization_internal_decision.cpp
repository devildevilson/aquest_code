#include "lua_initialization_internal.h"

#include "core/decision.h"
#include "core/control_functions.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_decision(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        auto ut = core.new_usertype<core::compiled_decision>(
          "compiled_decision", sol::no_constructor,
          "id", sol::readonly_property([] (const core::compiled_decision* self) -> std::string_view { return self->d->id; }),
          "ai_goal", sol::readonly_property([] (const core::compiled_decision* self) { return self->d->ai_goal; }),
          "major", sol::readonly_property([] (const core::compiled_decision* self) { return self->d->major; })
        );
        
        ut.set_function("get_name", &core::compiled_decision::get_name);
        ut.set_function("get_description", &core::compiled_decision::get_description);
        ut.set_function("get_confirm_text", &core::compiled_decision::get_confirm_text);
        ut.set_function("ai_potential", &core::compiled_decision::ai_potential);
        ut.set_function("potential", &core::compiled_decision::potential);
        ut.set_function("condition", &core::compiled_decision::condition);
        ut.set_function("ai_will_do", &core::compiled_decision::ai_will_do);
        ut.set_function("ai_check_frequency", &core::compiled_decision::ai_check_frequency);
        ut.set_function("run", &core::compiled_decision::run);
        ut.set_function("money_cost", &core::compiled_decision::money_cost);
        ut.set_function("authority_cost", &core::compiled_decision::authority_cost);
        ut.set_function("esteem_cost", &core::compiled_decision::esteem_cost);
        ut.set_function("influence_cost", &core::compiled_decision::influence_cost);
        ut.set_function("draw_name", &core::compiled_decision::draw_name);
        ut.set_function("draw_description", &core::compiled_decision::draw_description);
        ut.set_function("draw_confirm_text", &core::compiled_decision::draw_confirm_text);
        ut.set_function("draw_ai_potential", &core::compiled_decision::draw_ai_potential);
        ut.set_function("draw_potential", &core::compiled_decision::draw_potential);
        ut.set_function("draw_condition", &core::compiled_decision::draw_condition);
        ut.set_function("draw_ai_will_do", &core::compiled_decision::draw_ai_will_do);
        ut.set_function("draw_ai_check_frequency", &core::compiled_decision::draw_ai_check_frequency);
        ut.set_function("draw_effect", &core::compiled_decision::draw_effect);
        ut.set_function("draw_money_cost", &core::compiled_decision::draw_money_cost);
        ut.set_function("draw_authority_cost", &core::compiled_decision::draw_authority_cost);
        ut.set_function("draw_esteem_cost", &core::compiled_decision::draw_esteem_cost);
        ut.set_function("draw_influence_cost", &core::compiled_decision::draw_influence_cost);
        
        core.set_function("get_potential_interactions", &core::get_potential_interactions);
        core.set_function("get_potential_decisions", &core::get_potential_decisions);
      }
    }
  }
}
