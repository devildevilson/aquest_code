#ifndef DEVILS_ENGINE_SCRIPT_HEADER_H
#define DEVILS_ENGINE_SCRIPT_HEADER_H

#include <string_view>
// #include "container.h"
// #include "utils/sol.h"
// #include "input_data.h"
#include "interface.h"

// существует 5 типов того что я хочу получить: число, строка, объект, проверка условий, действие
// нужно ли это разбить на разные классы?
namespace devils_engine {
  namespace script {
    class interface;
    struct context;
    
//     template <typename T>
//     class base {
//     public:
//       base() noexcept : begin(nullptr) {}
//       base(const base &copy) noexcept = delete;
//       base(base &&move) noexcept = default;
//       ~base() noexcept { if (valid()) begin->~interface(); }
//       void init(container &&c, const interface* begin) { if (this->begin != nullptr) throw std::runtime_error("Double init script"); this->c = std::move(c); this->begin = begin; }
//       T compute(context* ctx) const { if constexpr (std::is_same_v<T, void>) { begin->process(ctx); return; } const auto ret = begin->process(ctx); return ret.get<T>(); }
//       void draw(context* ctx) const { begin->draw(ctx); }
//       bool valid() const { return begin != nullptr; }
//       size_t size() const { return c.mem_size(); }
//       
//       base & operator=(const base &copy) noexcept = delete;
//       base & operator=(base &&move) noexcept = default;
//     private:
// //       container c;
//       const interface* begin; 
//     };
    
    template <typename T>
    class base {
    public:
      base() noexcept : begin(nullptr) {}
      base(const interface* begin) noexcept : begin(begin) {}
      base(const base &copy) noexcept = default;
      base(base &&move) noexcept = default;
      base & operator=(const base &copy) noexcept = default;
      base & operator=(base &&move) noexcept = default;
      
      T compute(context* ctx) const { if constexpr (std::is_same_v<T, void>) { begin->process(ctx); return; } else { const auto ret = begin->process(ctx); return ret.get<T>(); } }
      void draw(context* ctx) const { begin->draw(ctx); }
      bool valid() const { return begin != nullptr; }
    private:
      const interface* begin; 
    };
    
    class number    : public base<double> {
    public:
      number() noexcept = default; 
      number(const interface* begin) noexcept : base(begin) {}  
      number(const number &copy) noexcept = default;
      number(number &&move) noexcept = default;
      number & operator=(const number &copy) noexcept = default;
      number & operator=(number &&move) noexcept = default;
    };
    
    class string    : public base<std::string_view> {
    public:
      string() noexcept = default; 
      string(const interface* begin) noexcept : base(begin) {}  
      string(const string &copy) noexcept = default;
      string(string &&move) noexcept = default;
      string & operator=(const string &copy) noexcept = default;
      string & operator=(string &&move) noexcept = default;
      
    };
    class condition : public base<bool> { 
    public:
      condition() noexcept = default; 
      condition(const interface* begin) noexcept : base(begin) {}  
      condition(const condition &copy) noexcept = default;
      condition(condition &&move) noexcept = default;
      condition & operator=(const condition &copy) noexcept = default;
      condition & operator=(condition &&move) noexcept = default;
    };
    
    class effect    : public base<void> {
    public:
      effect() noexcept = default; 
      effect(const interface* begin) noexcept : base(begin) {}  
      effect(const effect &copy) noexcept = default;
      effect(effect &&move) noexcept = default;
      effect & operator=(const effect &copy) noexcept = default;
      effect & operator=(effect &&move) noexcept = default;
    };
    
    template <typename T>
    class user_data : public base<T> {
    public:
      user_data() noexcept = default; 
      user_data(const interface* begin) noexcept : base<T>(begin) {}  
      user_data(const user_data &copy) noexcept = default;
      user_data(user_data &&move) noexcept = default;
      user_data & operator=(const user_data &copy) noexcept = default;
      user_data & operator=(user_data &&move) noexcept = default;
    };
    
        // нужно ли мне получать объект из скрипта? в самом скрипте это удобно, а во вне? неочевидно
    
//     void create_number(const input_data &input, number* num, const sol::object &obj);
//     void create_string(const input_data &input, string* num, const sol::object &obj);
//     void create_condition(const input_data &input, condition* num, const sol::object &obj);
//     void create_effect(const input_data &input, effect* num, const sol::object &obj);
  }
}

#endif
