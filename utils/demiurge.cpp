#include "demiurge.h"

#include "globals.h"
#include "interface_container.h"
#include "assert.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace devils_engine {
  namespace utils {
    demiurge::demiurge(interface_container* container) : container(container), m_status(status::count), m_choosed(0) {
      refresh();
    }
    
    demiurge::~demiurge() {}
    
    void demiurge::create_new_world() {
      // выходим из меню и перемещаемся в мап креатор
      // нужно вернуть статус наверное
      m_status = status::create_new_world;
    }
    
    void demiurge::refresh() {
      worlds.clear();
      
      // нужно создать немодифицируемые таблицы!!!
      std::filesystem::path worlds_path = global::root_directory();
#ifndef _NDEBUG
      {
        std::filesystem::directory_entry entry(worlds_path);
        ASSERT(entry.exists());
        ASSERT(entry.is_directory());
      }
#endif
      worlds_path /= "saves";
      
      std::filesystem::directory_entry entry(worlds_path);
      if (!entry.exists()) std::filesystem::create_directory(worlds_path);
      
      for (const auto &e : std::filesystem::directory_iterator(worlds_path)) {
        if (!e.is_directory()) continue;
        const std::string folder_name = e.path().filename();
//         std::cout << "folder_name " << folder_name << "\n"; 
        const auto &world_data_path = e.path() / "world_data";
        std::filesystem::directory_entry world_data(world_data_path);
        
        if (!world_data.exists()) continue;
        if (!world_data.is_regular_file() && !world_data.is_symlink()) continue; // чет не понимаю как узнать что в сивольной ссылке
        
        if (world_data.is_symlink()) { // кажется так
          const auto &p = std::filesystem::read_symlink(world_data);
          world_data = std::filesystem::directory_entry(p);
          if (!world_data.exists()) continue;
          if (!world_data.is_regular_file()) continue;
        }
        
        auto table = container->lua.create_table();
        table["path"] = world_data.path().string();
        
        std::ifstream map_data_file(world_data.path(), std::ios::binary | std::ios::in);
        {
          const int64_t prev_pos = map_data_file.tellg();
          ASSERT(prev_pos != -1);
          char c;
          map_data_file.read(&c, 1);
          while (c != '\0') { map_data_file.read(&c, 1); }
          const int64_t pos = map_data_file.tellg();
          ASSERT(pos != -1);
          map_data_file.seekg(prev_pos);
          std::string world_name(pos - prev_pos - 1, '\0');
          map_data_file.read(world_name.data(), world_name.size());
          ASSERT(pos-1 == map_data_file.tellg());
          map_data_file.seekg(pos); // это скорее всего не нужно
          table["world_name"] = world_name;
        }
        
        {
          const int64_t prev_pos = map_data_file.tellg();
          ASSERT(prev_pos != -1);
          char c;
          map_data_file.read(&c, 1);
          while (c != '\0') { map_data_file.read(&c, 1); }
          const int64_t pos = map_data_file.tellg();
          ASSERT(pos != -1);
          map_data_file.seekg(prev_pos);
          std::string technical_name(pos - prev_pos - 1, '\0');
          map_data_file.read(technical_name.data(), technical_name.size());
          ASSERT(pos-1 == map_data_file.tellg());
          map_data_file.seekg(pos); // это скорее всего не нужно
          table["folder_name"] = technical_name;
        }
        
        {
          const int64_t prev_pos = map_data_file.tellg();
          ASSERT(prev_pos != -1);
          char c;
          map_data_file.read(&c, 1);
          while (c != '\0') { map_data_file.read(&c, 1); }
          const int64_t pos = map_data_file.tellg();
          ASSERT(pos != -1);
          map_data_file.seekg(prev_pos);
          std::string world_settings(pos - prev_pos - 1, '\0');
          map_data_file.read(world_settings.data(), world_settings.size());
          ASSERT(pos-1 == map_data_file.tellg());
          map_data_file.seekg(pos);
          table["settings"] = world_settings;
        }
        
        worlds.push_back(table);
        
//         ASSERT(worlds.size() < 2);
      }
    }
    
    size_t demiurge::worlds_count() {
      return worlds.size();
    }
    
    sol::table demiurge::world(const size_t &index) const {
      if (index >= worlds.size()) throw std::runtime_error("Bad world index");
      return worlds[index];
    }
    
    void demiurge::choose_world(const size_t &index) {
      if (index >= worlds.size()) throw std::runtime_error("Bad world index");
      m_choosed = index;
      m_status = status::load_existing_world;
    }
    
    enum demiurge::status demiurge::status() const {
      return m_status;
    }
    
    size_t demiurge::choosed() const {
      return m_choosed;
    }
  }
}
