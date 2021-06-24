#ifndef INTERFACE_CONTAINER2_H
#define INTERFACE_CONTAINER2_H

#include "sol.h"
#include <cstdint>
#include <cstddef>
#include <string>

namespace devils_engine {
  namespace interface {
    struct context;
  }
  
  namespace game {
    struct context;
  }
  
  namespace utils {
    struct interface_container {
      struct timer {
        size_t m_current_time;
        size_t m_user_time;
        size_t m_accumulated_time;
        
        timer();
        void update(const size_t &time);
        
        void reset();
        size_t current_time() const;
        size_t user_time() const;
        size_t accumulated_time() const;
      };
      
      devils_engine::interface::context* ctx;
      game::context* game_ctx;
      sol::state lua;
      sol::environment env;
      timer t;
      
      sol::object moonnuklear_ctx;
      sol::object fonts[4]; // нужно сделать какой нибудь фонт конфиг
      sol::function get_ctx;
      sol::function get_font;
      sol::function free_font;
      
      // основные функции в которых я буду делать весь интерфейс
      
      // нужно или нет? постоянно хранить явно нет нужды
      // какие аргументы? по идее никаких не нужно
      //sol::function init_func; // зачем, если и так запускается скрипт?
      sol::function update_func;
      sol::table local_table;
      sol::object loading_table_tmp;
      
      sol::table serpent;
      sol::function serpent_line;
      
      struct create_info {
        devils_engine::interface::context* ctx;
        game::context* game_ctx;
      };
      interface_container(const create_info &info);
      
      void load_interface_file(const std::string_view &path);
      
      void update(const size_t &time); // тут ведь еще нужно передать геймстейт
      
      void update_loading_table(const sol::table &t); // должно вызываться раньше чем update
      void update_post_generating_table(const sol::table &t);
      void setup_map_generator_functions(const sol::table &t);
      void clear_map_generator_functions();
      
      // эта таблица должна быть "простой" (то есть состоять только из строк, чисел и таблиц)
      // таблица хранится в сохранениях, не уверен зачем это нужно правда
      sol::object get_serializable_table() const;
      // тут еще наверное нужно добавить таблицу которая будет сохраняться при выходе из приложения
      // то есть в ней было бы неплохо хранить например положение окон игрока
      sol::object get_persistent_table() const;
      void set_persistent_table(const sol::table &t);
      
      // нужно еще таблицу генерации получить, и было бы неплохо сделать "скрытую" генерацию
      // то есть игрок не видит промежуточные этапы генерации, по идее мы это можем сделать закрыв обзор задником каким то
      sol::object get_generator_table() const;
      
      std::string serialize_table(const sol::table &t);
      sol::table deserialize_table(const std::string &str);
      
      void get_fonts();
      void free_fonts();
    };
  }
}

#endif
