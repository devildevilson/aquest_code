#ifndef DEVILS_ENGINE_UTILS_ASTAR_SEARCH_H
#define DEVILS_ENGINE_UTILS_ASTAR_SEARCH_H

#include <vector>
#include <functional>
#include <cstdint>
#include "memory_pool.h"
#include "user_data.h"

#define ASTAR_SEARCH_NODE_DEFAULT_SIZE 3000

// этим же алгоритмом нужно обходить разные типы карт
// как быть? по идее нужно сделать просто один интерфейс для карт
// если здесь отделить листы и сакссессоры, то львиная доля проблем исчезнет

namespace devils_engine {
  namespace utils {
    class astar_search {
    public:
      using float_t = double;
      using vertex_cost_f = std::function<float_t(const uint32_t &, const uint32_t &, const user_data &)>;
      using goal_cost_f = std::function<float_t(const uint32_t &, const uint32_t &)>;
      using predicate_f = std::function<bool(const uint32_t &, const uint32_t &)>;
      using fill_successors_f = std::function<void(astar_search*, const uint32_t &, const user_data &)>;
      
      enum class state {
        not_initialised,
        searching,
        succeeded,
        failed,
        out_of_memory,
        invalid
      };
      
      struct node {
        node* parent; // used during the search to record the parent of successor nodes
        node* child; // used after the search for the application to view the search in reverse
        
        float_t g; // cost of this node + it's predecessors
        float_t h; // heuristic estimate of distance to goal
        float_t f; // sum of cumulative cost of predecessors and self and heuristic
        
        uint32_t tile_index;
        
        inline node() : parent(nullptr), child(nullptr), g(0.0), h(0.0), f(0.0), tile_index(UINT32_MAX) {}
        inline node(const uint32_t &tile_index) : parent(nullptr), child(nullptr), g(0.0), h(0.0), f(0.0), tile_index(tile_index) {}
      };
      
      static void set_vertex_cost_f(const vertex_cost_f &f);
      static void set_goal_cost_f(const goal_cost_f &f);
      static void set_fill_successors_f(const fill_successors_f &f);
      
      astar_search();
      ~astar_search();
      
      void cancel();
      // тут скорее требуется передать некую юзердату, как ее оформить?
      void set(const uint32_t &tile_start, const uint32_t &tile_end, const user_data &ud);
      state step();
      uint32_t step_count() const;
      
      void add_successor(const uint32_t &tile_index);
      void free_solution();
      std::vector<node*> solution();
      node* solution_raw();
      float solution_cost() const;
      
      node* solution_start() const;
      node* solution_goal() const;
      
      std::vector<node*> & open_list();
      std::vector<node*> & closed_list();
      
      void clear_memory();
    private:
//       typedef gheap<2, 1> binary_heap; // имеет смысл попробовать разную -арность у куч
      // здесь нужна логика, то есть нужно расчитать вес перехода из тайл в тайл с учетом подъема 
      // и биома, довольно тяжелая функция, ее наверное можно менять в зависимости от условий? хотя вряд ли нужно
      static vertex_cost_f neighbor_cost;
      static goal_cost_f goal_cost;
      static fill_successors_f fill_successors;
  
      bool canceled;
      state current_state;
      uint32_t steps;
      
      node* start;
      node* current;
      node* goal;
      
      // в условиях такой карты это поди будет жрать много памяти
      std::vector<node*> openlist; // heap
      std::vector<node*> closedlist; // vector
      std::vector<node*> successors;
      memory_pool<node, ASTAR_SEARCH_NODE_DEFAULT_SIZE*sizeof(node)> node_pool;
      
      //predicate_f predicate;
      struct user_data user_data;
      
      void free_all();
      void free_unused();
    };
  }
}

#endif
