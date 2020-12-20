#include "path_container.h"

#include "utils/assert.h"
#include <cstring>
#include "utils/astar_search.h"
#include "bin/core_structures.h"

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
    
    path_managment::path_managment(const uint32_t &finder_count) : 
      tmp_path(nullptr), 
      path_size(0), 
      start(UINT32_MAX), 
      end(UINT32_MAX), 
//       current_status(path_managment::status::idle), 
      task_id(1),
      finders(new utils::astar_search[finder_count]) 
    {}
    
    path_managment::~path_managment() {
      delete [] finders;
    }
    
    void path_managment::find_path(core::army* army, const uint32_t &start, const uint32_t &end) {
      auto pool = global::get<dt::thread_pool>();
      
      // по всей видимости придется пихать сюда все выделение
      // здесь мы его копируем и считаем все в одном потоке
      // на самом деле все было бы гораздо проще если мы добавим статус в армии и героев
      // нам нужно закончить раньше если мы выбрали опять те же армии
      // как интересно это сделано в других играх? мы если статус переносим в армии, то нам ни к чему атомик_бул
      // мы просто обойдем все объекты и поставим там стоп
      
//       current_status = status::finding_path;
      army->path = nullptr;
      army->start_tile = start;
      army->end_tile = end;
      pool->submitbase([this, army, start, end] () {
        const size_t local_task_id = task_id.fetch_add(1);
        
        // ставим максимум идентификатор в армии
        // если идентификаторы не совпадают, то выходим
        // так по идее ничего не нужно будет ждать слава богам хаоса
        
        utils::atomic_max(army->path_task, local_task_id);
        
//         if (army->path_state == core::path_finding_state::stop) { 
//           army->path_state = core::path_finding_state::idle;
//           return; 
//         }
//         
//         army->path_state = core::path_finding_state::stop;
//         while (army->path_state == core::path_finding_state::get_task || 
//                army->path_state == core::path_finding_state::finding_path ||
//                army->path_state == core::path_finding_state::stop) {
//           std::this_thread::sleep_for(std::chrono::microseconds(1)); // ждем, по идее у нас 
//         }
//         
//         army->path_state = core::path_finding_state::finding_path;
        
//         PRINT_VAR("army->path_task", army->path_task)
//         PRINT_VAR("local_task_id  ", local_task_id)
        
        if (army->path_task != local_task_id) return;
                       
        PRINT("find army path")
        
        size_t path_size = 0;
        auto path = find_path_raw(&army->path_task, start, end, local_task_id, path_size); // я могу получить 
        if (army->path_task != local_task_id) return;
                       
        path = path == nullptr ? reinterpret_cast<decltype(path)>(SIZE_MAX) : path;
        
        // как посчитать? добавить еще переменных в путь?
        // тут по идее нужно просто добавить в путь данные о весах
        
        army->path_size = path_size;
        army->current_path = 0;
        army->start_tile = UINT32_MAX;
        army->end_tile = UINT32_MAX;
        army->path = path; // если путь не найден то здесь будет нуллптр, нужно придумать какой то другой способ отличить готовый путь от ненайденного
//         current_status = status::finish;
//         army->path_state = core::path_finding_state::idle;
        //army->path_task = 0; // или не надо? может сбить код выше
        
        // после поиска пути мы просто должны расчитать какой путь мы можем пройти и его стоимость
        // затем, когда игрок нажимает на кнопку, единственное что мы делаем это вычитаем стоимость 
        // и ставим армию в верное положение, здесь только одна проблема как учесть тайл на котором кто то будет стоять?
      });
    }
    
    void path_managment::find_path(core::hero_troop* troop, const uint32_t &start, const uint32_t &end) {
      auto pool = global::get<dt::thread_pool>();
      
//       current_status = status::finding_path;
      troop->path = nullptr;
      pool->submitbase([this, troop, start, end] () {
        const size_t local_task_id = task_id.fetch_add(1);
        utils::atomic_max(troop->path_task, local_task_id);
        
        if (troop->path_task != local_task_id) return;
        size_t path_size = 0;
        auto path = find_path_raw(&troop->path_task, start, end, local_task_id, path_size);
        if (troop->path_task != local_task_id) return;
        path = path == nullptr ? reinterpret_cast<decltype(path)>(SIZE_MAX) : path;
        
        troop->start_tile = start;
        troop->end_tile = end;
        troop->path_size = path_size;
        troop->current_path = 0;
        troop->path = path;
//         current_status = status::finish;
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

    path_container* path_managment::find_path_raw(const uint32_t &start, const uint32_t &end, size_t &path_size) {
      std::atomic<size_t> tmp(0);
      return find_path_raw(&tmp, start, end, 0, path_size);
    }
    
    path_container* path_managment::find_path_raw(const std::atomic<size_t>* task_id_container, const uint32_t &start, const uint32_t &end, const size_t &current_task_id, size_t &path_size) {
      auto pool = global::get<dt::thread_pool>();
      const uint32_t id = pool->thread_index(std::this_thread::get_id());
      
      auto searcher = &finders[id];
      searcher->set(start, end, [] (const uint32_t &tile_index1, const uint32_t &tile_index2) -> bool {
        auto map = global::get<systems::map_t>()->map;
//         const auto &tile_data1 = render::unpack_data(map->get_tile(tile_index1));
        const auto &tile_data2 = render::unpack_data(map->get_tile(tile_index2));
        
        if (tile_data2.height > 0.5f) return false; // на гору подняться нельзя
        if (tile_data2.height < 0.0f) return false; // по воде пройти пока тоже нельзя
        
        (void)tile_index1;
        return true;
      });
      
      size_t counter = 0;
//       bool ignore_status = false;
      auto status = utils::astar_search::state::searching;
      while (status == utils::astar_search::state::searching) {
        if (*task_id_container != current_task_id) searcher->cancel();
        status = searcher->step();
        ++counter;
        PRINT_VAR("step counter", counter)
      }
      
      path_size = 0;
      if (status == utils::astar_search::state::failed) return nullptr;
      
      if (status != utils::astar_search::state::succeeded) {
        throw std::runtime_error("Searching error");
      }
      
      auto solution_array = searcher->solution();
      const size_t final_path_size = solution_array.size()-1;
      auto path_start = path_pool.create();
      auto tmp = path_start;
      for (uint32_t i = 0, counter = 0; i < solution_array.size(); ++i, ++counter) {
        if (counter >= path_container::container_size) {
          auto ptr = path_pool.create();
          tmp->next = ptr;
          tmp = ptr;
          counter = 0;
        }
        
        tmp->tile_path[counter].cost = solution_array[i]->g;
        tmp->tile_path[counter].tile = solution_array[i]->tile_index;
      }
      
      searcher->free_solution();
      
      // кажется тайлы строго расположены дрг за другом по стоимости
      // нужно ли тут что то считать? мне кажется что врядли
//       auto first_container = path_start;
//       for (size_t i = 0; i < final_path_size; ++i) {
//         //const size_t container_index = i / ai::path_container::container_size;
//         const size_t piece_index = i % ai::path_container::container_size;
//         PRINT_VAR("cost: ", first_container->tile_path[piece_index].cost)
//         
//         if (piece_index == ai::path_container::container_size-1) first_container = first_container->next;
//       }
      
//       army->path_size = solution_array.size();
//       army->path = path_start;
      
      // в других стратегиях сразу считалось что мы можем пройти, а что нет
      // тут так же имеет смысл сделать, чтобы потом просто выставить армию на нужный тайл
      // так можно не пихать отдельную работу в другую задачу
      // + ко всему у нас сразу там хранится стоимость которую можно использовать 
      // нужно просто добавить переменную с ходом и где то под рукой иметь функцию весов
      // (это должна быть одна и таже функция что и веса для поиска)
      // хотя наверное она мне не потребуется
      
//       (void)army;
      path_size = final_path_size;
      return path_start;
    }
  }
}
