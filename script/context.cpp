#include "context.h"

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
    
    double random_state::normalize(const uint64_t val) {
      return utils::rng_normalize(val);
    }
    
    random_state::random_state() {}
    random_state::random_state(const size_t &val1, const size_t &val2, const size_t &val3, const size_t &val4) :
      cur{ val1, val2, val3, val4 }
    {}
    
    random_state::random_state(const size_t &state_root, const size_t &state_container, const size_t &current_turn) :
      cur{ state_root, state_container, current_turn, utils::splitmix64::get_value(utils::splitmix64::rng({current_turn})) }
    {}
    
    uint64_t random_state::next() {
      using DEFAULT_GENERATOR_NAMESPACE::rng;
      using DEFAULT_GENERATOR_NAMESPACE::get_value;
      cur = rng(cur);
      return get_value(cur);
    }
    
    context::context() noexcept : type(SIZE_MAX), operator_type(SIZE_MAX), nest_level(0) {}
    
    bool context::draw(const draw_data* data) const {
      if (!draw_function) return true;
      return draw_function(data);
    }
  }
}
