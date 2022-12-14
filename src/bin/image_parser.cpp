#include "image_parser.h"

#include "render/vulkan_hpp_header.h"
#include "render/image_controller.h"
#include "render/image_container.h"
#include "render/image_container_constants.h"
#include "render/shared_structures.h"
#include "render/shared_battle_structures.h"
#include "render/container.h"

#include "utils/globals.h"
#include "utils/table_container.h"
#include "utils/serializator_helper.h"
#include "utils/systems.h"

#include "core/context.h"

#include "core/map.h"
#include "core/seasons.h"

#include "battle/map.h"

#include "data_parser.h"
#include "map_creator.h"

#include <filesystem>

#ifdef _WIN32
#define STBI_WINDOWS_UTF8
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace devils_engine {
  namespace utils {
    const check_table_value image_table[] = {
      {
        "id",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      }, 
      {
        "path",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "atlas_data",
        check_table_value::type::array_t,
        0, 0,
        {
          {
            "start_x",
            check_table_value::type::int_t,
            0, 0, {}
          },
          {
            "start_y",
            check_table_value::type::int_t,
            0, 0, {}
          },
          {
            "width",
            check_table_value::type::int_t,
            check_table_value::value_required, 0, {}
          },
          {
            "height",
            check_table_value::type::int_t,
            check_table_value::value_required, 0, {}
          },
          {
            "rows",
            check_table_value::type::int_t,
            check_table_value::value_required, 0, {}
          },
          {
            "columns",
            check_table_value::type::int_t,
            check_table_value::value_required, 0, {}
          },
          {
            "count",
            check_table_value::type::int_t,
            check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "scale",
        check_table_value::type::array_t,
        0, 0, 
        {
          {
            "width",
            check_table_value::type::int_t,
            0, 0, {}
          },
          {
            "height",
            check_table_value::type::int_t,
            0, 0, {}
          },
          {
            "filter",
            check_table_value::type::int_t, // ?????? ?????? ???????????? ?????? ?????? ???? ??????????, ?????????? ?????? ???? ????????????
            check_table_value::value_required, 0, {}
          }
        }
      },
      {
        "type",
        check_table_value::type::int_t, // ?????? ???????? ???????????? ???? ???????????
        check_table_value::value_required, 0, {}
      },
      {
        "sampler",
        check_table_value::type::int_t,
        0, 0, {}
      }
    };
    
    const check_table_value battle_biome_table[] = {
      {
        "id",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "texture1_face",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "texture1_top",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "texture2_face",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "texture2_top",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "texture3_face",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "texture3_top",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "dencity",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "probability1",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "probability2",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "probability3",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "min_scale1",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "max_scale1",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "min_scale2",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "max_scale2",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "min_scale3",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "max_scale3",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "max_zoom1",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "max_zoom2",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "max_zoom3",
        check_table_value::type::float_t,
        0, 0, {}
      },
      // ???
    };
    
    const check_table_value biome_table[] = {
      {
        "id",
        check_table_value::type::string_t,
        check_table_value::value_required, 0, {}
      },
      {
        "color",
        check_table_value::type::int_t,
        check_table_value::value_required, 0, {}
      },
      {
        "object1",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "object2",
        check_table_value::type::string_t,
        0, 0, {}
      },
      {
        "min_scale1",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "max_scale1",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "min_scale2",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "max_scale2",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "density",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "height1",
        check_table_value::type::float_t,
        0, 0, {}
      },
      {
        "height2",
        check_table_value::type::float_t,
        0, 0, {}
      },
    };
    
    render::image_t parse_image(const std::string_view &str_id, render::image_controller* controller) {
      std::string_view img_id;
      uint32_t layer;
      bool mirror_u;
      bool mirror_v;
      const bool ret = render::parse_image_id(str_id, img_id, layer, mirror_u, mirror_v);
      if (!ret) throw std::runtime_error("Bad texture id " + std::string(str_id));
      const auto image_id = std::string(img_id);
      auto view = controller->get_view(image_id);
      if (layer >= view->count) throw std::runtime_error("Image pack " + image_id + " does not have " + std::to_string(layer) + " amount of images");
      return view->get_image(layer, mirror_u, mirror_v);
    }
    
    render::image_t parse_image(const std::string_view &key, const sol::table &table, render::image_controller* controller) {
      auto t_proxy = table[key];
      if (!t_proxy.valid() || t_proxy.get_type() != sol::type::string) return render::image_t{GPU_UINT_MAX};
      
      const std::string_view str = t_proxy.get<std::string_view>();
//           PRINT(str)
      return parse_image(str, controller);
    }
    
    double parse_number(const std::string_view &key, const sol::table &table, const double &default_value) {
      const auto &proxy = table[key];
      if (!proxy.valid() || proxy.get_type() != sol::type::number) return default_value;
      return proxy.get<double>();
    }
    
    void add_image(const sol::table &table) {
      const auto proxy = table["id"];
      if (proxy.valid() && proxy.get_type() == sol::type::string) PRINT_VAR("image", proxy.get<std::string>())
      global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(utils::generator_table_container::additional_data::image), table);
    }
    
    bool validate_image(const uint32_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "image" + std::to_string(index);
      }
      
      const size_t size = sizeof(image_table) / sizeof(image_table[0]);
      recursive_check(check_str, "image", table, nullptr, image_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_image_and_save(const uint32_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container) {
      const bool ret = validate_image(index, table);
      if (!ret) return false;

      sol::state_view state(lua);
      auto str = table_to_string(lua, table, sol::table());
      if (str.empty()) throw std::runtime_error("Could not serialize image table");
      ASSERT(false);
      //container->add_image_data(std::move(str));
      
      return true;
    }
    
    struct img_data {
      struct atlas {
        uint32_t start_x;
        uint32_t start_y;
        uint32_t width;
        uint32_t height;
        uint32_t rows;
        uint32_t columns;
        uint32_t count;
        
        inline atlas() : start_x(0), start_y(0), width(0), height(0), rows(0), columns(0), count(0) {}
      };
      
      struct scale {
        uint32_t width;
        uint32_t height;
        uint32_t type;
        
        scale() : width(0), height(0), type(UINT32_MAX) {}
      };
      
      std::string id;
      std::string path;
      render::image_controller::image_type image_type;
      uint32_t sampler_type;
      struct atlas atlas;
      struct scale scale;
      
      render::vk_image_data_unique img;
      uint32_t final_width;
      uint32_t final_height;
      
      uint32_t slot;
      uint32_t offset;
      
      img_data() : sampler_type(UINT32_MAX), img(nullptr, nullptr, nullptr, nullptr) {}
    };
    
    void vk_copy_data(const img_data* data, const size_t &start, const std::vector<uint32_t> &indices, std::vector<vk::ImageBlit> &copies) {
      uint32_t counter = 0;
      uint32_t initial_width = data->atlas.start_x;
      uint32_t initial_height = data->atlas.start_y;
      
      for (size_t i = 0 ; i < data->atlas.rows; ++i) {
        uint32_t height = data->atlas.height * i + initial_height;
        //width = 0;
        
        for (size_t j = 0; j < data->atlas.columns; ++j) {
          size_t index = i * data->atlas.rows + j;
          if (index < start) continue;
          uint32_t width = data->atlas.width * j + initial_width;
          
          const vk::ImageBlit copy(
            vk::ImageSubresourceLayers{
              vk::ImageAspectFlagBits::eColor,
              0, 0, 1
            },
            {
              vk::Offset3D{int32_t(width), int32_t(height), 0},
              vk::Offset3D{int32_t(data->atlas.width), int32_t(data->atlas.height), 1}
            },
            vk::ImageSubresourceLayers{
              vk::ImageAspectFlagBits::eColor,
              0, indices[counter], 1 //(counter-1) + startingLayer
            },
            {
              vk::Offset3D{0, 0, 0},
              vk::Offset3D{int32_t(data->final_width), int32_t(data->final_height), 1}
            }
          );
          ++counter;
          
//           PRINT_VAR("start width", copy.srcOffsets[0].x)
//           PRINT_VAR("start height", copy.srcOffsets[0].y)
//           PRINT_VAR("atlas width", copy.srcOffsets[1].x)
//           PRINT_VAR("atlas height", copy.srcOffsets[1].y)
//           PRINT_VAR("final width", copy.dstOffsets[1].x)
//           PRINT_VAR("final height", copy.dstOffsets[1].y)
//           PRINT("\n")

          copies.push_back(copy);
          if (counter == indices.size()) return;
        }
      }
    }
    
    std::mutex & get_current_mutex() {
      auto map_data = global::get<systems::map_t>();
      auto battle_data = global::get<systems::battle_t>();
      if (map_data != nullptr && battle_data != nullptr) throw std::runtime_error("nor world map nor battle is loaded at this moment");
      if (battle_data != nullptr) return battle_data->map->mutex;
      return map_data->map->mutex;
    }
    
    void load_images(render::image_controller* controller, const std::vector<sol::table> &image_tables, const uint32_t &current_type) {
      const auto current_image_type = static_cast<render::image_controller::image_type>(current_type);
      std::vector<img_data> data_array; // , static_cast<int32_t>(render::image_controller::image_type::count)>
      for (size_t i = 0; i < image_tables.size(); ++i) {
        const auto table = image_tables[i];
        
        const int32_t type_check = table["type"];
        if (type_check < 0 || type_check >= static_cast<int32_t>(render::image_controller::image_type::count)) throw std::runtime_error("Bad image " + table["id"].get<std::string>() + " type");
        
        // ???????????? ???????? ???????????????? ?? ?????? ?????????? ???????? (???????????????? ?????????? ?????????????????? ???????? ?????????????????????? ?????????????? ???? ???????????? ????????????)
        // 
        if (current_type != uint32_t(type_check)) continue;
        
        data_array.emplace_back(); // [type_check]
        auto &img_datas = data_array.back();
        
        img_datas.id = table["id"];
        img_datas.path = table["path"];
        
        auto atlas_proxy = table["atlas"];
        auto scale_proxy = table["scale"];
        if (atlas_proxy.valid()) {
          auto start_x_proxy = atlas_proxy["start_x"];
          auto start_y_proxy = atlas_proxy["start_y"];
          if (start_x_proxy.valid() && start_x_proxy.get<int32_t>() < 0)  throw std::runtime_error("Bad image " + img_datas.id + " atlas start_x value");
          if (start_y_proxy.valid() && start_y_proxy.get<int32_t>() < 0)  throw std::runtime_error("Bad image " + img_datas.id + " atlas start_y value");
          if (table["atlas"]["width"].get<int32_t>() <= 0)   throw std::runtime_error("Bad image " + img_datas.id + " atlas width value");
          if (table["atlas"]["height"].get<int32_t>() <= 0)  throw std::runtime_error("Bad image " + img_datas.id + " atlas height value");
          if (table["atlas"]["rows"].get<int32_t>() <= 0)    throw std::runtime_error("Bad image " + img_datas.id + " atlas rows value");
          if (table["atlas"]["columns"].get<int32_t>() <= 0) throw std::runtime_error("Bad image " + img_datas.id + " atlas columns value");
          if (table["atlas"]["count"].get<int32_t>() <= 0)   throw std::runtime_error("Bad image " + img_datas.id + " atlas count value");
          
          img_datas.atlas.start_x = start_x_proxy.valid() ? start_x_proxy.get<uint32_t>() : 0;
          img_datas.atlas.start_y = start_y_proxy.valid() ? start_y_proxy.get<uint32_t>() : 0;
        }
        
        img_datas.scale.width = UINT32_MAX;
        img_datas.scale.height = UINT32_MAX;
        if (scale_proxy.valid()) {
          auto scale_w_proxy = scale_proxy["width"];
          auto scale_h_proxy = scale_proxy["height"];
          if (scale_w_proxy.valid() && scale_w_proxy.get<int32_t>() <= 0) throw std::runtime_error("Bad image " + img_datas.id +  " scale width value");
          if (scale_h_proxy.valid() && scale_h_proxy.get<int32_t>() <= 0) throw std::runtime_error("Bad image " + img_datas.id +  " scale height value");
          if (table["scale"]["filter"].get<int32_t>() < 0 || table["scale"]["filter"].get<int32_t>() >= 2) throw std::runtime_error("Bad image " + img_datas.id +    " scale type value");
          
          img_datas.scale.width   = scale_proxy.valid() && scale_w_proxy.valid() ? scale_w_proxy.get<uint32_t>() : UINT32_MAX;
          img_datas.scale.height  = scale_proxy.valid() && scale_h_proxy.valid() ? scale_h_proxy.get<uint32_t>() : UINT32_MAX;
          
          ASSERT(img_datas.scale.width != 0);
          ASSERT(img_datas.scale.height != 0);
//           ASSERT(img_datas.scale.width != UINT32_MAX);
//           ASSERT(img_datas.scale.height != UINT32_MAX);
        }
        
        auto sampler_proxy = table["sampler"];
        if (sampler_proxy.valid() && (sampler_proxy.get<int32_t>() < 0 || sampler_proxy.get<int32_t>() >= IMAGE_SAMPLERS_COUNT)) throw std::runtime_error("Bad image " + img_datas.id +   " sampler type");
        
        img_datas.atlas.width   = atlas_proxy.valid() ? table["atlas"]["width"] : 0;
        img_datas.atlas.height  = atlas_proxy.valid() ? table["atlas"]["height"] : 0;
        img_datas.atlas.rows    = atlas_proxy.valid() ? table["atlas"]["rows"] : 1;
        img_datas.atlas.columns = atlas_proxy.valid() ? table["atlas"]["columns"] : 1;
        img_datas.atlas.count   = atlas_proxy.valid() ? table["atlas"]["count"] : 1;
        img_datas.scale.type    = scale_proxy.valid() ? table["scale"]["filter"] : 0;
        img_datas.image_type = static_cast<render::image_controller::image_type>(type_check);
        img_datas.sampler_type = sampler_proxy.valid() ? sampler_proxy.get<uint32_t>() : 0;
        
//         size_t slash_index = img_datas.path.find_first_of('/');
//         if (slash_index == std::string::npos) {
//           slash_index = img_datas.path.find_first_of('\\');
//         }
//         
//         std::string current_mod = img_datas.path.substr(0, slash_index);
//         ASSERT(current_mod == "apates_quest");
        
        std::filesystem::path p(img_datas.path);
        auto itr = p.begin();
        std::string current_mod2 = (*itr).string();
        ASSERT(current_mod2 == "apates_quest");
        
        const std::string &root = global::root_directory();
        
        ++itr;
        std::filesystem::path final_p(root + (*itr).string());
        ++itr;
        for (; itr != p.end(); ++itr) {
          final_p /= *itr;
        }
        
//         for (const auto &p : final_p) {
//           PRINT(p)
//         }
        
        final_p.make_preferred();
//         PRINT(final_p)
//         PRINT(final_p.extension())
        
        std::filesystem::directory_entry e(final_p);
        if (!e.exists()) throw std::runtime_error("Could not find image " + final_p.string());
        if (!e.is_regular_file()) throw std::runtime_error("Bad image file " + final_p.string());
        // ?????????????? ???????? ???? ?????????????????? ????????????????????
        
        // ?????? ???????? ?????? ??????????????????
        int x,y,n;
        uint8_t* data = stbi_load(final_p.string().c_str(), &x, &y, &n, STBI_rgb_alpha);
        auto cont = global::get<render::container>();
        auto allocator = cont->vulkan->buffer_allocator;
        img_datas.img = render::create_image_unique(allocator, render::texture2D_staging({uint32_t(x), uint32_t(y)}), vma::MemoryUsage::eCpuOnly);
//         img_datas.img = controller->device->create(
//           yavf::ImageCreateInfo::texture2DStaging({uint32_t(x), uint32_t(y)}),
//           VMA_MEMORY_USAGE_CPU_ONLY
//         );
        memcpy(img_datas.img.ptr, data, x * y * STBI_rgb_alpha);
        stbi_image_free(data);
        
        img_datas.atlas.width  = img_datas.atlas.width  == 0 ? x  : img_datas.atlas.width;
        img_datas.atlas.height = img_datas.atlas.height == 0 ? y : img_datas.atlas.height;
        img_datas.final_width  = img_datas.scale.width  == UINT32_MAX ? img_datas.atlas.width  : img_datas.scale.width;
        img_datas.final_height = img_datas.scale.height == UINT32_MAX ? img_datas.atlas.height : img_datas.scale.height;
        if (current_image_type == render::image_controller::image_type::icon) {
          img_datas.final_width = 32;
          img_datas.final_height = 32;
        }
        //img_datas.scale.width  = img_datas.scale.width  == UINT32_MAX ? img_datas.img->info().extent.width  : img_datas.scale.width;
        //img_datas.scale.height = img_datas.scale.height == UINT32_MAX ? img_datas.img->info().extent.height : img_datas.scale.height;
        img_datas.slot = UINT32_MAX;
        img_datas.offset = UINT32_MAX;
        
        ASSERT(img_datas.final_width != 0);
        ASSERT(img_datas.final_height != 0);
      }
      
      if (current_image_type == render::image_controller::image_type::icon) {
        size_t counter = 0;
        for (const auto &d : data_array) {
          counter += d.atlas.count;
        }
        
        if (counter >= render::image_container::image_pool::max_size * 4) throw std::runtime_error("To many icon images");
      }
      
      // ?????????? ?????????? ???????????????? ?????? ???????? ?? ????????????????????
      
      struct copy_data {
        vk::Image src;
        vk::Image dst;
        std::vector<vk::ImageBlit> copies;
        vk::Filter filter;
        uint32_t dst_layers_count;
      };
      
      struct concrete_data {
        uint32_t width;
        uint32_t height;
        std::vector<size_t> indices;
        std::vector<copy_data> copy_datas;
      };
      
      std::vector<concrete_data> concrete_datas;
      for (size_t i = 0; i < data_array.size(); ++i) {
        uint32_t found_index = UINT32_MAX;
        for (size_t j = 0; j < concrete_datas.size(); ++j) {
          if (concrete_datas[j].width == data_array[i].final_width && concrete_datas[j].height == data_array[i].final_height) {
            found_index = j;
            break;
          }
        }
        
        if (found_index == UINT32_MAX) {
          concrete_datas.push_back({data_array[i].final_width, data_array[i].final_height, {}, {}});
          found_index = concrete_datas.size()-1;
        }
        
        concrete_datas[found_index].indices.push_back(i);
      }
      
      auto container = controller->container;
//       size_t counter = 0; // ???
      for (size_t i = 0; i < concrete_datas.size(); ++i) {
        auto &concrete_d = concrete_datas[i];
        size_t images_count = 0;
        for (const size_t index : concrete_d.indices) {
          const auto &d = data_array[index];
          images_count += d.atlas.count;
        }
        
        size_t current_atlas_index = 0;
        size_t atlas_start = 0;
        size_t offset = 0;
        while (images_count > 0) {
          const size_t pool_index = container->first_empty_pool();
          if (pool_index == SIZE_MAX) throw std::runtime_error("Seems like every pools are in use");
          size_t created_layers = std::min(images_count, render::image_container::image_pool::max_size);
          container->create_pool(pool_index, {concrete_d.width, concrete_d.height}, 1, created_layers);
          images_count -= created_layers;
          
          while (created_layers > 0) {
            const size_t index = concrete_d.indices[current_atlas_index];
            auto &d = data_array[index];
            const size_t count = d.atlas.count;
            const auto filter = d.scale.type == 0 ? vk::Filter::eLinear : vk::Filter::eNearest;
            auto pool = container->get_pool(pool_index);
            const uint32_t layers_count = pool->layers_count();
            concrete_d.copy_datas.push_back({d.img.handle, pool->image, {}, filter, layers_count}); // if (atlas_start == 0) 
            ASSERT(atlas_start < std::min(count, created_layers));
            
            if (d.slot == UINT32_MAX) {
              d.slot = pool_index;
              d.offset = offset;
            } else {
              ASSERT(d.slot+1 == pool_index);
            }
            
            offset = (offset + count) % render::image_container::image_pool::max_size;
            
            std::vector<uint32_t> indices;
            for (size_t k = atlas_start; k < std::min(count, created_layers); ++k) {
              const auto id = container->reserve_image(pool_index);
              ASSERT(id != UINT32_MAX);
              indices.push_back(id);
            }
            
            vk_copy_data(&d, atlas_start, indices, concrete_d.copy_datas.back().copies);
            
            atlas_start = created_layers < count ? count - created_layers : 0;
            created_layers = created_layers < count ? 0 : created_layers - count;
            current_atlas_index += created_layers < count ? 0 : 1;
          }
        }
      }
      
      {
        auto map = global::get<systems::map_t>()->map.get();
        std::unique_lock<std::mutex> lock(map->mutex);
        auto cont = global::get<render::container>();
        auto device = cont->vulkan->device;
        // ???????? ?????? ?????????? ???????????????????????????? ?????????? ?? ???????? ??????????????
        // ?? ?????? ??????????
        auto pool = cont->vulkan->transfer_command_pool; // ?????????????? ?????? (!)
        auto queue = cont->vulkan->graphics;
        auto fence = cont->vulkan->transfer_fence;
        
        render::do_command(
          device, pool, queue, fence,
          [&] (vk::CommandBuffer task) {
            const vk::CommandBufferBeginInfo info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            task.begin(info);
            
            for (const auto &concrete_data : concrete_datas) {
              for (const auto &copy_data : concrete_data.copy_datas) {
                {
                  const vk::ImageSubresourceRange range1{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
                  const auto [b1_info, src1, dst1] = render::make_image_memory_barrier(copy_data.src, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, range1);
                  task.pipelineBarrier(src1, dst1, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, b1_info);
                  
                  const vk::ImageSubresourceRange range2{vk::ImageAspectFlagBits::eColor, 0, 1, 0, copy_data.dst_layers_count};
                  const auto [b2_info, src2, dst2] = render::make_image_memory_barrier(copy_data.dst, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, range2);
                  task.pipelineBarrier(src2, dst2, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, b2_info);
                }
                
                task.blitImage(
                  copy_data.src, vk::ImageLayout::eTransferSrcOptimal,
                  copy_data.dst, vk::ImageLayout::eTransferDstOptimal,
                  copy_data.copies, 
                  copy_data.filter
                );
                
                {
                  const vk::ImageSubresourceRange range1{vk::ImageAspectFlagBits::eColor, 0, 1, 0, copy_data.dst_layers_count};
                  const auto [b1_info, src1, dst1] = render::make_image_memory_barrier(copy_data.dst, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, range1);
                  task.pipelineBarrier(src1, dst1, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, b1_info);
                }
              }
            }

            task.end();
          }
        );
        
//         auto task = controller->device->allocateGraphicTask();
//         
//         task->begin();
//         for (size_t i = 0; i < concrete_datas.size(); ++i) {
//           for (size_t j = 0; j < concrete_datas[i].copy_datas.size(); ++j) {
//             task->setBarrier(concrete_datas[i].copy_datas[j].src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
//             task->setBarrier(concrete_datas[i].copy_datas[j].dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//             task->copyBlit(
//               concrete_datas[i].copy_datas[j].src, 
//               concrete_datas[i].copy_datas[j].dst, 
//               concrete_datas[i].copy_datas[j].copies, 
//               concrete_datas[i].copy_datas[j].filter
//             );
//             task->setBarrier(concrete_datas[i].copy_datas[j].dst, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//           }
//         }
//         task->end();
//         
//         {
//           //auto map = global::get<systems::map_t>()->map;
//           //std::unique_lock<std::mutex> lock(map->mutex);
//           std::unique_lock<std::mutex> lock(get_current_mutex());
//           task->start();
//           task->wait();
//         }
//         
//         controller->device->deallocate(task);
      
        // ???????????????????????? ?????????????? ?????????????????? ?????????????????????? ???????????? ?????????????? ???? ?????????????? ????????????
        // ?????? ?????????????? ?? ?????? ?????????? ?????? ???? ???????????? ???????????????????? ???????????????????????? ?????????????????????? ?????? ???? ?????????????????????? ?????? ??????????????
//         for (size_t i = 0; i < concrete_datas.size(); ++i) {
//           for (size_t j = 0; j < concrete_datas[i].copy_datas.size(); ++j) {
//             controller->device->destroy(concrete_datas[i].copy_datas[j].src);
//           }
//         }
        // data_array ?????? ???? ???????? ???????????? ?????? ??????????????????
      }
      
      for (const auto &img_data : data_array) {
        const render::image_controller::input_data d{
          img_data.id,
          img_data.image_type,
          img_data.atlas.count,
          img_data.slot,
          img_data.offset,
          img_data.sampler_type
        };
        controller->register_images(d);
        
//         PRINT_VAR("img_data.id         ", img_data.id)
//         PRINT_VAR("img_data.image_type ", static_cast<uint32_t>(img_data.image_type))
//         PRINT_VAR("img_data.atlas.count", img_data.atlas.count)
//         PRINT_VAR("img_data.slot       ", img_data.slot)
//         PRINT_VAR("img_data.offset     ", img_data.offset)
      }
    }
    
    void load_biomes(render::image_controller* controller, core::seasons* seasons, const std::vector<sol::table> &biome_tables) {
//       auto to_data = global::get<utils::data_string_container>();
//       auto ctx = global::get<core::context>();
      
      // ???????????? ?????????????????? ????????????
//       size_t counter = 0;
//       for (const auto &table : biome_tables) {
//         if (!table["color"].valid()) throw std::runtime_error("Bad biome table");
//         
//         render::biome_data_t b;
//         if (auto texture_proxy = table["texture"]; texture_proxy.valid()) {
//           const std::string_view str = texture_proxy.get<std::string_view>();
// //           PRINT(str)
//           std::string_view img_id;
//           uint32_t layer;
//           bool mirror_u;
//           bool mirror_v;
//           const bool ret = render::parse_image_id(str, img_id, layer, mirror_u, mirror_v);
//           if (!ret) throw std::runtime_error("Bad texture id " + std::string(str));
//           const auto image_id = std::string(img_id);
//           auto view = controller->get_view(image_id);
//           if (layer >= view->count) throw std::runtime_error("Image pack " + image_id + " does not have " + std::to_string(layer) + " amount of images");
//           b.texture = view->get_image(layer, mirror_u, mirror_v);
//         } else b.texture = { GPU_UINT_MAX };
//         
//         {
//           render::color_t c;
//           c.container = table["color"];
//           b.color = c;
//         }
//         
//         if (auto texture_proxy = table["object1"]; texture_proxy.valid()) {
//           const std::string_view str = texture_proxy.get<std::string_view>();
//           std::string_view img_id;
//           uint32_t layer;
//           bool mirror_u;
//           bool mirror_v;
//           const bool ret = render::parse_image_id(str, img_id, layer, mirror_u, mirror_v);
//           if (!ret) throw std::runtime_error("Bad texture id " + std::string(str));
//           const auto image_id = std::string(img_id);
//           auto view = controller->get_view(image_id);
//           if (layer >= view->count) throw std::runtime_error("Image pack " + image_id + " does not have " + std::to_string(layer) + " amount of images");
//           b.object_texture1 = view->get_image(layer, mirror_u, mirror_v);
//         } else b.object_texture1 = { GPU_UINT_MAX };
//         
//         if (auto texture_proxy = table["object2"]; texture_proxy.valid()) {
//           const std::string_view str = texture_proxy.get<std::string_view>();
//           std::string_view img_id;
//           uint32_t layer;
//           bool mirror_u;
//           bool mirror_v;
//           const bool ret = render::parse_image_id(str, img_id, layer, mirror_u, mirror_v);
//           if (!ret) throw std::runtime_error("Bad texture id " + std::string(str));
//           const auto image_id = std::string(img_id);
//           auto view = controller->get_view(image_id);
//           if (layer >= view->count) throw std::runtime_error("Image pack " + image_id + " does not have " + std::to_string(layer) + " amount of images");
//           b.object_texture2 = view->get_image(layer, mirror_u, mirror_v);
//         } else b.object_texture2 = { GPU_UINT_MAX };
//         
//         b.min_scale1 = parse_number("min_scale1", table, 0.0);
//         b.max_scale1 = parse_number("max_scale1", table, 0.0);
//         b.min_scale2 = parse_number("min_scale2", table, 0.0);
//         b.max_scale2 = parse_number("max_scale2", table, 0.0);
//         b.density = parse_number("density", table, 0.0);
//         
//         ASSERT(counter < MAX_BIOMES_COUNT);
//         seasons->create_biome(counter, b);
//         ++counter;
//       }
//       
//       ASSERT(counter != 0);
//       if (seasons->biomes_count != 0) { ASSERT(seasons->biomes_count == counter); }
//       seasons->biomes_count = counter;
    }
    
    size_t add_biome(const sol::table &table) {
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(utils::generator_table_container::additional_data::biome), table);
    }
    
    size_t add_battle_biome(const sol::table &table) {
      const auto str = table["id"].get<std::string>();
      if (!str.empty()) PRINT_VAR("biome", str)
      return global::get<map::creator::table_container_t>()->add_table(static_cast<size_t>(utils::generator_table_container::additional_data::biome), table);
    }
    
    bool validate_biome(const uint32_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "biome" + std::to_string(index);
      }
      
      const size_t size = sizeof(biome_table) / sizeof(biome_table[0]);
      recursive_check(check_str, "biome", table, nullptr, biome_table, size, counter);
      
      return counter == 0;
    }
    
    bool validate_battle_biome(const uint32_t &index, const sol::table &table) {
      size_t counter = 0;
      auto id = table["id"];
      std::string check_str;
      if (id.valid()) {
        check_str = id;
      } else {
        check_str = "biome" + std::to_string(index);
      }
      
      const size_t size = sizeof(battle_biome_table) / sizeof(battle_biome_table[0]);
      recursive_check(check_str, "biome", table, nullptr, battle_biome_table, size, counter);
      
      return counter == 0;
    }
    
    void load_battle_biomes(render::image_controller* controller, const std::vector<sol::table> &biome_tables) {
      std::array<render::battle_biome_data_t, BATTLE_BIOMES_MAX_COUNT> biomes;
      ASSERT(!biome_tables.empty());
      
      size_t counter = 0;
      for (const auto &table : biome_tables) {
        auto &biome = biomes[counter];
        biomes[counter].dummy = 12.0f;
        
        biome.textures[0].face = parse_image("texture1_face", table, controller);
        biome.textures[0].top  = parse_image("texture1_top" , table, controller);
        biome.textures[1].face = parse_image("texture2_face", table, controller);
        biome.textures[1].top  = parse_image("texture2_top" , table, controller);
        biome.textures[2].face = parse_image("texture3_face", table, controller);
        biome.textures[2].top  = parse_image("texture3_top" , table, controller);
        
        //std::cout << "img " << table["texture1_face"].get<std::string>() << " index " << render::get_image_index(biome.textures[0].face) << " layer " << render::get_image_layer(biome.textures[0].face) << "\n";
        //std::cout << "img " << table["texture1_top"].get<std::string>()  << " index " << render::get_image_index(biome.textures[0].top)  << " layer " << render::get_image_layer(biome.textures[0].top)  << "\n";
        
        biome.density = parse_number("density", table, 0.0);
        biome.probabilities[0] = parse_number("probability1", table, 1.0);
        biome.probabilities[1] = parse_number("probability2", table, 1.0);
        biome.probabilities[2] = parse_number("probability3", table, 1.0);
        
        biome.scales[0] = glm::vec2(parse_number("min_scale1", table, 0.0), parse_number("max_scale1", table, 0.0));
        biome.scales[1] = glm::vec2(parse_number("min_scale2", table, 0.0), parse_number("max_scale2", table, 0.0));
        biome.scales[2] = glm::vec2(parse_number("min_scale3", table, 0.0), parse_number("max_scale3", table, 0.0));
        
        biome.zooms[0] = parse_number("max_zoom1", table, 0.6);
        biome.zooms[1] = parse_number("max_zoom2", table, 0.6);
        biome.zooms[2] = parse_number("max_zoom3", table, 0.6);
        
        ASSERT(biome.dummy == 12.0f);
        biome.dummy = 13.0f;
        ASSERT(biomes[counter].dummy == 13.0f);
        ASSERT(biome.density != 0.0f);
        ++counter;
      }
      
      global::get<systems::battle_t>()->map->set_biomes(biomes);
    }
  }
}


