#ifndef UTILS_HANDLE_H
#define UTILS_HANDLE_H

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace devils_engine {
  namespace utils {
    template <typename T>
    class handle {
    public:
      handle() noexcept : ptr(nullptr), token(SIZE_MAX) {}
      handle(std::nullptr_t) noexcept : ptr(nullptr), token(SIZE_MAX) {}
      handle(T* ptr, const size_t &token) noexcept : ptr(ptr), token(token) {}
      handle(const handle &copy) noexcept = default;
      handle(handle &&move) noexcept = default;
      ~handle() noexcept = default;
      handle & operator=(const handle &copy) noexcept = default;
      handle & operator=(handle &&move) noexcept = default;
      handle & operator=(std::nullptr_t) noexcept { ptr = nullptr; token = SIZE_MAX; return *this; }
      
      bool valid() const noexcept { return ptr != nullptr && ptr->object_token == token; }
            T* get()       noexcept { return valid() ? ptr : nullptr; }
      const T* get() const noexcept { return valid() ? ptr : nullptr; }
            T & operator*()       { if (!valid()) throw std::runtime_error("Invalid object"); return *ptr; }
      const T & operator*() const { if (!valid()) throw std::runtime_error("Invalid object"); return *ptr; }
            T* operator->()       { if (!valid()) throw std::runtime_error("Invalid object"); return ptr; }
      const T* operator->() const { if (!valid()) throw std::runtime_error("Invalid object"); return ptr; }
      
      bool operator==(const handle<T> &second) const noexcept { return this->ptr == second.ptr && this->token == second.token; }
      bool operator!=(const handle<T> &second) const noexcept { return !(*this == second); }
      
      // внутренние штуки, не должны выйти за пределы с++ кода
      size_t get_token() const noexcept { return token; }
      bool has_pointer() const noexcept { return ptr != nullptr; }
      T* raw_pointer() const noexcept { return ptr; }
    private:
      T* ptr;
      size_t token;
    };
    
//     template <typename T>
//     inline bool operator==(const handle<T> &first, const handle<T> &second) { return first.raw_pointer() == second.raw_pointer() && first.get_token() == second.get_token(); }
//     template <typename T>
//     inline bool operator!=(const handle<T> &first, const handle<T> &second) { return !(first == second); }
    template <typename T>
    inline bool operator==(const handle<T> &first, std::nullptr_t) noexcept { return first.get() == nullptr; }
    template <typename T>
    inline bool operator!=(const handle<T> &first, std::nullptr_t) noexcept { return first.get() != nullptr; }
    template <typename T>
    inline bool operator==(std::nullptr_t, const handle<T> &second) noexcept { return nullptr == second.get(); }
    template <typename T>
    inline bool operator!=(std::nullptr_t, const handle<T> &second) noexcept { return nullptr != second.get(); }
    template <typename T>
    inline bool operator==(const handle<T> &first, const T* second) noexcept { return first.get() == second; }
    template <typename T>
    inline bool operator!=(const handle<T> &first, const T* second) noexcept { return first.get() != second; }
    template <typename T>
    inline bool operator==(const T* first, const handle<T> &second) noexcept { return first == second.get(); }
    template <typename T>
    inline bool operator!=(const T* first, const handle<T> &second) noexcept { return first != second.get(); }
  }
}

namespace std {
  template <typename T>
  struct hash<devils_engine::utils::handle<T>> {
    size_t operator() (const devils_engine::utils::handle<T> &handle) const noexcept {
      return handle.get_token();
    }
  };
}

#define LUA_HANDLE_DECLARATION(type) \
  class lua_handle_##type { \
  public: \
    inline lua_handle_##type() noexcept : ptr(nullptr), token(SIZE_MAX) {} \
    inline lua_handle_##type(std::nullptr_t) noexcept : ptr(nullptr), token(SIZE_MAX) {} \
    inline lua_handle_##type(core::type* ptr, const size_t &token) noexcept : ptr(ptr), token(token) {} \
    inline lua_handle_##type(const lua_handle_##type &copy) noexcept = default; \
    inline lua_handle_##type(lua_handle_##type &&move) noexcept = default; \
    inline lua_handle_##type(const utils::handle<core::type> &h) noexcept : ptr(h.raw_pointer()), token(h.get_token()) {} \
    ~lua_handle_##type() noexcept = default; \
    inline lua_handle_##type & operator=(const lua_handle_##type &copy) noexcept = default; \
    inline lua_handle_##type & operator=(lua_handle_##type &&move) noexcept = default; \
    inline lua_handle_##type & operator=(std::nullptr_t) noexcept { ptr = nullptr; token = SIZE_MAX; return *this; } \
    \
    inline bool valid() const noexcept { return ptr != nullptr && ptr->object_token == token; } \
    inline core::type* get() const noexcept { return valid() ? ptr : nullptr; } \
    \
    inline bool operator==(const lua_handle_##type &second) const noexcept { return this->ptr == second.ptr && this->token == second.token; } \
    inline bool operator!=(const lua_handle_##type &second) const noexcept { return !(*this == second); } \
    \
    core::type* ptr; \
    size_t token; \
  };

#endif
