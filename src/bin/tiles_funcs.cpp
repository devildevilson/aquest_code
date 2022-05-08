#include "tiles_funcs.h"

#include "utils/globals.h"
#include "utils/systems.h"

#include "core/context.h"

#include "core/map.h"

namespace devils_engine {
  namespace core {
    float get_tile_height(const uint32_t &tile_index) {
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index " + std::to_string(tile_index));
      auto map = global::get<systems::map_t>()->map.get();
      if (!map->is_tile_data_on_gpu()) return map->get_tile_height(tile_index);
      
      auto ctx = global::get<systems::map_t>()->core_context.get();
      if (!ctx->is_tiles_created()) return 0.0;
      const auto tile = ctx->get_entity<core::tile>(tile_index);
      return tile->height;
    }
    
    province* get_tile_province(const uint32_t &tile_index) {
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index " + std::to_string(tile_index));
      auto map = global::get<systems::map_t>()->map.get();
      if (!map->is_tile_data_on_gpu()) return nullptr;
      
      auto ctx = global::get<systems::map_t>()->core_context.get();
      if (!ctx->is_tiles_created()) return nullptr;
      const size_t ctx_tiles_count = ctx->get_entity_count<core::tile>();
      if (ctx_tiles_count != tiles_count) return nullptr;
      const auto tile = ctx->get_entity<core::tile>(tile_index);
      if (tile->province == UINT32_MAX) return nullptr;
      return ctx->get_entity<province>(tile->province);
    }
    
    city* get_tile_city(const uint32_t &tile_index) {
      const size_t tiles_count = core::map::hex_count_d(core::map::detail_level);
      if (tile_index >= tiles_count) throw std::runtime_error("Bad tile index " + std::to_string(tile_index));
      auto map = global::get<systems::map_t>()->map.get();
      if (!map->is_tile_data_on_gpu()) return nullptr;
      
      auto ctx = global::get<systems::map_t>()->core_context.get();
      if (!ctx->is_tiles_created()) return nullptr;
      if (ctx->get_entity_count<core::tile>() == 0) return nullptr;
      const auto tile = ctx->get_entity<core::tile>(tile_index);
      if (tile->city == UINT32_MAX) return nullptr;
      return ctx->get_entity<city>(tile->city);
    }
  }
}
