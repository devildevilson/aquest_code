// наклир предлагает иммедиате мод гуи, что значит что мне нужно просто вызывать
// функции гуи каждый кадр и каждый кадр гуи как бы заново формируется
// в принципе это нормальный подход и он меня устраивает, но мне нужно
// каким то образом быстренько накидывать интерфейс + возможно накидывать его
// в каких нибудь файлах конфигурации? сделать это возможно, но для этого мне
// нужно определить какие то классы хранящие конфигурации интерфейса
// что тут может пойти не так? 

// было бы неплохо настраивать горячие клавиши, сразу в голову приходит создать 
// список возможных действий, по нему находить клавишу и сравнивать ее с тем 
// что нажато, но лучше задействовать тот класс клавиш который у меня уже имеется
// хотя действия в меню строго ограничены, и по идее очень легко определить
// когда и какое произойдет действие в интерфейсе

// вместо всего этого, лучше выделить несколько крупных меню и их полностью задизайнить в одном месте
// нажатие кнопки может привести к отрисовке следующего меню (возращаем указатель)
// сложнее быть с луа, нужно ли мне вообще там задавать интерфейс 
// (но там по прежнему будет проще, чем городить json или xml файлы и из них получать команды для наклира)

#ifndef INTERFACES_H
#define INTERFACES_H

#include "interface.h"
#include "interface_context.h"
#include "settings.h"

struct nuklear_data;

namespace devils_engine {
  namespace interface {
    class main_menu : public page {
    public:
      static const size_t blink_time = ONE_SECOND/2;
      
      struct create_info {
        context* nuklear;
        page* quit;
        page* settings;
        page* new_game;
      };
      main_menu(const create_info &info);
      
      page* proccess(const data::extent &screen_size, const utils::id &event, const size_t &time, size_t &focus) override;
      page* escape(size_t &focus) const override;
      
      void set_pointers(page* quit, page* settings, page* new_game);
    private:
      context* nuklear;
      page* quit;
      page* settings;
      page* new_game;
      size_t current_time;
    };
    
    class settings : public page {
    public:
      static const size_t blink_time = ONE_SECOND/2;
      
      struct create_info {
        context* nuklear;
        page* main_menu;
      };
      settings(const create_info &info);
      
      page* proccess(const data::extent &screen_size, const utils::id &event, const size_t &time, size_t &focus) override;
      page* escape(size_t &focus) const override;
      
      void set_pointers(page* main_menu, page* graphics, page* sound, page* mouse, page* controls);
    private:
      context* nuklear;
      page* main_menu;
      page* graphics;
      page* sound;
      page* mouse;
      page* controls;
      size_t current_time;
    };
    
    class graphics : public page {
    public:
      static const size_t blink_time = ONE_SECOND/2;
      
      struct create_info {
        context* nuklear;
        page* settings;
      };
      graphics(const create_info &info);
      
      page* proccess(const data::extent &screen_size, const utils::id &event, const size_t &time, size_t &focus) override;
      page* escape(size_t &focus) const override;
      
      void set_pointers(page* settings);
    private:
      struct utils::settings::graphics current_settings;
      context* nuklear;
      page* settings;
      size_t current_time;
    };
    
    class save_game : public page {
    public:
      static const size_t blink_time = ONE_SECOND/2;
      
      struct create_info {
        context* nuklear;
        page* main_menu;
      };
      save_game(const create_info &info);
      
      page* proccess(const data::extent &screen_size, const utils::id &event, const size_t &time, size_t &focus) override;
      page* escape(size_t &focus) const override;
      
      void set_pointers(page* main_menu);
    private:
      context* nuklear;
      page* main_menu;
      size_t current_time;
    };
    
    class quit_game : public page {
    public:
      static const size_t blink_time = ONE_SECOND/2;
      
      struct create_info {
        context* nuklear;
        page* main_menu;
        bool* quit;
      };
      quit_game(const create_info &info);
      
      page* proccess(const data::extent &screen_size, const utils::id &event, const size_t &time, size_t &focus) override;
      page* escape(size_t &focus) const override;
      
      void set_pointers(page* main_menu);
    private:
      context* nuklear;
      page* main_menu;
      bool* quit;
      size_t current_time;
    };
  }
}

#endif
