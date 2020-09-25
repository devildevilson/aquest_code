#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <string>
#include <stack>
#include <unordered_map>
#include "utility.h"

namespace devils_engine {
  namespace utils {
    struct interface_container;
    
    struct main_menu {
      struct entry {
        std::string_view window_name;
        uint32_t type; // по этому типу нужно передать некоторые структуры (например, менеджер сейвов)
        void* pointer;
        
        entry(const std::string_view &name, const uint32_t &type, interface_container* container);
        ~entry();
      };
      
      std::stack<entry> menu_stack;
      std::unordered_map<std::string, uint32_t> menu_types;
      interface_container* container;
      
      bool m_quit_game;
      std::string loading_path; 
      // как понять вообще что грузить? либо сейв, либо карту, в мультиплеере еще придется передать карту по сети
      // у меня файл с миром имеет название ворлд_дата и я так подозреваю всегда будет так
      // сохранения будут наверное иметь какое то расширение типо "aqs"
      
      main_menu(interface_container* container);
      void push(const std::string &menu);
      bool exist() const;
      void escape();
      void quit_game();
      std::string_view current_entry() const;
      void clear();
      bool advance_state(game_state &current, game_state &new_state);
    };
  }
}

#endif
