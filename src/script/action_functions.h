#ifndef DEVILS_ENGINE_SCRIPT_ACTION_FUNCTIONS2_H
#define DEVILS_ENGINE_SCRIPT_ACTION_FUNCTIONS2_H

#include "interface.h"
#include "object.h"
#include "action_commands_macro.h"
#include "core/stats.h"
#include "context.h"
#include "core.h"

// может быть имеет смысл все эффекты засунуть в отдельное перечисление?
// чтобы случайно не создать их где нибудь не в том месте, ну хотя мы можем использовать проверку скрипт_тайп

#define ADD_FLAG_TYPES_LIST \
  ADD_FLAG_TYPE_FUNC(character) \
  ADD_FLAG_TYPE_FUNC(realm) \
  ADD_FLAG_TYPE_FUNC(province) \
  ADD_FLAG_TYPE_FUNC(city) \
  ADD_FLAG_TYPE_FUNC(titulus) \
  ADD_FLAG_TYPE_FUNC(war) \
  ADD_FLAG_TYPE_FUNC(army) \
  ADD_FLAG_TYPE_FUNC(hero_troop) \
  ADD_FLAG_TYPE_FUNC(culture) \
  ADD_FLAG_TYPE_FUNC(religion) \

namespace devils_engine {
  namespace script {
    struct context;
    
    template <typename Th>
    class add_flag final : public interface {
    public:
      static const size_t type_index;
      add_flag(const interface* flag, const interface* time) noexcept;
      ~add_flag() noexcept;
      struct object process(context * ctx) const override;
      void draw(context * ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* flag;
      const interface* time;
    };
    
    class add_hook final : public interface {
    public:
      static const size_t type_index;
//       static const size_t context_types = object::type_bit::character;
//       static const size_t output_type = object::type_bit::invalid;
      add_hook(const interface* hook) noexcept;
      ~add_hook() noexcept;
      struct object process(context * ctx) const override;
      void draw(context * ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* hook;
    };
    
    class add_trait final : public interface {
    public:
      static const size_t type_index;
//       static const size_t context_types = object::type_bit::character;
//       static const size_t output_type = object::type_bit::invalid;
      add_trait(const interface* entity) noexcept;
      ~add_trait() noexcept;
      struct object process(context * ctx) const override;
      void draw(context * ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* entity;
    };
    
    class marry final : public interface {
    public:
      static const size_t type_index;
//       static const size_t context_types = object::type_bit::character;
//       static const size_t output_type = object::type_bit::invalid;
      marry(const interface* character) noexcept;
      ~marry() noexcept;
      struct object process(context * ctx) const override;
      void draw(context * ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* character;
    };
    
    class divorce final : public interface {
    public:
      static const size_t type_index;
//       static const size_t context_types = object::type_bit::character;
//       static const size_t output_type = object::type_bit::invalid;
      struct object process(context * ctx) const override;
      void draw(context * ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    };
    
    // в цк2 просто war, название из цк3
    // как передать в итеракцию титулы?
    class start_war final : public interface {
    public:
      static const size_t type_index;
//       static const size_t context_types = object::type_bit::character;
//       static const size_t output_type = object::type_bit::invalid;
      // реалмы теперь видимо не нужны
      start_war(const interface* target, const interface* cb, const interface* claimant, const interface* titles, const interface* attacker_realm, const interface* defender_realm) noexcept;
      ~start_war() noexcept;
      struct object process(context * ctx) const override;
      void draw(context * ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* target_script;
      const interface* cb_script;
      const interface* claimant_script;
      // некоторое количество целевых титулов, их может быть несколько? как задать? у меня же есть interface::next
      // либо тут можно указать лист
      const interface* titles_script;
      const interface* attacker_realm;
      const interface* defender_realm;
      // в цк2 еще можно было задать фракцию, модификатор к известности, заставить вступить в войну челиков из интриги
    };
    
    class end_war final : public interface {
    public:
      static const size_t type_index;
//       static const size_t context_types = object::type_bit::war;
//       static const size_t output_type = object::type_bit::invalid;
      end_war(const size_t &type) noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      size_t type; // тип окончания войны (победа атакующих, победа защищающихся и белый мир)
    };
    
    class add_attacker final : public interface {
    public:
      static const size_t type_index;
//       static const size_t context_types = object::type_bit::war;
//       static const size_t expected_types = object::type_bit::character;
//       static const size_t output_type = object::type_bit::invalid;
      add_attacker(const interface* character) noexcept;
      ~add_attacker() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* character;
    };
    
    class add_defender final : public interface {
    public:
      static const size_t type_index;
//       static const size_t context_types = object::type_bit::war;
//       static const size_t expected_types = object::type_bit::character;
//       static const size_t output_type = object::type_bit::invalid;
      add_defender(const interface* character) noexcept;
      ~add_defender() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* character;
    };
    
    class remove_participant final : public interface {
    public:
      static const size_t type_index;
//       static const size_t context_types = object::type_bit::war;
//       static const size_t expected_types = object::type_bit::character;
//       static const size_t output_type = object::type_bit::invalid;
      remove_participant(const interface* character) noexcept;
      ~remove_participant() noexcept;
      struct object process(context* ctx) const override;
      void draw(context* ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* character;
    };
    
    // set_called_to? set_casus_belli?
    
    class imprison final : public interface {
    public:
      static const size_t type_index;
//       static const size_t context_types = object::type_bit::character;
//       static const size_t expected_types = object::type_bit::character;
//       static const size_t output_type = object::type_bit::invalid;
      imprison(const interface* target, const interface* realm) noexcept;
      ~imprison() noexcept;
      struct object process(context * ctx) const override;
      void draw(context * ctx) const override;
      size_t get_type_id() const override;
      std::string_view get_name() const override;
    private:
      const interface* target;
      const interface* realm;
    };
    
    
    template <typename Th>
    const size_t add_flag<Th>::type_index = commands::values::add_flag;
    template <typename Th>
    add_flag<Th>::add_flag(const interface* flag, const interface* time) noexcept : flag(flag), time(time) {}
    template <typename Th>
    add_flag<Th>::~add_flag() noexcept { flag->~interface(); if (time != nullptr) time->~interface(); }
    template <typename Th>
    struct object add_flag<Th>::process(context* ctx) const {
      const auto flag_obj = flag->process(ctx);
      const auto flag_str = flag_obj.get<std::string_view>();
      size_t time_val = SIZE_MAX;
      if (time != nullptr) {
        const auto time_obj = time->process(ctx);
        time_val = time_obj.ignore() ? time_val : time_obj.get<double>();
      }

      auto cur = ctx->current.get<Th>();
      cur->add_flag(flag_str, time_val);
      
      return object();
    }
    
    template <typename Th>
    void add_flag<Th>::draw(context* ctx) const {
      const auto flag_obj = flag->process(ctx);
      
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      dd.set_arg(0, "flag", flag_obj);
      if (time != nullptr) {
        const auto time_obj = time->process(ctx);
        dd.set_arg(1, "time", time_obj);
      }
      
      ctx->draw(&dd);
    }
    
    template <typename Th>
    size_t add_flag<Th>::get_type_id() const { return type_id<Th>(); }
    template <typename Th>
    std::string_view add_flag<Th>::get_name() const { return commands::names[type_index]; }
    
// #define COMMON_STAT_FUNC(name, expected_context_bits, expected_type_bits) 
//     class add_##name final : public interface {                  
//     public:                                                      
//       static const size_t type_index;                            
//       /*static const size_t context_types = expected_context_bits;*/ 
//       /*static const size_t expected_types = expected_type_bits;*/   
//       /*static const size_t output_type = object::type_bit::invalid;*/ 
//       add_##name(const interface* value) noexcept;               
//       ~add_##name() noexcept;                                    
//       struct object process(context* ctx) const override;        
//       void draw(context* ctx) const override;                    
//     private:                                                     
//       const interface* value;                                    
//     };                                                           
//     
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::character, object::type_bit::number)
// #define CHARACTER_PENALTY_STAT_FUNC(name) STAT_FUNC(name##_penalty)
//     
//     CHARACTER_PENALTY_STATS_LIST
//     BASE_CHARACTER_STATS_LIST
//     
// #undef STAT_FUNC
// #undef CHARACTER_PENALTY_STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::realm, object::type_bit::number)
//     BASE_REALM_STATS_LIST                     
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::province, object::type_bit::number)
//     BASE_PROVINCE_STATS_LIST                  
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city, object::type_bit::number)
//     BASE_CITY_STATS_LIST                      
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::army, object::type_bit::number)
//     BASE_ARMY_STATS_LIST                      
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::troop, object::type_bit::number)
//     BASE_TROOP_STATS_LIST                     
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::character, object::type_bit::number)
//     BASE_HERO_STATS_LIST                      
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::hero_troop, object::type_bit::number)
//     BASE_HERO_TROOP_STATS_LIST                
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::realm | object::type_bit::hero_troop | object::type_bit::character, object::type_bit::number)
//     HERO_FACTOR_STATS_LIST                    
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province | object::type_bit::army | object::type_bit::troop, object::type_bit::number)
//     TROOP_FACTOR_STATS_LIST                   
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province | object::type_bit::character, object::type_bit::number)
//     SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST 
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city| object::type_bit::province | object::type_bit::realm, object::type_bit::number)
//     SHARED_PROVINCE_CITY_REALM_STATS_LIST     
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province, object::type_bit::number)
//     SHARED_PROVINCE_CITY_STATS_LIST           
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::hero_troop | object::type_bit::army, object::type_bit::number)
//     SHARED_HERO_TROOP_ARMY_STATS_LIST         
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::troop | object::type_bit::character, object::type_bit::number)
//     SHARED_TROOP_HERO_STATS_LIST              
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province | object::type_bit::realm | object::type_bit::character, object::type_bit::number)
//     RESOURCE_INCOME_STATS_LIST
// #undef STAT_FUNC
//     
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province | object::type_bit::realm | object::type_bit::character, object::type_bit::number)
//     RESOURCE_INCOME_FACTOR_STATS_LIST
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province | object::type_bit::realm | object::type_bit::character, object::type_bit::number)
//     BUILD_FACTOR_STATS_LIST                   
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::character | object::type_bit::realm | object::type_bit::army, object::type_bit::number)
//     ARMY_FACTOR_STATS_LIST                    
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city, object::type_bit::number)
//     LIEGE_RESOURCE_INCOME_STATS_LIST          
// #undef STAT_FUNC
//     
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::realm, object::type_bit::number)
//     VASSAL_RESOURCE_INCOME_FACTOR_STATS_LIST  
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::character | object::type_bit::realm, object::type_bit::number)
//     RESOURCE_STATS_LIST
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city, object::type_bit::number)
//     CITY_RESOURCE_STATS_LIST
// #undef STAT_FUNC
// 
// #define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::army, object::type_bit::number)
//     ARMY_RESOURCE_STATS_LIST
// #undef STAT_FUNC
// 
// #undef COMMON_STAT_FUNC
  }
}

#endif
