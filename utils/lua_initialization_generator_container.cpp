#include "lua_initialization.h"

#include "bin/generator_container.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_generator_container(sol::state_view lua) {
      auto generator = lua["generator"].get_or_create<sol::table>();
      auto container = generator.new_usertype<map::generator::container>("container",
        sol::no_constructor,
        "add_entity", &map::generator::container::add_entity,
        "set_entity_count", &map::generator::container::set_entity_count,
        "get_child", &map::generator::container::get_child,
        "get_childs_count", &map::generator::container::get_childs_count,
        "add_child", &map::generator::container::add_child,
        //"get_childs", &map::generator::container::get_childs,
        "get_childs", [] (const map::generator::container* self, const uint32_t &type, const uint32_t &index) { return std::ref(self->get_childs(type, index)); },
        "entities_count", &map::generator::container::entities_count,
        "type", &map::generator::container::type,
        "set_tile_template", &map::generator::container::set_tile_template,
        "set_entity_template", &map::generator::container::set_entity_template,
        "compute_memory_size", &map::generator::container::compute_memory_size,
        "set_data_f32", &map::generator::container::set_data<float>,
        "set_data_i32", &map::generator::container::set_data<int32_t>,
        "set_data_u32", &map::generator::container::set_data<uint32_t>,
        "set_data_vec3", [] (map::generator::container* self, const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type, const std::tuple<float, float, float> &vec) -> void {
          const glm::vec3 final_vec = glm::vec3(std::get<0>(vec), std::get<1>(vec), std::get<2>(vec));
          self->set_data<glm::vec3>(type, index, parameter_type, final_vec);
        },
        "get_data_f32", &map::generator::container::get_data<float>,
        "get_data_i32", &map::generator::container::get_data<int32_t>,
        "get_data_u32", &map::generator::container::get_data<uint32_t>,
        "get_data_vec3", [] (map::generator::container* self, const uint32_t &type, const uint32_t &index, const uint32_t &parameter_type) -> std::tuple<float, float, float> {
          const auto data = self->get_data<glm::vec3>(type, index, parameter_type);
          return std::tie(data.x, data.y, data.z);
        }
      );
      
      auto data_type_enum = generator.new_enum("data_type",
        {
          std::make_pair(std::string_view("uint_t"), map::generator::data_type::uint_t),
          std::make_pair(std::string_view("int_t"), map::generator::data_type::int_t),
          std::make_pair(std::string_view("float_t"), map::generator::data_type::float_t)
        }
      );
    }
  }
}
