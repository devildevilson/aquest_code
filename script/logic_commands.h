#ifndef DEVILS_ENGINE_SCRIPT_LOGIC_COMMANDS_H
#define DEVILS_ENGINE_SCRIPT_LOGIC_COMMANDS_H

#include "logic_commands_macro.h"
#include "interface.h"
#include "object.h"

namespace devils_engine {
  namespace script {
    struct context;
    
#define LOGIC_BLOCK_COMMAND_FUNC(func_name, expected, output) \
    class func_name : public interface {                  \
    public:                                               \
      static const size_t type_index;                     \
      static const size_t expected_types = expected;      \
      static const size_t output_type = output;           \
      func_name(const interface* childs) noexcept;        \
      ~func_name() noexcept;                              \
      struct object process(context* ctx) const override; \
      void draw(context* ctx) const override;             \
    private:                                              \
      const interface* childs;                            \
    };
    
    LOGIC_BLOCK_COMMANDS_LIST
    
#undef LOGIC_BLOCK_COMMAND_FUNC
  }
}

#endif
