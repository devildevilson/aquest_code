#include "localization_container.h"

#include <array>
#include <iostream>
#include <charconv>
#include "shared_mathematical_constant.h"
#include "assert.h"
#include "constexpr_funcs.h"

// имеет смысл поискать число в ключе, и тогда мы можем избавиться от чисел в персонаж

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
    
    bool check_number(const std::string_view &str, double &num) {
      const auto res = std::from_chars(str.begin(), str.end(), num);
      return res.ec != std::errc::invalid_argument && res.ec != std::errc::result_out_of_range;
    }
    
//     size_t unpack_keys(const std::string_view &keys, std::array<std::string_view, unpacked_array_size> &unpacked_keys) {
//       size_t array_size = 0;
//       size_t pos = 0;
//       while (pos != std::string::npos) {
//         const size_t new_pos = keys.find('.', pos);
//         const auto str = keys.substr(pos, new_pos-pos);
//         const size_t index = array_size;
//         if (index >= unpacked_array_size) throw std::runtime_error("Maximum table depth is " + std::to_string(unpacked_array_size));
//         ++array_size;
//         unpacked_keys[index] = str;
//         pos = new_pos == std::string::npos ? new_pos : new_pos+1;
//       }
//       
//       return array_size;
//     }
    
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
    
    void container::set_current_locale(const locale &l) {
      current_locale = l;
    }
    
    container::locale container::get_current_locale() {
      return current_locale;
    }
    
    container::locale container::get_fall_back_locale() {
      return fall_back_locale;
    }
    
    container::container(sol::state_view v) : v(v) {}
    void container::set_table(const locale &loc, const sol::table &t) {
      const bool valid = is_valid_table(t);
      if (!valid) throw std::runtime_error("Trying to set invalid table");
      
      auto itr = localization.find(loc);
      if (itr == localization.end()) {
        localization.emplace(loc, t);
        return;
      }
      
      sol::table base_table = itr->second;
      set_table(base_table, t);
    }
    
    void container::set(const locale &loc, const std::string_view &key, const sol::object obj) {
      if (key.empty()) throw std::runtime_error("Empty key");
      if (obj.get_type() == sol::type::table && !is_valid_table(obj.as<sol::table>())) throw std::runtime_error("Trying to set invalid table");
      
      auto itr = localization.find(loc);
      if (itr == localization.end()) {
        itr = localization.emplace(loc, v.create_table(0, 100)).first;
      }
      
      std::array<std::string_view, unpacked_array_size> unpacked_keys;
      const size_t size = divide_token(key, ".", unpacked_array_size, unpacked_keys.data());
      //const size_t size = unpack_keys(key, unpacked_keys);
      
      sol::table &base_table = itr->second;
      for (size_t i = 0; i < size-1; ++i) {
        const auto current_key = unpacked_keys[i];
        
        double num = 0.0;
        if (check_number(current_key, num)) {
          auto proxy = base_table[num];
          if (proxy.valid() && proxy.get_type() != sol::type::table) throw std::runtime_error("Bad value type. Key " + std::string(current_key));
          base_table = proxy.get<sol::table>();
          continue;
        }
        
        auto proxy = base_table[current_key];
        if (proxy.valid() && proxy.get_type() != sol::type::table) throw std::runtime_error("Bad value type. Key " + std::string(current_key));
        base_table = proxy.get_or_create<sol::table>();
      }
      
      const auto &last_key = unpacked_keys.back();
      double num = 0.0;
      if (check_number(last_key, num)) { base_table[num] = obj; return; }
      base_table[last_key] = obj;
    }
    
    sol::object container::get(const locale &loc, const std::string_view &key) {
      if (key.empty()) return sol::object(sol::nil);
      
      auto itr = localization.find(loc);
      if (itr == localization.end()) return sol::object(sol::nil);
      
      std::array<std::string_view, unpacked_array_size> unpacked_keys;
      const size_t size = divide_token(key, ".", unpacked_array_size, unpacked_keys.data());
      //const size_t size = unpack_keys(key, unpacked_keys);
      
      sol::table &base_table = itr->second;
      for (size_t i = 0; i < size-1; ++i) {
        const auto current_key = unpacked_keys[i];
        
        double num = 0.0;
        if (check_number(current_key, num)) {
          auto proxy = base_table[num];
          if (!proxy.valid()) return sol::object(sol::nil);
          if (proxy.valid() && proxy.get_type() != sol::type::table) return sol::object(sol::nil);
          base_table = proxy.get<sol::table>();
          continue;
        }
        
        auto proxy = base_table[current_key];
        if (!proxy.valid()) return sol::object(sol::nil);
        if (proxy.valid() && proxy.get_type() != sol::type::table) return sol::object(sol::nil);
        base_table = proxy.get<sol::table>();
      }
      
      const auto &last_key = unpacked_keys.back();
      double num = 0.0;
      if (check_number(last_key, num)) return base_table[num];
      return base_table[last_key];
    }
    
    void container::clear() {
      
    }
    
    std::string container::serialize(const std::function<std::string(const sol::table &)> &f) {
      std::string container = "return {";
      for (const auto &pair : localization) {
        const std::string loc_key = std::string(pair.first.str());
        const std::string table = f(pair.second);
        container += loc_key + "=" + table + ",";
      }
      
      container += "}";
      return container;
    }
    
    void container::deserialize(const std::string &data) {
      const auto ret = v.safe_script(data);
      if (!ret.valid()) {
        sol::error err = ret;
        std::cout << err.what();
        throw std::runtime_error("There is lua error");
      }
      
      const sol::table t = ret;
      for (const auto &pair : t) {
        ASSERT(pair.first.get_type() == sol::type::string);
        ASSERT(pair.second.get_type() == sol::type::table);
        
        const std::string_view loc_key = pair.first.as<std::string_view>();
        container::locale loc(loc_key);
        const sol::table t = pair.second.as<sol::table>();
        ASSERT(localization.find(loc) == localization.end());
        localization.insert(std::make_pair(loc, t));
      }
    }
    
    void container::set_table(sol::table current_table, sol::table new_table) {
      for (const auto &pair : new_table) {
        auto proxy = current_table[pair.first];
        if (!proxy.valid()) {
          current_table[pair.first] = pair.second;
          continue;
        }
        
        if (proxy.get_type() == sol::type::table && pair.second.get_type() == sol::type::table) {
          set_table(proxy.get<sol::table>(), pair.second.as<sol::table>());
          continue;
        }
        
//         if (proxy.get_type() == pair.second.get_type()) {
//           if (proxy.get_type() == sol::type::string) {
//             if (proxy.get<std::string_view>() == pair.second.as<std::string_view>()) continue;
//           } else if (proxy.get_type() == sol::type::number) {
//             if (std::abs(proxy.get<double>() - pair.second.as<double>()) < EPSILON) continue;
//           }
//         }
        
        // мы должны заменить одну перменную на другую
        if (pair.first.get_type() == sol::type::string) std::cout << "Shadowing value " << pair.first.as<std::string_view>() << "\n";
        else if (pair.first.get_type() == sol::type::number) std::cout << "Shadowing value " << pair.first.as<double>() << "\n";
        
        current_table[pair.first] = pair.second;
      }
    }
    
    container::locale container::current_locale;
    container::locale const container::fall_back_locale = locale("en");
    
    bool is_valid_locale(const std::string_view &key) {
      return key.length() <= sizeof(size_t);
    }
    
    bool is_valid_table(const size_t &current_layer, const sol::table &t) {
      if (current_layer >= unpacked_array_size) return false;
      
      for (const auto &pair : t) {
        const bool valid_key = pair.first.get_type() == sol::type::number || pair.first.get_type() == sol::type::string;
        const bool valid_value = pair.second.get_type() == sol::type::number || pair.second.get_type() == sol::type::string || pair.second.get_type() == sol::type::table;
        
        if (!valid_key || !valid_value) return false;
        if (pair.second.get_type() == sol::type::table && !is_valid_table(current_layer+1, pair.second.as<sol::table>())) return false;
      }
      
      return true;
    }
    
    bool is_valid_table(const sol::table &t) {
      const size_t current_layer = 0;
      for (const auto &pair : t) {
        const bool valid_key = pair.first.get_type() == sol::type::number || pair.first.get_type() == sol::type::string;
        const bool valid_value = pair.second.get_type() == sol::type::number || pair.second.get_type() == sol::type::string || pair.second.get_type() == sol::type::table;
        
        if (!valid_key || !valid_value) return false;
        if (pair.second.get_type() == sol::type::table && !is_valid_table(current_layer+1, pair.second.as<sol::table>())) return false;
      }
      
      return true;
    }
  }
}
