#ifndef DEVILS_ENGINE_SCRIPT_GET_SCOPE_COMMANDS_H
#define DEVILS_ENGINE_SCRIPT_GET_SCOPE_COMMANDS_H

#include "object.h"
#include "interface.h"
#include "get_scope_commands_macro.h"

namespace devils_engine {
  namespace script {
    struct context;
    
#define GET_SCOPE_COMMAND_FUNC(func_name, context_types_bits, output_type_bit, unused) \
    class func_name final : public interface {                \
    public:                                                   \
      static const size_t type_index;                         \
      static const size_t context_types = context_types_bits; \
      static const size_t output_type = output_type_bit;      \
      struct object process(context* ctx) const override;     \
      void draw(context* ctx) const override;                 \
    };
    
    GET_SCOPE_COMMANDS_FINAL_LIST
    
#undef GET_SCOPE_COMMAND_FUNC
  }
}

#endif
