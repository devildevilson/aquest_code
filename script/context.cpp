#include "context.h"

// не нужно
// #include "utils/globals.h"
// #include "bin/game_time.h"

namespace devils_engine {
  namespace script {
    draw_data::draw_data() : type(SIZE_MAX), operator_type(SIZE_MAX), nest_level(0) {}
    draw_data::draw_data(context* ctx) : 
      id(ctx->id), 
      method_name(ctx->method_name), 
      prev_function_name(ctx->prev_function_name), 
      type(ctx->type), 
      operator_type(ctx->operator_type), 
      nest_level(ctx->nest_level), 
      current(ctx->current) 
    {}
    
    void draw_data::set_arg(const uint32_t &index, const std::string_view &name, const object &obj) {
      arguments[index].first = name;
      arguments[index].second = obj;
    }
    
//     double random_state::normalize(const uint64_t val) {
//       return utils::rng_normalize(val);
//     }
//     
//     random_state::random_state() {}
//     random_state::random_state(const size_t &val1, const size_t &val2, const size_t &val3, const size_t &val4) :
//       cur{ val1, val2, val3, val4 }
//     {}
//     
//     random_state::random_state(const size_t &state_root, const size_t &state_container, const size_t &current_turn) :
//       cur{ state_root, state_container, current_turn, utils::splitmix64::get_value(utils::splitmix64::rng({current_turn})) }
//     {}
//     
//     uint64_t random_state::next() {
//       using DEFAULT_GENERATOR_NAMESPACE::rng;
//       using DEFAULT_GENERATOR_NAMESPACE::get_value;
//       cur = rng(cur);
//       return get_value(cur);
//     }
    
    double context::normalize_value(const uint64_t value) {
      return utils::rng_normalize(value);
    }
    
    context::context() noexcept : type(SIZE_MAX), operator_type(SIZE_MAX), nest_level(0), id_hash(0), method_hash(0), current_turn(0) {}
    context::context(const std::string_view &id, const std::string_view &method_name, const size_t &current_turn) noexcept : 
      id(id), 
      method_name(method_name), 
      type(SIZE_MAX), 
      operator_type(SIZE_MAX), 
      nest_level(0),
//       id_hash(string_hash(id)),
//       method_hash(string_hash(method_name)),
      id_hash(std::hash<std::string_view>()(id)), // стандартный хеш использует MurmurHash2 - должен давать хорошие результаты
      method_hash(std::hash<std::string_view>()(method_name)),
      current_turn(current_turn)
    {}
    
    void context::set_data(const std::string_view &id, const std::string_view &method_name, const size_t &current_turn) noexcept {
      this->id = id;
      this->method_name = method_name;
      this->current_turn = current_turn;
      id_hash = std::hash<std::string_view>()(id);
      method_hash = std::hash<std::string_view>()(method_name);
    }
    
    void context::set_data(const std::string_view &id, const std::string_view &method_name) noexcept {
      this->id = id;
      this->method_name = method_name;
      id_hash = std::hash<std::string_view>()(id);
      method_hash = std::hash<std::string_view>()(method_name);
    }
    
    bool context::draw(const draw_data* data) const {
      if (!draw_function) return true;
      return draw_function(data);
    }
    
    // здесь довольно много вычислений, но они суперпростые
    static uint64_t mix_value(const uint64_t &val) {
      return utils::splitmix64::get_value(utils::splitmix64::rng({val}));
    }
    
    uint64_t context::get_random_value(const size_t &static_state) const {
      using DEFAULT_GENERATOR_NAMESPACE::rng;
      using DEFAULT_GENERATOR_NAMESPACE::get_value;
      using state = DEFAULT_GENERATOR_NAMESPACE::state;
      
      // хеши наверное тоже нужно замиксить
      const state s = { mix_value(id_hash), mix_value(method_hash), mix_value(current_turn), static_state };
      return get_value(rng(s));
    }
  }
}
