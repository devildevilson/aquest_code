#include "string_bank.h"
#include "utils/assert.h"
#include <stdexcept>
#include <iostream>
#include <cstring>

namespace devils_engine {
  namespace utils {
    locale::locale(const uint32_t locale) : container(locale) {}
    locale::locale(const std::string_view &locale) : container(0) {
      if (locale.empty()) throw std::runtime_error("Empty locale id is not supported");
      if (locale.length() >= 4) throw std::runtime_error("Bad locale " + std::string(locale) + ". Locale id must be 3 or less length");
      
      auto mem = reinterpret_cast<char*>(&container);
      memcpy(mem, locale.data(), std::min(locale.length(), size_t(4)));
    }
    
    uint32_t locale::get_internal_code() const {
      return container;
    }
    
    std::string locale::to_string() const {
      char tmp[5] = {0};
      auto mem = reinterpret_cast<const char*>(&container);
      memcpy(tmp, mem, 4);
      tmp[4] = '\0';
      return std::string(tmp);
    }
    
    bool operator==(const locale &l1, const locale &l2) {
      return l1.get_internal_code() == l2.get_internal_code();
    }
    
    bool operator!=(const locale &l1, const locale &l2) {
      return l1.get_internal_code() != l2.get_internal_code();
    }
    
    locale localization::current_locale() {
      return m_current_locale;
    }
    
    void localization::set_current_locale(const std::string_view &locale) {
      m_current_locale = (struct locale)(locale);
    }
    
    void localization::set_current_locale(const struct locale &locale) {
      m_current_locale = locale;
    }
    
    locale localization::fallback_locale() {
      return m_fallback_locale;
    }
    
    localization::string_bank::string_bank(localization* loc, const uint32_t &bank_id) : m_bank_id(bank_id), loc(loc) {}
    size_t localization::string_bank::insert_string(const std::string &id, const std::string_view &locale, const std::string &str) {
      const struct locale l(locale);
      const size_t id_code = loc->get_id(id);
      uint32_t string_index = UINT32_MAX;
      if (id_code == SIZE_MAX) {
        string_index = container.size();
        container.emplace_back();
        const size_t code = make_id(string_index);
        loc->string_id.emplace(id, code);
      } else {
        const uint32_t bank_index = uint32_t(id_code >> 32);
        if (m_bank_id != bank_index) {
          //const auto &str = loc->get_string(id_code);
          //throw std::runtime_error("String id " + id + " is already registered to string: " + str);
          throw std::runtime_error("The string " + id + " belongs to the another string bank");
        }
        
        string_index = uint32_t(id_code);
      }
      
      auto &string_locales = container[string_index];
      for (const auto &cont : string_locales) {
        if (cont.locale == l) throw std::runtime_error("Trying to rewrite string " + id + " locale");
      }
      
      string_locales.push_back({l, str});
      const size_t code = make_id(string_index);
      return code;
    }
    
    std::string_view localization::string_bank::get_string(const uint32_t &index, const locale &l) {
      if (index >= container.size()) throw std::runtime_error("Bad container index");
      auto &string_locales = container[index];
      for (const auto &cont : string_locales) {
        if (cont.locale == l) return cont.str;
      }
      
      return "";
    }
    
    size_t localization::string_bank::make_id(const uint32_t &str_index) const {
      return (size_t(m_bank_id) << 32) | size_t(str_index);
    }
    
    size_t localization::string_bank::size() const {
      return container.size();
    }
    
    uint32_t localization::string_bank::bank_id() const {
      return m_bank_id;
    }
    
    localization::~localization() {
      for (auto ptr : banks) {
        bank_pool.destroy(ptr);
      }
    }
    
    std::pair<localization::string_bank*, uint32_t> localization::create_bank() {
      auto ptr = bank_pool.create(this, banks.size());
      auto pair = std::make_pair(ptr, banks.size());
      banks.push_back(ptr);
      return pair;
    }
    
    localization::string_bank* localization::get_bank(const uint32_t &index) const {
      if (index >= banks.size()) return nullptr;
      return banks[index];
    }
    
    std::string_view localization::get_string(const size_t &id, const std::string &str_id) const {
      const uint32_t bank_index = uint32_t(id >> 32);
      const uint32_t string_index = uint32_t(id);
      if (bank_index >= banks.size()) throw std::runtime_error("Bad string bank index");
      auto bank = banks[bank_index];
      const auto &str = bank->get_string(string_index, m_current_locale);
      if (str.empty()) {
        // не можем найти строчку, нужно в консоль вывести
        if (!str_id.empty()) std::cout << "Could not find string " << str_id << " for the current locale " << m_current_locale.to_string() << "\n";
        else std::cout << "Could not find string for the current locale " << m_current_locale.to_string() << "\n";
        return bank->get_string(string_index, m_fallback_locale);
      }
      return str;
    }
    
    std::string_view localization::get_string(const std::string &id) const {
      auto itr = string_id.find(id);
      if (itr == string_id.end()) return id;
      const auto &str = get_string(itr->second, id);
      if (str.empty()) return id;
      return str;
    }
    
    size_t localization::get_id(const std::string &id) const {
      auto itr = string_id.find(id);
      if (itr == string_id.end()) return SIZE_MAX;
      return itr->second;
    }
    
    locale localization::m_current_locale = locale("en");
    locale localization::m_fallback_locale = locale("en");
  }
}
