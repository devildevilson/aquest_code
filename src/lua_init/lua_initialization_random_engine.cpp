#include "lua_initialization_hidden.h"

#include "utils/random_engine.h"
#include "utils/magic_enum_header.h"
#include "utils/utility.h"

namespace devils_engine {
  namespace utils {
    static sol::object weighted_distribution(random_engine_st* self, sol::table num_table) {
      double sum = 0.0;
      for (const auto &pair : num_table) {
        if (pair.second.get_type() != sol::type::number) continue;
        
        const double num = pair.second.as<double>();
        sum += num;
      }
      
      const double rand = self->norm();
      const double final_num = rand * sum;
      
      double cumulative = 0.0;
      for (const auto &pair : num_table) {
        if (pair.second.get_type() != sol::type::number) continue;
        
        const double num = pair.second.as<double>();
        cumulative += num;
        if (final_num < cumulative) return pair.first; // возвращаем ключ
      }
      
      return sol::object(sol::nil);
    }
    
    void setup_lua_random_engine(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      utils.new_usertype<utils::random_engine_st>("random_engine", sol::constructors<utils::random_engine_st(), utils::random_engine_st(const uint64_t &)>(),
        "set_seed", &utils::random_engine_st::set_seed,
        "num", [] (utils::random_engine_st* self) -> int64_t { return uns_to_signed64(self->num()); },
        "norm", &utils::random_engine_st::norm,
        "index", [] (utils::random_engine_st* self, const size_t &size) -> size_t { return TO_LUA_INDEX(self->index(size)); },
        "unit3", [] (utils::random_engine_st* self) -> std::tuple<float, float, float> { const auto vec = self->unit3(); return std::tie(vec.x, vec.y, vec.z); },
        "random_at_most", &utils::random_engine_st::random_at_most,
        "closed", [] (utils::random_engine_st* self, const double a, const double b) -> double { return self->closed(a, b); },
        "probability", &utils::random_engine_st::probability,
        "gaussian_distribution", &utils::random_engine_st::gaussian_distribution,
        "weighted_distribution", &weighted_distribution
      );
    }
  }
}
