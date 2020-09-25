#include "main_menu.h"
#include "interface_container.h"
#include "demiurge.h"

namespace devils_engine {
  namespace utils {
    enum menu_data_type {
      invalid,
      save_game_manager,
      worlds_manager,
      settings,
      multiplayer_manager, // ну то есть мы должны просмотреть доступные сервера, и иметь возможность подключиться в лобби
      back_to_menu,
      menu_data_type_count
    };
    
    main_menu::entry::entry(const std::string_view &name, const uint32_t &type, interface_container* container) : window_name(name), type(type), pointer(nullptr) {
      switch (type) {
        case invalid: break;
        case save_game_manager: break;
        case worlds_manager: pointer = new demiurge(container); break;
        case settings: break;
        case multiplayer_manager: break;
        case back_to_menu: break;
        default: throw std::runtime_error("Bad menu type");
      }
    }
    
    main_menu::entry::~entry() {
      switch (type) {
        case invalid: break;
        case save_game_manager: break;
        case worlds_manager: delete reinterpret_cast<demiurge*>(pointer); pointer = nullptr; break;
        case settings: break;
        case multiplayer_manager: break;
        case back_to_menu: break;
        //default: throw std::runtime_error("Bad menu type");
        default: assert(false);
      }
    }
    
    const size_t menu_layer = utils::interface_container::last_layer();
    
    sol::object make_object(sol::state_view lua, const uint32_t &type, void* pointer) {
      switch (type) {
        case invalid: break;
        case save_game_manager: break;
        case worlds_manager: return sol::make_object(lua, reinterpret_cast<demiurge*>(pointer));
        case settings: break;
        case multiplayer_manager: break;
        case back_to_menu: break;
        //default: throw std::runtime_error("Bad menu type");
        default: throw std::runtime_error("Bad menu type");
      }
      
      return sol::make_object(lua, sol::nil);
    }
    
    main_menu::main_menu(interface_container* container) : container(container), m_quit_game(false) {
      menu_types.insert(std::make_pair("main_menu", invalid));            // я так понимаю создание даже меню будет не здесь
      menu_types.insert(std::make_pair("main_menu_map", invalid));
      menu_types.insert(std::make_pair("worlds_window", worlds_manager));
      menu_types.insert(std::make_pair("back_to_main_menu", back_to_menu));
    }
    
    void main_menu::push(const std::string &menu) {
      if (menu.empty()) throw std::runtime_error("Bad window name");
      if (!exist() && (menu != "main_menu" && menu != "main_menu_map")) throw std::runtime_error("Trying to open main_menu from unknown source");
      auto itr = menu_types.find(menu);
      if (itr == menu_types.end()) throw std::runtime_error("Could not find menu " + menu);
      std::cout << "main_menu::push " << menu << "\n";
      menu_stack.emplace(itr->first, itr->second, container);
      // нужен тип
      const bool ret = container->close_layer(menu_layer);
      if (menu == "back_to_main_menu") return;
//       ASSERT(menu == "main_menu" || ret);
      container->open_layer(menu_layer, menu_stack.top().window_name, {sol::make_object(container->lua, this), make_object(container->lua, menu_stack.top().type, menu_stack.top().pointer)});
    }
    
    bool main_menu::exist() const {
      return !menu_stack.empty();
    }
    
    void main_menu::escape() {
      if (!exist()) return;
      container->close_layer(menu_layer);
      menu_stack.pop();
      if (!exist()) return;
      container->open_layer(menu_layer, menu_stack.top().window_name, {sol::make_object(container->lua, this), make_object(container->lua, menu_stack.top().type, menu_stack.top().pointer)});
    }
    
    void main_menu::quit_game() {
      // неплохо было бы предусмотреть сохранение игры при выходе
      m_quit_game = true;
    }
    
    std::string_view main_menu::current_entry() const {
      return menu_stack.top().window_name;
    }
    
    void main_menu::clear() {
      while (exist()) menu_stack.pop();
    }
    
    bool main_menu::advance_state(game_state &current, game_state &new_state) {
      if (menu_stack.empty()) return false;
      const auto type = menu_stack.top().type;
      bool ret = false;
      switch (type) {
        case invalid: break;
        case save_game_manager: break;
        case worlds_manager: {
          auto ptr = reinterpret_cast<demiurge*>(menu_stack.top().pointer);
          if (ptr->status() == demiurge::status::create_new_world) {
            current = game_state::loading;
            new_state = game_state::create_map;
            ret = true;
          }
          
          if (ptr->status() == demiurge::status::load_existing_world) {
            // а здесь нужно получить путь откуда грузить
            current = game_state::loading;
            new_state = game_state::map;
            auto proxy = ptr->world(ptr->choosed())["path"];
            if (!proxy.valid()) throw std::runtime_error("Broken world table");
            if (proxy.get_type() != sol::type::string) throw std::runtime_error("Broken world table");
            loading_path = proxy.get<std::string>();
            ret = true;
          }
          break;
        }
        case settings: break;
        case multiplayer_manager: break;
        case back_to_menu: {
          current = game_state::menu;
          new_state = game_state::menu;
          ret = true;
          break;
        }
        //default: throw std::runtime_error("Bad menu type");
        default: throw std::runtime_error("Bad menu type");
      }
      
      return ret;
    }
  }
}
