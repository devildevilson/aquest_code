#include "template_strings.h"

#include <iostream>
#include "utils/assert.h"

namespace devils_engine {
  namespace utils {
    template <typename STATE, typename RNG_F, typename GET_VALUE_F>
    uint64_t random_at_most(const uint64_t &max, STATE &state, const RNG_F &rng_f, const GET_VALUE_F &get_value_f) {
      ASSERT(max < UINT64_MAX-1);
      // max <= RAND_MAX < ULONG_MAX, so this is okay.
      const uint64_t num_bins = max+1;
      const uint64_t num_rand = UINT64_MAX;
      const uint64_t bin_size = num_rand / num_bins;
      const uint64_t defect   = num_rand % num_bins;
      
      uint64_t x;
      do {
        state = rng_f(state);
        x = get_value_f(state);
      } while (num_rand - defect <= x); // This is carefully written not to overflow
      return x/bin_size;
    }
    
    template_strings::random::random(random_state &gen) : gen(gen) {}
    template_strings::random::~random() {
      for (auto w : wrappers) {
        w->~wrapper();
      }
    }
    void template_strings::random::add(wrapper* w) {
      wrappers.push_back(w);
    }

    std::string template_strings::random::get() {
      ASSERT(!wrappers.empty());
      if (wrappers.size() == 1) return wrappers[0]->get();
      
//       gen = rng_func(gen);
//       const double val = utils::rng_normalize(get_value_func(gen));

      //std::uniform_int_distribution<size_t> dis(0, wrappers.size()-1);
      // в оригинале автор добавлял к результату dis 0.5, зачем?
      //const size_t rand_index = dis(gen);
      //const size_t rand_index = size_t(val * double(wrappers.size()-1));
      const size_t rand_index = random_at_most(wrappers.size()-1, gen, rng_func, get_value_func);
      return wrappers[rand_index]->get();
    }

    enum template_strings::wrapper::type template_strings::random::type() { return type::random; }
    void template_strings::random::print(const std::string &indent) {
      std::cout << indent << "random" << '\n';
      const std::string next_indent = " " + indent;
      for (auto w : wrappers) {
        w->print(next_indent);
      }
    }

    template_strings::sequence::~sequence() {
      for (auto w : wrappers) {
        w->~wrapper();
      }
    }

    void template_strings::sequence::add(wrapper* w) {
      wrappers.push_back(w);
    }

    std::string template_strings::sequence::get() {
      if (wrappers.empty()) return "";
      if (wrappers.size() == 1) return wrappers[0]->get();

      std::string str;
      for (auto w : wrappers) {
        str.append(w->get());
      }

      return str;
    }

    enum template_strings::wrapper::type template_strings::sequence::type() { return type::sequence; }
    void template_strings::sequence::print(const std::string &indent) {
      std::cout << indent << "sequence" << '\n';
      const std::string next_indent = " " + indent;
      for (auto w : wrappers) {
        w->print(next_indent);
      }
    }

    template_strings::reverser::~reverser() {
      w->~wrapper();
    }

    void template_strings::reverser::add(wrapper* w) {
      this->w = w;
    }

    std::string template_strings::reverser::get() {
      std::string str = w->get();
      if (str.empty()) return "";
      std::reverse(str.begin(), str.end());
      return str;
    }

    enum template_strings::wrapper::type template_strings::reverser::type() { return type::capitalizer; }
    void template_strings::reverser::print(const std::string &indent) {
      std::cout << indent << "reverser" << '\n';
      const std::string next_indent = " " + indent;
      w->print(next_indent);
    }

    template_strings::capitalizer::~capitalizer() {
      w->~wrapper();
    }

    void template_strings::capitalizer::add(wrapper* w) {
      this->w = w;
    }

    std::string template_strings::capitalizer::get() {
      std::string str = w->get();
      if (str.empty()) return "";
      str[0] = std::toupper(str[0]);
      return str;
    }

    enum template_strings::wrapper::type template_strings::capitalizer::type() { return type::capitalizer; }
    void template_strings::capitalizer::print(const std::string &indent) {
      std::cout << indent << "capitalizer" << '\n';
      const std::string next_indent = " " + indent;
      w->print(next_indent);
    }

    template_strings::table_literal::table_literal(const char c, random_state &gen, table_type &table) : c(c), gen(gen), table(table) {}
    template_strings::table_literal::~table_literal() {}
    void template_strings::table_literal::add(wrapper*) {}
    std::string template_strings::table_literal::get() {
      auto itr = table.find(c);
      if (itr == table.end()) return "";
      //std::uniform_int_distribution<size_t> dis(0, itr->second.size()-1);
      //const size_t rand_index = dis(gen);
      const size_t rand_index = random_at_most(itr->second.size()-1, gen, rng_func, get_value_func);
      return itr->second[rand_index];
    }

    enum template_strings::wrapper::type template_strings::table_literal::type() { return type::table_literal; }
    void template_strings::table_literal::print(const std::string &indent) {
      std::cout << indent << "table_literal" << '\n';
    }

    template_strings::word_literal::word_literal(const std::string &str) : str(str) {}
    template_strings::word_literal::~word_literal() {}
    void template_strings::word_literal::add(wrapper*) {}
    std::string template_strings::word_literal::get() { return str; }
    enum template_strings::wrapper::type template_strings::word_literal::type() { return type::word_literal; }
    void template_strings::word_literal::print(const std::string &indent) {
      std::cout << indent << "word_literal" << '\n';
    }

    template_strings::empty_literal::~empty_literal() {}
    void template_strings::empty_literal::add(wrapper*) {}
    std::string template_strings::empty_literal::get() { return ""; }
    enum template_strings::wrapper::type template_strings::empty_literal::type() { return type::empty_literal; }
    void template_strings::empty_literal::print(const std::string &indent) {
      std::cout << indent << "empty_literal" << '\n';
    }

    template_strings::template_strings() : state(init_func(1)), pattern_size(0), pattern_memory(nullptr), generator(nullptr) {}
    template_strings::template_strings(const uint64_t &seed) : state(init_func(seed)), pattern_size(0), pattern_memory(nullptr), generator(nullptr) {}
    template_strings::~template_strings() {
      clear();
    }

    void template_strings::set_table(const char c, const std::vector<std::string> &strings) {
      table[c] = strings;
    }

    template <typename T, typename... Args>
    T* create_wrapper(const size_t &mem_size, size_t &current_size, char* memory, Args&& ...args) {
      ASSERT(current_size + sizeof(T) <= mem_size);
      auto wrap = new (&memory[current_size]) T(std::forward<Args>(args)...);
      current_size += sizeof(T);
      return wrap;
    }

    enum class wrapper {
      reverser,
      capitalizer
    };

    // ~(%f|%a)(~%f|%a)
    // (qasfsfsq%fsdqsdsf)(...)
    void template_strings::set_pattern(const std::string &s) {
      std::vector<wrapper*> stack;
      const auto word_add_func = [this, &stack] (std::string &word, size_t &current_size) {
        if (word.empty()) return;

        wrapper* w = create_wrapper<word_literal>(pattern_size, current_size, pattern_memory, word);

        while (stack.back()->type() == wrapper::type::reverser || stack.back()->type() == wrapper::type::capitalizer) {
          auto tmp = stack.back();
          stack.pop_back();
          tmp->add(w);
          w = tmp;
        }

        stack.back()->add(w);
        word.clear();
      };

      const auto table_add_func = [this, &stack] (const char c, size_t &current_size) {
        wrapper* w = create_wrapper<table_literal>(pattern_size, current_size, pattern_memory, c, state, table);

        while (stack.back()->type() == wrapper::type::reverser || stack.back()->type() == wrapper::type::capitalizer) {
          auto tmp = stack.back();
          stack.pop_back();
          tmp->add(w);
          w = tmp;
        }

        stack.back()->add(w);
      };

//       std::vector<std::vector<produce_func>> funcs;
//       funcs.emplace_back();
      size_t word_start = 0;
      size_t current_size = sizeof(sequence);
      for (size_t i = 0; i < s.length(); ++i) {
        const char current = s[i];
        switch (current) {
          case '(': {
            if (word_start != i) current_size += sizeof(word_literal);
            current_size += sizeof(random);
            current_size += sizeof(sequence);
            word_start = i+1;
            break;
          }

          case ')': {
            if (word_start != i) current_size += sizeof(word_literal);
            word_start = i+1;
            break;
          }

          case '|': {
            if (word_start != i) current_size += sizeof(word_literal);
            current_size += sizeof(sequence);
            word_start = i+1;
            break;
          }

          case '%': {
            if (word_start != i) current_size += sizeof(word_literal);
            current_size += sizeof(table_literal);
            ++i;
            word_start = i+1;
            break;
          }

          case '~': {
            if (word_start != i) current_size += sizeof(word_literal);
            word_start = i+1;
            current_size += sizeof(reverser);
            break;
          }

          case '!': {
            if (word_start != i) current_size += sizeof(word_literal);
            word_start = i+1;
            current_size += sizeof(capitalizer);
            break;
          }
        }
      }

      clear();

      pattern_size = current_size;
      pattern_memory = new char[pattern_size];
      size_t current_memory = 0;
      stack.push_back(create_wrapper<sequence>(pattern_size, current_memory, pattern_memory));
      std::string word;
      for (size_t i = 0; i < s.length(); ++i) {
        const char current = s[i];
        switch (current) {
          case '(': {
            word_add_func(word, current_memory);

            stack.push_back(create_wrapper<random>(pattern_size, current_memory, pattern_memory, std::ref(state)));
            auto s = create_wrapper<sequence>(pattern_size, current_memory, pattern_memory);
            stack.back()->add(s);
            stack.push_back(s);
            break;
          }

          case ')': {
            word_add_func(word, current_memory);

            ASSERT(stack.size() > 2);
            stack.pop_back();
            auto cur = stack.back();
            stack.pop_back();
            while (stack.back()->type() == wrapper::type::reverser || stack.back()->type() == wrapper::type::capitalizer) {
              stack.back()->add(cur);
              cur = stack.back();
              stack.pop_back();
            }

            stack.back()->add(cur);

            break;
          }

          case '|': {
            word_add_func(word, current_memory);

            ASSERT(stack.size() > 1);
            ASSERT(stack.back()->type() == wrapper::type::sequence);
            stack.pop_back();
            auto s = create_wrapper<sequence>(pattern_size, current_memory, pattern_memory);
            stack.back()->add(s);
            stack.push_back(s);
            break;
          }

          case '%': {
            word_add_func(word, current_memory);

            ASSERT(s.length() > i+1);
            const char c = s[i+1];
            table_add_func(c, current_memory);
            ++i;

            break;
          }

          case '~': {
            word_add_func(word, current_memory);

            auto wrap = create_wrapper<reverser>(pattern_size, current_memory, pattern_memory);
            stack.push_back(wrap);
            break;
          }

          case '!': {
            word_add_func(word, current_memory);

            auto wrap = create_wrapper<capitalizer>(pattern_size, current_memory, pattern_memory);
            stack.push_back(wrap);
            break;
          }

          default:
            word.push_back(current);
            break;
        }
      }

      ASSERT(stack.size() == 1);
      generator = stack.back();
      stack.pop_back();
    }

    std::string template_strings::produce() {
      return generator->get();
    }

    void template_strings::print() {
      generator->print("");
    }
    
    void template_strings::clear() {
      if (generator != nullptr) {
        generator->~wrapper();
        delete [] pattern_memory;
        pattern_size = 0;
      }
    }
  }
}
