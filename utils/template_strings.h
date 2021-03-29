#ifndef TEMPLATE_STRINGS_H
#define TEMPLATE_STRINGS_H

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <random>
#include "linear_rng.h"

// вдохновитель https://github.com/skeeto/fantasyname

namespace devils_engine {
  namespace utils {
    class template_strings {
    public:
      using table_type = std::unordered_map<char, std::vector<std::string>>;

      class wrapper {
      public:
        enum class type {
          random,
          sequence,
          reverser,
          capitalizer,
          table_literal,
          word_literal,
          empty_literal
        };

        virtual ~wrapper() = default;
        virtual void add(wrapper* w) = 0;
        virtual std::string get() = 0;
        virtual enum type type() = 0;
        virtual void print(const std::string &indent) = 0;
      };

      class random : public wrapper {
      public:
        using random_state = xoshiro256starstar::state;
        constexpr static const auto init_func = xoshiro256starstar::init;
        constexpr static const auto rng_func = xoshiro256starstar::rng;
        constexpr static const auto get_value_func = xoshiro256starstar::get_value;
        
        random(random_state &gen);
        ~random();
        void add(wrapper* w) override;
        std::string get() override;
        enum type type() override;
        void print(const std::string &indent) override;
      private:
        random_state &gen;
        std::vector<wrapper*> wrappers;
      };

      class sequence : public wrapper {
      public:
        ~sequence();
        void add(wrapper* w) override;
        std::string get() override;
        enum type type() override;
        void print(const std::string &indent) override;
      private:
        std::vector<wrapper*> wrappers;
      };

      class reverser : public wrapper {
      public:
        ~reverser();
        void add(wrapper* w) override;
        std::string get() override;
        enum type type() override;
        void print(const std::string &indent) override;
      private:
        wrapper* w;
      };

      class capitalizer : public wrapper {
      public:
        ~capitalizer();
        void add(wrapper* w) override;
        std::string get() override;
        enum type type() override;
        void print(const std::string &indent) override;
      private:
        wrapper* w;
      };

      class table_literal : public wrapper {
      public:
        using random_state = xoshiro256starstar::state;
        constexpr static const auto init_func = xoshiro256starstar::init;
        constexpr static const auto rng_func = xoshiro256starstar::rng;
        constexpr static const auto get_value_func = xoshiro256starstar::get_value;
        
        table_literal(const char c, random_state &gen, table_type &table);
        ~table_literal();
        void add(wrapper* w) override;
        std::string get() override;
        enum type type() override;
        void print(const std::string &indent) override;
      private:
        char c;
        random_state &gen;
        table_type &table;
      };

      class word_literal : public wrapper {
      public:
        word_literal(const std::string &str);
        ~word_literal();
        void add(wrapper* w) override;
        std::string get() override;
        enum type type() override;
        void print(const std::string &indent) override;
      private:
        std::string str;
      };

      class empty_literal : public wrapper {
      public:
        ~empty_literal();
        void add(wrapper* w) override;
        std::string get() override;
        enum type type() override;
        void print(const std::string &indent) override;
      };
      
      using random_state = xoshiro256starstar::state;
      constexpr static const auto init_func = xoshiro256starstar::init;
      constexpr static const auto rng_func = xoshiro256starstar::rng;
      constexpr static const auto get_value_func = xoshiro256starstar::get_value;

      template_strings();
      template_strings(const uint64_t &seed);
      ~template_strings();

      void set_table(const char c, const std::vector<std::string> &strings);
      void set_pattern(const std::string &s);
      std::string produce();
      void print();
      
      void clear();
    private:
      //std::mt19937_64 gen;
      random_state state;
      table_type table;
      size_t pattern_size;
      char* pattern_memory;
      wrapper* generator;
    };
  }
}

#endif
