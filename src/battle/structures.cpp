#include "structures.h"

#include "utils/globals.h"
#include "utils/systems.h"
#include "utils/thread_pool.h"
#include "utils/string_container.h"

#include "unit_states_container.h"
#include "map.h"
#include "lua_states.h"
#include "context.h"

namespace devils_engine {
  namespace battle {
    unit::unit() : status(status::idle), unit_gpu_index(UINT32_MAX), current_state(nullptr), next_state(nullptr), state_time(0), user_time(0), change_counter(0) {
      
    }
    
    unit::~unit() {
      
    }
    
    // лучше здесь запоминать следующий стейт и не сразу в него переходить
    void unit::set_state(const std::string_view &name) {
      auto ctx = global::get<systems::battle_t>()->context.get();
      auto map = global::get<systems::battle_t>()->unit_states_map.get();
      // нужно хранить индексы в мапе
      const size_t index = map->get(name);
      if (index == SIZE_MAX) throw std::runtime_error("Could not find state " + std::string(name));
      auto next_state = ctx->get_entity<core::state>(index);
      //set_state(next_state);
      // здесь по идее важно чтобы next_state всегда был нуллптр
      ASSERT(this->next_state == nullptr);
      this->next_state = next_state;
    }
    
    void unit::reset_timer() { user_time = 0; }
    void unit::update(const size_t &time) {
      state_time += time;
      user_time += time;
      change_counter = 0;
      
      //auto changed_state = next_state;
      //next_state = nullptr;
      //if (changed_state != nullptr) set_state(changed_state);
      ASSERT(next_state == nullptr);
      
      if (state_time >= current_state->time) set_state(current_state->next);
    }
    
    double unit::random() {
      rng = rng_func(rng);
      return utils::rng_normalize(get_value_func(rng));
    }
    
    double unit::random(const double &upper) {
      const double val = random();
      return upper * val;
    }
    
    double unit::random(const double &lower, const double &upper) {
      const double val = random();
      return glm::mix(lower, upper, val);
    }
    
    std::string_view unit::state() const {
      return current_state->id;
    }
    
    void unit::seed_random(const uint64_t &seed) {
      rng = init_func(seed);
    }
    
    void unit::set_state(const struct core::state* state) {
      do {
        // вызываем функцию, если она есть
        // по идее в функции может быть переход на новый стейт
        ++change_counter;
        if (change_counter >= MAX_STATE_CHANGES) throw std::runtime_error("Long state sequence");
        
        state_time = 0;
        current_state = state;
        
        // если приходит сюда nullptr, то что мы делаем? по идее нужно удалить объект
        // но в этом случае скорее всего ошибка
        if (current_state == nullptr) throw std::runtime_error("Null state");
        
        if (current_state->func_index != UINT32_MAX) {
          auto lua_states = global::get<systems::battle_t>()->lua_states.get();
          auto pool = global::get<dt::thread_pool>();
          const uint32_t state_index = pool->thread_index(std::this_thread::get_id());
          //lua_states->registered_functions[state_index][current_state->func_index](this);
          const auto sol_ret = lua_states->registered_functions[current_state->func_index][state_index](this);
          if (!sol_ret.valid()) {
            sol::error err = sol_ret;
            std::cout << err.what() << "\n";
            throw std::runtime_error("Unit function lua error. State: " + current_state->id);
          }
          
          // в этом случае луа функция спокойно завершается
          // можно ли придумать такое использование функции чтобы был важен запуск функции из нового стейта?
          // в общем то тут только запуск функций происходит без особых изменений состояний
          while (next_state != nullptr) {
            current_state = next_state;
            next_state = nullptr;
            
            ++change_counter;
            if (change_counter >= MAX_STATE_CHANGES) throw std::runtime_error("Long state sequence");
            
            const uint32_t index = current_state->func_index;
            const auto &func = lua_states->registered_functions[index][state_index];
            const auto sol_ret = func(this);
            if (!sol_ret.valid()) {
              sol::error err = sol_ret;
              std::cout << err.what() << "\n";
              throw std::runtime_error("Unit function lua error. State: " + current_state->id);
            }
          }
          
          if (current_state == nullptr) throw std::runtime_error("Null state");
        }
        
        state = current_state->next;
      } while (current_state->time == 0);
      
      // мы должны поставить новые данные текстурок
      auto map = global::get<systems::battle_t>()->map.get();
      auto data = map->get_unit_data(unit_gpu_index);
      //data.pos.w = glm::uintBitsToFloat(current_state->texture_offset);
      //data.dir.w = glm::uintBitsToFloat(current_state->texture_count);
      data.pos.w = glm::uintBitsToFloat(current_state->texture.container);
      // я могу положить еще скейл, где мне его взять? по идее он просто у юнита должен быть
      data.dir.w = scale;
      map->set_unit_data(unit_gpu_index, data);
    }
    
    glm::vec4 unit::get_pos() const {
      auto map = global::get<systems::battle_t>()->map.get();
      auto data = map->get_unit_data(unit_gpu_index);
      return glm::vec4(data.pos.x, data.pos.y, data.pos.z, 1.0f);
    }
    
    void unit::set_pos(const glm::vec4 &pos) {
      auto map = global::get<systems::battle_t>()->map.get();
      auto data = map->get_unit_data(unit_gpu_index);
      data.pos.x = pos.x;
      data.pos.y = pos.y;
      data.pos.z = pos.z;
      map->set_unit_data(unit_gpu_index, data);
    }
    
    glm::vec4 unit::get_dir() const {
      auto map = global::get<systems::battle_t>()->map.get();
      auto data = map->get_unit_data(unit_gpu_index);
      return glm::vec4(data.dir.x, data.dir.y, data.dir.z, 0.0f);
    }
    
    void unit::set_dir(const glm::vec4 &dir) {
      auto map = global::get<systems::battle_t>()->map.get();
      auto data = map->get_unit_data(unit_gpu_index);
      data.dir.x = dir.x;
      data.dir.y = dir.y;
      data.dir.z = dir.z;
      map->set_unit_data(unit_gpu_index, data);
    }
    
    troop::troop() : type(nullptr), tile_index(UINT32_MAX), stats{0}, unit_count(0), unit_offset(UINT32_MAX) {}
  }
}
