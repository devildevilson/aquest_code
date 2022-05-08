#ifndef INTERFACE2_H
#define INTERFACE2_H

#include <cstdint>
#include <cstddef>
#include <array>
#include <limits>
#include <functional>
#include <string>
#include <unordered_map>
// #include "utils/bit_field.h"
// #include "declare_structures.h"
// #include "interface_context.h"

#include "utils/sol.h"

// теперь этот класс это враппер над контейнером
// чисто проверки чтобы пользователь не открывал лаеры на зарегестрированных местах
// другое дело что теперь я не могу передать в функцию еще один объект
// нужно придумывать какие то способы 

// struct nk_context;

namespace devils_engine {
  namespace utils {
    struct interface_container;
    
    class interface {
    public:
      enum special_layers {
        player_interface_layer,
        main_menu_layer,
        special_layers_count
      };
      
//       struct window_info {
//         // должны ли мы здесь что нибудь возвращать? мы должны собрать данные для десизион и эвентов
//         // это мы собираем по специальным функциям + мы должны нарисовать портреты и какие то другие картинки
//         // возвращаем закрытие окна
//         //std::function<bool(nk_context*, interface*, const void*)> window;
//         sol::protected_function window;
//         uint32_t type;
//         core::structure target;
//       };
      
      interface(interface_container* container);
//       ~interface();
      
      bool is_layer_visible(const uint32_t &index) const;
      bool close_layer(const uint32_t &index);
      
      // как открыть окно? по идее нам достаточно передать название окна, нужно еще передать какой то объект, как проверить тип?
      // проверить можно если только на все структуры прилепить специальную переменную с типом структуры (как в вулкане)
      // думаю что так можно сделать, но потом
      uint32_t open_layer(const std::string_view &window_id, sol::object data); // тут чаще всего персонаж, но не всегда
      bool update_data(const uint32_t &layer_id, sol::object data);
      
      bool escape();
      
      void update();
    
//       void register_layer(const std::string &name, const window_info &window);
//       const window_info* get_data(const std::string &name) const;
      
      // чистить функции? с одной стороны память чистить лучше чем не чистить
      // с другой стороны не думаю что будет сильно много уникальных функций
      // скорее мы будем вызывать по возможности другие функции
//       void clear();
      
//       sol::state & get_state();
//       sol::object & get_ctx();
    private:
      interface_container* container;
      size_t current_layer;
    };
  }
}

#endif
