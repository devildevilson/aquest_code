#include "interface_context.h"

#include "render/window.h"
#include "utils/globals.h"
#include "render/container.h"
#include "render/image_container.h"

#include <fstream>
#include "nlohmann/json.hpp"

#define STB_RECT_PACK_IMPLEMENTATION
// #include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
// #include "stb_truetype.h"

#define NK_IMPLEMENTATION
#include "render/nuklear_header.h"

#include "render/vulkan_hpp_header.h"

#include <GLFW/glfw3.h>

struct fonts_settings {
  const char* name;
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

    std::vector<char> str(len+1);
    memcpy(str.data(), text, len);
    str[len] = '\0';

    glfwSetClipboardString(reinterpret_cast<render::window*>(usr.ptr)->handle, str.data());
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
  
  nk_handle image_data_to_nk_handle(const image_handle_data &data) {
    static_assert(sizeof(nk_handle) == sizeof(image_handle_data));
    nk_handle hndl;
    memcpy(&hndl, &data, sizeof(data));
    return hndl;
  }
  
  image_handle_data nk_handle_to_image_data(const nk_handle &handle) {
    static_assert(sizeof(nk_handle) == sizeof(image_handle_data));
    image_handle_data data;
    memcpy(&data, &handle, sizeof(handle));
    return data;
  }
  
  namespace interface {
    void load_font_settings(const nlohmann::json &json, std::vector<context::fonts_settings2> &fonts) {
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
    
    context::context(render::container* render_container, render::window* window, render::image_container* container) : render_container(render_container), container(container), fonts_data(fonts::count) {
      nlohmann::json j;
      {
        std::fstream file(global::root_directory()+"fonts/fonts.json");
        ASSERT(file);
        file >> j;
      }
      
      const auto [w, window_height] = window->size();
//       static const float data_window_height = 720.0f;
      
      memset(fonts, 0, sizeof(fonts[0]) * fonts::count);
      load_font_settings(j, fonts_data);
      
      // UINT32_MAX - дает поянть что мы в первый раз зашли в функцию
      remake_font_atlas(UINT32_MAX, window_height);
      nk_init_default(&ctx, &fonts[fonts::technical]->handle);

      ctx.clip.copy = clipbardCopy;
      ctx.clip.paste = clipbardPaste;
      ctx.clip.userdata = nk_handle_ptr(window);
    }
    
    context::~context() {
      container->destroy_pool(0);
      nk_font_atlas_clear(&atlas);
      nk_buffer_free(&cmds);
      nk_free(&ctx);
    }
    
    void context::remake_font_atlas(const uint32_t &window_width, const uint32_t &window_height) {
      if (window_width != UINT32_MAX) nk_font_atlas_clear(&atlas);
      //device->destroy(view->image());
      container->destroy_pool(0);
      memset(fonts, 0, sizeof(fonts[0]) * fonts::count);
      
      // нужно определиться к каким размерам экрана подгонять размер шрифта
      static const float data_window_height = 720.0f;
      
      //null.texture = image_data_to_nk_handle({GPU_UINT_MAX, GPU_UINT_MAX});
      //null.uv = {0.0f, 0.0f};
      
      null.texture.ptr = nullptr;
      null.uv = {0.0f, 0.0f};

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
            fonts[i] = nk_font_atlas_add_from_memory(&atlas, font_mem, font_size, size, nullptr);
          } else {
//             fonts[i] = fonts[fonts::technical];
//             ASSERT(fonts[i] != nullptr);
          }
        }
        if (fonts[fonts::technical] == nullptr) throw std::runtime_error("System font must exist");
        image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
//         std::cout << "backed atlas width " << w << " height " << h << "\n";
        
        {
          const auto device = render_container->vulkan->device;
          const auto physical_device = render_container->vulkan->physical_device;
          const auto transfer_command_pool = render_container->vulkan->transfer_command_pool;
          const auto queue = render_container->vulkan->graphics;
          const auto fence = render_container->vulkan->transfer_fence;
          const auto st_info = render::texture2D_staging({uint32_t(w), uint32_t(h)});
          const auto [staging, staging_mem] = render::create_image_unique(device, physical_device, st_info, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
          auto ptr = device.mapMemory(staging_mem.get(), 0, VK_WHOLE_SIZE);
          const size_t imageSize = w * h * 4;
          memcpy(ptr, image, imageSize);
          device.unmapMemory(staging_mem.get());
          
          container->create_pool(0, {static_cast<uint32_t>(w), static_cast<uint32_t>(h)}, 1, 1);
          const auto cont_img = container->get_image(0);
          font_atlas_image = render::create_image(render::get_image_index(cont_img), render::get_image_layer(cont_img), NUKLEAR_SAMPLER_ID);
          auto pool = container->get_pool(0);
          //pool->image

//           img = device->create(yavf::ImageCreateInfo::texture2D({static_cast<uint32_t>(w), static_cast<uint32_t>(h)},
//                                                                 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
//                               VMA_MEMORY_USAGE_GPU_ONLY);
          auto img = vk::Image(pool->image);
          
          const vk::ImageSubresourceRange range_staging(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
          const vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, render::get_image_layer(cont_img), 1);
          
          const vk::ImageCopy c{
            {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
            {0,0,0},
            {vk::ImageAspectFlagBits::eColor, 0, render::get_image_layer(cont_img), 1},
            {0,0,0},
            {uint32_t(w),uint32_t(h),1}
          };
          
          const vk::CommandBufferAllocateInfo alloc_info(transfer_command_pool, vk::CommandBufferLevel::ePrimary, 1);
          auto task = device.allocateCommandBuffers(alloc_info)[0];
          
          const vk::CommandBufferBeginInfo binfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
          task.begin(binfo);
          {
            const auto [b_info, srcStage, dstStage] = render::make_image_memory_barrier(staging.get(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, range_staging);
            task.pipelineBarrier(srcStage, dstStage, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, b_info);
          }
          
          {
            const auto [b_info, srcStage, dstStage] = render::make_image_memory_barrier(img, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, range);
            task.pipelineBarrier(srcStage, dstStage, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, b_info);
          }
          
          task.copyImage(staging.get(), vk::ImageLayout::eTransferSrcOptimal, img, vk::ImageLayout::eTransferDstOptimal, c);
          
          {
            const auto [b_info, srcStage, dstStage] = render::make_image_memory_barrier(img, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, range);
            task.pipelineBarrier(srcStage, dstStage, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, b_info);
          }
          
          task.end();
          
          const vk::SubmitInfo submit(nullptr, nullptr, task, nullptr);
          queue.submit(submit, fence);
          
          const auto res = device.waitForFences(fence, VK_TRUE, SIZE_MAX);
          if (res != vk::Result::eSuccess) throw std::runtime_error("waitForFences failed");
          
          device.freeCommandBuffers(transfer_command_pool, task);
          device.resetFences(fence);
        }

        //nk_font_atlas_end(&atlas, nk_handle_ptr(view), &null);
        //nk_font_atlas_end(&atlas, nk_handle_image(font_atlas_image), &null);
        //nk_font_atlas_end(&atlas, image_data_to_nk_handle({IMAGE_TYPE_DEFAULT, font_atlas_image.container}), &null);
        font_atlas_image_data.type = IMAGE_TYPE_DEFAULT;
        font_atlas_image_data.data = font_atlas_image.container;
        font_atlas_image_data.stencil = GPU_UINT_MAX;
        font_atlas_image_data.color = GPU_UINT_MAX;
        nk_font_atlas_end(&atlas, nk_handle_ptr(&font_atlas_image_data), &null);
      }
      
      // window_width == UINT32_MAX - означает что создаем в первый раз
      if (window_width != UINT32_MAX) nk_style_set_font(&ctx, &fonts[fonts::technical]->handle);      
    }
    
    bool is_interface_hovered(nk_context* ctx, const std::string_view &except) {
      struct nk_window* iter;
      assert(ctx);
      if (!ctx) return 0;
      
      iter = ctx->begin;
      while (iter) {
        struct nk_window* local_iter = iter;
        iter = iter->next;
        
        /* check if window is being hovered */
        if(local_iter->flags & NK_WINDOW_HIDDEN) continue;
        if (except == std::string_view(local_iter->name_string)) continue;
        
        /* check if window popup is being hovered */
        if (local_iter->popup.active && local_iter->popup.win && nk_input_is_mouse_hovering_rect(&ctx->input, local_iter->popup.win->bounds)) return true;

        if (local_iter->flags & NK_WINDOW_MINIMIZED) {
          struct nk_rect header = local_iter->bounds;
          header.h = ctx->style.font->height + 2 * ctx->style.window.header.padding.y;
          if (nk_input_is_mouse_hovering_rect(&ctx->input, header)) return true;
        } else if (nk_input_is_mouse_hovering_rect(&ctx->input, local_iter->bounds)) return true;
      }
      
      return false;
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
