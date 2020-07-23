#include "overlay.h"
#include "interface_context.h"
#include "utils/globals.h"
#include <stdexcept>
#include <random>
#include <iostream>
#include "fmt/format.h"
// #include "generator_system.h"
#include "generator_system2.h"
// #include "map_generators.h"
#include "map_generators2.h"
#include "utils/random_engine.h"
#include "FastNoise.h"

namespace devils_engine {
  namespace overlay {
    void debug(const uint32_t &picked_tile_index) {
      if (picked_tile_index == UINT32_MAX) return;
      
      auto interface = global::get<interface::context>();
      auto ctx = &interface->ctx;
      
      auto old_color = ctx->style.window.background;
      interface::style::background_color bg_color(ctx, nk_color{old_color.r, old_color.g, old_color.b, 128});
      
      nk_style_set_font(ctx, &global::get<interface::context>()->fonts[fonts::technical]->handle);
      
      if (nk_begin(ctx, "debug interface", nk_rect(5, 5, 400, 400), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
        nk_layout_row_static(ctx, 30.0f, 400, 1);
        {
          const std::string str = "Tile index " + std::to_string(picked_tile_index);
          nk_label(ctx, str.c_str(), NK_TEXT_LEFT);
        }
      }
      nk_end(ctx);
      
    }
    
#define INITIAL_STEP 0
#define LAND_WEATHER_BIOMES 1
#define GOVERMENTS_PROVINCES 2
    
    state debug_generator(systems::generator* gen, map::generator::context* context, sol::table &table) {
      //if (current_generator > GOVERMENTS_PROVINCES) return false;
      
      auto interface = global::get<interface::context>();
      auto ctx = &interface->ctx;
      
      static uint32_t random_seed = 1;
      static char buffer[128] = "1";
      static int32_t buffer_len = 1;
      
      static float ocean_percentage = 0.7f;
      static int plates_count = 199;
      
      static uint32_t current_generator = INITIAL_STEP;
      state current_state = state::waiting;
      
      if (current_generator > GOVERMENTS_PROVINCES) return state::end;
      
      if (nk_begin(ctx, "modify generator data", nk_rect(5, 5, 400, 400), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
        nk_layout_row_static(ctx, 30.0f, 400, 1);
        switch (current_generator) {
          case INITIAL_STEP: {
            nk_label(ctx, "Tectonic plates generator", NK_TEXT_ALIGN_LEFT);
            
            {
              nk_layout_row_static(ctx, 30.0f, 300, 2);
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
            
            nk_layout_row_static(ctx, 30.0f, 400, 1);
            {
//               const int32_t old_value = data.plates_count;
//               const int32_t val = nk_slide_int(ctx, 40, old_value, 300, 1);
//               data.plates_count = val;
              //plates_count = nk_slide_int(ctx, 40, plates_count, 300, 1);
              plates_count = nk_propertyi(ctx, "plates_count", 40, plates_count, 300, 1, 0.5f);
            }
            
            {
              //const float old_value = data.ocean_percentage;
              //const float val = nk_slide_float(ctx, 0.0f, old_value, 1.0f, 0.01f);
              //data.ocean_percentage = val;
              ocean_percentage = nk_propertyf(ctx, "ocean_percentage", 0.0f, ocean_percentage, 1.0f, 0.01f, 0.005f);
            }
            
            if (nk_button_label(ctx, "Generate")) {
              // создаем здесь генератор
              //++current_generator;
              //std::cout << "Generate" << "\n";
              // тут по идее мы будем обращаться к конфигу для того чтобы это все создать
//               systems::generator<map::generator_context> system(2 * 1024);
//               system.add_generator<map::beginner>(map::beginner::create_info{pool});
//               system.add_generator<map::plates_generator>(map::plates_generator::create_info{pool});
//               system.add_generator<map::plate_datas_generator>(map::plate_datas_generator::create_info{pool});
              
              gen->clear();
              gen->add(map::default_generator_pairs[0]);
              gen->add(map::default_generator_pairs[1]);
              gen->add(map::default_generator_pairs[2]);
              ++current_generator;
              
              table["userdata"]["plates_count"] = plates_count;
              table["userdata"]["ocean_percentage"] = ocean_percentage;
              
              context->random = new utils::random_engine_st(random_seed);
              context->noise = new FastNoise(random_seed);
              
              // мне либо сейчас быстренько переписывать генератор под новые условия
              // либо потом переписывать еще больше кода
              // как переписать?
              // у карты по идее должны быть режимы отображения (рандомный цвет по индексу, текстурка биома, цвет по высоте, цвет из банка цветов)
              // их нужно както заполнять, по идее меняем режим отбражения, обходим нужный компонент и засовываем его в тайл
              // это покрывает более менее все использования 
              current_state = state::constructed_generator;
            }
            
            break;
          }
          
          case LAND_WEATHER_BIOMES: {
            nk_label(ctx, "Biomes generator", NK_TEXT_ALIGN_LEFT);
            
            table["userdata"]["noise_multiplier"] = nk_propertyf(ctx, "noise_multiplier", 0.0f, table["userdata"]["noise_multiplier"], 0.5f, 0.01f, 0.005f);
            table["userdata"]["blur_ratio"] = nk_propertyf(ctx, "blur_ratio", 0.0f, table["userdata"]["blur_ratio"], 1.0f, 0.01f, 0.005f);
            table["userdata"]["blur_water_ratio"] = nk_propertyf(ctx, "blur_water_ratio", 0.0f, table["userdata"]["blur_water_ratio"], 2.0f, 0.01f, 0.005f);
            table["userdata"]["blur_iterations_count"] = nk_propertyi(ctx, "blur_iterations_count", 0, table["userdata"]["blur_iterations_count"], 10, 1, 0.005f);
            
            if (nk_button_label(ctx, "Generate")) {
              gen->clear();
              gen->add(map::default_generator_pairs[3]);
              gen->add(map::default_generator_pairs[4]);
              gen->add(map::default_generator_pairs[5]);
              gen->add(map::default_generator_pairs[6]);
              gen->add(map::default_generator_pairs[7]);
              gen->add(map::default_generator_pairs[8]);
              gen->add(map::default_generator_pairs[9]);
              gen->add(map::default_generator_pairs[10]);
              gen->add(map::default_generator_pairs[11]);
              gen->add(map::default_generator_pairs[12]);
              ++current_generator;
              
              current_state = state::constructed_generator;
            }
            break;
          }
          
          case GOVERMENTS_PROVINCES: {
            nk_label(ctx, "Countries generator", NK_TEXT_ALIGN_LEFT);
            
            table["userdata"]["provinces_count"] = nk_propertyi(ctx, "provinces_count", 1000, table["userdata"]["provinces_count"], 5000, 50, float(5000 - 1000) / 400.0f);
            table["userdata"]["history_iterations_count"] = nk_propertyi(ctx, "history_iterations_count", 200, table["userdata"]["history_iterations_count"], 1000, 10, float(1000 - 200) / 400.0f);
            
            if (nk_button_label(ctx, "Generate")) {
              gen->clear();
              gen->add(map::default_generator_pairs[13]);
              gen->add(map::default_generator_pairs[14]);
              gen->add(map::default_generator_pairs[15]);
              gen->add(map::default_generator_pairs[16]);
              gen->add(map::default_generator_pairs[17]);
              ++current_generator;
              
              current_state = state::constructed_generator;
            }
            break;
          }
          
          //default: throw std::runtime_error("not implemented");
        }
        nk_end(ctx);
      }
      
      return current_state;
    }
  }
}
