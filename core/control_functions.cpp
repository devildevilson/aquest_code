#include "control_functions.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "context.h"
#include "script/context.h"
#include "utils/lua_initialization_handle_types.h"

namespace devils_engine {
  namespace core {
    template <typename T>
    sol::table get_potential_interactions(sol::this_state s, core::character* actor, T recipient, const enum core::interaction::type type) {
      sol::state_view lua = s;
      
      auto ctx = global::get<systems::map_t>()->core_context;
      const size_t count = ctx->get_entity_count<core::interaction>();
      script::context scr_ctx;
      scr_ctx.root = script::object(actor);
      scr_ctx.prev = script::object(recipient);
      scr_ctx.map["actor"] = script::object(actor);
      scr_ctx.map["recipient"] = script::object(recipient);
      
      auto table = lua.create_table(30, 0);
      for (size_t i = 0; i < count; ++i) {
        auto inter = ctx->get_entity<core::interaction>(i);
        if (inter->type != type) continue;
        if (!inter->potential.compute(&scr_ctx)) continue; // может потенциально изменить контекст, желательно это дело запретить
        
        // как то так
        auto obj = sol::make_object(s, compiled_interaction(inter, scr_ctx)); // тут неполучится перемещать, нужно копировать
        table.add(obj);
      }
      
      return table;
    }
    
    sol::table get_potential_interactions(sol::this_state s, const sol::object &actor, const sol::object &recipient) {
      if (!actor.is<core::character*>()) throw std::runtime_error("Interaction actor object must be a character");
      
      auto ch = actor.as<core::character*>();
           if (recipient.is<core::character*>())                return get_potential_interactions(s, ch, recipient.as<core::character*>(), core::interaction::type::character);
      else if (recipient.is<core::province*>())                 return get_potential_interactions(s, ch, recipient.as<core::province*>(), core::interaction::type::province);
      else if (recipient.is<core::city*>())                     return get_potential_interactions(s, ch, recipient.as<core::city*>(), core::interaction::type::city);
      else if (recipient.is<core::titulus*>())                  return get_potential_interactions(s, ch, recipient.as<core::titulus*>(), core::interaction::type::title);
      else if (recipient.is<utils::lua_handle_realm>()) { 
        const auto h = recipient.as<utils::lua_handle_realm>(); 
        return get_potential_interactions(s, ch, utils::handle<core::realm>(h.ptr, h.token), core::interaction::type::realm); 
      } else if (recipient.is<utils::lua_handle_army>()) {
        const auto h = recipient.as<utils::lua_handle_army>();
        return get_potential_interactions(s, ch, utils::handle<core::army>(h.ptr, h.token), core::interaction::type::army);
      } else if (recipient.is<utils::lua_handle_hero_troop>()) {
        const auto h = recipient.as<utils::lua_handle_hero_troop>();
        return get_potential_interactions(s, ch, utils::handle<core::hero_troop>(h.ptr, h.token), core::interaction::type::hero_troop); 
      } else if (recipient.is<utils::lua_handle_war>()) { 
        const auto h = recipient.as<utils::lua_handle_war>();
        return get_potential_interactions(s, ch, utils::handle<core::war>(h.ptr, h.token), core::interaction::type::war); 
      }
//       else if (recipient.is<core::province*>()) return get_potential_character_interactions(ch, recipient.as<core::character*>());
//       else if (recipient.is<core::province*>()) return get_potential_character_interactions(ch, recipient.as<core::character*>());
//       else if (recipient.is<core::province*>()) return get_potential_character_interactions(ch, recipient.as<core::character*>());
//       else if (recipient.is<core::province*>()) return get_potential_character_interactions(ch, recipient.as<core::character*>());
//       else if (recipient.is<core::province*>()) return get_potential_character_interactions(ch, recipient.as<core::character*>());
      else throw std::runtime_error("Bad recipient object type");
      
      return sol::nil;
    }
    
    sol::table get_potential_decisions(sol::this_state s, const sol::object &actor) {
      if (!actor.is<core::character*>()) throw std::runtime_error("Interaction actor object must be a character");
      sol::state_view lua = s;
      
      auto ch = actor.as<core::character*>();
      auto ctx = global::get<systems::map_t>()->core_context;
      const size_t count = ctx->get_entity_count<core::decision>();
      script::context scr_ctx;
      scr_ctx.root = script::object(ch);
      scr_ctx.map["actor"] = script::object(ch);
      
      auto table = lua.create_table(30, 0);
      for (size_t i = 0; i < count; ++i) {
        auto d = ctx->get_entity<core::decision>(i);
        if (!d->potential.compute(&scr_ctx)) continue;
        
        auto obj = sol::make_object(s, compiled_decision(d, scr_ctx));
        table.add(obj);
      }
      
      return table;
    }
  }
}
