#ifndef SETTINGS_H
#define SETTINGS_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include "id.h"
#include "input.h"

namespace devils_engine {
  namespace utils {
    struct settings { // в принципе свои все настройки я знаю, поэтому я могу хранить их просто в структуре
      struct graphics { // графика и рендер
        uint32_t width;  // игнорим это если фуллскрин
        uint32_t height;
        uint32_t video_mode;
        bool fullscreen;
        bool projection;
        // фов?
        
        graphics();
        
        // нужно вернуть здесь видео моды
        size_t video_modes_count() const;
        std::tuple<uint32_t, uint32_t, uint32_t> get_video_mode(const size_t &index) const;
        void find_video_mode();
        
        void apply();
      };
      
      struct game { // технические игровые настройки (мышка, камера, ???)
        float camera_movement;   // это сенса в камере
        float camera_movement_x; // имеет смысл сделать задаваемый разброс скорости от высоты
        float camera_movement_y;
        float sens;              // сенса на мышке в том случае если используем свой курсор
        float sens_x;
        float sens_y;
        
        // здесь еще появятся ограничители фпс
        float target_fps;
        
        bool game_cursor;        // для этого видимо придется еще и свой рендер сделать
        
        game();
      };
      
      struct keys {
        using itr_type = phmap::flat_hash_map<utils::id, input::keys::event_keys_container>::iterator;
        
        // эвенты и две кнопки? нужно наверное еще описание какое и название не техническое
        //std::unordered_map<id, std::pair<int, int>> mapping;
        utils::id awaiting_key;
        uint32_t container_for_event;
        //void* key_iterator;
        //size_t key_iterator;
        uint32_t container_for_iteration;
        itr_type current_iterator;
        
        keys();
        void setup_default_mapping();
        // как указать что мы сейчас заполняем?
        // 
        bool is_awaits_key() const;
        utils::id event_awaits_key() const;
        bool sey_key_to(const uint32_t &container_for_event, const utils::id &event_id);
        void update(const int key);
        size_t events_count(const uint32_t &container_for_iteration) const;
        std::tuple<utils::id, int, int> get_next_event(const uint32_t &container);
      };
      
      // звуки, и все видимо
      
      struct graphics graphics;
      struct game game;
      struct keys keys;
      bool dumped;
      
      settings();
      ~settings();
      void load_settings(const std::string &path = "");
      void dump_settings(const std::string &path = ""); // нужно ли здесь указывать путь? пусть останется придумаю че делать с этим позже
    };
  }
}

#endif
