#ifndef CONDITION_COMMANDS_MACRO_H
#define CONDITION_COMMANDS_MACRO_H

#define CONCAT(a, b) a##b
#define PENALTY_STAT(stat) CONCAT(stat, _penalty)

#define CONDITION_COMMANDS_LIST \
  CONDITION_COMMAND_FUNC(is_ai) \
  CONDITION_COMMAND_FUNC(is_player) \
  CONDITION_COMMAND_FUNC(is_independent) \
  CONDITION_COMMAND_FUNC(is_vassal) \
  CONDITION_COMMAND_FUNC(is_male) \
  CONDITION_COMMAND_FUNC(is_female) \
  CONDITION_COMMAND_FUNC(is_prisoner) \
  CONDITION_COMMAND_FUNC(is_married) \
  CONDITION_COMMAND_FUNC(is_sick) \
  CONDITION_COMMAND_FUNC(is_in_war) \
  CONDITION_COMMAND_FUNC(is_in_society) \
  CONDITION_COMMAND_FUNC(is_hero) \
  CONDITION_COMMAND_FUNC(is_clan_head) \
  CONDITION_COMMAND_FUNC(is_religious_head) \
  CONDITION_COMMAND_FUNC(is_father_alive) \
  CONDITION_COMMAND_FUNC(is_mother_alive) \
  CONDITION_COMMAND_FUNC(has_dynasty) \
  CONDITION_COMMAND_FUNC(can_change_religion) \
  CONDITION_COMMAND_FUNC(can_call_crusade) \
  CONDITION_COMMAND_FUNC(can_grant_title) \
  CONDITION_COMMAND_FUNC(can_marry) \
  CONDITION_COMMAND_FUNC(has_dead_friends) \
  CONDITION_COMMAND_FUNC(has_dead_rivals) \
  CONDITION_COMMAND_FUNC(has_owner) \
  CONDITION_COMMAND_FUNC(has_dead_lovers) \
  CONDITION_COMMAND_FUNC(has_dead_brothers) \
  CONDITION_COMMAND_FUNC(has_dead_sisters) \
  CONDITION_COMMAND_FUNC(has_dead_siblings) \
  CONDITION_COMMAND_FUNC(has_dead_childs) \
  CONDITION_COMMAND_FUNC(has_concubines) \
  CONDITION_COMMAND_FUNC(has_alive_friends) \
  CONDITION_COMMAND_FUNC(has_alive_rivals) \
  CONDITION_COMMAND_FUNC(has_alive_lovers) \
  CONDITION_COMMAND_FUNC(has_alive_brothers) \
  CONDITION_COMMAND_FUNC(has_alive_sisters) \
  CONDITION_COMMAND_FUNC(has_alive_siblings) \
  CONDITION_COMMAND_FUNC(has_alive_childs) \
  CONDITION_COMMAND_FUNC(belongs_to_culture) \
  CONDITION_COMMAND_FUNC(belongs_to_culture_group) \
  CONDITION_COMMAND_FUNC(has_trait) \
  CONDITION_COMMAND_FUNC(has_modificator) \
  CONDITION_COMMAND_FUNC(has_flag) \
  CONDITION_COMMAND_FUNC(has_title) \
  CONDITION_COMMAND_FUNC(has_nickname) \
  CONDITION_COMMAND_FUNC(bit_is_set) \
  CONDITION_COMMAND_FUNC(bit_is_unset) \
  CONDITION_COMMAND_FUNC(realm_has_enacted_law) \
  CONDITION_COMMAND_FUNC(realm_has_law_mechanic) \
  CONDITION_COMMAND_FUNC(is_among_most_powerful_vassals) \
  CONDITION_COMMAND_FUNC(age)
  
//   CONDITION_COMMAND_FUNC(money)
//   CONDITION_COMMAND_FUNC(military)
//   CONDITION_COMMAND_FUNC(managment)
//   CONDITION_COMMAND_FUNC(diplomacy)
//   CONDITION_COMMAND_FUNC(health)
//   CONDITION_COMMAND_FUNC(fertility)
//   CONDITION_COMMAND_FUNC(strength)
//   CONDITION_COMMAND_FUNC(agility)
//   CONDITION_COMMAND_FUNC(intellect)
//   CONDITION_COMMAND_FUNC(military_penalty)
//   CONDITION_COMMAND_FUNC(managment_penalty)
//   CONDITION_COMMAND_FUNC(diplomacy_penalty)
//   CONDITION_COMMAND_FUNC(health_penalty)
//   CONDITION_COMMAND_FUNC(fertility_penalty)  
//   CONDITION_COMMAND_FUNC(strength_penalty)   
//   CONDITION_COMMAND_FUNC(agility_penalty)    
//   CONDITION_COMMAND_FUNC(intellect_penalty)  
//   CONDITION_COMMAND_FUNC(demesne_size)             
//   CONDITION_COMMAND_FUNC(ai_rationality)           
//   CONDITION_COMMAND_FUNC(ai_zeal)                  
//   CONDITION_COMMAND_FUNC(ai_greed)                 
//   CONDITION_COMMAND_FUNC(ai_honor)                 
//   CONDITION_COMMAND_FUNC(ai_ambition)              
//   CONDITION_COMMAND_FUNC(authority_income_mod)     
//   CONDITION_COMMAND_FUNC(esteem_income_mod)        
//   CONDITION_COMMAND_FUNC(influence_income_mod)     
//   CONDITION_COMMAND_FUNC(income_mod)               
//   CONDITION_COMMAND_FUNC(authority_income)         
//   CONDITION_COMMAND_FUNC(esteem_income)            
//   CONDITION_COMMAND_FUNC(influence_income)         
//   CONDITION_COMMAND_FUNC(income)                   
//   CONDITION_COMMAND_FUNC(authority)                
//   CONDITION_COMMAND_FUNC(esteem)                   
//   CONDITION_COMMAND_FUNC(influence)                

  // пока не очень понятно что с этим делать
//   CONDITION_COMMAND_FUNC(culture_flex),            
//   CONDITION_COMMAND_FUNC(religion_flex),           
//   CONDITION_COMMAND_FUNC(assassinate_chance_mod),  
//   CONDITION_COMMAND_FUNC(arrest_chance_mod),       
//   CONDITION_COMMAND_FUNC(plot_power_mod),          
//   CONDITION_COMMAND_FUNC(murder_plot_power_mod),   
//   CONDITION_COMMAND_FUNC(defensive_plot_power_mod),
//   CONDITION_COMMAND_FUNC(plot_discovery_chance),   
  
#endif
