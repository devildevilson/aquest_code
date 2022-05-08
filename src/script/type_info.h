#ifndef DEVILS_ENGINE_SCRIPT_TYPE_INFO_H
#define DEVILS_ENGINE_SCRIPT_TYPE_INFO_H

#include <type_traits>
#include <cstddef>
#include <atomic>
#include "type_traits.h"
#include "utils/constexpr_funcs.h"

#define SCRIPT_FIRST_TYPE_ID 5

// я вот что подумал: можно сократить количество темплейтов, если указывать хендл как основной тип
// + к этому можно хранить в object в принципе любой объект <= 16байт
// 16байт - это типичный размер, наверное, для всех хендлов + для double, std::string_view, glm::vec4
// остается вопрос const/nonconst, скорее всего у меня ряд вещей будет строго константными везде
// в остальных случаях неважно конст/неконст буду хранить не конст

namespace devils_engine {
  namespace script {    
    // remove_cv_t не убирает константность с указателя, что предпочтительно
    static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<int* const>>, int*>);
    static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<const int*>>, const int*>);
    
    template <typename T>
    constexpr size_t type_id() {
      using type = std::remove_reference_t<std::remove_cv_t<T>>; // std::remove_pointer_t
      const auto name = detail::get_type_name<type>();
      return string_hash(name);
    }
  }
}

#endif
