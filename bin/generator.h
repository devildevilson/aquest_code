#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>
#include <vector>
#include <atomic>
#include <functional>

#include "generator_context2.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

// вообще государство концентрируется на персонажах
// и мне нужно продумать как организовать персонажей
// хороший вопрос, константная структура персонажа?
// вообще конечно лучше бы нет, но какие то вещи строго определены
// портрет, отношения, семья, предметы, титулы, (характеристики лучше бы из конфига подгружать)
// для того чтобы сгенерировать государства нужно продумать хорошо систему титулов
// по сути государство - это набор определенных титулов у персонажа
// если у персонажа нет сюзерена - то это отдельное государство
// нужно сделать каркас персонажа и добавить его в контейнер как отдельную сущность

namespace devils_engine {
  namespace systems {
    class generator {
    public:
      using function = std::function<void(map::generator::context*, sol::table&)>;
      using part = std::pair<std::string, function>;
      
      generator();
      void add(const std::string &hint, const function &func);
      void generate(map::generator::context* context, sol::table &data);
      std::string hint() const;
      size_t size() const;
      size_t current() const;
    private:
      std::vector<part> parts;
      std::atomic<size_t> current_part;
    };
  }
}

#endif
