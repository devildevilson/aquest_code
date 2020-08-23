#include "pipeline_mode_updater.h"
#include "stages.h"
#include "utils/globals.h"

namespace devils_engine {
  namespace render {
    updater::updater() : m_render_mode(1), m_water_mode(0), m_render_slot(0), m_water_slot(3), m_color(0.0f, 0.0f, 0.0f) {}
      
    uint32_t updater::render_mode() { return m_render_mode; }
    uint32_t updater::water_mode() { return m_water_mode; }
    uint32_t updater::render_slot() { return m_render_slot; }
    uint32_t updater::water_slot() { return m_water_slot; }
    glm::vec3 updater::water_color() { return m_color; }
    
    void updater::set_render_mode(const uint32_t &render_mode) { m_render_mode = render_mode; }
    void updater::set_water_mode(const uint32_t &water_mode) { m_water_mode = water_mode; }
    void updater::set_render_slot(const uint32_t &render_slot) { m_render_slot = render_slot; }
    void updater::set_water_slot(const uint32_t &water_slot) { m_water_slot = water_slot; }
    void updater::set_water_color(const glm::vec3 &color) { m_color = color; }
    
    void updater::update() {
      global::get<render::tile_render>()->change_rendering_mode(m_render_mode, m_water_mode, m_render_slot, m_water_slot, m_color);
      global::get<render::tile_connections_render>()->change_rendering_mode(m_render_mode, m_water_mode, m_render_slot, m_water_slot, m_color);
    }
  }
}
