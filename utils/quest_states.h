#ifndef QUEST_STATES_H
#define QUEST_STATES_H

#include "quest_state.h"

namespace sol {
  class state;
}

namespace devils_engine {
  namespace utils {
    class main_menu_state : public quest_state {
    public:
      main_menu_state();
      void enter() override;
      bool load(quest_state* prev_state) override;
      void update(const size_t &time) override;
      void clean() override; // чистим главное меню
    };
    
    class map_creation_state : public quest_state {
    public:
      map_creation_state();
      void enter() override;
      bool load(quest_state* prev_state) override;
      void update(const size_t &time) override;
      void clean() override;
    };
    
    class world_map_state : public quest_state {
    public:
      world_map_state();
      void enter() override;
      bool load(quest_state* prev_state) override;
      void update(const size_t &time) override;
      void clean() override;
    };
    
    class battle_state : public quest_state {
    public:
      struct battle_generator_data;
      
      battle_state();
      ~battle_state();
      void enter() override;
      bool load(quest_state* prev_state) override;
      void update(const size_t &time) override;
      void clean() override;
      
      void create_state();
      void destroy_state();
    private:
//       sol::state* lua;
      battle_generator_data* ctx;
    };
    
    class encounter_state : public quest_state {
    public:
      encounter_state();
      void enter() override;
      bool load(quest_state* prev_state) override;
      void update(const size_t &time) override;
      void clean() override;
    };
  }
}

#endif
