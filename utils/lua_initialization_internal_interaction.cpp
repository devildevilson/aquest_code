#include "lua_initialization_internal.h"

#include "core/interaction.h"

namespace devils_engine {
  namespace utils {
    namespace internal {
      void setup_lua_interaction(sol::state_view lua) {
        auto core = lua["core"].get_or_create<sol::table>();
        auto ut = core.new_usertype<core::compiled_interaction>(
          "compiled_interaction", sol::no_constructor,
          "id", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->id; },
          "interface_id", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->interface_id; },
          "send_name", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->send_name; },
          "highlighted_reason", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->highlighted_reason; },
          "notification_text", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->notification_text; },
          "prompt", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->prompt; },
          "options_heading", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->options_heading; },
          "reply_item_key", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->reply_item_key; },
          "pre_answer_yes_key", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->pre_answer_yes_key; },
          "pre_answer_no_key", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->pre_answer_no_key; },
          "pre_answer_maybe_key", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->pre_answer_maybe_key; },
          "pre_answer_maybe_breakdown_key", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->pre_answer_maybe_breakdown_key; },
          "answer_block_key", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->answer_block_key; },
          "answer_accept_key", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->answer_accept_key; },
          "answer_reject_key", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->answer_reject_key; },
          "answer_acknowledge_key", [] (const core::compiled_interaction* self) -> std::string_view { return self->native->answer_acknowledge_key; },
          "type", sol::readonly_property([] (const core::compiled_interaction* self) { return TO_LUA_INDEX(static_cast<size_t>(self->native->type)); }),
          "get_flag", [] (const core::compiled_interaction* self, const size_t &flag) { return self->native->flag_container.get(FROM_LUA_INDEX(flag)); }
          //"ai_target_type", [] (const core::compiled_interaction* self) { return TO_LUA_INDEX(static_cast<size_t>(self->native->ai_target_type)); }
        );
        
        ut.set_function("set_variable", &core::compiled_interaction::set_variable);
        ut.set_function("is_pending", &core::compiled_interaction::is_pending);
        ut.set_function("potential", &core::compiled_interaction::potential);
        ut.set_function("condition", &core::compiled_interaction::condition);
        ut.set_function("auto_accept", &core::compiled_interaction::auto_accept);
        ut.set_function("is_highlighted", &core::compiled_interaction::is_highlighted);
        ut.set_function("can_send", &core::compiled_interaction::can_send);
        ut.set_function("send", &core::compiled_interaction::send);
        ut.set_function("accept", &core::compiled_interaction::accept);
        ut.set_function("decline", &core::compiled_interaction::decline);
        ut.set_function("options_count", &core::compiled_interaction::options_count);
        ut.set_function("money_cost", &core::compiled_interaction::money_cost);
        ut.set_function("authority_cost", &core::compiled_interaction::authority_cost);
        ut.set_function("esteem_cost", &core::compiled_interaction::esteem_cost);
        ut.set_function("influence_cost", &core::compiled_interaction::influence_cost);
        ut.set_function("cooldown", &core::compiled_interaction::cooldown);
        ut.set_function("cooldown_against_recipient", &core::compiled_interaction::cooldown_against_recipient);
        ut.set_function("get_name", &core::compiled_interaction::get_name);
        ut.set_function("get_description", &core::compiled_interaction::get_description);
        ut.set_function("get_on_decline_summary", &core::compiled_interaction::get_on_decline_summary);
        ut.set_function("draw_name", &core::compiled_interaction::draw_name);
        ut.set_function("draw_description", &core::compiled_interaction::draw_description);
        ut.set_function("draw_on_decline_summary", &core::compiled_interaction::draw_on_decline_summary);
        ut.set_function("draw_potential", &core::compiled_interaction::draw_potential);
        ut.set_function("draw_condition", &core::compiled_interaction::draw_condition);
        ut.set_function("draw_auto_accept", &core::compiled_interaction::draw_auto_accept);
        ut.set_function("draw_is_highlighted", &core::compiled_interaction::draw_is_highlighted);
        ut.set_function("draw_can_send", &core::compiled_interaction::draw_can_send);
        ut.set_function("draw_on_accept", &core::compiled_interaction::draw_on_accept);
        ut.set_function("draw_on_decline", &core::compiled_interaction::draw_on_decline);
        ut.set_function("draw_on_auto_accept", &core::compiled_interaction::draw_on_auto_accept);
//         ut.set_function("draw_pre_auto_accept", &core::compiled_interaction::draw_pre_auto_accept);
        ut.set_function("draw_on_blocked_effect", &core::compiled_interaction::draw_on_blocked_effect);
        ut.set_function("draw_on_send", &core::compiled_interaction::draw_on_send);
        ut.set_function("draw_cooldown", &core::compiled_interaction::draw_cooldown);
        ut.set_function("draw_cooldown_against_recipient", &core::compiled_interaction::draw_cooldown_against_recipient);
        ut.set_function("draw_money_cost", &core::compiled_interaction::draw_money_cost);
        ut.set_function("draw_authority_cost", &core::compiled_interaction::draw_authority_cost);
        ut.set_function("draw_esteem_cost", &core::compiled_interaction::draw_esteem_cost);
        ut.set_function("draw_influence_cost", &core::compiled_interaction::draw_influence_cost);
        ut.set_function("draw_ai_accept", &core::compiled_interaction::draw_ai_accept);
        ut.set_function("draw_ai_will_do", &core::compiled_interaction::draw_ai_will_do);
        ut.set_function("draw_ai_frequency", &core::compiled_interaction::draw_ai_frequency);
        ut.set_function("draw_ai_potential", &core::compiled_interaction::draw_ai_potential);
        ut.set_function("draw_ai_target_quick_check", &core::compiled_interaction::draw_ai_target_quick_check);
        ut.set_function("get_option_id", [] (const core::compiled_interaction* self, const size_t &index) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->get_option_id(final_index);
        });
        ut.set_function("get_option_name", [] (core::compiled_interaction* self, const size_t &index) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->get_option_name(final_index);
        });
        ut.set_function("get_option_description", [] (core::compiled_interaction* self, const size_t &index) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->get_option_description(final_index);
        });
        ut.set_function("get_option_flag", [] (const core::compiled_interaction* self, const size_t &index) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->get_option_flag(final_index);
        });
        ut.set_function("get_option_potential", [] (core::compiled_interaction* self, const size_t &index) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->get_option_potential(final_index);
        });
        ut.set_function("get_option_condition", [] (core::compiled_interaction* self, const size_t &index) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->get_option_condition(final_index);
        });
        ut.set_function("get_option_starts_enabled", [] (core::compiled_interaction* self, const size_t &index) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->get_option_starts_enabled(final_index);
        });
        ut.set_function("get_option_can_be_changed", [] (core::compiled_interaction* self, const size_t &index) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->get_option_can_be_changed(final_index);
        });
        ut.set_function("toggle_option", [] (core::compiled_interaction* self, const size_t &index) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->toggle_option(final_index);
        });
        ut.set_function("option_state", [] (core::compiled_interaction* self, const size_t &index) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->option_state(final_index);
        });
        ut.set_function("draw_option_name", [] (core::compiled_interaction* self, const size_t &index, const sol::function &func) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->draw_option_name(final_index, func);
        });
        ut.set_function("draw_option_description", [] (core::compiled_interaction* self, const size_t &index, const sol::function &func) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->draw_option_description(final_index, func);
        });
        ut.set_function("draw_option_potential", [] (core::compiled_interaction* self, const size_t &index, const sol::function &func) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->draw_option_potential(final_index, func);
        });
        ut.set_function("draw_option_condition", [] (core::compiled_interaction* self, const size_t &index, const sol::function &func) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->draw_option_condition(final_index, func);
        });
        ut.set_function("draw_option_starts_enabled", [] (core::compiled_interaction* self, const size_t &index, const sol::function &func) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->draw_option_starts_enabled(final_index, func);
        });
        ut.set_function("draw_option_can_be_changed", [] (core::compiled_interaction* self, const size_t &index, const sol::function &func) {
          const size_t final_index = FROM_LUA_INDEX(index);
          return self->draw_option_can_be_changed(final_index, func);
        });
        
        auto int_t = core["interaction"].get_or_create<sol::table>();
        int_t.new_enum(
          "flags",
          {
#define INTERACTION_FLAG_FUNC(name) std::make_pair(std::string_view(#name), static_cast<size_t>(core::interaction::flags::name)+1),
            INTERACTION_FLAGS_LIST
#undef INTERACTION_FLAG_FUNC
          }
        );
        
        int_t.new_enum(
          "types",
          {
#define INTERACTION_TYPE_FUNC(name) std::make_pair(std::string_view(#name), static_cast<size_t>(core::interaction::type::name)+1),
            INTERACTION_TYPES_LIST
#undef INTERACTION_TYPE_FUNC
          }
        );
        
        int_t.new_enum(
          "ai_target_type",
          {
#define INTERACTION_AI_TARGET_TYPE_FUNC(name) std::make_pair(std::string_view(#name), static_cast<size_t>(core::interaction::ai_target_type::name)+1),
            INTERACTION_AI_TARGET_TYPES_LIST
#undef INTERACTION_AI_TARGET_TYPE_FUNC
          }
        );
      }
      
    }
  }
}
