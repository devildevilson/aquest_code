#include "path_container.h"

#include "utils/assert.h"
#include <cstring>
#include "utils/astar_search_mt.h"
#include "core/army.h"
#include "core/hero_troop.h"

#include "utils/thread_pool.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "bin/map.h"
#include "utils/works_utils.h"

namespace devils_engine {
  namespace ai {
    path_container::piece::piece() : cost(0.0f), tile(UINT32_MAX) {}
    path_container::path_container() : next(nullptr) { ASSERT(tile_path[1].tile == UINT32_MAX); ASSERT(tile_path[2].tile == UINT32_MAX); }
    
    path_container* advance_container(path_container* container, const size_t &index) {
      auto cont = container;
      size_t counter = 0;
      while (counter < index && cont != nullptr) {
        cont = cont->next;
        ++counter;
      }
      
      return cont;
    }
    
    path_managment::path_managment(const uint32_t &finders_count) : 
      task_id(1),
      finder(new utils::astar_search_mt(finders_count)) 
    {}
    
    path_managment::~path_managment() {
      delete finder;
    }
    
    void path_managment::find_path(path_finding_data* object, const uint32_t &start, const uint32_t &end, const utils::user_data &data, const vertex_cost_f &func) {
      auto pool = global::get<dt::thread_pool>();
      
      // по всей видимости придется пихать сюда все выделение
      // здесь мы его копируем и считаем все в одном потоке
      // на самом деле все было бы гораздо проще если мы добавим статус в армии и героев
      // нам нужно закончить раньше если мы выбрали опять те же армии
      // как интересно это сделано в других играх? мы если статус переносим в армии, то нам ни к чему атомик_бул
      // мы просто обойдем все объекты и поставим там стоп
      
      pool->submitbase([this, object, start = start, end = end, data = data, func = func] () {
        size_t local_task_id = task_id.fetch_add(1);
        if (local_task_id == 0) local_task_id = task_id.fetch_add(1);
        
        // ставим максимум идентификатор в армии
        // если идентификаторы не совпадают, то выходим
        // так по идее ничего не нужно будет ждать слава богам хаоса
        
        utils::atomic_max(object->path_task, local_task_id);
        if (object->path_task != local_task_id) return;
        
        auto old_path = object->path.exchange(nullptr);
        if (old_path != nullptr && old_path != reinterpret_cast<path_container*>(SIZE_MAX)) free_path(old_path);
        object->start_tile = start;
        object->end_tile = end;
        object->path_size = 0;
                       
        PRINT("find army path")
        
        size_t path_size = 0;
        auto path = find_path_raw(&object->path_task, start, end, local_task_id, data, func, path_size);
        if (object->path_task != local_task_id) { 
          if (path_size != 0) free_path(path);
          return; 
        }
        object->path_task = 0;
        
        path = path == nullptr ? reinterpret_cast<decltype(path)>(SIZE_MAX) : path;
        
        // как посчитать? добавить еще переменных в путь?
        // тут по идее нужно просто добавить в путь данные о весах
        
        object->path_size = path_size;
        object->current_path = 0;
        object->start_tile = UINT32_MAX;
        object->end_tile = UINT32_MAX;
        object->path = path; // если путь не найден то здесь будет нуллптр, нужно придумать какой то другой способ отличить готовый путь от ненайденного
      });
    }
    
    void path_managment::free_path(path_container* ptr) {
      while (ptr != nullptr) {
        auto next = ptr->next;
        path_pool.destroy(ptr);
        ptr = next;
      }
    }
    
    // единственная проблема в том что я могу захотеть искать несколько путей
    // либо искать пути для всего выделения в одном потоке, что конечно по дурацки немного
    // да и ко всему прочему придется копировать стаф, как иначе? кажется что лучше скопировать
    // наверное придется скопировать вещи в массив и искать все в одном потоке
    // + так мы можем по идее прикинуть пути так чтобы они не заканчивались на одном тайле все
//     void path_managment::choosed_path() { choosed_tile = true; }

    path_container* path_managment::find_path_raw(const uint32_t &start, const uint32_t &end, const utils::user_data &data, const vertex_cost_f &func, size_t &path_size) {
      std::atomic<size_t> tmp(0);
      return find_path_raw(&tmp, start, end, 0, data, func, path_size);
    }
    
    path_container* path_managment::find_path_raw(
      const std::atomic<size_t>* task_id_container, 
      const uint32_t &start, const uint32_t &end, 
      const size_t &current_task_id, 
      const utils::user_data &data,
      const vertex_cost_f &func,
      size_t &path_size
    ) {
      auto searcher = finder;
      searcher->set(start, end, data, func);
      
      auto status = utils::astar_search_mt::state::searching;
      while (status == utils::astar_search_mt::state::searching) {
        if (*task_id_container != current_task_id) searcher->cancel();
        status = searcher->step();
        PRINT_VAR("step counter", searcher->step_count())
      }
      
      path_size = 0;
      if (status == utils::astar_search_mt::state::failed) return nullptr;
      
      if (status != utils::astar_search_mt::state::succeeded) {
        throw std::runtime_error("Searching error");
      }
      
      uint32_t counter = 0;
      uint32_t final_size = 0;
      auto cur_node = searcher->solution_start();
      auto path_start = path_pool.create();
      auto cur_path = path_start;
      while (cur_node != nullptr) {
        if (counter >= path_container::container_size) {
          auto ptr = path_pool.create();
          cur_path->next = ptr;
          cur_path = ptr;
          counter = 0;
        }
        
        cur_path->tile_path[counter].cost = cur_node->g;
        cur_path->tile_path[counter].tile = cur_node->tile_index;
        cur_node = cur_node->child;
        
        ++counter;
        ++final_size;
      }
      
//       const auto &solution_array = searcher->solution();
//       const size_t final_path_size = solution_array.size();
//       auto path_start = path_pool.create();
//       auto tmp = path_start;
//       for (uint32_t i = 0, counter = 0; i < solution_array.size(); ++i, ++counter) {
//         if (counter >= path_container::container_size) {
//           auto ptr = path_pool.create();
//           tmp->next = ptr;
//           tmp = ptr;
//           counter = 0;
//         }
//         
//         // тут желательно сохранять не g, а стоимость с тайла на тайл
//         tmp->tile_path[counter].cost = solution_array[i]->g;
//         tmp->tile_path[counter].tile = solution_array[i]->tile_index;
//       }
      
      searcher->free_solution();
      
      path_size = final_size;
      return path_start;
    }
  }
}
