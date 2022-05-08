#ifndef INTERFACE_H
#define INTERFACE_H

#include <vector>
#include <chrono>
#include "utils/id.h"
#include "utils/typeless_container.h"
#include "utils/shared_time_constant.h"
#include "interface_context.h"

namespace devils_engine {
  namespace interface {
    class page {
    public:
      virtual ~page() {}
      
      virtual page* proccess(const data::extent &screen_size, const utils::id &event, const size_t &time, size_t &focus) = 0; // возвращает либо себя, если еще не закончил, либо следующую страницу
      virtual page* escape(size_t &focus) const = 0;   // виртуал? (это не важно)
    };

    class container {
    public:
      static const size_t first_pressed_delay = ONE_SECOND/3;
      static const size_t pressed_delay = ONE_SECOND/6;
      
      container(const size_t &containerSize);
      ~container();

      void proccess(const data::extent &screen_size, const size_t &time);
      
      void set_default_page(page* page_ptr);
      void open();
      bool is_opened() const;
      
      template <typename T, typename... Args>
      T* add_page(Args&&... args) {
        T* ptr = page_container.create<T>(std::forward<Args>(args)...);
        pages.push_back(ptr);
        
        return ptr;
      }
      
      // команды клавиатуры, это все?
//       void next();
//       void prev();
//       void increase();
//       void decrease();
//       void choose();
      void escape();
      void send_event(const utils::id &event, const size_t &time, const size_t &frame_time);
    private:
      // должен определять стиль интерфейса, шрифты, создавать окно, рисовать интерфейс
      page* default_page;
      page* current_page;
      
//       data::command last_command;
      utils::id command_event;
      size_t command_time;
      utils::id last_event;
      size_t delay_time;
      size_t focus;
      
      utils::typeless_container page_container;
      std::vector<page*> pages;
    };
  }
}

#endif
