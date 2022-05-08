#ifndef PATH_CONTAINER_H
#define PATH_CONTAINER_H

#include <cstdint>
#include <atomic>
#include <queue>
#include "utils/memory_pool.h"
#include "utils/memory_pool_mt.h"
#include "utils/user_data.h"

// так чет не могу придумать как организовать поиск пути, мне нужно учитывать что задача может еще выполняться
// и что нужно выполнить более старую задачу, завести вместо статуса количество задач?

// самый легкий способ получить хороший патх файндер это создать много поисковиков по количеству потоков, но это костыль
// мне нужно что то переделать для того чтобы не создавать 8 штук поисковиков, возможно нужно создать мультипоточный поисковик
// основная задача разделить массивы друг от друга, 

// к сожалению в луа придется искать путь синхронно, особенно если потребуется нестандартная функция вычисления веса 
// перехода с тайла на тайл, но для армии то вполне можно использовать стандартную функцию

namespace devils_engine {
  namespace utils {
    class astar_search_mt;
  }
  
  namespace ai {
    struct path_finding_data;
    
    struct path_container { // тут делать деструктор ни к чему
      struct piece {
        float cost;
        uint32_t tile;
        
        piece();
      };
      
      static const size_t container_size = 256;
      piece tile_path[container_size];
      path_container* next;
      
      path_container();
    };
    
    path_container* advance_container(path_container* container, const size_t &index);
    
    struct path_managment {
      using float_t = double;
      using vertex_cost_f = std::function<float_t(const uint32_t &, const uint32_t &, const utils::user_data &)>; // same as in utils/astar_search_mt.h
      
      utils::memory_pool_mt<path_container, 1000*sizeof(path_container)> path_pool; // по идее если мы прост почистим память, то ничего страшного не случится
      std::atomic<size_t> task_id;
      
      // переписал маленько под многопоток
      utils::astar_search_mt* finder;
      
      path_managment(const uint32_t &finders_count);
      ~path_managment();
      
      void find_path(path_finding_data* object, const uint32_t &start, const uint32_t &end, const utils::user_data &data = utils::user_data(), const vertex_cost_f &func = nullptr);
      void free_path(path_container* ptr);
      
      path_container* find_path_raw(const uint32_t &start, const uint32_t &end, const utils::user_data &data, const vertex_cost_f &func, size_t &path_size);
      path_container* find_path_raw(
        const std::atomic<size_t>* task_id_container, 
        const uint32_t &start, const uint32_t &end, 
        const size_t &current_task_id, 
        const utils::user_data &data,
        const vertex_cost_f &func,
        size_t &path_size
      );
    };
  }
}

#endif
