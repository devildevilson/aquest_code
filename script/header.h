#ifndef DEVILS_ENGINE_SCRIPT_HEADER_H
#define DEVILS_ENGINE_SCRIPT_HEADER_H

#include <string_view>
#include "container.h"
#include "utils/sol.h"
#include "input_data.h"
#include "interface.h"

// существует 5 типов того что я хочу получить: число, строка, объект, проверка условий, действие
// нужно ли это разбить на разные классы?
namespace devils_engine {
  namespace script {
    class interface;
    struct context;
    
    template <typename T>
    class base {
    public:
      base() noexcept : begin(nullptr) {}
      ~base() noexcept { begin->~interface(); }
      void init(container &&c, const interface* begin) { if (this->begin != nullptr) throw std::runtime_error("Double init script"); this->c = std::move(c); this->begin = begin; }
      T compute(context* ctx) const { if constexpr (std::is_same_v<T, void>) { begin->process(ctx); return; } const auto ret = begin->process(ctx); return ret.get<T>(); }
      void draw(context* ctx) const { begin->draw(ctx); }
      bool valid() const { return begin != nullptr; }
    private:
      container c;
      const interface* begin; 
    };
    
    class number : public base<double> {};
    class string : public base<std::string_view> {};
    class condition : public base<bool> {};
    class effect : public base<void> {};
    
        // нужно ли мне получать объект из скрипта? в самом скрипте это удобно, а во вне? неочевидно
//     class object {
//     public:
//       object compute() const;
//     private:
//       container c;
//       const interface* begin;
//     };
    
    void create_number(const input_data &input, number* num, const sol::object &obj);
    void create_string(const input_data &input, string* num, const sol::object &obj);
    void create_condition(const input_data &input, condition* num, const sol::object &obj);
    void create_effect(const input_data &input, effect* num, const sol::object &obj);
  }
}

#endif
