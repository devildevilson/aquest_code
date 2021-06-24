#include "objects_selection.h"

#include "core_structures.h"
#include "utils/interface_container.h"

namespace devils_engine {
  namespace utils {
    bool valid_object(const sol::object &obj) {
      return obj.is<core::army*>() || obj.is<core::hero_troop*>() || obj.is<core::city*>(); //  || obj.is<core::structure>()
    }
    
    template <typename T>
    T* get_from_object(const sol::object &obj) {
      return obj.is<T*>() ? obj.as<T*>() : nullptr;
    }
    
    void* fast_get_from_object(const sol::object &obj) {
      void* ptr = nullptr;
      ptr = ptr == nullptr ? get_from_object<core::army>(obj) : ptr;
      ptr = ptr == nullptr ? get_from_object<core::hero_troop>(obj) : ptr;
      ptr = ptr == nullptr ? get_from_object<core::city>(obj) : ptr;
      return ptr;
    }
    
    objects_selection::objects_selection() : count(0) {}
    objects_selection::~objects_selection() {}
    
    int64_t objects_selection::has(const sol::object &obj) const {
      auto ptr = fast_get_from_object(obj);
      if (ptr == nullptr) return -1;
      
      for (size_t i = 0; i < count; ++i) {
        auto obj_ptr = fast_get_from_object(objects[i]);
        if (ptr == obj_ptr) return i;
      }
      
      return -1;
    }
    
    sol::object objects_selection::get(const int64_t &index) const {
      if (index < 0 || index >= int64_t(count)) return sol::nil;
      return objects[index];
    }
    
    int64_t objects_selection::add(const sol::object &obj) {
      if (count >= maximum_selected_objects) return -1;
      if (!valid_object(obj)) return -1;
      if (has(obj) != -1) return -1;
      
      const size_t index = count;
      ++count;
      
      objects[index] = obj;
      return index;
    }
    
    int64_t objects_selection::raw_add(const sol::object &obj) {
      if (count >= maximum_selected_objects) return -1;
      if (!valid_object(obj)) return -1;
      
      const size_t index = count;
      ++count;
      
      objects[index] = obj;
      return index;
    }
    
    bool objects_selection::remove(const int64_t &index) {
      if (index < 0 || index >= int64_t(count)) return false;
      
      objects[index] = objects[count-1];
      objects[count-1] = sol::object(sol::nil);
      --count;
      
      return true;
    }
    
    void objects_selection::clear() {
      for (size_t i = 0; i < count; ++i) {
        objects[i] = sol::object(sol::nil);
      }
      
      count = 0;
    }
    
    void objects_selection::sort(const sol::function &predicate) {
      std::sort(objects.begin(), objects.begin()+count, [&predicate] (const sol::object &first, const sol::object &second) -> bool {
        const auto ret = predicate(first, second);
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Lua error in selection sort");
        }
        
        const bool final_ret = ret;
        return final_ret;
      });
    }
    
    bool objects_selection::has_army() const {
      for (size_t i = 0; i < count; ++i) {
        if (objects[i].is<core::army*>()) return true;
      }
      
      return false;
    }
    
    bool objects_selection::has_hero() const {
      for (size_t i = 0; i < count; ++i) {
        if (objects[i].is<core::hero_troop*>()) return true;
      }
      
      return false;
    }
    
    bool objects_selection::has_city() const {
      for (size_t i = 0; i < count; ++i) {
        if (objects[i].is<core::city*>()) return true;
      }
      
      return false;
    }
    
    bool objects_selection::has_structure() const {
      throw std::runtime_error("Not implementing yet");
    }
    
    bool objects_selection::has_unit() const {
      for (size_t i = 0; i < count; ++i) {
        if (objects[i].is<core::hero_troop*>() || objects[i].is<core::army*>()) return true;
      }
      
      return false;
    }
    
    bool objects_selection::has_building() const {
      for (size_t i = 0; i < count; ++i) {
        if (objects[i].is<core::city*>()) return true;
      }
      
      return false;
    }
    
    void objects_selection::copy(objects_selection* primary) {
      primary->clear();
      
      primary->count = count;
      primary->objects = objects;
    }
    
    void objects_selection::copy_armies(objects_selection* primary) {
      primary->clear();
      
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (obj.is<core::army*>()) primary->raw_add(obj);
      }
    }
    
    void objects_selection::copy_heroes(objects_selection* primary) {
      primary->clear();
      
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (obj.is<core::hero_troop*>()) primary->raw_add(obj);
      }
    }
    
    void objects_selection::copy_cities(objects_selection* primary) {
      primary->clear();
      
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (obj.is<core::city*>()) primary->raw_add(obj);
      }
    }
    
    void objects_selection::copy_structures(objects_selection* primary) {
      UNUSED_VARIABLE(primary);
      throw std::runtime_error("Not implemented yet");
    }
    
    void objects_selection::copy_units(objects_selection* primary) {
      primary->clear();
      
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (obj.is<core::army*>() || obj.is<core::hero_troop*>()) primary->raw_add(obj);
      }
    }
    
    void objects_selection::copy_buildings(objects_selection* primary) {
      primary->clear();
      
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (obj.is<core::city*>()) primary->raw_add(obj);
      }
    }
    
    void objects_selection::add(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        primary->add(obj);
      }
    }
    
    void objects_selection::add_armies(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (obj.is<core::army*>()) primary->add(obj);
      }
    }
    
    void objects_selection::add_heroes(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (obj.is<core::hero_troop*>()) primary->add(obj);
      }
    }
    
    void objects_selection::add_cities(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (obj.is<core::city*>()) primary->add(obj);
      }
    }
    
    void objects_selection::add_structures(objects_selection* primary) {
      UNUSED_VARIABLE(primary);
      throw std::runtime_error("Not implemented yet");
    }
    
    void objects_selection::add_units(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (obj.is<core::army*>() || obj.is<core::hero_troop*>()) primary->add(obj);
      }
    }
    
    void objects_selection::add_buildings(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (obj.is<core::city*>()) primary->add(obj);
      }
    }
    
    void objects_selection::remove(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        const int64_t index = primary->has(obj);
        primary->remove(index);
      }
    }
    
    void objects_selection::remove_armies(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (!obj.is<core::army*>()) continue;
        
        const int64_t index = primary->has(obj);
        primary->remove(index);
      }
    }
    
    void objects_selection::remove_heroes(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (!obj.is<core::hero_troop*>()) continue;
        
        const int64_t index = primary->has(obj);
        primary->remove(index);
      }
    }
    
    void objects_selection::remove_cities(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (!obj.is<core::city*>()) continue;
        
        const int64_t index = primary->has(obj);
        primary->remove(index);
      }
    }
    
    void objects_selection::remove_structures(objects_selection* primary) {
      UNUSED_VARIABLE(primary);
      throw std::runtime_error("Not implemented yet");
    }
    
    void objects_selection::remove_units(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (!obj.is<core::army*>() && !obj.is<core::hero_troop*>()) continue;
        
        const int64_t index = primary->has(obj);
        primary->remove(index);
      }
    }
    
    void objects_selection::remove_buildings(objects_selection* primary) {
      for (size_t i = 0; i < count; ++i) {
        const auto &obj = objects[i];
        if (!obj.is<core::city*>()) continue;
        
        const int64_t index = primary->has(obj);
        primary->remove(index);
      }
    }
  }
}
