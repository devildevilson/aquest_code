#ifndef ARMY_H
#define ARMY_H

#include <array>
#include "utils/structures_utils.h"
#include "declare_structures.h"
#include "stats.h"
#include "ai/path_container.h"
#include "ai/path_finding_data.h"
#include "troop.h"

namespace devils_engine {
  namespace core {
    // армия создается только когда мы собираем войска
    // при роспуске эту структуру удаляем
    // тут нужно указать еще позицию на карте конкретную, для анимации
    // тут еще должно быть хранилище для пути в армии + манипуляция этим путем
    // отряды можно передать, нападение на другую армию, нападение на город, 
    // зайти в город? засада? быстрый марш? пока сделаем базовые штуки
    // а как сделана принадлежность армий? пока что вообще ни как
    struct army : public ai::path_finding_data, public utils::flags_container, public utils::modificators_container, public utils::events_container {
      static const structure s_type = structure::army;
      static const size_t modificators_container_size = 10;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      static const size_t max_troops_count = 20;
      
      // отряды + полководцы (главный полководец должен быть всегда первым)
      std::array<troop, max_troops_count> troops; // возьмем в качестве отправной точки 20 юнитов
      uint32_t troops_count;
      // позиция (просто указатель на тайл?)
      uint32_t tile_index;
      // характеристики армии? передвижение, атака, защита (?), все характеристики для отрядов
      std::array<stat_container, army_stats::count> computed_stats; // 
      std::array<stat_container, army_stats::count> current_stats;
      // какие то спутники? (спутники по идее у персонажа)
      // графика (иконка и отображение на карте)
      //render::image_t map_img; // на карте это нужно отображать с флагом
      
      uint32_t army_gpu_slot;
      
      float current_pos;
      size_t current_time;
      bool advancing;
      
//       phmap::flat_hash_map<const modificator*, size_t> modificators;
//       phmap::flat_hash_map<const event*, event_container> events;
//       phmap::flat_hash_map<std::string_view, size_t> flags; // забыл зачем тут size_t
      //modificators_container<modificators_container_size> modificators; // по идее их меньше чем треитов
      //events_container<events_container_size> events; // должно хватить
      //flags_container<flags_container_size> flags; // флагов довольно много
      
      army();
      ~army();
      
      void set_pos(const glm::vec3 &pos);
      glm::vec3 get_pos() const;
      void set_img(const render::image_t &img);
      render::image_t get_img() const; // куда пихнуть геральдику? так не хочется еще заводить переменные для армий
      
      size_t can_advance() const;
      bool can_full_advance() const;
      void advance();
      void stop();
      void find_path(const uint32_t end_tile_index);
      void clear_path();
      
      void update(const size_t &time);
    };
  }
}

#endif
