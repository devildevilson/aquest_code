#ifndef DEVILS_ENGINE_SCRIPT_LOGIC_COMMANDS_MACRO_H
#define DEVILS_ENGINE_SCRIPT_LOGIC_COMMANDS_MACRO_H

// почему тут названия с большой буквы? потому что часть этих названий присутствует в качестве ключевых слов луа

#define EXPECT script::object::type_bit::valid_boolean
#define OUPUT script::object::type_bit::boolean

#define LOGIC_BLOCK_COMMANDS_LIST               \
  LOGIC_BLOCK_COMMAND_FUNC(AND,  EXPECT, OUPUT) \
  LOGIC_BLOCK_COMMAND_FUNC(OR,   EXPECT, OUPUT) \
  LOGIC_BLOCK_COMMAND_FUNC(NAND, EXPECT, OUPUT) \
  LOGIC_BLOCK_COMMAND_FUNC(NOR,  EXPECT, OUPUT) \
  LOGIC_BLOCK_COMMAND_FUNC(XOR,  EXPECT, OUPUT) \
  LOGIC_BLOCK_COMMAND_FUNC(IMPL, EXPECT, OUPUT) \
  LOGIC_BLOCK_COMMAND_FUNC(EQ,   EXPECT, OUPUT) \
  LOGIC_BLOCK_COMMAND_FUNC(AND_sequence,  EXPECT, OUPUT) \
  LOGIC_BLOCK_COMMAND_FUNC(OR_sequence,   EXPECT, OUPUT) \

#endif
