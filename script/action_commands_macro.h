#ifndef ACTION_COMMANDS_MACRO_H
#define ACTION_COMMANDS_MACRO_H

// пока что добавил функции для персонажа, 
// попытался немного по категориям их раскидать
// сначала идут "легкие" функции персонажа
// затем секреты и "крючки", затем треиты, модификаторы, клеймы, договор о ненападении, 
// фракции (нужно изменить название государства игрока (realm)), статусы отношений и остальное
// статы прибавляются к базовым? если не к базовым то после пересчета они отвалятся
// 

#define ACTION_COMMANDS_LIST \
  ACTION_COMMAND_FUNC(add_money) \
  ACTION_COMMAND_FUNC(add_flag) \
  ACTION_COMMAND_FUNC(marry) \
  \
  ACTION_COMMAND_FUNC(add_hook) \
  \
  ACTION_COMMAND_FUNC(add_trait) \
  ACTION_COMMAND_FUNC(start_war)
  
  
  // перки, у меня наверное это дело будет по другому
//   ACTION_COMMAND_FUNC(add_diplomacy_lifestyle_perk_points)
//   ACTION_COMMAND_FUNC(add_diplomacy_lifestyle_xp)
//   ACTION_COMMAND_FUNC(add_perk)
//   ACTION_COMMAND_FUNC(refund_all_perks) 
//   ACTION_COMMAND_FUNC(refund_perks) 



#endif
