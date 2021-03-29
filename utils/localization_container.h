#ifndef LOCALIZATION_CONTAINER_H
#define LOCALIZATION_CONTAINER_H

#include "parallel_hashmap/phmap.h"
#include "sol.h"

// этого будет достаточно для локализации, другое дело что мне не нравится что будет расходоваться слишком много памяти
// и нельзя этот класс будет использовать в других луа стейтах, в принципе использовать в других стейтах ненужно
// но памяти реально много будет занимать, что делать? фиг знает, оставим так

namespace devils_engine {
  namespace localization {
    class container {
    public:
      struct locale {
        size_t container;
        
        locale();
        locale(const std::string_view &str_locale);
        locale(const locale &loc) = default;
        locale(locale &&loc) = default;        
        locale & operator=(const locale &loc) = default;
        locale & operator=(locale &&loc) = default;
        
        std::string_view str() const;
        friend bool operator==(const locale &first, const locale &second);
        friend bool operator!=(const locale &first, const locale &second);
      };
      
      container(sol::state_view v);
      void set(const locale &loc, const std::string_view &key, const sol::object obj); 
      sol::object get(const locale &loc, const std::string_view &key);
    private:
      sol::state_view v;
      phmap::flat_hash_map<locale, sol::table> localization;
    };
  }
}

#endif
