#ifndef SCRIPT_BLOCK_COMMANDS_MACRO_H
#define SCRIPT_BLOCK_COMMANDS_MACRO_H

#define CONDITION_SCRIPT_BLOCK_COMMANDS_LIST \
  SCRIPT_BLOCK_COMMAND_FUNC(condition) \
  SCRIPT_BLOCK_COMMAND_FUNC(condition_or) \
  SCRIPT_BLOCK_COMMAND_FUNC(block_default) \
  SCRIPT_BLOCK_COMMAND_FUNC(block_or)
  
#define ACTION_SCRIPT_BLOCK_COMMANDS_LIST \
  SCRIPT_BLOCK_COMMAND_FUNC(action) \
  SCRIPT_BLOCK_COMMAND_FUNC(random_vassal)

//   SCRIPT_BLOCK_COMMAND_FUNC(random_courtier)
//   SCRIPT_BLOCK_COMMAND_FUNC(any_courtier)
//   SCRIPT_BLOCK_COMMAND_FUNC(any_vassal)
//   SCRIPT_BLOCK_COMMAND_FUNC(every_vassal)
  
// к сожалению у нас остаются некоторые функции которые попадают и в CONDITION_SCRIPT_BLOCK_COMMANDS_LIST и в ACTION_SCRIPT_BLOCK_COMMANDS_LIST
// например self_realm, что делать? мешать я их не могу, алиасы вводить? специальный третий тип (нейтральный)? 
// нейтральный тип полезен чтобы не плодить лишние функции, третий тип скорее всего более менее вариант

#endif
