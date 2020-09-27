#include "interface_container.h"

#include <iostream>

#include "bin/interface_context.h"
#include "lua_initialization.h"
#include "globals.h"
#include "input.h"

namespace devils_engine {
  namespace utils {
    timer::timer() : current_time(SIZE_MAX), end_time(0) {}
    timer::timer(const size_t &end_time) : current_time(0), end_time(end_time) {}
    timer::~timer() { set_invalid(); }
    bool timer::valid() const { return end_time != 0; }
    size_t timer::current() const { return current_time; }
    size_t timer::end() const { return end_time; }
    double timer::norm() const { return valid() ? double(current_time) / double(end_time) : 0.0; }
    void timer::reset() { current_time = 0; }
    void timer::update(const size_t &time) { current_time += valid() ? time : 0; }
    void timer::set_invalid() { end_time = 0; }

    timer_view::timer_view(timer* t) : t(t) {}
    timer_view::~timer_view()          { if (t) t->set_invalid(); t = nullptr; }
    bool timer_view::valid() const     { return t == nullptr ? false : t->valid(); }
    size_t timer_view::current() const { return t == nullptr ? 0     : t->current(); }
    size_t timer_view::end() const     { return t == nullptr ? 0     : t->end(); }
    double timer_view::norm() const    { return t == nullptr ? 0.0f  : t->norm(); }
    void timer_view::reset()           { if (t) t->reset(); }

    static_assert(interface_container::fonts_count == fonts::count);

    size_t interface_container::first_layer() { return 0; }
    size_t interface_container::last_layer() { return maximum_openned_layers-1; }

    interface_container::interface_container() : layers_table(lua.create_table()) {
      lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math, sol::lib::string, sol::lib::utf8, sol::lib::bit32, sol::lib::package);
      // нужно сделать свой собственный require видимо, который будет подгружать только нужные файлы
      setup_lua_package_path(lua);
      const std::string script_dir = global::root_directory() + "scripts/";
      // в донт старв тогезер скрипты загружались в специальном сэндбоксе, который во первых не позволял
      // загружать бинарники, а также запрещал использование некоторых функций (например os.execute)
      // а io.open путь выходящий за пределы папки с игрой приводил к ошибке, мне нужно сделать примерно то же самое
//       lua.require_file("nk", script_dir + "moonnuklear.so", true, sol::load_mode::binary); // не работает, либо нужно делать сэндбокс (сол энвайронмент?), либо разрешить тогда уж че там
      auto script_from_file_result = lua.safe_script_file(script_dir + "interface_init.lua");
      if (!script_from_file_result.valid()) {
        sol::error err = script_from_file_result;
        std::cout << err.what();
        throw std::runtime_error("Error in interface_init.lua file");
      }

      std::tuple<sol::function, sol::function, sol::function> ret = script_from_file_result;
      get_ctx = std::get<0>(ret);
      get_font = std::get<1>(ret);
      free_font = std::get<2>(ret);

      {
        auto ctx = &global::get<devils_engine::interface::context>()->ctx;
        auto res = get_ctx(sol::make_light(ctx));
        if (!res.valid()) {
          sol::error err = res;
          std::cout << err.what();
          throw std::runtime_error("Could not make interface context");
        }

        sol::object obj = res.get<sol::object>();
        moonnuklear_ctx = obj;

        // std::cout << "context      " << ctx << '\n';
        // std::cout << "default font " << ctx->style.font << '\n';
      }

      // {
      //   for (uint32_t i = 0; i < fonts::count; ++i) {
      //     auto font = global::get<devils_engine::interface::context>()->fonts[i];
      //     //assert(font != nullptr);
      //     if (font == nullptr) continue;
      //     std::cout << "font to lua " << font << "\n";
      //     auto res = get_font(sol::make_light(font));
      //     if (!res.valid()) {
      //       sol::error err = res;
      //       std::string what = err.what();
      //       std::cout << what << std::endl;
      //       throw std::runtime_error("Could not make interface font");
      //     }
      //
      //     fonts[i] = res.get<sol::object>();
      //   }
      // }
      make_fonts();

      // {
      //   auto script_from_file_result = lua.safe_script_file(script_dir + "interface_test.lua");
      //   if (!script_from_file_result.valid()) {
      //     sol::error err = script_from_file_result;
      //     std::cout << err.what();
      //     throw std::runtime_error("Error in interface_test.lua file");
      //   }
      //
      //   lua["test"](moonnuklear_ctx);
      // }

      lua["get_font"] = [this] (const size_t &index) {
        if (index >= fonts::count) throw std::runtime_error("Bad font index");
        return fonts[index];
      };
    }

    void interface_container::draw(const size_t &time) {
      for (uint32_t i = 0; i < timers.size(); ++i) {
        if (!timers[i].valid()) continue;
        timers[i].update(time);
      }

       for (size_t i = 0; i < maximum_openned_layers; ++i) {
        if (!close_layers.get(i)) continue;
        openned_layers[i].function = sol::nil;
        //openned_layers[i].data = sol::make_object(lua, sol::nil);
        openned_layers[i].args.clear();
//         openned_layers[i].args.shrink_to_fit();
        close_layers.set(i, false);
      }

      for (size_t i = 0; i < open_layers.size(); ++i) {
        if (open_layers[i].second == sol::nil) continue;
        if (openned_layers[i].function != sol::nil) throw std::runtime_error("Trying to override interface layer " + std::to_string(i));
        openned_layers[i].function = open_layers[i].second;
        openned_layers[i].args = std::move(open_layers[i].first);
        openned_layers[i].ret = sol::make_object(lua, sol::nil);
        open_layers[i].second = sol::nil;
        //open_layers[i].first = sol::make_object(lua, sol::nil);
        open_layers[i].first.clear();
//         open_layers[i].first.shrink_to_fit();
      }

      for (size_t i = 0; i < openned_layers.size(); ++i) {
        if (openned_layers[i].function == sol::nil) continue;
        const auto ret = openned_layers[i].function(moonnuklear_ctx, sol::as_args(openned_layers[i].args));
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Error in interface function");
        }

        openned_layers[i].ret = ret;
        input::increase_layer();
      }
    }

    bool interface_container::close_layer(const uint32_t &index) {
      if (index >= maximum_openned_layers) throw std::runtime_error("Bad layer index, maximum is " + std::to_string(maximum_openned_layers));
      if (openned_layers[index].function == sol::nil) return false;
      close_layers.set(index, true);
      return true;
    }

    void interface_container::force_close_layer(const uint32_t &index) {
      if (index >= maximum_openned_layers) throw std::runtime_error("Bad layer index, maximum is " + std::to_string(maximum_openned_layers));
      openned_layers[index].function = sol::nil;
      openned_layers[index].args.clear();
    }

    void interface_container::close_all() {
      for (uint32_t i = 0; i < maximum_openned_layers; ++i) { close_layers.set(i, true); }
    }

//     bool interface_container::open_layer(const uint32_t &index, const std::string_view &name, const sol::object &data) {
//       if (index >= maximum_openned_layers) throw std::runtime_error("Bad layer index, maximum is " + std::to_string(maximum_openned_layers));
//       if (openned_layers[index].function != sol::nil) return false;
//       auto proxy = layers_table[name];
//       if (proxy.valid()) throw std::runtime_error("Could not find layer " + std::string(name));
//       if (proxy.get_type() != sol::type::function) throw std::runtime_error(std::string(name) + " is not a function");
//       open_layers[index].second = proxy.get<sol::function>();
//       open_layers[index].first = data;
//       return true;
//     }

    bool interface_container::open_layer(const uint32_t &index, const std::string_view &name, const std::initializer_list<sol::object> &data) {
      if (index >= maximum_openned_layers) throw std::runtime_error("Bad layer index, maximum is " + std::to_string(maximum_openned_layers));
      if (!close_layers.get(index) && openned_layers[index].function != sol::nil) return false;
      auto proxy = layers_table[name];
      if (!proxy.valid()) throw std::runtime_error("Could not find layer " + std::string(name));
      if (proxy.get_type() != sol::type::function) throw std::runtime_error(std::string(name) + " is not a function");
      open_layers[index].second = proxy.get<sol::function>();
      open_layers[index].first = data;

      return true;
    }

    bool interface_container::force_open_layer(const uint32_t &index, const std::string_view &name, const std::initializer_list<sol::object> &data) {
      if (index >= maximum_openned_layers) throw std::runtime_error("Bad layer index, maximum is " + std::to_string(maximum_openned_layers));
      if (openned_layers[index].function != sol::nil) return false;
      auto proxy = layers_table[name];
      if (!proxy.valid()) throw std::runtime_error("Could not find layer " + std::string(name));
      if (proxy.get_type() != sol::type::function) throw std::runtime_error(std::string(name) + " is not a function");
      openned_layers[index].function = proxy.get<sol::function>();
      openned_layers[index].args = data;

      return true;
    }

    bool interface_container::update_data(const uint32_t &index, const std::initializer_list<sol::object> &data) {
      if (index >= maximum_openned_layers) throw std::runtime_error("Bad layer index, maximum is " + std::to_string(maximum_openned_layers));
      if (openned_layers[index].function == sol::nil) return false;
      openned_layers[index].args = data;
      return true;
    }

    bool interface_container::is_visible(const uint32_t &index) const {
      if (index >= maximum_openned_layers) throw std::runtime_error("Bad layer index, maximum is " + std::to_string(maximum_openned_layers));
      return (!close_layers.get(index) && openned_layers[index].function != sol::nil) || open_layers[index].second != sol::nil;
    }

    timer_view interface_container::create_timer(const size_t &end_time) {
      for (uint32_t i = 0; i < timers.size(); ++i) {
        if (timers[i].valid()) continue;
        timers[i] = timer(end_time);
        return timer_view(&timers[i]);
      }

      return timer_view(nullptr);
    }

    void interface_container::release_timer(timer_view t) {
      t.~timer_view();
    }

    void interface_container::process_script_file(const std::string &path, const bool return_function, const std::string &function_name) {
      auto ret = lua.script_file(path);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("Bad interface script");
      }

      if (return_function) {
        if (ret.get_type() != sol::type::function) throw std::runtime_error("Script return is not a function");
        if (function_name.empty()) throw std::runtime_error("Bad function name");
        if (layers_table[function_name].valid()) throw std::runtime_error("Trying to override function " + std::string(function_name));
        sol::function f = ret;
        layers_table[function_name] = f;
      }
    }

    void interface_container::process_script(const std::string_view &script, const bool return_function, const std::string &function_name) {
      auto ret = lua.script(script);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("Bad interface script");
      }

      if (return_function) {
        if (ret.get_type() != sol::type::function) throw std::runtime_error("Script return is not a function");
        if (function_name.empty()) throw std::runtime_error("Bad function name");
        if (layers_table[function_name].valid()) throw std::runtime_error("Trying to override function " + std::string(function_name));
        sol::function f = ret;
        layers_table[function_name] = f;
      }
    }

    void interface_container::register_function(const std::string_view &lua_name, const std::string_view &container_name) {
      auto proxy = lua[lua_name];
      if (!proxy.valid()) throw std::runtime_error("Could not find function " + std::string(lua_name));
      if (proxy.get_type() != sol::type::function) throw std::runtime_error(std::string(lua_name) + " is not a function");
      if (layers_table[container_name].valid()) throw std::runtime_error("Trying to override function " + std::string(container_name));
      layers_table[container_name] = proxy.get<sol::function>();
      lua[lua_name] = sol::nil;
    }

    void interface_container::init_constants() {
      setup_lua_constants(lua);
    }

    void interface_container::init_input() {
      setup_lua_input(lua);
    }

    void interface_container::init_types() {
      setup_lua_types(lua);
    }

    void interface_container::init_game_logic() {
      setup_lua_game_logic(lua);
    }

    void interface_container::free_fonts() {
      for (uint32_t i = 0; i < fonts::count; ++i) {
        auto res = free_font(fonts[i]);
        if (!res.valid()) {
          sol::error err = res;
          std::cout << err.what();
          throw std::runtime_error("Could not free interface font");
        }
      }
    }

    void interface_container::make_fonts() {
      for (uint32_t i = 0; i < fonts::count; ++i) {
        auto font = global::get<devils_engine::interface::context>()->fonts[i];
        //assert(font != nullptr);
        if (font == nullptr) continue;
        //std::cout << "font to lua " << font << "\n";
        auto res = get_font(sol::make_light(font));
        if (!res.valid()) {
          sol::error err = res;
          std::string what = err.what();
          std::cout << what << std::endl;
          throw std::runtime_error("Could not make interface font");
        }

        fonts[i] = res.get<sol::object>();
      }
    }
  }
}
