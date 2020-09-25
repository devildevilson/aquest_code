#ifndef QUEST_STATES_H
#define QUEST_STATES_H

#include "quest_state.h"

namespace devils_engine {
  namespace utils {
    class main_menu_state : public quest_state {
    public:
      main_menu_state();
      void enter() override;
      bool load() override;
      void update(const size_t &time) override;
      void clean() override; // чистим главное меню
    };
    
    class map_creation_state : public quest_state {
    public:
      map_creation_state();
      void enter() override;
      bool load() override;
      void update(const size_t &time) override;
      void clean() override;
    };
    
    class world_map_state : public quest_state {
    public:
      world_map_state();
      void enter() override;
      bool load() override;
      void update(const size_t &time) override;
      void clean() override;
    };
    
    class battle_state : public quest_state {
    public:
      battle_state();
      void enter() override;
      bool load() override;
      void update(const size_t &time) override;
      void clean() override;
    };
    
    class encounter_state : public quest_state {
    public:
      encounter_state();
      void enter() override;
      bool load() override;
      void update(const size_t &time) override;
      void clean() override;
    };
  }
}

#endif
