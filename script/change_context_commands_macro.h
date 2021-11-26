#ifndef DEVILS_ENGINE_SCRIPT_CHANGE_CONTEXT_COMMANDS_MACRO_H
#define DEVILS_ENGINE_SCRIPT_CHANGE_CONTEXT_COMMANDS_MACRO_H

#define INPUT_CHARACTER script::object::type_bit::character
#define INPUT_REALM script::object::type_bit::realm
#define INPUT_VALID_NUMBER script::object::type_bit::valid_number
#define INPUT_NOTHING script::object::type_bit::invalid

#define OUTPUT_REALM script::object::type_bit::realm
#define OUTPUT_NUMBER script::object::type_bit::number
#define OUTPUT_BOOL script::object::type_bit::boolean
#define OUTPUT_NOTHING script::object::type_bit::invalid // или 0? ну функции действительно что то возвращают

#define GET_SCOPE_COMMANDS_LIST \
  GET_SCOPE_COMMAND_FUNC(self_realm, INPUT_CHARACTER, OUTPUT_REALM) \

// оставить рандомного вассала тут? повлияет довольно сильно на реализацию, проверку для сложного объекта мы проводим отдельно при создании
// так что тут совершенно неважно где эта функция окажется в конечном итоге, какие подводные камни в обоих случаях?
// если делать как скоуп, то нам нужно создать дополнительные контейнеры (дополнительный экшон), но при этом меньше писанины
// лан, думаю что пусть так остается
#define CHANGE_CONTEXT_COMMANDS_LIST \
  CHANGE_CONTEXT_COMMAND_FUNC(_vassal, INPUT_REALM | INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_courtier, INPUT_REALM | INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_sibling, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_child, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_brother, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_sister, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_concubine, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_acquaintance, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_good_acquaintance, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_bad_acquaintance, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_pal, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_foe, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_sympathy, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_dislike, INPUT_CHARACTER, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_member, INPUT_REALM, INPUT_NOTHING, OUTPUT_NOTHING) \
  CHANGE_CONTEXT_COMMAND_FUNC(_elector, INPUT_REALM, INPUT_NOTHING, OUTPUT_NOTHING) \
  
// has_vassal отличается от random_vassal и every_vassal тем что он может использоваться только в кондишенах
// а random_vassal и every_vassal только в экшонах (разделить я легко это могу с помощью выходных типов)
// ко всему прочему has_vassal - это явно функция которая меняет контекст в самой себе 
// (ну то есть функция которая последовательно обходит все объекты определенного типа и вызывает для них определенные проверки)
// в свою очередь random_vassal - может быть просто вызван как скоуп (потому что возвращает одно значение)
// а если random_vassal может быть вызван как скоуп, то он еще может использоваться в сложных объектах
// при этом self_realm - это строго функция возвращающая контекст и она должна иметь возможность использоваться в сложных объектах
// вассалов можно использовать с персонажем? или обязательно строго указывать реалм? мы по умолчанию можем иметь ввиду собственный реалм
// has_vassal - по идее возвращает количество вассалов подходящих под условие (там как раз можно указать макимальное количество или просто или в процентах)

// нужно разделить листы, у меня будет много функций вида has_*что-то*, они довольно сильно отличаются от других функций смены контекста

#endif
