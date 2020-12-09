#ifndef PATH_CONTAINER_H
#define PATH_CONTAINER_H

#include <cstdint>
#include <atomic>
#include "utils/memory_pool.h"
#include "utils/memory_pool_mt.h"

// так чет не могу придумать как организовать поиск пути, мне нужно учитывать что задача может еще выполняться
// и что нужно выполнить более старую задачу, завести вместо статуса количество задач?

namespace devils_engine {
  namespace core {
    struct army;
    struct hero_troop;
  }
  
  namespace utils {
    class astar_search;
  }
  
  namespace ai {
    struct path_container { // тут делать деструктор ни к чему
      static const size_t container_size = 256;
      uint32_t tile_path[container_size]; // размер? размер нам нужен только один раз
      path_container* next;
      
      path_container();
    };
    
    // где то путь нужно создавать + как организовать поиск?
    // поиск вытащить нужно в отдельный поток,
    
    struct path_managment {
      enum class status {
        idle,
        finding_path,
        finish,
        stop,
        
      };
      
      utils::memory_pool_mt<path_container, 1000*sizeof(path_container)> path_pool; // по идее если мы прост почистим память, то ничего страшного не случится
      path_container* tmp_path;
      size_t path_size; // сколько тайлов мы пройдем
      uint32_t start, end; // текущие начало конец
//       std::atomic<status> current_status; 
//       std::atomic_bool choosed_tile;
      std::atomic<size_t> task_id;
      // статусы видимо нужны для каждого потока
      // а может и нет
      
      // поиск создать по потокам? или немного его переделать под многопоток?
      // переделать поди лучше? так будет проще чистить переодически поиск
      // + если переделать то будет выглядеть лучше все это дело
      // можем ли мы все это сделать на статической памяти? нужно будет тогда выделять очень крупный кусок
      // в поисковике нет особо ничего что можно сделать многопоточным (ну кроме функций)
      // нужно тогда сделать контейнер со всем этим делом
      utils::astar_search* finders; // было бы неплохо заиметь быстрый способ получить индекс потока
      
      path_managment(const uint32_t &finder_count);
      ~path_managment();
      
      void find_path(core::army* army, const uint32_t &start, const uint32_t &end);
      void find_path(core::hero_troop* troop, const uint32_t &start, const uint32_t &end);
      void free_path(path_container* ptr);
//       void choosed_path();
      // поиск пути для ии: мы обойдем все армии и найдем путь напрямую
      // наверное это же будет функцией для работы в потоках
      
      path_container* find_path_raw(const uint32_t &start, const uint32_t &end, size_t &path_size);
      path_container* find_path_raw(const std::atomic<size_t>* task_id_container, const uint32_t &start, const uint32_t &end, const size_t &current_task_id, size_t &path_size);
    };
  }
}

#endif
