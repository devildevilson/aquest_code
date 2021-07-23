#ifndef SCRIPT_BLOCK_COMMANDS_MACRO_H
#define SCRIPT_BLOCK_COMMANDS_MACRO_H

#define CONDITION_SCRIPT_BLOCK_COMMANDS_LIST \
  SCRIPT_BLOCK_COMMAND_FUNC(condition) \
  SCRIPT_BLOCK_COMMAND_FUNC(has_vassal) \
  SCRIPT_BLOCK_COMMAND_FUNC(AND) \
  SCRIPT_BLOCK_COMMAND_FUNC(OR) \
  SCRIPT_BLOCK_COMMAND_FUNC(NAND) \
  SCRIPT_BLOCK_COMMAND_FUNC(NOR)
  
#define ACTION_SCRIPT_BLOCK_COMMANDS_LIST \
  SCRIPT_BLOCK_COMMAND_FUNC(action) \
  SCRIPT_BLOCK_COMMAND_FUNC(random_vassal) \
  SCRIPT_BLOCK_COMMAND_FUNC(every_vassal)

#define SCRIPT_BLOCK_COMMANDS_LIST \
  SCRIPT_BLOCK_COMMAND_FUNC(common_block) \
  SCRIPT_BLOCK_COMMAND_FUNC(self_realm) \
  SCRIPT_BLOCK_COMMAND_FUNC(prev) \
  SCRIPT_BLOCK_COMMAND_FUNC(root) \
  SCRIPT_BLOCK_COMMAND_FUNC(current)
  
  
  
  //SCRIPT_BLOCK_COMMAND_FUNC(this) // нужно придумать адекватное название, можно наверное использовать current
  
//   SCRIPT_BLOCK_COMMAND_FUNC(random_courtier)
//   SCRIPT_BLOCK_COMMAND_FUNC(any_courtier)
//   SCRIPT_BLOCK_COMMAND_FUNC(any_vassal)
//   SCRIPT_BLOCK_COMMAND_FUNC(every_vassal)
  
// к сожалению у нас остаются некоторые функции которые попадают и в CONDITION_SCRIPT_BLOCK_COMMANDS_LIST и в ACTION_SCRIPT_BLOCK_COMMANDS_LIST
// например self_realm, что делать? мешать я их не могу, алиасы вводить? специальный третий тип (нейтральный)? 
// нейтральный тип полезен чтобы не плодить лишние функции, третий тип скорее всего более менее вариант
  
// если не делать нейтральный тип то нужно придумать разные названия для экшона и для кондитиона
// хотя почему нужно? потому что нужно тогда указывать откуда брать функции
// в принципе несложно
  
// добавил нейтральный тип, в нем будут функции такие как realm, leader, heir, religion, 
// то есть переходы по указателям базовых сущностей у других сущностей в игре
// например realm переходит к собственной стране если она есть (может быть дать рабочее название self_realm?)
  
// у нас тут добавятся функции AND, OR, NAND, NOR (большии буквами)
// это отдельные функции в которых будет учитываться кондишон блок (если он фалс то просто пропускаем эту часть)

#endif
