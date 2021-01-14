#ifndef UTILITY_H
#define UTILITY_H

#include <cstdint>
#include <cstddef>

#include "shared_time_constant.h"
#include "shared_application_constant.h"
#include "shared_mathematical_constant.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

#include "assert.h"

#define PRINT_VEC4(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w << "\n";
#define PRINT_VEC3(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << "\n";
#define PRINT_VEC2(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << "\n";
#define PRINT_VAR(name, var) std::cout << name << ": " << var << "\n";
#define PRINT(var) std::cout << var << "\n";
#define DELETE_PTR(ptr) delete ptr; ptr = nullptr;
#define DELETE_ARR(arr) delete [] arr; arr = nullptr;
#define STRINGIFY(a) #a
#define CONCAT(a, b) a##b

constexpr size_t align_to(const size_t &memory, const size_t &aligment) {
  return (memory + aligment - 1) / aligment * aligment;
}

namespace devils_engine {
  namespace utils {
    // как задавать состояния игрока? поидее мы меняем эти состояния в зависимости от ситуации в игре
    enum player_states {
      player_in_menu,
    //   player_on_global_map,
    //   player_on_battle_map,
    //   player_on_hero_battle_map,
      player_states_count
    };

    player_states current_player_state();
    void set_player_state(const player_states &state);
  }

  enum class game_state {
    loading,
    menu,
    create_map,
    map,
    battle,  
    encounter,  // геройская битва
    count
  };
}

#endif
