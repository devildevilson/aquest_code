#ifndef QUEST_STATES_H
#define QUEST_STATES_H

#include "quest_state.h"

namespace sol {
  class state;
}

namespace devils_engine {
  namespace utils {
    class main_menu_loading_state : public quest_state {
    public:
      main_menu_loading_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override;
    };
    
    class main_menu_state : public quest_state {
    public:
      main_menu_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override; // чистим главное меню
    };
    
    class map_creation_loading_state : public quest_state {
    public:
      map_creation_loading_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override;
    };
    
    class map_creation_state : public quest_state {
    public:
      map_creation_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override;
    };
    
    class map_creation_generation_state : public quest_state {
    public:
      map_creation_generation_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override;
    };
    
    class world_map_loading_state : public quest_state {
    public:
      world_map_loading_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override;
    };
    
    class world_map_state : public quest_state {
    public:
      world_map_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override;
    };
    
    class battle_loading_state : public quest_state {
    public:
      struct battle_generator_data;
      
      battle_loading_state();
      ~battle_loading_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override;
      
      void create_state();
      void destroy_state();
    private:
//       sol::state* lua;
      battle_generator_data* ctx;
    };
    
    class battle_state : public quest_state {
    public:
      battle_state();
      ~battle_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override;
    };
    
    class encounter_loading_state : public quest_state {
    public:
      encounter_loading_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override;
    };
    
    class encounter_state : public quest_state {
    public:
      encounter_state();
      void enter(quest_state* prev_state) override;
      uint32_t update(const size_t &time, quest_state* prev_state) override;
      void clean(quest_state* next_state) override;
    };
  }
}

#endif
