#include "map_creator.h"
#include "interface_context.h"
#include "utils/globals.h"
#include "fmt/format.h"
#include "generator_system2.h"
#include "map_generators2.h"
#include "utils/random_engine.h"
#include "FastNoise.h"
#include "generator_container.h"
#include "render/render_mode_container.h"

#include <stdexcept>
#include <random>
#include <iostream>

namespace devils_engine {
  namespace map {
    property_int::property_int(const create_info &info) : 
      min(info.min), 
      default_val(info.default_val),
      max(info.max), 
      step(info.step), 
      pixel_step(info.pixel_step), 
      prop_name(info.prop_name), 
      var_name(info.var_name) 
    {}
    
    void property_int::draw(sol::table &table) {
      auto interface = global::get<interface::context>();
      auto ctx = &interface->ctx;
      
      nk_layout_row_dynamic(ctx, 30.0f, 1);
      table["userdata"][var_name] = nk_propertyi(ctx, prop_name.c_str(), min, table["userdata"][var_name], max, step, pixel_step);
    }
    
    void property_int::set_default_value(sol::table &table) {
      table["userdata"][var_name] = default_val;
    }
    
    property_float::property_float(const create_info &info) :
      min(info.min), 
      default_val(info.default_val), 
      max(info.max), 
      step(info.step), 
      pixel_step(info.pixel_step), 
      prop_name(info.prop_name), 
      var_name(info.var_name) 
    {}
    
    void property_float::draw(sol::table &table) {
      auto interface = global::get<interface::context>();
      auto ctx = &interface->ctx;
      
      nk_layout_row_dynamic(ctx, 30.0f, 1);
      table["userdata"][var_name] = nk_propertyf(ctx, prop_name.c_str(), min, table["userdata"][var_name], max, step, pixel_step);
    }
    
    void property_float::set_default_value(sol::table &table) {
      table["userdata"][var_name] = default_val;
    }
    
    step::step(const bool first, const size_t &container_size, const std::string &name, const std::vector<map::generator_pair> &pairs, const std::string &rendering_mode) : 
      first(first), 
      container(container_size), 
      pairs(pairs), 
      name(name), 
      rendering_mode(rendering_mode)
    {}
    
    step::~step() {
      for (auto p : variables) {
        container.destroy(p);
      }
    }
    
    int32_t step::prepare(systems::generator &gen, map::generator::context* context, sol::table &table) {
      auto interface = global::get<interface::context>();
      auto ctx = &interface->ctx;
      
      static uint32_t random_seed = 1;
      static char buffer[16] = "1";
      static int32_t buffer_len = 1;
      
      int32_t code = 0;
      if (nk_begin(ctx, name.c_str(), nk_rect(5, 5, 400, 400), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
        nk_layout_row_static(ctx, 30.0f, 400, 1);
        nk_label(ctx, name.c_str(), NK_TEXT_ALIGN_LEFT);
        
        if (first) {
          const float ratio[] = {0.6, 0.4};
          nk_layout_row(ctx, NK_DYNAMIC, 30.0f, 2, ratio);
          nk_edit_string(ctx, NK_EDIT_SIMPLE, buffer, &buffer_len, 11, nk_filter_decimal);
          if (nk_button_label(ctx, "Randomize")) {
            std::random_device dev;
            random_seed = dev();
            const std::string str = fmt::format(FMT_STRING("{}"), random_seed);
//                 std::cout << "Randomized " << str << " size " << str.size() << " length " << str.length() << "\n";
            memcpy(buffer, str.c_str(), str.size());
            buffer_len = str.length();
          }
          
          buffer[buffer_len] = '\0';
          const size_t num = atol(buffer);
//               ASSERT(num != 0);
          
          if (num > UINT32_MAX) {
            random_seed = UINT32_MAX;
          } else {
            random_seed = num;
          }
          
          if (num > UINT32_MAX) {
            const std::string str = fmt::format(FMT_STRING("{}"), random_seed);
            memcpy(buffer, str.c_str(), str.size());
            buffer_len = str.size();
          }
        }
        
        for (auto p : variables) {
          p->draw(table);
        }
        
        nk_layout_row_static(ctx, 30.0f, 199, 2);
        if (first) nk_spacing(ctx, 1);
        else {
          if (nk_button_label(ctx, "Back")) code = -1;
        }
        
        if (nk_button_label(ctx, "Generate")) {
          gen.clear();
          for (const auto &pair : pairs) {
            gen.add(pair);
          }
          
          if (first) {
            union transform {
              uint32_t valu;
              int32_t vali;
            };
            
            transform t;
            t.valu = random_seed;
            
            context->random->set_seed(random_seed);
            context->noise->SetSeed(t.vali);
          }
          
          gen.generate(context, table);
          render::mode(rendering_mode);
          code = 1;
        }
      }
      nk_end(ctx);
      
      return code;
    }
    
    creator::creator() : table(lua.create_table()), current_step(0), random(1), noise(1) {
      ctx.container = global::get<map::generator::container>();
      ctx.map = global::get<core::map>();
      ctx.noise = &noise;
      ctx.random = &random;
      ctx.pool = global::get<dt::thread_pool>();
      
      table["userdata"] = lua.create_table();
    }
    
    creator::~creator() {
      for (auto p : steps) {
        steps_pool.destroy(p);
      }
    }
    
    step* creator::create(const bool first, const size_t &container_size, const std::string &name, const std::vector<map::generator_pair> &pairs, const std::string &rendering_mode) {
      auto step = steps_pool.create(first, container_size, name, pairs, rendering_mode);
      steps.push_back(step);
      return step;
    }
    
    void creator::generate() {
      current_step += steps[current_step]->prepare(gen, &ctx, table);
    }
    
    sol::table & creator::get_table() {
      return table;
    }
    
    bool creator::finished() const {
      return current_step == steps.size();
    }
  }
}
