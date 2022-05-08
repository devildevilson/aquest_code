#ifndef STRING_BANK_H
#define STRING_BANK_H

#include <vector>
#include <string>
//#include <unordered_map>
#include "parallel_hashmap/phmap.h"
#include "memory_pool.h"

// нужно будет сериализовать

namespace devils_engine {
  namespace utils {
    struct locale {
      uint32_t container;
      
      locale(const uint32_t locale);
      locale(const std::string_view &locale);
      uint32_t get_internal_code() const;
      std::string to_string() const;
    };
    
    bool operator==(const locale &l1, const locale &l2);
    bool operator!=(const locale &l1, const locale &l2);
    
    class localization {
      friend class string_bank;
    public:
      static locale current_locale();
      static void set_current_locale(const std::string_view &locale);
      static void set_current_locale(const struct locale &locale);
      static locale fallback_locale();
      
      class string_bank {
      public:
        struct locale_strings {
          struct locale locale;
          std::string str;
        };
        
        string_bank(localization* loc, const uint32_t &bank_id);
        string_bank(const string_bank &b) = delete;
        string_bank(string_bank&& b) = delete;
        string_bank & operator=(const string_bank &b) = delete;
        string_bank & operator=(string_bank&& b) = delete;
        ~string_bank() = default;
        
        size_t insert_string(const std::string &id, const std::string_view &locale, const std::string &str);
        std::string_view get_string(const uint32_t &index, const locale &l);
        size_t make_id(const uint32_t &str_index) const;
        size_t size() const;
        uint32_t bank_id() const;
      private:
        uint32_t m_bank_id;
        localization* loc;
        std::vector<std::vector<locale_strings>> container;
      };
      
      localization() = default;
      localization(const localization &l) = delete;
      localization(localization&& l) = delete;
      localization & operator=(const localization &b) = delete;
      localization & operator=(localization&& b) = delete;
      ~localization();
      
      std::pair<string_bank*, uint32_t> create_bank();
      string_bank* get_bank(const uint32_t &index) const;
      std::string_view get_string(const size_t &id, const std::string &str_id = "") const;
      std::string_view get_string(const std::string &id) const;
      size_t get_id(const std::string &id) const;
    private:
      memory_pool<string_bank, sizeof(string_bank)*15> bank_pool;
      std::vector<string_bank*> banks;
      //std::unordered_map<std::string, size_t> string_id;
      phmap::flat_hash_map<std::string, size_t> string_id;
      
      static locale m_current_locale;
      static locale m_fallback_locale;
    };
  }
}

#endif
