#ifndef SCRIPT_TARGET_H
#define SCRIPT_TARGET_H

#include <cstdint>

namespace devils_engine {
  namespace script {
    // это всегда должна быть конкретная игровая сущность
    // на вход подавать script_data?
    // нужно бы переименовать тип, чтобы он не пересекался с переменными
    struct target_t {
      uint32_t type;
      void* data;
      size_t token; // для многих вещей важно указать токен
      
      inline target_t() noexcept : type(UINT32_MAX), data(nullptr), token(SIZE_MAX) {}
      inline target_t(const uint32_t &type, void* data, const size_t &token) noexcept : type(type), data(data), token(token) {}
      inline target_t(const target_t &copy) noexcept = default;
      inline target_t(target_t &&move) noexcept = default;
      template <typename T>
      inline target_t(T* ptr) noexcept : type(static_cast<uint32_t>(T::s_type)), data(ptr), token(SIZE_MAX) {}
      template <typename T>
      inline target_t(T* ptr, const size_t &token) noexcept : type(static_cast<uint32_t>(T::s_type)), data(ptr), token(token) {}
      
      inline target_t & operator=(const target_t &copy) noexcept = default;
      inline target_t & operator=(target_t &&copy) noexcept = default;
    };
    
    // по идее нужно везде использовать safe_target, 
    // но с ним будет минимум одно обращение к тяжелым массивам
    // которое не сказать чтобы нужно, мне нужно перед тем как 
    // запускать скрипт превратить safe_target в target
//     struct safe_target {
//       uint32_t type;
//       size_t token;
//     };
  }
}

#endif
