#include "lua_initialization.h"

#include "render/shared_render_utility.h"
#include "magic_enum.hpp"
#include "linear_rng.h"

namespace devils_engine {
  namespace utils {
    static double create_pair_u32f32(const uint32_t &u32, const float &f32) {
      union convert { uint64_t u; double d; };
      const uint32_t &data = glm::floatBitsToUint(f32);
      const uint64_t packed_data = (uint64_t(u32) << 32) | uint64_t(data);
      convert c;
      c.u = packed_data;
      return c.d;
    }
    
    static std::tuple<uint32_t, float> unpack_pair_u32f32(const double &data) {
      union convert { uint64_t u; double d; };
      convert c;
      c.d = data;
      const uint64_t num = c.u;
      const uint32_t data1 = uint32_t(num >> 32);
      const uint32_t data2 = uint32_t(num);
      const float fdata = glm::uintBitsToFloat(data2);
      return std::tie(data1, fdata);
    }
    
    static double make_pair_u32u32(const uint32_t &u1, const uint32_t &u2) {
      union convert { uint64_t u; double d; };
      const uint64_t packed_data = (uint64_t(u1) << 32) | uint64_t(u2);
      convert c;
      c.u = packed_data;
      return c.d;
    }
    
    static std::tuple<uint32_t, uint32_t> unpack_pair_u32u32(const double &d) {
      union convert { uint64_t u; double d; };
      convert c;
      c.d = d;
      const uint64_t num = c.u;
      const uint32_t data1 = uint32_t(num >> 32);
      const uint32_t data2 = uint32_t(num);
      return std::tie(data1, data2);
    }
    
    void setup_lua_safe_utils(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      utils.set_function("prng32", render::prng);
      utils.set_function("prng32_2", render::prng2);
      utils.set_function("prng_normalize32", render::prng_normalize);
      utils.set_function("prng64", [] (const uint64_t &value) {
        return splitmix64::get_value(splitmix64::rng({value}));
      });
      utils.set_function("prng64_2", [] (const uint64_t &value1, const uint64_t &value2) {
        //if (value1 + value2 == 0) throw std::runtime_error("Summ of prng64_2 args must not be 0"); // ???
        return xoroshiro128starstar::get_value(xoroshiro128starstar::rng({value1, value2}));
      });
      utils.set_function("prng_normalize64", rng_normalize);
      
      utils.set_function("make_color", [] (const double r, const double g, const double b, const double a) {
        const uint8_t ur = uint8_t(255.0 * glm::clamp(r, 0.0, 1.0));
        const uint8_t ug = uint8_t(255.0 * glm::clamp(g, 0.0, 1.0));
        const uint8_t ub = uint8_t(255.0 * glm::clamp(b, 0.0, 1.0));
        const uint8_t ua = uint8_t(255.0 * glm::clamp(a, 0.0, 1.0));
        const uint32_t c = (uint32_t(ur) << 24) | (uint32_t(ug) << 16) | (uint32_t(ub) << 8) | (uint32_t(ua) << 0);
        return c;
      });
      
      utils.set_function("unpack_color", [] (const uint32_t data) -> std::tuple<double, double, double, double> {
        const uint8_t ur = uint8_t(data >> 24);
        const uint8_t ug = uint8_t(data >> 16);
        const uint8_t ub = uint8_t(data >>  8);
        const uint8_t ua = uint8_t(data >>  0);
        return std::make_tuple(255.0 / double(ur), 255.0 / double(ug), 255.0 / double(ub), 255.0 / double(ua));
      });
      
      utils.set_function("init_array", [] (const size_t &size, sol::object default_value, sol::this_state s) -> sol::table {
        sol::state_view view(s);
        auto t = view.create_table(size, 0);
        if (default_value.is<sol::table>()) {
          for (size_t i = 0; i < size; ++i) {
            t.add(view.create_table(30, 0));
          }
        } else {
          for (size_t i = 0; i < size; ++i) {
            t.add(default_value);
          }
        }
        return t;
      });
      
      utils.set_function("create_table", [] (sol::object arr_size, sol::object hash_size, sol::this_state s) -> sol::table {
        sol::state_view view(s);
        const uint32_t narr = arr_size.is<uint32_t>() ? arr_size.as<uint32_t>() : 100;
        const uint32_t nhash = hash_size.is<uint32_t>() ? hash_size.as<uint32_t>() : 100;
        return view.create_table(narr, nhash);
      });
      
      utils.set_function("create_pair_u32f32", &create_pair_u32f32);
      utils.set_function("unpack_pair_u32f32", &unpack_pair_u32f32);
      
      utils.set_function("create_pair_u32u32", &make_pair_u32u32);
      utils.set_function("unpack_pair_u32u32", &unpack_pair_u32u32);
    }
  }
}
