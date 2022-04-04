#include "interaction.h"

#include "target_type.h"
#include "structures_header.h"
#include "interaction_filter_functions.h"
#include "utils/utility.h"
#include "utils/lua_initialization_handle_types.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "bin/game_time.h"

#include <iostream>
#include <limits>

#define MAKE_MAP_PAIR(name) std::make_pair(type_names[static_cast<size_t>(interaction::type::name)], interaction::type::name)

namespace devils_engine {
  namespace core {
    bool check_valid_id_string(const std::string_view &str) {
      const size_t dot_index = str.find('.');
      const size_t colon_index = str.find(':');
      return dot_index == std::string_view::npos && colon_index == std::string_view::npos;
    }
    
    const std::string_view type_names[] = {
#define INTERACTION_TYPE_FUNC(name) #name,
        INTERACTION_TYPES_LIST
#undef INTERACTION_TYPE_FUNC
    };
    
    const phmap::flat_hash_map<std::string_view, enum interaction::type> type_map = {
#define INTERACTION_TYPE_FUNC(name) MAKE_MAP_PAIR(name),
        INTERACTION_TYPES_LIST
#undef INTERACTION_TYPE_FUNC
    };
    
    interaction::interaction() : options_count(0) {}
    
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

    script::object get_valid_script_object(const sol::object &obj) {
      const auto t = obj.get_type();
      
#define FIRST_CONDITION(type) if (obj.is<core::type*>()) return script::object(obj.as<core::type*>());
#define COMMON_CONDITION(type) else if (obj.is<core::type*>()) return script::object(obj.as<core::type*>());
#define CONST_CONDITION(type) else if (obj.is<core::type*>()) return script::object(obj.as<const core::type*>());
#define HANDLE_CONDITION(type) else if (obj.is<utils::lua_handle_##type>()) { const auto h = obj.as<utils::lua_handle_##type>(); return script::object(utils::handle<core::type>(h.ptr, h.token)); }
      
      FIRST_CONDITION(character)
//       COMMON_CONDITION(biome)
      COMMON_CONDITION(building_type)
      CONST_CONDITION(holding_type)
      CONST_CONDITION(city_type)
      CONST_CONDITION(trait)
      CONST_CONDITION(modificator)
      CONST_CONDITION(troop_type)
//       GAME_STRUCTURE_FUNC(decision)
//       GAME_STRUCTURE_FUNC(interaction)
      CONST_CONDITION(religion_group)
      COMMON_CONDITION(religion)
      CONST_CONDITION(culture_group)
      COMMON_CONDITION(culture)
      CONST_CONDITION(law)
//       GAME_STRUCTURE_FUNC(event)
      COMMON_CONDITION(titulus)
      CONST_CONDITION(casus_belli)
      COMMON_CONDITION(city)
      COMMON_CONDITION(province)
//       GAME_STRUCTURE_FUNC(tile)
      COMMON_CONDITION(dynasty)
      HANDLE_CONDITION(realm)
      HANDLE_CONDITION(hero_troop)
      HANDLE_CONDITION(troop)
      HANDLE_CONDITION(army)
      HANDLE_CONDITION(war)
      
#undef FIRST_CONDITION
#undef COMMON_CONDITION
#undef CONST_CONDITION
#undef HANDLE_CONDITION
      
      else if (t == sol::type::boolean) return script::object(obj.as<bool>());
      else if (t == sol::type::number) return script::object(obj.as<double>());
      
      throw std::runtime_error("Bad object data");
      
      return script::object();
    }
    
    // тут потенциально могут придти разные таблицы, и нужно чистить мапу
    struct make_context {
      script::context* ctx;
      phmap::flat_hash_map<std::string_view, script::object> mem;
      
      make_context(script::context* ctx, const sol::table &data) : ctx(ctx), mem(ctx->map) {
        for (const auto &pair : data) {
          if (pair.first.get_type() != sol::type::string) continue;
          const auto str = pair.first.as<std::string_view>(); // существуют ли эти строки пока существует дата?
          if (str == "root" || str == "prev" || str == "actor" || str == "recipient") throw std::runtime_error("Rewriting " + std::string(str) + " is not allowed");
          const size_t dot_place = str.find('.');
          const size_t colon_place = str.find(':');
          if (dot_place != std::string_view::npos || colon_place != std::string_view::npos) throw std::runtime_error("Expecting a valid variable name");
          
          if (const auto t = pair.second.get_type(); t != sol::type::boolean && t != sol::type::number && t != sol::type::lightuserdata && t != sol::type::userdata) 
            throw std::runtime_error("Expecting boolean on number or valid object");
          
          const auto obj = get_valid_script_object(pair.second);
          ctx->map[str] = obj;
        }
      }
      
      ~make_context() {
        ctx->map.clear();
        ctx->map = std::move(mem);
      }
    };
    
    struct make_function {
      script::context* ctx;
      
      make_function(script::context* ctx, const sol::function &func) {
        ctx->draw_function = [func] (const script::draw_data* data) -> bool {
          const auto ret = func(data);
          CHECK_ERROR_THROW(ret);
          return ret.get_type() == sol::type::boolean ? ret.get<bool>() : true;
        };
      }
      
      ~make_function() { ctx->draw_function = nullptr; }
    };
    
    struct remember_context {
      script::context* ctx;
      script::context mem;
      
      remember_context(script::context* ctx) : ctx(ctx), mem(*ctx) {
        const size_t current_turn = global::get<systems::core_t>()->game_calendar->current_turn();
        ctx->current_turn = current_turn;
      }
      
      ~remember_context() { *ctx = std::move(mem); }
    };
    
    void setup_context(
      script::context* ctx, 
      const script::object &actor, 
      const script::object &recipient, 
      const script::object &secondary_actor, 
      const script::object &secondary_recipient
    ) {
      ctx->root = actor;
      ctx->prev = recipient;
      ctx->map["actor"] = actor;
      ctx->map["recipient"] = recipient;
      if (secondary_actor.valid() && !secondary_actor.ignore()) ctx->map["secondary_actor"] = secondary_actor;
      if (secondary_recipient.valid() && !secondary_recipient.ignore()) ctx->map["secondary_recipient"] = secondary_recipient;
    }
    
    bool check_variables(const core::interaction* native, script::context* ctx) {
      for (size_t i = 0; i < native->variables.size() && !native->variables[i].name.empty(); ++i) {
        const auto &var = native->variables[i];
        const auto itr = ctx->map.find(var.name);
        if (itr == ctx->map.end()) throw std::runtime_error("Variable " + var.name + " is not set");
        const script::object &obj = itr->second;
        if (!obj.valid() || obj.ignore()) throw std::runtime_error("Variable " + var.name + " is not set");
        
        script::change_scope cs(ctx, obj, script::object());
        const bool ret = var.scripted_filter.compute(ctx);
        if (!ret) return false;
      }
      
      return true;
    }
    
    bool check_cost(const script::number* script, const uint32_t &resource_id, script::context* ctx) {
      if (script == nullptr || !script->valid()) return true;
      const double cost = script->compute(ctx);
      auto c = ctx->root.get<core::character*>();
      const double cur = c->resources.get(resource_id);
      return cur >= cost;
    }
    
    compiled_interaction::compiled_interaction() : id(SIZE_MAX) {}
    compiled_interaction::compiled_interaction(const interaction* native, const script::context &ctx, const size_t &id) : 
      native(native), ctx(ctx), id(id)
    {}
    
    void compiled_interaction::set_variable(const std::string_view &name, const sol::object &obj) {
      for (size_t i = 0; i < native->variables.size() && !native->variables[i].name.empty(); ++i) {
        const auto &var = native->variables[i];
        if (var.name != name) continue;
        
        const auto scr_obj = get_valid_script_object(obj);
        ctx.map[var.name] = scr_obj;
        // тут фильтр пройти? если не по фильтру то что? нужно сделать отдельно отрисовку и проверку фильтра
        return;
      }
      
      // тут вылететь с ошибкой "мол не нашли?", тогда нужно предоставить названия переменных и количество (тип?)
      // потом у меня будет написанный фильтр, почему бы его не использовать? тоже полезно
    }
      
    bool compiled_interaction::is_pending() const { return id != SIZE_MAX; }
    
    // тут не должно быть никаких изменений контекста (то есть не должно быть никаких новых переменных)
    bool compiled_interaction::potential() {
      return native->potential.compute(&ctx);
    }
    
    // здесь могут быть изменения контекста (новые переменные в контексте)
    bool compiled_interaction::condition() {
      remember_context rc(&ctx);
      return native->condition.compute(&ctx);
    }
    
    // эти вещи проверяются уже с обновленной интеракцией
    bool compiled_interaction::auto_accept() {
      remember_context rc(&ctx);
      
      // тут то поди уже не нужны вот эти все проверки
//       const bool condition_ret = native->condition.compute(&ctx);
//       if (!condition_ret) return false;
//       
//       const bool var_ret = check_variables(native, &ctx);
//       if (!var_ret) return false;
//       
//       if (native->can_send.valid()) {
//         const bool ret = native->can_send.compute(&ctx);
//         if (!ret) return false;
//       }
      
      return native->auto_accept.compute(&ctx);
    }
    
    bool compiled_interaction::is_highlighted() {
      remember_context rc(&ctx);
      return native->is_highlighted.compute(&ctx);
    }
    
    bool compiled_interaction::can_send() {
      remember_context rc(&ctx);
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return false;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return false;
      
      {
        const bool ret = check_cost(&native->money_cost, core::character_resources::money, &ctx);
        if (!ret) return false;
      }
      
      {
        const bool ret = check_cost(&native->authority_cost, core::character_resources::authority, &ctx);
        if (!ret) return false;
      }
      
      {
        const bool ret = check_cost(&native->esteem_cost, core::character_resources::esteem, &ctx);
        if (!ret) return false;
      }
      
      {
        const bool ret = check_cost(&native->influence_cost, core::character_resources::influence, &ctx);
        if (!ret) return false;
      }
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        return ret;
      }
      
      return true;
    }
    
    bool compiled_interaction::send() {
      //remember_context rc(&ctx);
      remember_context rc(&ctx);
      
      //script::context ctx(native->id, "on_send", 0);
      //setup_context(&ctx, actor, recipient, secondary_actor, secondary_recipient);
      
      const bool potential_ret = native->potential.compute(&ctx);
      if (!potential_ret) return false;
      
      // кондишен поди проверяется после того как мы установили дополнительные переменные
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return false;
      
      // что тут? тут нужно посчитать все условия, и сделать собственно интеракцию
      // как она делается? нужно запустить некоторые эффекты в зависимости от ответа другой стороны
      // по идее тут нужно сразу запустить on_send (по крайней мере запомнить и сразу запустить)
      // а дальше в зависимости от того что получается из auto_accept нужно либо сразу запустить
      // on_accept либо подождать ответа от другой стороны
      
      // тут вообще то нужно еще проверить дополнительные таргеты
      // через луа действительно можно запрогать все что угодно, но в основном эти вещи будут использоваться 
      // в ии коде, там нужно придумать строго название используемых переменных, например:
      // secondary_actor, secondary_recipient, а надо ли вообще задавать данные через таблицу луа?
      // хороший вопрос, я так понимаю скорее нет чем да

      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return false;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return false;
      }
      
      // стоимость
      if (native->money_cost.valid()) {
        const double cost = native->money_cost.compute(&ctx);
        auto c = ctx.root.get<core::character*>();
        const double cur = c->resources.get(core::character_resources::money);
        if (cur < cost) return false;
        c->resources.add(core::character_resources::money, -cost);
      }
      
      if (native->authority_cost.valid()) {
        const double cost = native->authority_cost.compute(&ctx);
        auto c = ctx.root.get<core::character*>();
        const double cur = c->resources.get(core::character_resources::authority);
        if (cur < cost) return false;
        c->resources.add(core::character_resources::authority, -cost);
      }
      
      if (native->esteem_cost.valid()) {
        const double cost = native->esteem_cost.compute(&ctx);
        auto c = ctx.root.get<core::character*>();
        const double cur = c->resources.get(core::character_resources::esteem);
        if (cur < cost) return false;
        c->resources.add(core::character_resources::esteem, -cost);
      }
      
      if (native->influence_cost.valid()) {
        const double cost = native->influence_cost.compute(&ctx);
        auto c = ctx.root.get<core::character*>();
        const double cur = c->resources.get(core::character_resources::influence);
        if (cur < cost) return false;
        c->resources.add(core::character_resources::influence, -cost);
      }
      
      // тут (точнее наверное немного отложенно, хотя можно и тут) будет вычисляться on_send
      if (native->on_send.valid()) { 
        if (!native->can_send.valid()) throw std::runtime_error("Trying to compute on_send without can_send condition");
        native->on_send.compute(&ctx);
      }
      
      // а auto_accept и accept будет вычислен уже в ходе компа, за это время интеракция может стать невалидной
      // нужно передать интеракцию и текущий контекст в ресипиента
      
      const auto obj = ctx.prev; // должен храниться recipient
      if (obj.get_type() == script::object::type::character) {
        auto c = obj.get<core::character*>();
        c->interactions.push_back({native, std::move(ctx), global::gen_id()});
      }
      
      //const bool auto_accept = native->auto_accept.valid() ? native->auto_accept.compute(&ctx) : false;
      
      return true;
    }
    
    bool compiled_interaction::accept() {
      remember_context rc(&ctx);
      
      // здесь другая сторона должна согласится или отказаться от интеракции
      // по идее после того как дали согласие выкинем это дело из интеракций персонажа
      // что делать если auto_accept == true? по идее нужно посчитать accept где то заранее
      // а нужно ли вывести интерфейс с тем что произошло? наверное, 
      // то есть тут нужно проверить если auto_accept, то выйдем
      
      if (!is_pending()) return false;
      
      size_t index = SIZE_MAX;
      if (ctx.prev.get_type() == script::object::type::character) {
        auto c = ctx.prev.get<core::character*>();
        for (size_t i = 0; i < c->interactions.size(); ++i) {
          if (c->interactions[i].id == id) { index = i; break; }
        }
      }
      
      if (index == SIZE_MAX) throw std::runtime_error("Trying to accept proccessed interaction");
      
      const bool auto_accept = native->auto_accept.valid() ? native->auto_accept.compute(&ctx) : false;
      if (auto_accept) return false;
      
      if (native->on_accept.valid()) { native->on_accept.compute(&ctx); }
      
      if (ctx.prev.get_type() == script::object::type::character) {
        auto c = ctx.prev.get<core::character*>();
        std::swap(c->interactions[index], c->interactions.back());
        c->interactions.pop_back();
      }
      
      // так же нужно сделать так чтобы случайно не запустить это дело дважды
      
      return true;
    }
    
    bool compiled_interaction::decline() {
      if (!is_pending()) return false;
      
      size_t index = SIZE_MAX;
      if (ctx.prev.get_type() == script::object::type::character) {
        auto c = ctx.prev.get<core::character*>();
        for (size_t i = 0; i < c->interactions.size(); ++i) {
          if (c->interactions[i].id == id) { index = i; break; }
        }
      }
      
      if (index == SIZE_MAX) throw std::runtime_error("Trying to decline proccessed interaction");
      
      const bool auto_accept = native->auto_accept.valid() ? native->auto_accept.compute(&ctx) : false;
      if (auto_accept) return false;
      
      if (native->on_decline.valid()) { native->on_decline.compute(&ctx); }
      
      if (ctx.prev.get_type() == script::object::type::character) {
        auto c = ctx.prev.get<core::character*>();
        std::swap(c->interactions[index], c->interactions.back());
        c->interactions.pop_back();
      }
      
      return true;
    }
    
    size_t compiled_interaction::options_count() const { return native->options_count; }
    std::string_view compiled_interaction::get_option_id(const size_t &index) const { 
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      return native->send_options[index].id;
    }
    
    // potential?
    std::string_view compiled_interaction::get_option_name(const size_t &index) {
      remember_context rc(&ctx);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      return native->send_options[index].name_script.compute(&ctx);
    }
    
    std::string_view compiled_interaction::get_option_description(const size_t &index) {
      remember_context rc(&ctx);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      return native->send_options[index].description_script.compute(&ctx);
    }
    
    std::string_view compiled_interaction::get_option_flag(const size_t &index) const {
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      return native->send_options[index].flag;
    }
    
    bool compiled_interaction::get_option_potential(const size_t &index) {
      remember_context rc(&ctx);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      return native->send_options[index].potential.compute(&ctx);
    }
    
    bool compiled_interaction::get_option_condition(const size_t &index) {
      remember_context rc(&ctx);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      const bool potential = native->send_options[index].potential.compute(&ctx);
      if (!potential) return false;
      
      if (native->send_options[index].condition.valid())
        return native->send_options[index].condition.compute(&ctx);
      
      return true;
    }
    
    bool compiled_interaction::get_option_starts_enabled(const size_t &index) {
      remember_context rc(&ctx);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      const bool potential = native->send_options[index].potential.compute(&ctx);
      if (!potential) return false;
      
      if (native->send_options[index].starts_enabled.valid())
        return native->send_options[index].starts_enabled.compute(&ctx);
      
      return false;
    }
    
    bool compiled_interaction::get_option_can_be_changed(const size_t &index) {
      remember_context rc(&ctx);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      const bool potential = native->send_options[index].potential.compute(&ctx);
      if (!potential) return false;
      
      if (native->send_options[index].can_be_changed.valid())
        return native->send_options[index].can_be_changed.compute(&ctx);
      
      return true;
    }
    
    bool compiled_interaction::toggle_option(const size_t &index) {
      remember_context rc(&ctx);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      const bool potential = native->send_options[index].potential.compute(&ctx);
      if (!potential) return false;
      
      bool can_be_changed = true;
      if (native->send_options[index].can_be_changed.valid())
        can_be_changed = native->send_options[index].can_be_changed.compute(&ctx);
      
      if (!can_be_changed) return false;
      
      const std::string_view flag = native->send_options[index].flag;
      const auto obj = ctx.map[flag];
      ctx.map[flag] = script::object(!obj.get<bool>());
      
      return true;
    }
    
    bool compiled_interaction::option_state(const size_t &index) {
      remember_context rc(&ctx);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      const bool potential = native->send_options[index].potential.compute(&ctx);
      if (!potential) return false;
      
      const std::string_view flag = native->send_options[index].flag;
      const auto obj = ctx.map[flag];
      return obj.get<bool>();
    }
    
    void compiled_interaction::draw_option_name(const size_t &index, const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      ctx.set_data(native->id, "option_name");
      native->send_options[index].name_script.draw(&ctx);
    }
    
    void compiled_interaction::draw_option_description(const size_t &index, const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      ctx.set_data(native->id, "option_description");
      native->send_options[index].description_script.draw(&ctx);
    }
    
    void compiled_interaction::draw_option_potential(const size_t &index, const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      ctx.set_data(native->id, "option_potential");
      native->send_options[index].potential.draw(&ctx);
    }
    
    void compiled_interaction::draw_option_condition(const size_t &index, const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      const bool potential = native->send_options[index].potential.compute(&ctx);
      if (!potential) return;
      
      ctx.set_data(native->id, "option_condition");
      native->send_options[index].condition.draw(&ctx);
    }
    
    void compiled_interaction::draw_option_starts_enabled(const size_t &index, const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      const bool potential = native->send_options[index].potential.compute(&ctx);
      if (!potential) return;
      
      ctx.set_data(native->id, "option_starts_enabled");
      native->send_options[index].starts_enabled.draw(&ctx);
    }
    
    void compiled_interaction::draw_option_can_be_changed(const size_t &index, const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      if (index >= native->options_count) 
        throw std::runtime_error("Options count is " + std::to_string(native->options_count) + ", index " + std::to_string(index));
      
      const bool potential = native->send_options[index].potential.compute(&ctx);
      if (!potential) return;
      
      ctx.set_data(native->id, "option_can_be_changed");
      native->send_options[index].can_be_changed.draw(&ctx);
    }
    
    // при каких условиях я могу вычислить стоимость? по идее мне для этого нужно указать все переменные
    // в цк3 от потенциальной жены зависит количество престижа - стоимость (или добавит) женитьбы
    // наверное можно вернуть NAN если вычислить стоимость невозможно (хотя в каких то ситуациях возможно)
    double compiled_interaction::money_cost() {
      remember_context rc(&ctx);
      
      const double dNAN = std::numeric_limits<double>::quiet_NaN();
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return dNAN;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return dNAN;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return dNAN;
      }
      
      return native->money_cost.compute(&ctx);
    }
    
    double compiled_interaction::authority_cost() {
      remember_context rc(&ctx);
      
      const double dNAN = std::numeric_limits<double>::quiet_NaN();
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return dNAN;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return dNAN;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return dNAN;
      }
      
      return native->authority_cost.compute(&ctx);
    }
    
    double compiled_interaction::esteem_cost() {
      remember_context rc(&ctx);
      
      const double dNAN = std::numeric_limits<double>::quiet_NaN();
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return dNAN;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return dNAN;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return dNAN;
      }
      
      return native->esteem_cost.compute(&ctx);
    }
    
    double compiled_interaction::influence_cost() {
      remember_context rc(&ctx);
      
      const double dNAN = std::numeric_limits<double>::quiet_NaN();
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return dNAN;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return dNAN;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return dNAN;
      }
      
      return native->influence_cost.compute(&ctx);
    }
    
    double compiled_interaction::cooldown() {
      remember_context rc(&ctx);
      
      const double dNAN = std::numeric_limits<double>::quiet_NaN();
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return dNAN;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return dNAN;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return dNAN;
      }
      
      return native->cooldown.compute(&ctx);
    }
    
    double compiled_interaction::cooldown_against_recipient() {
      remember_context rc(&ctx);
      
      const double dNAN = std::numeric_limits<double>::quiet_NaN();
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return dNAN;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return dNAN;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return dNAN;
      }
      
      return native->cooldown_against_recipient.compute(&ctx);
    }
    
    // что тут нужно проверить? явно имя должно быть известно еще до условий
    std::string_view compiled_interaction::get_name() {
      remember_context rc(&ctx);
      return native->name_script.compute(&ctx);
    }
    
    // так же и описание
    std::string_view compiled_interaction::get_description() {
      remember_context rc(&ctx);
      return native->description_script.compute(&ctx);
    }
    
    // а вот здесь нужно уже прикинуть отношение другой стороны, а с этим проверить довольно много всего
    std::string_view compiled_interaction::get_on_decline_summary() {
      remember_context rc(&ctx);
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return "";
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return "";
      
      // can_send можно пока не использовать
      
      return native->on_decline_summary.compute(&ctx);
    }
    
    void compiled_interaction::draw_name(const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "name");
      native->name_script.draw(&ctx);
    }
    
    void compiled_interaction::draw_description(const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "description");
      native->description_script.draw(&ctx);
    }
    
    void compiled_interaction::draw_on_decline_summary(const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return;
      
      ctx.set_data(native->id, "on_decline_summary");
      native->on_decline_summary.draw(&ctx);
    }
    
    // может при отрисовки не париться по этому поводу?
    
    void compiled_interaction::draw_potential(const sol::function &func) {
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "potential");
      native->potential.draw(&ctx);
    }
    
    void compiled_interaction::draw_condition(const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "condition");
      native->condition.draw(&ctx);
    }
    
    void compiled_interaction::draw_auto_accept(const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "auto_accept");
      native->auto_accept.draw(&ctx);
    }
    
    void compiled_interaction::draw_is_highlighted(const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "is_highlighted");
      native->is_highlighted.draw(&ctx);
    }
    
    void compiled_interaction::draw_can_send(const sol::function &func) {
      remember_context rc(&ctx);
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;

      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return;
      
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "draw_can_send");
      native->is_highlighted.draw(&ctx);
    }
    
    void compiled_interaction::draw_on_accept(const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      // тут нужно посчитать условия (auto_accept?), хотя может там уже не нужны проверки
      
      ctx.set_data(native->id, "on_accept");
      native->on_accept.draw(&ctx);
    }
    
    void compiled_interaction::draw_on_decline( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "on_decline");
      native->on_decline.draw(&ctx);
    }
    
    void compiled_interaction::draw_on_auto_accept( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "on_auto_accept");
      native->on_auto_accept.draw(&ctx);
    }
    
    // не думаю что это нужно
//     void compiled_interaction::draw_pre_auto_accept( const sol::function &func) {
//       remember_context rc(&ctx);
//       make_function mf(&ctx, func);
//       ctx.set_data(native->id, "pre_auto_accept");
//       native->pre_auto_accept.draw(&ctx);
//     }
    
    void compiled_interaction::draw_on_blocked_effect( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "on_blocked_effect");
      native->on_blocked_effect.draw(&ctx);
    }
    
    void compiled_interaction::draw_on_send( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      const bool potential_ret = native->potential.compute(&ctx);
      if (!potential_ret) return;
      
      // кондишен поди проверяется после того как мы установили дополнительные переменные
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return;
      }
      
      ctx.set_data(native->id, "on_send");
      native->on_send.draw(&ctx);
    }
        
    void compiled_interaction::draw_cooldown( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return;
      }
      
      ctx.set_data(native->id, "cooldown");
      native->cooldown.draw(&ctx);
    }
    
    void compiled_interaction::draw_cooldown_against_recipient( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return;
      }
      
      ctx.set_data(native->id, "cooldown_against_recipient");
      native->cooldown_against_recipient.draw(&ctx);
    }
    
    void compiled_interaction::draw_money_cost( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return;
      }
      
      ctx.set_data(native->id, "money_cost");
      native->money_cost.draw(&ctx);
    }
    
    void compiled_interaction::draw_authority_cost( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return;
      }
      
      ctx.set_data(native->id, "authority_cost");
      native->authority_cost.draw(&ctx);
    }
    
    void compiled_interaction::draw_esteem_cost( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return;
      }

      ctx.set_data(native->id, "esteem_cost");
      native->esteem_cost.draw(&ctx);
    }
    
    void compiled_interaction::draw_influence_cost( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;
      
      const bool var_ret = check_variables(native, &ctx);
      if (!var_ret) return;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return;
      }
      
      ctx.set_data(native->id, "influence_cost");
      native->influence_cost.draw(&ctx);
    }
        
    void compiled_interaction::draw_ai_accept( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      // эти ли проверки ожидаются здесь? не уверен
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;
      
      if (native->can_send.valid()) {
        const bool ret = native->can_send.compute(&ctx);
        if (!ret) return;
      }
      
      ctx.set_data(native->id, "ai_accept");
      native->ai_accept.draw(&ctx);
    }
    
    void compiled_interaction::draw_ai_will_do( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      
      const bool condition_ret = native->condition.compute(&ctx);
      if (!condition_ret) return;
      
      ctx.set_data(native->id, "ai_will_do");
      native->ai_will_do.draw(&ctx);
    }
    
    // ?
    void compiled_interaction::draw_ai_frequency( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "ai_frequency");
      native->ai_frequency.draw(&ctx);
    }
    
    void compiled_interaction::draw_ai_potential( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "ai_potential");
      native->ai_potential.draw(&ctx);
    }
    
    void compiled_interaction::draw_ai_target_quick_check( const sol::function &func) {
      remember_context rc(&ctx);
      make_function mf(&ctx, func);
      ctx.set_data(native->id, "ai_target_quick_check");
      native->ai_target_quick_check.draw(&ctx);
    }
        
    bool validate_interaction(const size_t &index, const sol::table &table) {
      UNUSED_VARIABLE(index);
      size_t counter = 0;
      std::string_view id;
      if (const auto proxy = table["id"]; proxy.valid() && proxy.get_type() == sol::type::string) {
        id = proxy.get<std::string_view>();
      } else { PRINT("Interaction must have an id"); ++counter; return false; }
      
      if (!check_valid_id_string(id)) throw std::runtime_error("String \"" + std::string(id) + "\" is invalid id string");
      
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
      
      if (const auto proxy = table["on_decline_summary"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid on_decline_summary"); ++counter; }
      }
      
      if (const auto proxy = table["type"]; !(proxy.valid() && (proxy.get_type() == sol::type::string || proxy.get_type() == sol::type::number))) {
        PRINT("Interaction " + std::string(id) + " must have a type"); ++counter;
      }
      
      // инпут изменится на список переменных
//       if (const auto proxy = table["input"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
//         PRINT("Interaction " + std::string(id) + " must have an expected input table"); ++counter;
//       }
      
      // должен ли потентиал присутствовать всегда?
      if (const auto proxy = table["potential"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Interaction " + std::string(id) + " must have a potential check script"); ++counter;
      }
      
      if (const auto proxy = table["condition"]; !(proxy.valid() && proxy.get_type() == sol::type::table)) {
        PRINT("Interaction " + std::string(id) + " must have a condition check script"); ++counter;
      }
      
      if (const auto proxy = table["auto_accept"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table && proxy.get_type() != sol::type::boolean) { 
          PRINT("Interaction " + std::string(id) + " must have a valid auto_accept check script"); ++counter;
        }
      }
      
      if (const auto proxy = table["is_highlighted"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table && proxy.get_type() != sol::type::boolean) { 
          PRINT("Interaction " + std::string(id) + " must have a valid is_highlighted check script"); ++counter;
        }
      }
      
      if (const auto proxy = table["should_use_extra_icon"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table && proxy.get_type() != sol::type::boolean) { 
          PRINT("Interaction " + std::string(id) + " must have a valid should_use_extra_icon check script"); ++counter;
        }
      }
      
      if (const auto proxy = table["can_send"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table && proxy.get_type() != sol::type::boolean) { 
          PRINT("Interaction " + std::string(id) + " must have a valid can_send check script"); ++counter;
        }
      }
      
      if (const auto proxy = table["interface_id"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("interface_id must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["send_name"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("send_name must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["highlighted_reason"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("highlighted_reason must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["notification_text"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("notification_text must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["prompt"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("prompt must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["options_heading"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("options_heading must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["reply_item_key"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("reply_item_key must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["pre_answer_yes_key"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("pre_answer_yes_key must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["pre_answer_no_key"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("pre_answer_no_key must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["pre_answer_maybe_key"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("pre_answer_maybe_key must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["pre_answer_maybe_breakdown_key"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("pre_answer_maybe_breakdown_key must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["answer_block_key"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("answer_block_key must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["answer_accept_key"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("answer_accept_key must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["answer_reject_key"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("answer_reject_key must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["answer_acknowledge_key"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::string) { PRINT("answer_acknowledge_key must be a valid string in interaction " + std::string(id)); ++counter; }
      }
      
      if (const auto proxy = table["cooldown"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table && proxy.get_type() != sol::type::number) { 
          PRINT("Interaction " + std::string(id) + " must have a valid cooldown script"); ++counter;
        }
      }
      
      if (const auto proxy = table["cooldown_against_recipient"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table && proxy.get_type() != sol::type::number) { 
          PRINT("Interaction " + std::string(id) + " must have a valid cooldown_against_recipient script"); ++counter;
        }
      }
      
      if (const auto proxy = table["on_send"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid on_send effect script"); ++counter; }
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
      
      if (const auto proxy = table["on_blocked_effect"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid on_blocked_effect effect script"); ++counter; }
      }
      
//       if (const auto proxy = table["pre_auto_accept"]; proxy.valid()) {
//         if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid pre_auto_accept effect script"); ++counter; }
//       }
      
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
      
      if (const auto proxy = table["money_cost"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid money_cost number script"); ++counter; }
      }
      
      if (const auto proxy = table["authority_cost"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid authority_cost number script"); ++counter; }
      }
      
      if (const auto proxy = table["esteem_cost"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid esteem_cost number script"); ++counter; }
      }
      
      if (const auto proxy = table["influence_cost"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid influence_cost number script"); ++counter; }
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
            
            if (!check_valid_id_string(opt_id)) throw std::runtime_error("String \"" + std::string(opt_id) + "\" is invalid id string for option in interaction " + std::string(id));
            
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
            
            if (const auto proxy = opt["starts_enabled"]; proxy.valid()) {
              if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " option " + std::string(opt_id) + " must have a starts_enabled check script"); ++counter; }
            }
            
            if (const auto proxy = opt["can_be_changed"]; proxy.valid()) {
              if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " option " + std::string(opt_id) + " must have a can_be_changed check script"); ++counter; }
            }
            
//             if (const auto proxy = opt["effect"]; proxy.valid()) {
//               if (proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " option " + std::string(opt_id) + " must have a valid effect script"); ++counter; }
//             }
            
            if (const auto proxy = opt["ai_will_do"]; !(proxy.valid() && (proxy.get_type() == sol::type::number || proxy.get_type() == sol::type::table))) {
              PRINT("Interaction " + std::string(id) + " option " + std::string(opt_id) + " must have an ai_will_do number script"); ++counter;
            }
            
            std::string_view var_flag;
            if (const auto proxy = opt["flag"]; proxy.valid() && proxy.get_type() == sol::type::string) {
              var_flag = proxy.get<std::string_view>();
            } else { PRINT("Interaction " + std::string(id) + " variable must have a valid flag"); ++counter; return false; }
            
            if (!check_valid_id_string(var_flag)) throw std::runtime_error("String \"" + std::string(var_flag) + "\" is invalid flag string for option in interaction " + std::string(id));
          }
        }
      }
      
      if (const auto proxy = table["variables"]; proxy.valid()) {
        if (proxy.get_type() != sol::type::number && proxy.get_type() != sol::type::table) { PRINT("Interaction " + std::string(id) + " must have a valid array variables scripts"); ++counter; }
        else {
          const auto t = proxy.get<sol::table>();
          for (const auto &pair : t) {
            if (pair.second.get_type() != sol::type::table) continue;
            
            const auto var = pair.second.as<sol::table>();
            std::string_view var_name;
            if (const auto proxy = var["name"]; proxy.valid() && proxy.get_type() == sol::type::string) {
              var_name = proxy.get<std::string_view>();
            } else { PRINT("Interaction " + std::string(id) + " variable must have a name"); ++counter; return false; }
            
            if (!check_valid_id_string(var_name)) throw std::runtime_error("String \"" + std::string(var_name) + "\" is invalid id string for option in interaction " + std::string(id));
            
            if (const auto proxy = var["type"]; !(proxy.valid() && (proxy.get_type() == sol::type::string || proxy.get_type() == sol::type::number))) {
              PRINT("Interaction " + std::string(id) + " variable " + std::string(var_name) + " must have a valid type"); ++counter;
            }
            
            if (const auto proxy = var["filter"]; !(proxy.valid() && proxy.get_type() == sol::type::string)) {
              PRINT("Interaction " + std::string(id) + " variable " + std::string(var_name) + " must have a valid filter id"); ++counter;
            }
            
            if (const auto proxy = var["scripted_filter"]; !(proxy.valid() && (proxy.get_type() == sol::type::table || proxy.get_type() == sol::type::number))) {
              PRINT("Interaction " + std::string(id) + " variable " + std::string(var_name) + " must have a valid scripted_filter"); ++counter;
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
      
      script::create_condition(inter_input, &interaction->potential,             table["potential"]);
      script::create_condition(inter_input, &interaction->condition,             table["conditions"]);
      script::create_condition(inter_input, &interaction->auto_accept,           table["auto_accept"]);
      script::create_condition(inter_input, &interaction->is_highlighted,        table["is_highlighted"]);
      script::create_condition(inter_input, &interaction->should_use_extra_icon, table["should_use_extra_icon"]);
      script::create_condition(inter_input, &interaction->can_send,              table["can_send"]);
      script::create_condition(inter_input, &interaction->ai_potential,          table["ai_potential"]); // это разве кондишен?
      
      //script::create_effect(inter_input, &interaction->immediate,         table["immediate"]);
      script::create_effect(inter_input, &interaction->on_send,           table["on_send"]);
      script::create_effect(inter_input, &interaction->on_accept,         table["on_accept"]);
      script::create_effect(inter_input, &interaction->on_auto_accept,    table["on_auto_accept"]);
      script::create_effect(inter_input, &interaction->on_decline,        table["on_decline"]);
      //script::create_effect(inter_input, &interaction->pre_auto_accept,   table["pre_auto_accept"]);
      script::create_effect(inter_input, &interaction->on_blocked_effect, table["on_blocked_effect"]);
      
      script::create_number(inter_input, &interaction->ai_accept,    table["ai_accept"]);
      script::create_number(inter_input, &interaction->ai_will_do,   table["ai_will_do"]);
      script::create_number(inter_input, &interaction->ai_frequency, table["ai_frequency"]);
      //script::create_number(inter_input, &interaction->ai_potential, table["ai_potential"]);
      
      script::create_number(inter_input, &interaction->cooldown,    table["cooldown"]);
      script::create_number(inter_input, &interaction->cooldown_against_recipient,   table["cooldown_against_recipient"]);
      script::create_number(inter_input, &interaction->money_cost,     table["money_cost"]);
      script::create_number(inter_input, &interaction->authority_cost, table["authority_cost"]);
      script::create_number(inter_input, &interaction->esteem_cost,    table["esteem_cost"]);
      script::create_number(inter_input, &interaction->influence_cost, table["influence_cost"]);
      
      if (!interaction->name_script.valid()) throw std::runtime_error("Interaction " + interaction->id + " must have name script");
      if (!interaction->potential.valid())   throw std::runtime_error("Interaction " + interaction->id + " must have potential script");
      if (!interaction->condition.valid())   throw std::runtime_error("Interaction " + interaction->id + " must have condition script");
      // необходим ли ai_will_do и ai_potential? не уверен, но в некоторых типах интеракции ии придется игнорировать ai_will_do
      if (!interaction->ai_will_do.valid())  throw std::runtime_error("Interaction " + interaction->id + " must have ai_will_do script");
      
      if (const auto proxy = table["send_options"]; proxy.valid()) {
        size_t counter = 0;
        const auto t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (pair.second.get_type() != sol::type::table) continue;
          if (counter >= core::interaction::max_options_count) 
            throw std::runtime_error("Too many interaction options, maximum is " + std::to_string(core::interaction::max_options_count));
          
          const auto opt = pair.second.as<sol::table>();
          interaction->send_options[counter].id = opt["id"];
          interaction->send_options[counter].flag = opt["flag"];
          script::create_string(   inter_input, &interaction->send_options[counter].name_script,        opt["name"]);
          script::create_string(   inter_input, &interaction->send_options[counter].description_script, opt["description"]);
          script::create_condition(inter_input, &interaction->send_options[counter].potential,          opt["potential"]);
          script::create_condition(inter_input, &interaction->send_options[counter].condition,          opt["condition"]);
//           script::create_effect(   inter_input, &interaction->send_options[counter].effect,             opt["effect"]);
          script::create_number(   inter_input, &interaction->send_options[counter].ai_will_do,         opt["ai_will_do"]);
          script::create_condition(inter_input, &interaction->send_options[counter].starts_enabled,     opt["starts_enabled"]);
          script::create_condition(inter_input, &interaction->send_options[counter].can_be_changed,     opt["can_be_changed"]);
          
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
      
      if (const auto proxy = table["variables"]; proxy.valid()) {
        size_t counter = 0;
        const auto t = proxy.get<sol::table>();
        for (const auto &pair : t) {
          if (pair.second.get_type() != sol::type::table) continue;
          if (counter >= core::interaction::max_variables_count) 
            throw std::runtime_error("Too many interaction variables, maximum is " + std::to_string(core::interaction::max_variables_count));
          
          const auto opt = pair.second.as<sol::table>();
          interaction->variables[counter].name = opt["name"];
          script::create_number(inter_input, &interaction->variables[counter].scripted_filter, opt["scripted_filter"]);
          
          const auto proxy = opt["type"];
          enum interaction::type t;
          if (proxy.get_type() == sol::type::string) {
            const auto str = proxy.get<std::string_view>();
            const auto itr = type_map.find(str);
            if (itr == type_map.end()) throw std::runtime_error("Could not find variable type " + std::string(str));
            t = itr->second;
          } else if (proxy.get_type() == sol::type::number) {
            const size_t lua_index = proxy.get<size_t>();
            const size_t type = FROM_LUA_INDEX(lua_index);
            if (type >= static_cast<size_t>(interaction::type::count)) 
              throw std::runtime_error("Could not parse type number " + std::to_string(lua_index) + " which is more than allowed " + std::to_string(static_cast<size_t>(interaction::type::count)+1));
            t = static_cast<enum interaction::type>(type);
          } else throw std::runtime_error("Bad 'type' variable type");
          
          interaction->variables[counter].type = t;
          
          // и еще найти нужный фильтр
          // как мы делаем фильтр? по идее две функции: итератор + условие
          // первая для ИИ вторая для того чтобы игрок мог задать это дело в луа
          const std::string_view filter_name = opt["filter"].get<std::string_view>();
          switch (interaction->variables[counter].type) {
            case interaction::type::character: {
              const auto itr = character_filters::map.find(filter_name);
              if (itr == character_filters::map.end()) throw std::runtime_error("Could not find filter " + std::string(filter_name) + " in character filters");
              interaction->variables[counter].filter_data.name = character_filters::names[itr->second];
              interaction->variables[counter].filter_data.index = itr->second;
              break;
            }
            case interaction::type::province: {
              const auto itr = province_filters::map.find(filter_name);
              if (itr == province_filters::map.end()) throw std::runtime_error("Could not find filter " + std::string(filter_name) + " in province filters");
              interaction->variables[counter].filter_data.name = province_filters::names[itr->second];
              interaction->variables[counter].filter_data.index = itr->second;
              break;
            }
            case interaction::type::city: {
              const auto itr = city_filters::map.find(filter_name);
              if (itr == city_filters::map.end()) throw std::runtime_error("Could not find filter " + std::string(filter_name) + " in city filters");
              interaction->variables[counter].filter_data.name = city_filters::names[itr->second];
              interaction->variables[counter].filter_data.index = itr->second;
              break;
            }
            case interaction::type::title: {
              const auto itr = title_filters::map.find(filter_name);
              if (itr == title_filters::map.end()) throw std::runtime_error("Could not find filter " + std::string(filter_name) + " in title filters");
              interaction->variables[counter].filter_data.name = title_filters::names[itr->second];
              interaction->variables[counter].filter_data.index = itr->second;
              break;
            }
            case interaction::type::realm: {
              assert(false);
              break;
            }
            case interaction::type::army: {
              assert(false);
              break;
            }
            case interaction::type::hero_troop: {
              assert(false);
              break;
            }
            case interaction::type::war: {
              assert(false);
              break;
            }
            default: throw std::runtime_error("Bad interaction type");
          }
          
          
        }
      }
    }
  }
}
