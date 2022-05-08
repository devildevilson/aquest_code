#ifndef DEVILS_ENGINE_GAME_LOGIC_H
#define DEVILS_ENGINE_GAME_LOGIC_H

#include <atomic>
#include <mutex>
#include <string>
#include "dsml.hpp"

#define TURN_STATES_LIST \
  TURN_STATE_FUNC(preparing_turn)       \
  TURN_STATE_FUNC(ai_turn)              \
  TURN_STATE_FUNC(player_turn)          \
  TURN_STATE_FUNC(need_player_decision) \

#define TURN_EVENTS_LIST \
  TURN_EVENT_FUNC(advance)  \
  TURN_EVENT_FUNC(end_turn) \
  
namespace devils_engine {
  namespace core {
    struct character;
    struct compiled_decision;
    struct compiled_interaction;
    struct city;
    struct army;
    struct hero_troop;
  }
  
  namespace game {
    // где хранить?
    //template <typename decl>
    struct turn_status {
      struct state_decl;
      
      enum class state {
        initial,
#define TURN_STATE_FUNC(name) name,
        TURN_STATES_LIST
#undef TURN_STATE_FUNC
      };
      
      size_t player_index;
      core::character* current_player;
      mutable std::mutex mutex;
      dsml::Sm<state_decl, turn_status> m_sm;
      
      enum state state;
      
      turn_status();
      void set_state(const enum state s);
      enum state get_state() const;
    };
    
    bool can_end_turn();
    void end_turn();
    
//     void update_player(core::character* c);
//     core::character* get_player();
//     bool current_player_turn();
//     bool player_end_turn();
//     void advance_state();
//     void advance_generator();
    
    // 4 типа того что происходит? решение/взаимодействие, строительство, передвижение, нападение
    
    // запустить решение мы можем только в ходе игрока, соответсвенно здесь нужно убедиться что сейчас ход игрока и решение взято именно для него
    bool can_run_decision(const core::compiled_decision* d, std::string &err_str);
    bool run_decision(core::compiled_decision* d);
    
    bool can_send_interaction(const core::compiled_interaction* i, std::string &err_str);
    bool send_interaction(core::compiled_interaction* i);
    
    bool can_construct_building(const core::city* c, const uint32_t &building_index, std::string &err_str);
    bool construct_building(core::city* c, const uint32_t &building_index);
    
    bool can_move_unit(const core::army* a, const uint32_t &end_tile_index, std::string &err_str);
    bool move_unit(core::army* a, const uint32_t &end_tile_index);
    
    bool can_move_unit(const core::hero_troop* h, const uint32_t &end_tile_index, std::string &err_str);
    bool move_unit(core::hero_troop* h, const uint32_t &end_tile_index);
  
    // что такое атака? если у нас юнит стоит на тайле рядом с целью и пытается перейти на тайл с целью
    // если цель уже занята, то что? армия остается на своем тайле и может быть использована как подкрепление в битве
    bool can_attack(const core::army* a, const core::city* ec); // армия атакует город -> армия переходит в состояние пендинг_баттл и город переходит в состояние пендинг_баттл
    bool can_attack(const core::army* a, const core::army* ea); // армия атакует армию -> пендинг_баттл для обеих армий
    bool can_attack(const core::army* a, const core::hero_troop* eh); // армия атакует героя -> нам либо нужен какой то выбор, либо армия просто прогоняет героя на соседнюю клетку
    
    bool can_attack(const core::hero_troop* h, const core::city* ec);
    bool can_attack(const core::hero_troop* h, const core::army* ea);
    bool can_attack(const core::hero_troop* h, const core::hero_troop* eh);
    
    bool attack(core::army* a, core::city* ec);
    bool attack(core::army* a, core::army* ea);
    bool attack(core::army* a, core::hero_troop* eh);
    
    bool attack(core::hero_troop* h, core::city* ec);
    bool attack(core::hero_troop* h, core::army* ea);
    bool attack(core::hero_troop* h, core::hero_troop* eh);
    
    
  }
}

#endif

// где то тут надо бы расположить все геймплейные функции, для которых нужно подтвержение
// хода + возможно передача каких то данных по сети, какие функции это могут быть:
// перемещение войск, выбор ответа в эвенте, какое то решение, изменение закона,
// женитьба, дипломатия (начало войны например), 
