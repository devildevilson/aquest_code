#include "astar_search_mt.h"

#include "globals.h"
#include "thread_pool.h"

namespace devils_engine {
  namespace utils {
    struct node_compare {
      bool operator() (const astar_search_mt::node* first, const astar_search_mt::node* second) const {
        return first->f > second->f;
      }
    };
    
    astar_search_mt::vertex_cost_f astar_search_mt::neighbor_cost;
    astar_search_mt::goal_cost_f astar_search_mt::goal_cost;
    astar_search_mt::fill_successors_f astar_search_mt::fill_successors;
    void astar_search_mt::set_vertex_cost_f(const vertex_cost_f &f) { neighbor_cost = f; }
    void astar_search_mt::set_goal_cost_f(const goal_cost_f &f) { goal_cost = f; }
    void astar_search_mt::set_fill_successors_f(const fill_successors_f &f) { fill_successors = f; }
    
    astar_search_mt::astar_search_mt(const uint32_t &threads_count) : count(threads_count), search_datas(new thread_search_data[count]) {}
    
    astar_search_mt::~astar_search_mt() {
      clear_memory();
      
      delete [] search_datas;
    }
    
    void astar_search_mt::cancel() {
      const uint32_t index = get_thread_id();
      search_datas[index].canceled = true;
    }
    
    void astar_search_mt::set(const uint32_t &tile_start, const uint32_t &tile_end, const user_data &ud, const vertex_cost_f &vertex_func) {
      const uint32_t index = get_thread_id();
      auto &t_data = search_datas[index];
      
      t_data.canceled = false;
  
      t_data.vertex_func = vertex_func;
      t_data.user_data = ud;
      
      t_data.start = node_pool.create(tile_start);
      t_data.goal = node_pool.create(tile_end);
      
      t_data.current_state = state::searching;
      
      t_data.start->g = 0.0f;
      //start->h =  start->vertex->goal_distance_estimate(goal->vertex);
      t_data.start->h = goal_cost(t_data.start->tile_index, t_data.goal->tile_index);
      t_data.start->f = t_data.start->g + t_data.start->h;
      
      t_data.openlist.push_back(t_data.start);
      std::push_heap(t_data.openlist.begin(), t_data.openlist.end(), node_compare());
      
      t_data.steps = 0;
    }
    
    astar_search_mt::state astar_search_mt::step() {
      const uint32_t index = get_thread_id();
      auto t_data = &search_datas[index];
      
      ASSERT(t_data->current_state > state::not_initialised && t_data->current_state < state::invalid && "Search is not initialized");
  
      // Next I want it to be safe to do a searchstep once the search has succeeded...
      if (t_data->current_state == state::succeeded || t_data->current_state == state::failed) return t_data->current_state;

      // Failure is defined as emptying the open list as there is nothing left to 
      // search...
      // New: Allow user abort
      if (t_data->openlist.empty() || t_data->canceled) {
        free_all(t_data);
        t_data->current_state = state::failed;
        return t_data->current_state;
      }
      
      // Incremement step count
      ++t_data->steps;

      // Pop the best node (the one with the lowest f) 
      node *n = t_data->openlist.front(); // get pointer to the node
      std::pop_heap(t_data->openlist.begin(), t_data->openlist.end(), node_compare());
      t_data->openlist.pop_back();

      // Check for the goal, once we pop that we're done
      //if (n->vertex->isGoal(goal->vertex)) {
      if (n->tile_index == t_data->goal->tile_index) {
        // The user is going to use the Goal Node he passed in 
        // so copy the parent pointer of n 
        t_data->goal->parent = n->parent;
        t_data->goal->g = n->g;

        // A special case is that the goal was passed in as the start state
        // so handle that here
        //if (!n->vertex->isSameState(start->vertex)) {
        if (n->tile_index != t_data->start->tile_index) {
          node_pool.destroy(n);

          // set the child pointers in each node (except Goal which has no child)
          node *nodeChild = t_data->goal;
          node *nodeParent = t_data->goal->parent;

          do {
            nodeParent->child = nodeChild;

            nodeChild = nodeParent;
            nodeParent = nodeParent->parent;
          
          } while (nodeChild != t_data->start); // Start is always the first node by definition
        }

        // delete nodes that aren't needed for the solution
        free_unused(t_data);
        t_data->current_state = state::succeeded;
        return t_data->current_state;
      } else { // not goal

        // We now need to generate the successors of this node
        // The user helps us to do this, and we keep the new nodes in
        // m_Successors ...

        t_data->clear_successors(); // empty vector of successor nodes to n

        // User provides this functions and uses AddSuccessor to add each successor of
        // node 'n' to m_Successors
        //add_successor(n->tile_index);
        fill_successors(this, n->tile_index, t_data->user_data);
        
//         ASSERT(t_data->successors.size() <= 6);
//         ASSERT(t_data->successors.size() > 0);
        ASSERT(t_data->successors_count <= 6);
        ASSERT(t_data->successors_count > 0);
        
        // Now handle each successor to the current node ...
        for (auto successor = t_data->successors.begin(); successor != t_data->successors.end(); ++successor) {
          // The g value for this successor ...
          //const float newg = n->g + n->vertex->cost((*successor)->vertex);
          float_t n_cost = 0.0f;
          if (t_data->vertex_func) {
            n_cost = t_data->vertex_func(n->tile_index, (*successor)->tile_index, t_data->user_data);
          } else {
            n_cost = neighbor_cost(n->tile_index, (*successor)->tile_index, t_data->user_data); // тут видимо иногда неверно приходит значение
          }
          
          ASSERT(n_cost > 0.0);
//           ASSERT(n_cost < 1000.0);
          const float_t newg = n->g + n_cost;

          // Now we need to find whether the node is on the open or closed lists
          // If it is but the node that is already on them is better (lower g)
          // then we can forget about this successor
          
//           PRINT_VAR("openlist.size()", openlist.size())
//           PRINT_VAR("closedlist.size()", closedlist.size())
          // First linear search of open list to find node
          auto openlist_result = t_data->openlist.begin();
          for (; openlist_result != t_data->openlist.end(); ++openlist_result) {
            //if ((*openlist_result)->vertex->isSameState((*successor)->vertex)) break;
            if ((*openlist_result)->tile_index == (*successor)->tile_index) break;
          }

          if (openlist_result != t_data->openlist.end()) {
            // we found this state on open

            if ((*openlist_result)->g <= newg) {
              node_pool.destroy(*successor);

              // the one on Open is cheaper than this one
              continue;
            }
          }

          auto closedlist_result = t_data->closedlist.begin();
          for (; closedlist_result != t_data->closedlist.end(); ++closedlist_result) {
            //if ((*closedlist_result)->vertex->isSameState((*successor)->vertex)) break;
            if ((*closedlist_result)->tile_index == (*successor)->tile_index) break;
          }

          if (closedlist_result != t_data->closedlist.end()) {
            // we found this state on closed

            if ((*closedlist_result)->g <= newg) {
              // the one on Closed is cheaper than this one
              node_pool.destroy(*successor);

              continue;
            }
          }

          // This node is the best node so far with this particular state
          // so lets keep it and set up its AStar specific data ...

          (*successor)->parent = n;
          (*successor)->g = newg;
          //(*successor)->h = (*successor)->vertex->goal_distance_estimate(goal->vertex);
          (*successor)->h = goal_cost((*successor)->tile_index, t_data->goal->tile_index);
          (*successor)->f = (*successor)->g + (*successor)->h;

          // Successor in closed list
          // 1 - Update old version of this node in closed list
          // 2 - Move it from closed to open list
          // 3 - Sort heap again in open list

          if (closedlist_result != t_data->closedlist.end()) {
            // Update closed node with successor node AStar data
            *(*closedlist_result) = *(*successor);
            
//             (*closedlist_result)->parent = (*successor)->parent;
//             (*closedlist_result)->g      = (*successor)->g;
//             (*closedlist_result)->h      = (*successor)->h;
//             (*closedlist_result)->f      = (*successor)->f;

            // Free successor node
            node_pool.destroy(*successor);

            // Push closed node into open list 
            t_data->openlist.push_back(*closedlist_result);

            // Remove closed node from closed list
            //closedlist.erase(closedlist_result);
            std::swap(*closedlist_result, t_data->closedlist.back());
            t_data->closedlist.pop_back();

            // Sort back element into heap
            std::push_heap(t_data->openlist.begin(), t_data->openlist.end(), node_compare());

            // Fix thanks to ...
            // Greg Douglas <gregdouglasmail@gmail.com>
            // who noticed that this code path was incorrect
            // Here we have found a new state which is already CLOSED
          } else if (openlist_result != t_data->openlist.end()) {
            // Successor in open list
            // 1 - Update old version of this node in open list
            // 2 - sort heap again in open list
            
            // Update open node with successor node AStar data
            *(*openlist_result) = *(*successor);
//             (*openlist_result)->parent = (*successor)->parent;
//             (*openlist_result)->g      = (*successor)->g;
//             (*openlist_result)->h      = (*successor)->h;
//             (*openlist_result)->f      = (*successor)->f;

            // Free successor node
            node_pool.destroy(*successor);

            // re-make the heap 
            // make_heap rather than sort_heap is an essential bug fix
            // thanks to Mike Ryynanen for pointing this out and then explaining
            // it in detail. sort_heap called on an invalid heap does not work
            std::make_heap(t_data->openlist.begin(), t_data->openlist.end(), node_compare());
          } else {
            // New successor
            // 1 - Move it from successors to open list
            // 2 - sort heap again in open list
            
            // Push successor node into open list
            t_data->openlist.push_back(*successor);

            // Sort back element into heap
            std::push_heap(t_data->openlist.begin(), t_data->openlist.end(), node_compare());
          }
        }

        // push n onto Closed, as we have expanded it now
        t_data->closedlist.push_back(n);

      } // end else (not goal so expand)

      return t_data->current_state; // Succeeded bool is false at this point.
    }
    
    uint32_t astar_search_mt::step_count() const {
      const uint32_t index = get_thread_id();
      return search_datas[index].steps;
    }
    
    void astar_search_mt::add_successor(const uint32_t &tile_index) {
      const uint32_t index = get_thread_id();
      const uint32_t s_index = search_datas[index].successors_count;
      ++search_datas[index].successors_count;
      assert(s_index < maximum_tile_neighbours);
      search_datas[index].successors[s_index] = node_pool.create(tile_index);
    }
    
    void astar_search_mt::free_solution() {
      const uint32_t index = get_thread_id();
      auto &t_data = search_datas[index];
      
      node* n = t_data.start;
  
      if (t_data.start->child != nullptr) {
        do {
          node *del = n;
          n = n->child;
          node_pool.destroy(del);
          
          del = nullptr;
        } while (n != t_data.goal);

        node_pool.destroy(n); // Delete the goal
      } else {
        node_pool.destroy(t_data.start);
        node_pool.destroy(t_data.goal);
      }
      
      t_data.start = nullptr;
      t_data.goal = nullptr;
    }
    
    std::vector<astar_search_mt::node*> astar_search_mt::solution() {
      const uint32_t index = get_thread_id();
      return solution(index);
    }
    
    float astar_search_mt::solution_cost() const {
      const uint32_t index = get_thread_id();
      const auto &t_data = search_datas[index];
      if (t_data.goal != nullptr && t_data.current_state == state::succeeded) {
        return t_data.goal->g;
      } 
      
      return -1.0f;
    }
    
    astar_search_mt::node* astar_search_mt::solution_start() const {
      const uint32_t index = get_thread_id();
      return solution_start(index);
    }
    
    astar_search_mt::node* astar_search_mt::solution_goal() const {
      const uint32_t index = get_thread_id();
      return solution_goal(index);
    }
    
    std::vector<astar_search_mt::node*> astar_search_mt::solution(const uint32_t &thread_index) {
      const auto &t_data = search_datas[thread_index];
      if (t_data.start->tile_index == t_data.goal->tile_index) return { t_data.start };
  
      std::vector<astar_search_mt::node*> sol;
      
      for (auto cur = t_data.start; cur != nullptr; cur = cur->child) {
        sol.push_back(cur);
      }
      
//       node* n = t_data.start;
//       sol.push_back(n);
//       while (n != t_data.goal) {
//         n = n->child;
//         sol.push_back(n);
//       }
      
      return sol;
    }
    
    astar_search_mt::node* astar_search_mt::solution_start(const uint32_t &thread_index) const {
      const auto &t_data = search_datas[thread_index];
      return t_data.start;
    }
    
    astar_search_mt::node* astar_search_mt::solution_goal(const uint32_t &thread_index) const {
      const auto &t_data = search_datas[thread_index];
      return t_data.goal;
    }
    
    void astar_search_mt::clear_memory() {
      for (uint32_t i = 0; i < count; ++i) {
        free_all(&search_datas[i]);
        auto &t_data = search_datas[i];
        t_data.openlist.shrink_to_fit();
        t_data.closedlist.shrink_to_fit();
        for (size_t j = 0; j < t_data.successors.size(); ++j) { t_data.successors[j] = nullptr; }
      }
      
      node_pool.clear();
    }
    
    astar_search_mt::thread_search_data::thread_search_data() : 
      canceled(false),
      successors_count(0),
      current_state(state::not_initialised),
      steps(0),
      start(nullptr),
      current(nullptr),
      goal(nullptr),
      successors{nullptr}
    {}
    
    void astar_search_mt::thread_search_data::clear_successors() {
      successors_count = 0;
      for (auto &s : successors) { s = nullptr; }
    }
    
    uint32_t astar_search_mt::get_thread_id() const {
      return global::get<dt::thread_pool>()->thread_index(std::this_thread::get_id());
    }
    
    void astar_search_mt::free_all(thread_search_data* data) {
      for (size_t i = 0; i < data->openlist.size(); ++i) {
        node_pool.destroy(data->openlist[i]);
      }
      data->openlist.clear();
      
      for (size_t i = 0; i < data->closedlist.size(); ++i) {
        node_pool.destroy(data->closedlist[i]);
      }
      data->closedlist.clear();
      
      if (data->start != nullptr) node_pool.destroy(data->start);
      
      data->start = nullptr;
      data->goal = nullptr;
    }
    
    void astar_search_mt::free_unused(thread_search_data* data) {
      for (size_t i = 0; i < data->openlist.size(); ++i) {
        if (data->openlist[i]->child == nullptr) node_pool.destroy(data->openlist[i]);
      }
      data->openlist.clear();
      
      for (size_t i = 0; i < data->closedlist.size(); ++i) {
        if (data->closedlist[i]->child == nullptr) node_pool.destroy(data->closedlist[i]);
      }
      data->closedlist.clear();
    }
  }
}
