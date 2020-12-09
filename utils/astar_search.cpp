#include "astar_search.h"

#include "globals.h"
#include "systems.h"
#include "bin/map.h"
#include "render/shared_structures.h"

#include <iostream>

namespace devils_engine {
  namespace utils {
    struct node_compare {
      bool operator() (const astar_search::node* first, const astar_search::node* second) const {
        return first->f > second->f;
      }
    };
    
    void astar_search::set_vertex_cost_f(const vertex_cost_f &f) { neighbor_cost = f; }
    void astar_search::set_goal_cost_f(const goal_cost_f &f) { goal_cost = f; }
    
    astar_search::astar_search() : canceled(false), current_state(state::not_initialised), steps(0), start(nullptr), current(nullptr), goal(nullptr) {}
    astar_search::~astar_search() {
      free_all();
    }
    
    void astar_search::cancel() { canceled = true; }
    
    void astar_search::set(const uint32_t &tile_start, const uint32_t &tile_end, const predicate_f &f) {
      canceled = false;
  
      predicate = f;
      
      start = node_pool.create(tile_start);
      goal = node_pool.create(tile_end);
      
      current_state = state::searching;
      
      start->g = 0.0f;
      //start->h =  start->vertex->goal_distance_estimate(goal->vertex);
      start->h = goal_cost(start->tile_index, goal->tile_index);
      start->f = start->g + start->h;
      
      openlist.push_back(start);
      std::push_heap(openlist.begin(), openlist.end(), node_compare());
      
      steps = 0;
    }
    
    astar_search::state astar_search::step() {
      ASSERT(current_state > state::not_initialised && current_state < state::invalid && "Search is not initialized");
  
      // Next I want it to be safe to do a searchstep once the search has succeeded...
      if (current_state == state::succeeded || current_state == state::failed) return current_state;

      // Failure is defined as emptying the open list as there is nothing left to 
      // search...
      // New: Allow user abort
      if (openlist.empty() || canceled) {
        free_all();
        current_state = state::failed;
        return current_state;
      }
      
      // Incremement step count
      ++steps;

      // Pop the best node (the one with the lowest f) 
      node *n = openlist.front(); // get pointer to the node
      std::pop_heap(openlist.begin(), openlist.end(), node_compare());
      openlist.pop_back();

      // Check for the goal, once we pop that we're done
      //if (n->vertex->isGoal(goal->vertex)) {
      if (n->tile_index == goal->tile_index) {
        // The user is going to use the Goal Node he passed in 
        // so copy the parent pointer of n 
        goal->parent = n->parent;
        goal->g = n->g;

        // A special case is that the goal was passed in as the start state
        // so handle that here
        //if (!n->vertex->isSameState(start->vertex)) {
        if (n->tile_index != start->tile_index) {
          node_pool.destroy(n);

          // set the child pointers in each node (except Goal which has no child)
          node *nodeChild = goal;
          node *nodeParent = goal->parent;

          do {
            nodeParent->child = nodeChild;

            nodeChild = nodeParent;
            nodeParent = nodeParent->parent;
          
          } while (nodeChild != start); // Start is always the first node by definition
        }

        // delete nodes that aren't needed for the solution
        free_unused();
        current_state = state::succeeded;
        return current_state;
      } else { // not goal

        // We now need to generate the successors of this node
        // The user helps us to do this, and we keep the new nodes in
        // m_Successors ...

        successors.clear(); // empty vector of successor nodes to n

        // User provides this functions and uses AddSuccessor to add each successor of
        // node 'n' to m_Successors
        add_successor(n->tile_index);
//         for (size_t i = 0; i < n->vertex->degree(); ++i) {
//           size_t mem = i;
//           const graph::edge* edge = n->vertex->next_edge(mem);
//           const components::vertex* vertex = edge->vertices.first == n->vertex ? edge->vertices.second : edge->vertices.first;
//           
//           if (!vertex->is_active()) continue; // !vertex->is_valid() || 
//           if (!predicate(n->vertex, vertex, edge)) continue;
//           
//           node* tmp = node_pool.newElement();
//           
//           tmp->edge = edge;
//           tmp->vertex = vertex;
//           
//           successors.push_back(tmp);
//         }
// 
//         // ????
//         if (n->vertex->degree() == 0) {
//           // free the nodes that may previously have been added 
//           for (auto successor = successors.begin(); successor != successors.end(); ++successor) {
//             node_pool.deleteElement((*successor));
//           }
// 
//           successors.clear(); // empty vector of successor nodes to n
// 
//           // free up everything else we allocated
//           node_pool.deleteElement(n);
//           free_all();
// 
//           current_state = state::out_of_memory;
//           return current_state;
//         }
        
        ASSERT(successors.size() <= 6);
        ASSERT(successors.size() > 0);
        
        // Now handle each successor to the current node ...
        for (auto successor = successors.begin(); successor != successors.end(); ++successor) {
          // The g value for this successor ...
          //const float newg = n->g + n->vertex->cost((*successor)->vertex);
          const float_t n_cost = neighbor_cost(n->tile_index, (*successor)->tile_index); // тут видимо иногда неверно приходит значение
          ASSERT(n_cost > 0.0);
//           ASSERT(n_cost < 1000.0);
          const float_t newg = n->g + n_cost;

          // Now we need to find whether the node is on the open or closed lists
          // If it is but the node that is already on them is better (lower g)
          // then we can forget about this successor
          
//           PRINT_VAR("openlist.size()", openlist.size())
//           PRINT_VAR("closedlist.size()", closedlist.size())
          // First linear search of open list to find node
          auto openlist_result = openlist.begin();
          for (; openlist_result != openlist.end(); ++openlist_result) {
            //if ((*openlist_result)->vertex->isSameState((*successor)->vertex)) break;
            if ((*openlist_result)->tile_index == (*successor)->tile_index) break;
          }

          if (openlist_result != openlist.end()) {
            // we found this state on open

            if ((*openlist_result)->g <= newg) {
              node_pool.destroy(*successor);

              // the one on Open is cheaper than this one
              continue;
            }
          }

          auto closedlist_result = closedlist.begin();
          for (; closedlist_result != closedlist.end(); ++closedlist_result) {
            //if ((*closedlist_result)->vertex->isSameState((*successor)->vertex)) break;
            if ((*closedlist_result)->tile_index == (*successor)->tile_index) break;
          }

          if (closedlist_result != closedlist.end()) {
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
          (*successor)->h = goal_cost((*successor)->tile_index, goal->tile_index);
          (*successor)->f = (*successor)->g + (*successor)->h;

          // Successor in closed list
          // 1 - Update old version of this node in closed list
          // 2 - Move it from closed to open list
          // 3 - Sort heap again in open list

          if (closedlist_result != closedlist.end()) {
            // Update closed node with successor node AStar data
            *(*closedlist_result) = *(*successor);
            
//             (*closedlist_result)->parent = (*successor)->parent;
//             (*closedlist_result)->g      = (*successor)->g;
//             (*closedlist_result)->h      = (*successor)->h;
//             (*closedlist_result)->f      = (*successor)->f;

            // Free successor node
            node_pool.destroy(*successor);

            // Push closed node into open list 
            openlist.push_back(*closedlist_result);

            // Remove closed node from closed list
            //closedlist.erase(closedlist_result);
            std::swap(*closedlist_result, closedlist.back());
            closedlist.pop_back();

            // Sort back element into heap
            std::push_heap(openlist.begin(), openlist.end(), node_compare());

            // Fix thanks to ...
            // Greg Douglas <gregdouglasmail@gmail.com>
            // who noticed that this code path was incorrect
            // Here we have found a new state which is already CLOSED
          } else if (openlist_result != openlist.end()) {
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
            std::make_heap(openlist.begin(), openlist.end(), node_compare());
          } else {
            // New successor
            // 1 - Move it from successors to open list
            // 2 - sort heap again in open list
            
            // Push successor node into open list
            openlist.push_back(*successor);

            // Sort back element into heap
            std::push_heap(openlist.begin(), openlist.end(), node_compare());
          }
        }

        // push n onto Closed, as we have expanded it now
        closedlist.push_back(n);

      } // end else (not goal so expand)

      return current_state; // Succeeded bool is false at this point.
    }
    
    uint32_t astar_search::step_count() const { return steps; }
    
    void astar_search::add_successor(const uint32_t &tile_index) {
      // это нужно будет переделывать для других типов карт
      auto map = global::get<systems::map_t>()->map;
      const auto &tile_data = render::unpack_data(map->get_tile(tile_index));
      const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
      for (uint32_t i = 0; i < n_count; ++i) {
        const uint32_t n_index = tile_data.neighbours[i];
        if (!predicate(tile_index, n_index)) continue;
        
        auto node = node_pool.create(n_index);
        successors.push_back(node);
      }
    }
    
    void astar_search::free_solution() {
      node* n = start;
  
      if (start->child != nullptr) {
        do {
          node *del = n;
          n = n->child;
          node_pool.destroy(del);
          
          del = nullptr;
        } while (n != goal);

        node_pool.destroy(n); // Delete the goal
      } else {
        node_pool.destroy(start);
        node_pool.destroy(goal);
      }
      
      start = nullptr;
      goal = nullptr;
    }
    
    std::vector<astar_search::node*> astar_search::solution() {
      if (start->tile_index == goal->tile_index) return { start };
  
      std::vector<astar_search::node*> sol;
      
      node* n = start;
      sol.push_back(n);
      while (n != goal) {
        n = n->child;
        sol.push_back(n);
      }
      
      return sol;
    }
    
    float astar_search::solution_cost() const {
      if (goal != nullptr && current_state == state::succeeded) {
        return goal->g;
      } 
      
      return -1.0f;
    }
    
    astar_search::node* astar_search::solution_start() const {
      return start;
    }
    
    astar_search::node* astar_search::solution_goal() const {
      return goal;
    }
    
    std::vector<astar_search::node*> & astar_search::open_list() {
      return openlist;
    }
    
    std::vector<astar_search::node*> & astar_search::closed_list() {
      return closedlist;
    }
    
    void astar_search::clear_memory() {
      free_all();
      
      openlist.shrink_to_fit();
      closedlist.shrink_to_fit();
      successors.shrink_to_fit();
      node_pool.clear();
    }
    
    void astar_search::free_all() {
      for (size_t i = 0; i < openlist.size(); ++i) {
        node_pool.destroy(openlist[i]);
      }
      openlist.clear();
      
      for (size_t i = 0; i < closedlist.size(); ++i) {
        node_pool.destroy(closedlist[i]);
      }
      closedlist.clear();
      
      if (start != nullptr) node_pool.destroy(start);
      
      start = nullptr;
      goal = nullptr;
    }
    
    void astar_search::free_unused() {
      for (size_t i = 0; i < openlist.size(); ++i) {
        if (openlist[i]->child == nullptr) node_pool.destroy(openlist[i]);
      }
      openlist.clear();
      
      for (size_t i = 0; i < closedlist.size(); ++i) {
        if (closedlist[i]->child == nullptr) node_pool.destroy(closedlist[i]);
      }
      closedlist.clear();
    }
    
    astar_search::vertex_cost_f astar_search::neighbor_cost;
    astar_search::goal_cost_f astar_search::goal_cost;
  }
}
