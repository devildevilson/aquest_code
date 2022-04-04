#ifndef DEVILS_ENGINE_SCRIPT_NUMERIC_COMMANDS_MACRO_H
#define DEVILS_ENGINE_SCRIPT_NUMERIC_COMMANDS_MACRO_H

/*
#define NUMERIC_COMMANDS_LIST \
  NUMERIC_COMMAND_FUNC(add) \
  NUMERIC_COMMAND_FUNC(sub) \
  NUMERIC_COMMAND_FUNC(multiply) \
  NUMERIC_COMMAND_FUNC(divide) \
  NUMERIC_COMMAND_FUNC(mod) \
  NUMERIC_COMMAND_FUNC(min) \
  NUMERIC_COMMAND_FUNC(max) \
  NUMERIC_COMMAND_FUNC(sin) \
  NUMERIC_COMMAND_FUNC(cos) \
  NUMERIC_COMMAND_FUNC(abs) \
  NUMERIC_COMMAND_FUNC(ceil) \
  NUMERIC_COMMAND_FUNC(floor) \
  NUMERIC_COMMAND_FUNC(sqrt) \
  NUMERIC_COMMAND_FUNC(sqr) \
  NUMERIC_COMMAND_FUNC(frac) \
  
*/

// тут еще должны добавиться функции для получения значений из отношений между персонажами

#define NUMBER_OBJECT_TYPE_BITS (script::object::type_bit::boolean | script::object::type_bit::number)
#define EXPECTED_DATA NUMBER_OBJECT_TYPE_BITS
#define OUTPUT_DATA script::object::type_bit::number

// имеет смысл сделать sequence версии блочных функций (производим операцию до первого ignore)
// тоже самое в логике
#define NUMERIC_COMMANDS_LIST2 \
  NUMERIC_COMMAND_BLOCK_FUNC(add, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_BLOCK_FUNC(multiply, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_BLOCK_FUNC(sub, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_BLOCK_FUNC(divide, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_BLOCK_FUNC(mod, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_BLOCK_FUNC(min, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_BLOCK_FUNC(max, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_BLOCK_FUNC(add_sequence, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_BLOCK_FUNC(multiply_sequence, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_BLOCK_FUNC(min_sequence, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_BLOCK_FUNC(max_sequence, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_FUNC(sin, EXPECTED_DATA, OUTPUT_DATA)   \
  NUMERIC_COMMAND_FUNC(cos, EXPECTED_DATA, OUTPUT_DATA)   \
  NUMERIC_COMMAND_FUNC(abs, EXPECTED_DATA, OUTPUT_DATA)   \
  NUMERIC_COMMAND_FUNC(ceil, EXPECTED_DATA, OUTPUT_DATA)  \
  NUMERIC_COMMAND_FUNC(floor, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_FUNC(round, EXPECTED_DATA, OUTPUT_DATA) \
  NUMERIC_COMMAND_FUNC(sqrt, EXPECTED_DATA, OUTPUT_DATA)  \
  NUMERIC_COMMAND_FUNC(sqr, EXPECTED_DATA, OUTPUT_DATA)   \
  NUMERIC_COMMAND_FUNC(frac, EXPECTED_DATA, OUTPUT_DATA)  \
  NUMERIC_COMMAND_FUNC(inv, EXPECTED_DATA, OUTPUT_DATA)   \
  
#endif
