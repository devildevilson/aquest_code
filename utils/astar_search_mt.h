#ifndef DEVILS_ENGINE_UTILS_ASTAR_SEARCH_MT_H
#define DEVILS_ENGINE_UTILS_ASTAR_SEARCH_MT_H

#include <vector>
#include <functional>
#include <cstdint>
#include <array>
#include "memory_pool_mt.h"
#include "user_data.h"

#define ASTAR_SEARCH_MT_NODE_DEFAULT_SIZE 3000

// этим же алгоритмом нужно обходить разные типы карт
// как быть? по идее нужно сделать просто один интерфейс для карт
// если здесь отделить листы и сакссессоры, то львиная доля проблем исчезнет

namespace devils_engine {
  namespace utils {
    class astar_search_mt {
    public:
      using float_t = double;
      using vertex_cost_f = std::function<float_t(const uint32_t &, const uint32_t &, const user_data &)>;
      using goal_cost_f = std::function<float_t(const uint32_t &, const uint32_t &)>;
      using predicate_f = std::function<bool(const uint32_t &, const uint32_t &)>;
      using fill_successors_f = std::function<void(astar_search_mt*, const uint32_t &, const user_data &)>;
      
      static const size_t maximum_tile_neighbours = 6;
      
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
      
      astar_search_mt(const uint32_t &threads_count);
      ~astar_search_mt();
      
      // функции которые нужно использовать внутри потоков
      void cancel();
      // тут скорее требуется передать некую юзердату, как ее оформить?
      void set(const uint32_t &tile_start, const uint32_t &tile_end, const user_data &ud, const vertex_cost_f &vertex_func = nullptr);
      state step();
      uint32_t step_count() const;
      
      void add_successor(const uint32_t &tile_index);
      void free_solution();
      std::vector<node*> solution();
      float solution_cost() const;
      node* solution_start() const;
      node* solution_goal() const;
      
      // функции которые можно использовать вне потоков
      std::vector<node*> solution(const uint32_t &thread_index);
      node* solution_start(const uint32_t &thread_index) const;
      node* solution_goal(const uint32_t &thread_index) const;
      
      // тут будем чистить всю память для всех потоков, я не уверен насколько это практично
      // поиск пути скорее всего будет задействован во всех ходах
      void clear_memory();
    private:
      static vertex_cost_f neighbor_cost;
      static goal_cost_f goal_cost;
      static fill_successors_f fill_successors;
      
      struct thread_search_data {
        bool canceled;
        uint32_t successors_count;
        state current_state;
        uint32_t steps;
        
        node* start;
        node* current;
        node* goal;
        
        struct user_data user_data;
        
        // 8 потоков довольно быстро забьют память, нужно придумать какой то иной способ
        // во первых successors может быть не больше 6 по идее, во вторых... что?
        // я не думаю что у меня получится что то сделать с природой этих массивов
        std::vector<node*> openlist; // heap
        std::vector<node*> closedlist; // vector
        std::array<node*, maximum_tile_neighbours> successors;
        
        vertex_cost_f vertex_func;
        
        thread_search_data();
        void clear_successors();
      };
      
      uint32_t count;
      thread_search_data* search_datas;
      memory_pool_mt<node, ASTAR_SEARCH_MT_NODE_DEFAULT_SIZE*sizeof(node)> node_pool;
      
      uint32_t get_thread_id() const;
      void free_all(thread_search_data* data);
      void free_unused(thread_search_data* data);
    };
  }
}

#endif
