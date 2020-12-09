#ifndef ASTAR_SEARCH_H
#define ASTAR_SEARCH_H

#include <vector>
#include <functional>
#include <cstdint>
#include "memory_pool.h"

#define ASTAR_SEARCH_NODE_DEFAULT_SIZE 300

// этим же алгоритмом нужно обходить разные типы карт
// как быть? по идее нужно сделать просто один интерфейс для карт

namespace devils_engine {
  namespace utils {
    class astar_search {
    public:
      using float_t = double;
      using vertex_cost_f = std::function<float_t(const uint32_t &, const uint32_t &)>;
      using goal_cost_f = std::function<float_t(const uint32_t &, const uint32_t &)>;
      using predicate_f = std::function<bool(const uint32_t &, const uint32_t &)>;
      
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
        
        node() : parent(nullptr), child(nullptr), g(0.0), h(0.0), f(0.0), tile_index(UINT32_MAX) {}
        node(const uint32_t &tile_index) : parent(nullptr), child(nullptr), g(0.0), h(0.0), f(0.0), tile_index(tile_index) {}
      };
      
      static void set_vertex_cost_f(const vertex_cost_f &f);
      static void set_goal_cost_f(const goal_cost_f &f);
      
      astar_search();
      ~astar_search();
      
      void cancel();
      void set(const uint32_t &tile_start, const uint32_t &tile_end, const predicate_f &f);
      state step();
      uint32_t step_count() const;
      
      void add_successor(const uint32_t &tile_index);
      void free_solution();
      std::vector<node*> solution();
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
  
      bool canceled;
      state current_state;
      uint32_t steps;
      
      node* start;
      node* current;
      node* goal;
      
      // в условиях такой карты это поди будет жрать много памяти
      std::vector<node*> openlist; // heap
      std::vector<node*> closedlist; // vector
    //   std::vector<Node*> solution; // запишем ответ в вектор
      std::vector<node*> successors;
      memory_pool<node, ASTAR_SEARCH_NODE_DEFAULT_SIZE*sizeof(node)> node_pool;
      
      predicate_f predicate;
      
      void free_all();
      void free_unused();
    };
  }
}

#endif
