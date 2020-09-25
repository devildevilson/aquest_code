#ifndef DEMIURGE_H
#define DEMIURGE_H

#include <vector>
#include <string>
#include "sol.h"

namespace devils_engine {
  namespace utils {
    struct interface_container;
    
    class demiurge {
    public:
//       struct world_data {
//         std::string name;
//         std::string folder;
//         std::string settings;
//       };
      
      enum class status {
        create_new_world,
        load_existing_world,
        count
      };
      
      demiurge(interface_container* container);
      ~demiurge();
      
      void create_new_world();
      void refresh();
      size_t worlds_count();
      // хотя это копирование идиотское, + неизвестно когда почистит луа все это дело, лучше наверное сам буду хранить в таблицах
      sol::table world(const size_t &index) const;
      void choose_world(const size_t &index);
      enum status status() const;
      size_t choosed() const;
    private:
      std::vector<sol::table> worlds;
      interface_container* container;
      enum status m_status;
      size_t m_choosed;
    };
  }
}

#endif
