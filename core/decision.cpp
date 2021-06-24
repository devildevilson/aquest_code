#include "decision.h"

#include "utils/utility.h"
#include "script/script_block_functions.h"

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
    
    bool decision::check_shown_condition(const script::target &root, const script::target &helper) const {
      // осталось решить что с рнд, рнд нужно каждый раз приводить в дефолтное состояние
      // 
      script::random_state rnd { 5235545 };
      
      script::context ctx{
        {},
        {},
        &rnd,
        nullptr
      };
      
      ctx.array_data.emplace_back(root);
      ctx.array_data.emplace_back(helper);
      
      //return script::condition(root, ctx, potential_count, potential_array) == TRUE_BLOCK;
      return script::condition(root, ctx, 1, &potential) == TRUE_BLOCK;
    }
    
    bool decision::check_condition(const script::context &ctx) const {
      const script::target t = check_input(ctx);
      //return script::condition(t, ctx, condition_count, condition_array) == TRUE_BLOCK;
      return script::condition(t, ctx, 1, &condition) == TRUE_BLOCK;
    }
    
    void decision::iterate_conditions(const script::context &ctx) const {
      const script::target t = check_input(ctx);
      assert(ctx.itr_func != nullptr);
      script::condition(t, ctx, 1, &condition);
    }
    
    void decision::iterate_actions(const script::context &ctx) const {
      const script::target t = check_input(ctx);
      assert(ctx.itr_func != nullptr);
      script::action(t, ctx, 1, &effect);
    }
    
    bool decision::run(const script::context &ctx) const {
      const script::target t = check_input(ctx);
      const bool check = script::condition(t, ctx, 1, &condition) == TRUE_BLOCK;
      if (!check) return false;
      
      //script::action(t, ctx, effect_count, effect_array);
      script::action(t, ctx, 1, &effect);
      return true;
    }
    
    std::string_view decision::get_name(const script::context &ctx) const {
      // пока что вернем строку без условий
      UNUSED_VARIABLE(ctx);
      return name_id;
    }
    
    std::string_view decision::get_description(const script::context &ctx) const {
      UNUSED_VARIABLE(ctx);
      return description_id;
    }
    
    bool decision::check_ai(const script::context &ctx) const {
      UNUSED_VARIABLE(ctx);
      // проверка ии, она пишется отдельным скриптом
      return false;
    }
    
    bool decision::run_ai(const script::context &ctx) const {
      UNUSED_VARIABLE(ctx);
      // я думал что запуск для ии будет чем то отличаться, но вряд ли
      return false;
    }
    
    script::target decision::check_input(const script::context &ctx) const {
      // тут наверное нужно проверить входные данные, как это сделать?
      // входные данные у нас записаны в инпут, и наверное нужно просто проверить типы?
      // то есть
      assert(ctx.array_data.size() == input_count);
      assert(ctx.array_data.size() >= 1);
      for (size_t i = 0; i < input_count; ++i) {
        const bool check = 
          ctx.array_data[i].command_type == input_array[i].command_type && 
          ctx.array_data[i].number_type == input_array[i].number_type && 
          ctx.array_data[i].helper2 == input_array[i].helper2;
          
        if (!check) throw std::runtime_error("Bad " + std::to_string(i+1) + " decision argument");
      }
      
      assert(ctx.array_data[0].command_type == script::command_type::invalid);
      assert(ctx.array_data[0].number_type == script::number_type::object);
      assert(ctx.array_data[0].helper2 < static_cast<uint32_t>(core::structure::count));
      assert(ctx.array_data[0].data != nullptr);
      return {ctx.array_data[0].helper2, ctx.array_data[0].data};
    }
  }
}
