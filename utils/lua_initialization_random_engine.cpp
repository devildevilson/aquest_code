#include "lua_initialization_hidden.h"

#include "random_engine.h"
#include "magic_enum.hpp"

namespace devils_engine {
  namespace utils {
    void setup_lua_random_engine(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      utils.new_usertype<utils::random_engine_st>("random_engine", sol::constructors<utils::random_engine_st(), utils::random_engine_st(const uint64_t &)>(),
        "set_seed", &utils::random_engine_st::set_seed,
        "num", &utils::random_engine_st::num,
        "norm", &utils::random_engine_st::norm,
        "index", [] (utils::random_engine_st* self, const size_t &size) -> size_t { return self->index(size)+1; },
        "unit3", [] (utils::random_engine_st* self) -> std::tuple<float, float, float> { const auto vec = self->unit3(); return std::tie(vec.x, vec.y, vec.z); },
        "random_at_most", &utils::random_engine_st::random_at_most,
        //"closed", &utils::random_engine_st::closed,
        //"closed", &utils::random_engine_st::closed,
        "closed", [] (utils::random_engine_st* self, const double a, const double b) -> double { return self->closed(a, b); },
        "probability", &utils::random_engine_st::probability,
        "gaussian_distribution", &utils::random_engine_st::gaussian_distribution
      );
    }
  }
}
