#include "lua_initialization_hidden.h"

//#include "globals.h"
#include "bin/camera.h"

namespace devils_engine {
  namespace utils {
    void setup_lua_camera(sol::state_view lua) {
      auto camera = lua["camera"].get_or_create<sol::table>();
      camera.set_function("valid", [] () { return camera::get_camera() != nullptr; });
      
      camera.set_function("move", [] (const double &horisontal_move, const double &vertical_move) {
        auto c = camera::get_camera();
        if (c == nullptr) throw std::runtime_error("Bad game state for camera manipulation");
        // где сделать вычисление переменных? хочется все же в луа, 
        // у меня этаже функция будет для перемещения по кнопке и по мышке
        c->move(horisontal_move, vertical_move);
      });
      
      camera.set_function("zoom", [] () {
        auto c = camera::get_camera();
        if (c == nullptr) throw std::runtime_error("Bad game state for camera manipulation");
        const float zoom_norm = (c->zoom() - c->min_zoom()) / (c->max_zoom() - c->min_zoom());
        return zoom_norm;
      });
      
      camera.set_function("pos", [] () -> std::tuple<double, double, double> {
        auto c = camera::get_camera();
        if (c == nullptr) throw std::runtime_error("Bad game state for camera manipulation");
        const auto pos = c->current_pos();
        return std::tie(pos.x, pos.y, pos.z);
      });
      
      camera.set_function("dir", [] () -> std::tuple<double, double, double> {
        auto c = camera::get_camera();
        if (c == nullptr) throw std::runtime_error("Bad game state for camera manipulation");
        const auto dir = c->dir();
        return std::tie(dir.x, dir.y, dir.z);
      });
      
      camera.set_function("front", [] () -> std::tuple<double, double, double> {
        auto c = camera::get_camera();
        if (c == nullptr) throw std::runtime_error("Bad game state for camera manipulation");
        const auto front = c->front();
        return std::tie(front.x, front.y, front.z);
      });
      
      camera.set_function("right", [] () -> std::tuple<double, double, double> {
        auto c = camera::get_camera();
        if (c == nullptr) throw std::runtime_error("Bad game state for camera manipulation");
        const auto right = c->right();
        return std::tie(right.x, right.y, right.z);
      });
      
      camera.set_function("up", [] () -> std::tuple<double, double, double> {
        auto c = camera::get_camera();
        if (c == nullptr) throw std::runtime_error("Bad game state for camera manipulation");
        const auto up = c->up();
        return std::tie(up.x, up.y, up.z);
      });
      
      camera.set_function("set_end_point", [] (const double x, const double y, const double z) {
        auto c = camera::get_camera();
        if (c == nullptr) throw std::runtime_error("Bad game state for camera manipulation");
        c->set_end_point(glm::vec3(x, y, z));
      });
    }
  }
}
