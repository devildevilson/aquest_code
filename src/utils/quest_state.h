#ifndef DEVILS_ENGINE_UTILS_QUEST_STATE_H
#define DEVILS_ENGINE_UTILS_QUEST_STATE_H

#include <cstddef>
#include <cstdint>
#include <vector>
// #include "render/stage.h"

namespace devils_engine {
//   namespace render {
//     class stage;
//   }
  
  namespace utils {
    // не думаю что этот класс будет держать в себе какие то данные
    // скорее только логику, можно его сделать в виде массива при начальной загрузке
    class quest_state {
    public:
      enum {
//         main_menu,
//         map_creation,
//         world_map,
//         battle,
//         encounter,
//         count
        // тут понятно что будут использоваться менюшные хоткеи
        main_menu_loading,
        main_menu,
        world_map_generator_loading,
        world_map_generator,
        world_map_generating,
        // здесь должны быть хоткеи карты + менюшные когда запустится меню
        world_map_loading, // причем в лоадинге меню запускать ненужно
        world_map,
        // хоткеи битвы + меню
        battle_map_loading,
        battle_map,
        // хоткеи столкновения + меню
        encounter_loading,
        encounter,
        count
      };
      
      inline quest_state(const uint32_t current_state) : m_current_state(current_state), m_next_state(UINT32_MAX) {}
      virtual ~quest_state() = default;
      virtual void enter(quest_state* prev_state) = 0;  // возможно здесь нужно почистить некоторые предыдущие стейт
      //virtual bool load(quest_state* prev_state) = 0;   // запускаем функцию загрузки (предположительно асинхронно)
      virtual uint32_t update(const size_t &time, quest_state* prev_state) = 0; // обновляем как нибудь состояние (наверное время пригодится)
      virtual void clean(quest_state* next_state) = 0;  // прежде чем войти в другое состояние, чистим это
//       virtual void mouse_input(const size_t &time, const uint32_t &tile_index) = 0;
//       virtual void key_input(const size_t &time, const bool loading) = 0; // тут нужно учитывать инпут в меню
      // как давать понять когда переходить из состояния в состояние, в общем у нас каждое состояние может куда перейти
      inline uint32_t next_state() const { return m_next_state; }
      inline uint32_t current_state() const { return m_current_state; }
    protected:
      uint32_t m_current_state;
      uint32_t m_next_state;
    };
    
    // вместо этого нужно сделать рендер по слотам
//     class state_controller { // не уверен в целесообразности этого класса
//     public:
//       void change_state(const uint32_t &new_state);
//       void set_render_stage(const uint32_t &slot, render::stage* stage); // слоты для рендера очень помогут удалять и создавать ресурсы
//       void clean_render_stage(const uint32_t &slot);
//       void update(const size_t &time);
//       void update_render();
//     private:
//       uint32_t pending_state;
//       quest_state* current_state;
//       std::vector<render::stage*> rendering_stages;
//     };
  }
}

#endif
