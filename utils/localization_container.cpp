#include "localization_container.h"

#include <array>

namespace std {
  template <>
  struct hash<devils_engine::localization::container::locale> {
    size_t operator()(const devils_engine::localization::container::locale &loc) const {
      return loc.container;
    }
  };
}

namespace devils_engine {
  namespace localization {
    const size_t unpacked_array_size = 64;
    
    size_t unpack_keys(const std::string_view &keys, std::array<std::string_view, unpacked_array_size> &unpacked_keys) {
      size_t array_size = 0;
      size_t pos = 0;
      while (pos != std::string::npos) {
        const size_t new_pos = keys.find('.', pos);
        const auto str = keys.substr(pos, new_pos-pos);
        const size_t index = array_size;
        if (index >= unpacked_array_size) throw std::runtime_error("Maximum table depth is " + std::to_string(unpacked_array_size));
        ++array_size;
        unpacked_keys[index] = str;
        pos = new_pos == std::string::npos ? new_pos : new_pos+1;
      }
      
      return array_size;
    }
    
    container::locale::locale() : container(0) {}
    container::locale::locale(const std::string_view &str_locale) : container(0) {
      if (str_locale.length() > sizeof(size_t)) throw std::runtime_error("Locale key is too long. Maximum 8 characters");
      memcpy(&container, str_locale.data(), str_locale.length());
    }
    
    std::string_view container::locale::str() const {
      auto data = reinterpret_cast<const char*>(&container);
      return std::string_view(data, sizeof(size_t));
    }
    
    bool operator==(const container::locale &first, const container::locale &second) {
      return first.container == second.container;
    }
    
    bool operator!=(const container::locale &first, const container::locale &second) {
      return first.container != second.container;
    }
    
    container::container(sol::state_view v) : v(v) {}
    void container::set(const locale &loc, const std::string_view &key, const sol::object obj) {
      if (key.empty()) throw std::runtime_error("Empty key");
      
      auto itr = localization.find(loc);
      if (itr == localization.end()) {
        itr = localization.emplace(loc, v.create_table(0, 100)).first;
      }
      
      std::array<std::string_view, unpacked_array_size> unpacked_keys;
      const size_t size = unpack_keys(key, unpacked_keys);
      
      sol::table &base_table = itr->second;
      for (size_t i = 0; i < size-1; ++i) {
        auto proxy = base_table[unpacked_keys[i]];
        if (proxy.valid() && proxy.get_type() != sol::type::table) throw std::runtime_error("Bad value type. Key " + std::string(unpacked_keys[i]));
        base_table = proxy.get_or_create<sol::table>();
      }
      
      const auto &last_key = unpacked_keys.back();
      base_table[last_key] = obj;
    }
    
    sol::object container::get(const locale &loc, const std::string_view &key) {
      auto nil_object = sol::make_object(v, sol::nil);
      if (key.empty()) return nil_object;
      
      auto itr = localization.find(loc);
      if (itr == localization.end()) return nil_object;
      
      std::array<std::string_view, unpacked_array_size> unpacked_keys;
      const size_t size = unpack_keys(key, unpacked_keys);
      
      sol::table &base_table = itr->second;
      for (size_t i = 0; i < size-1; ++i) {
        auto proxy = base_table[unpacked_keys[i]];
        if (!proxy.valid()) return nil_object;
        if (proxy.valid() && proxy.get_type() != sol::type::table) return nil_object;
        base_table = proxy.get<sol::table>();
      }
      
      const auto &last_key = unpacked_keys.back();
      return base_table[last_key];
    }
  }
}
