#include "interface_context.h"

#include "render/window.h"
// #include "Helper.h"
#include "utils/globals.h"
#include "render/container.h"

#include <fstream>
#include "nlohmann/json.hpp"

#define STB_RECT_PACK_IMPLEMENTATION
// #include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
// #include "stb_truetype.h"

#define NK_IMPLEMENTATION
#include "render/nuklear_header.h"
#include <GLFW/glfw3.h>

struct fonts_settings {
  const char* name;
  float size;
};

struct fonts_settings2 {
  std::string name;
  float size;
};

const fonts_settings fonts_names[] = {
  //"Junicode.ttf",
  //"Inconsolata-Regular.ttf",
  //"NotoSans-Black.ttf",
//   "CODE2000.TTF", // этот шрифт платный
  {"joystix monospace.ttf", 13.0f},      // technical
  {"Nightmare_Hero_Normal.ttf", 100.0f}, // menu 
  {"Anton-Regular.ttf", 52.0f},      // interface
  {"NotoSans-Black.ttf", 13.0f}          // description
};

// нужно чекать какие символы присутствуют в шрифтах
const nk_rune unicode_glyph_ranges[] = {
  0x0020, 0x00FF,
//   0x2200, 0x22FF, // Mathematical Operators
//   0x25A0, 0x25FF,
//   0x2580, 0x259F,
//   0x3000, 0x303F, // CJK Symbols and Punctuation
//   0x3040, 0x309F, // Hiragana
//   0x30A0, 0x30FF, // Katakana
//   0x0370, 0x03FF, // Greek and Coptic
//   0xFF00, 0xFFEF, // Halfwidth and Fullwidth Forms
//   0x4E00, 0x9FFF, // CJK Unified Ideographs
  0
};

namespace devils_engine {
  void clipbardPaste(nk_handle usr, nk_text_edit *edit) {
      const char *text = glfwGetClipboardString(reinterpret_cast<render::window*>(usr.ptr)->handle);

      if (text) nk_textedit_paste(edit, text, nk_strlen(text));
  }

  void clipbardCopy(nk_handle usr, const char *text, const int len) {
    if (len == 0) return;

    char str[len+1];
    memcpy(str, text, len);
    str[len] = '\0';

    glfwSetClipboardString(reinterpret_cast<render::window*>(usr.ptr)->handle, str);
  }
  
  nk_handle nk_handle_image(const render::image_t &img) {
    const size_t cont = img.container;
    auto ptr = reinterpret_cast<void*>(cont);
    return nk_handle{ptr};
  }

  render::image_t image_nk_handle(const nk_handle &handle) {
    const size_t tmp = reinterpret_cast<size_t>(handle.ptr);
    return render::image_t{uint32_t(tmp)};
  }

  struct nk_image image_to_nk_image(const render::image_t &img) {
    return nk_image_handle(nk_handle_image(img));
  }
  
  namespace interface {
    void load_font_settings(const nlohmann::json &json, std::vector<fonts_settings2> &fonts) {
      for (auto itr = json.begin(); itr != json.end(); ++itr) {
        if (itr.value().is_object()) {
          size_t index = fonts::count;
          
          if (itr.key() == "technical") {
            index = fonts::technical;
          } else if (itr.key() == "menu") {
            index = fonts::menu;
          } else if (itr.key() == "interface") {
            index = fonts::interface;
          } else if (itr.key() == "description") {
            index = fonts::description;
          } else continue;
          
          for (auto font_itr = itr.value().begin(); font_itr != itr.value().end(); ++font_itr) {
            if (font_itr.value().is_string() && font_itr.key() == "name") {
              fonts[index].name = font_itr.value().get<std::string>();
              continue;
            }
            
            if (font_itr.value().is_number() && font_itr.key() == "size") {
              fonts[index].size = font_itr.value().get<float>();
              continue;
            }
          }
          continue;
        }
      }
      
    }
    
    void load_font(const std::string &path, std::vector<char> &mem) {
      std::fstream file(path, std::ios::in | std::ios::binary);
      if (!file) throw std::runtime_error("Could not load file "+path);
      
      file.ignore(std::numeric_limits<std::streamsize>::max());
      size_t length = file.gcount();
      file.clear();   //  Since ignore will have set eof.
      file.seekg(0, std::ios_base::beg);
      
      mem.resize(length);
      file.read(mem.data(), length);
    }
    
    context::context(yavf::Device* device, render::window* window) : device(device) {
      nlohmann::json j;
      {
        std::fstream file(global::root_directory()+"fonts/fonts.json");
        ASSERT(file);
        file >> j;
      }
      
//       int window_width, window_height;
//       glfwGetWindowSize(window->handle(), &window_width, &window_height);
      uint32_t window_height = window->surface.extent.height;
      static const float data_window_height = 720.0f;
      
      memset(fonts, 0, sizeof(fonts[0]) * fonts::count);
      
      std::vector<fonts_settings2> fonts_data(fonts::count);
      load_font_settings(j, fonts_data);

      nk_buffer_init_default(&cmds);

      {
        const void *image;
        int w, h;
        nk_font_atlas_init_default(&atlas);
        nk_font_atlas_begin(&atlas);
        
        std::vector<char> memory[fonts::count];
        for (size_t i = 0; i < fonts::count; ++i) {
          if (!fonts_data[i].name.empty()) {
            load_font(global::root_directory()+"fonts/"+ fonts_data[i].name, memory[i]);
            void* font_mem = memory[i].data();
            const size_t font_size = memory[i].size();
            // размер шрифта должен определяться используя dpi экрана
            // а с размером шрифта должны меняться некоторые элементы интерфейса (или все?)
            const float size = fonts_data[i].size * (float(window_height) / data_window_height);
//             struct nk_font_config config = nk_font_config(size);
//             config.oversample_h = 1;
//             config.oversample_v = 1;
//             config.range = &unicode_glyph_ranges[0];
// //             config.range = nk_font_cyrillic_glyph_ranges();
//             fonts[i] = nk_font_atlas_add_from_memory(&atlas, font_mem, font_size, size, &config);
            fonts[i] = nk_font_atlas_add_from_memory(&atlas, font_mem, font_size, size, nullptr);
          } else {
//             fonts[i] = fonts[fonts::technical];
//             ASSERT(fonts[i] != nullptr);
          }
        }
        if (fonts[fonts::technical] == nullptr) throw std::runtime_error("System font must exist");
        image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
//         std::cout << "backed atlas width " << w << " height " << h << "\n";
        
        yavf::Image* img = nullptr;
        //yavf::ImageView* view = nullptr;
        {
          auto staging = device->create(yavf::ImageCreateInfo::texture2DStaging({static_cast<uint32_t>(w), static_cast<uint32_t>(h)}), VMA_MEMORY_USAGE_CPU_ONLY);

          const size_t imageSize = w * h * 4;
          memcpy(staging->ptr(), image, imageSize);

          img = device->create(yavf::ImageCreateInfo::texture2D({static_cast<uint32_t>(w), static_cast<uint32_t>(h)},
                                                                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                              VMA_MEMORY_USAGE_GPU_ONLY);

          yavf::TransferTask* task = device->allocateTransferTask();

          task->begin();
          task->setBarrier(staging, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
          task->setBarrier(img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
          task->copy(staging, img);
          task->setBarrier(img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          task->end();

          task->start();
          task->wait();

          device->deallocate(task);
          device->destroy(staging);

          view = img->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

          // тут нужен еще сэмплер и дескриптор
          yavf::Sampler sampler;
          {
            auto container = global::get<render::container>();
            int enable = VK_FALSE;
            float level = 1.0f;
            if (container->is_properties_presented(render::container::physical_device_sampler_anisotropy)) {
              enable = VK_TRUE;
              level = 16.0f;
            }
            
            yavf::SamplerMaker sm(device);

            sampler = sm.addressMode(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT)
                        .anisotropy(enable, level)
                        .borderColor(VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK)
                        .compareOp(VK_FALSE, VK_COMPARE_OP_GREATER)
                        .filter(VK_FILTER_NEAREST, VK_FILTER_NEAREST)
                        .lod(0.0f, 1.0f)
                        .mipmapMode(VK_SAMPLER_MIPMAP_MODE_NEAREST)
                        .unnormalizedCoordinates(VK_FALSE)
                        .create("default_nuklear_sampler");

    //         img->setSampler(sampler);
          }

          {
            yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
            yavf::DescriptorSetLayout sampled_image_layout = device->setLayout(SAMPLED_IMAGE_LAYOUT_NAME);
            {
              yavf::DescriptorLayoutMaker dlm(device);

              if (sampled_image_layout == VK_NULL_HANDLE) {
                sampled_image_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).create(SAMPLED_IMAGE_LAYOUT_NAME);
              }
            }

            yavf::DescriptorMaker dm(device);

            auto d = dm.layout(sampled_image_layout).create(pool)[0];

            const size_t i = d->add({sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER});
            view->setDescriptor(d, i);
            atlas_descriptor = d;
            descriptor_index = i;
          }
        }

        //nk_font_atlas_end(&atlas, nk_handle_ptr(view), &null);
        nk_font_atlas_end(&atlas, nk_handle_image(render::image_t{UINT32_MAX}), &null);
      }

      nk_init_default(&ctx, &fonts[fonts::technical]->handle);

      ctx.clip.copy = clipbardCopy;
      ctx.clip.paste = clipbardPaste;
      ctx.clip.userdata = nk_handle_ptr(window);
    }
    
    context::~context() {
      nk_font_atlas_clear(&atlas);
      nk_buffer_free(&cmds);
      nk_free(&ctx);
    }
    
    void context::remake_font_atlas(const uint32_t &window_width, const uint32_t &window_height) {
      nk_font_atlas_clear(&atlas);
      device->destroy(view->image());
      memset(fonts, 0, sizeof(fonts[0]) * fonts::count);
      
      // нужно определиться к каким размерам экрана подгонять размер шрифта
      static const float data_window_height = 720.0f;
      
      nlohmann::json j;
      {
        std::fstream file(global::root_directory()+"fonts/fonts.json");
        ASSERT(file);
        file >> j;
      }
      
      std::vector<fonts_settings2> fonts_data(fonts::count);
      load_font_settings(j, fonts_data);
      
      {
        const void *image;
        int w, h;
        nk_font_atlas_init_default(&atlas);
        nk_font_atlas_begin(&atlas);
        
        std::vector<char> memory[fonts::count];
        for (size_t i = 0; i < fonts::count; ++i) {
          if (!fonts_data[i].name.empty()) {
            load_font(global::root_directory()+"fonts/"+ fonts_data[i].name, memory[i]);
            void* font_mem = memory[i].data();
            const size_t font_size = memory[i].size();
            // размер шрифта должен определяться используя dpi экрана
            // а с размером шрифта должны меняться некоторые элементы интерфейса (или все?)
            const float size = fonts_data[i].size * (float(window_height) / data_window_height);
//             struct nk_font_config config = nk_font_config(size);
//             config.oversample_h = 1;
//             config.oversample_v = 1;
//             config.range = &unicode_glyph_ranges[0];
// //             config.range = nk_font_cyrillic_glyph_ranges();
//             fonts[i] = nk_font_atlas_add_from_memory(&atlas, font_mem, font_size, size, &config);
            fonts[i] = nk_font_atlas_add_from_memory(&atlas, font_mem, font_size, size, nullptr);
          } else {
//             fonts[i] = fonts[fonts::technical];
//             ASSERT(fonts[i] != nullptr);
          }
        }
        if (fonts[fonts::technical] == nullptr) throw std::runtime_error("System font must exist");
        image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
//         std::cout << "backed atlas width " << w << " height " << h << "\n";
        
        yavf::Image* img = nullptr;
        //yavf::ImageView* view = nullptr;
        {
          auto staging = device->create(yavf::ImageCreateInfo::texture2DStaging({static_cast<uint32_t>(w), static_cast<uint32_t>(h)}), VMA_MEMORY_USAGE_CPU_ONLY);

          const size_t imageSize = w * h * 4;
          memcpy(staging->ptr(), image, imageSize);

          img = device->create(yavf::ImageCreateInfo::texture2D({static_cast<uint32_t>(w), static_cast<uint32_t>(h)},
                                                                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                              VMA_MEMORY_USAGE_GPU_ONLY);

          yavf::TransferTask* task = device->allocateTransferTask();

          task->begin();
          task->setBarrier(staging, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
          task->setBarrier(img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
          task->copy(staging, img);
          task->setBarrier(img, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
          task->end();

          task->start();
          task->wait();

          device->deallocate(task);
          device->destroy(staging);

          view = img->createView(VK_IMAGE_VIEW_TYPE_2D, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

          // тут нужен еще сэмплер и дескриптор
          yavf::Sampler sampler = device->sampler("default_nuklear_sampler");

          { 
            atlas_descriptor->at(descriptor_index) = {sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};
            view->setDescriptor(atlas_descriptor, descriptor_index);
            atlas_descriptor->update(descriptor_index);
          }
        }

        //nk_font_atlas_end(&atlas, nk_handle_ptr(view), &null);
        nk_font_atlas_end(&atlas, nk_handle_image(render::image_t{UINT32_MAX}), &null);
      }
      
      nk_style_set_font(&ctx, &fonts[fonts::technical]->handle);
      
      (void)window_width;
    }
    
    namespace style {
      borders::borders(nk_context* ctx, const float &window, const float &group) : ctx(ctx) {
        nk_style* s = &ctx->style;
        old_window_border = s->window.border;
        old_group_border = s->window.group_border;
        s->window.border = window;
        s->window.group_border = group;
      }
    
      borders::~borders() {
        nk_style* s = &ctx->style;
        s->window.border = old_window_border;
        s->window.group_border = old_group_border;
      }
      
      background_color::background_color(nk_context* ctx, const nk_color &color) : ctx(ctx) {
        nk_style* s = &ctx->style;
        old_color = s->window.background;
        old_style_item = s->window.fixed_background;
        s->window.background = color;
        s->window.fixed_background = nk_style_item_color(color);
      }
      
      background_color::~background_color() {
        nk_style* s = &ctx->style;
        s->window.background = old_color;
        s->window.fixed_background = old_style_item;
      }
    }
  }
}
