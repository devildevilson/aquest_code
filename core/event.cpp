#include "event.h"

namespace devils_engine {
  namespace core {
    const structure event::s_type;
    event::event() : 
      name_id(SIZE_MAX), 
      description_id(SIZE_MAX), 
      image{GPU_UINT_MAX}, 
      mtth(SIZE_MAX), 
      options_count(0) 
    {}
    
    event::option::option() : 
      name_id(SIZE_MAX), 
      desc_id(SIZE_MAX) 
    {}
    
//     struct current_context {
//       utils::target_data target;
//       bool cond;
//       uint32_t cond_function;
//     };
//     
//     void event::fire(character* c) {
//       std::stack<utils::target_data> target_stack;
//       target_stack.push({utils::target_data::type::character, {c}});
//       std::stack<bool> bool_stack;
//       bool_stack.push(true);
//       
//       bool cond_var = true;
//       for (const auto &opt : conditions) {
//         //if (!cond_var) break;
//         
//         switch (opt.type) {
//           case 0: {
//             const auto &cond = opt.condition;
//             bool_stack.top() = bool_stack.top() && cond->func(target_stack.top(), 123, nullptr);
//             
//             // 
//             
//             break;
//           }
//         }
//       }
//     }
  }
}
