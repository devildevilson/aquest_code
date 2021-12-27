#include "condition_functions.h"

#include "context.h"
#include "core/structures_header.h"
#include "core.h"

namespace devils_engine {
  namespace script {
#define CONDITION_COMMAND_FUNC(name)                  \
    struct object name::process(context* ctx) const { \
      auto c = ctx->current.get<core::character*>();  \
      return object(c->name());                       \
    }                                                 \
    
    CHARACTER_GET_BOOL_NO_ARGS_COMMANDS_LIST
    
#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name)                  \
    struct object name::process(context* ctx) const { \
      auto c = ctx->current.get<core::character*>();  \
      return object(c->get_##name());                 \
    }                                                 \
    
    CHARACTER_GET_NUMBER_NO_ARGS_COMMANDS_LIST
    
#undef CONDITION_COMMAND_FUNC

#define CONDITION_ARG_COMMAND_FUNC(name, value_type_bit, constness, value_type) \
    struct object name::process(context* ctx) const { \
      auto c = ctx->current.get<core::character*>();  \
      const auto ent_obj = entity->process(ctx);      \
      auto ent = ent_obj.get<constness value_type>(); \
      return object(c->name(ent));                    \
    }                                                 \
    
    CHARACTER_GET_BOOL_ONE_ARG_COMMANDS_LIST
    
#undef CONDITION_ARG_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name)                  \
    struct object name::process(context* ctx) const { \
      auto c = ctx->current.get<utils::handle<core::realm>>();  \
      return object(c->name());                       \
    }                                                 \

    REALM_GET_BOOL_NO_ARGS_COMMANDS_LIST

#undef CONDITION_COMMAND_FUNC

#define CONDITION_ARG_COMMAND_FUNC(name, unused1, unused2, type) \
    struct object name::process(context* ctx) const { \
      auto c = ctx->current.get<utils::handle<core::realm>>();  \
      return object(c->name(data));                   \
    }                                                 \
    
    REALM_GET_BOOL_EXISTED_ARG_COMMANDS_LIST
    
#undef CONDITION_ARG_COMMAND_FUNC
    
#define STAT_FUNC(name) \
    const size_t name::type_index = commands::values::name; \
    const size_t name::context_types;                       \
    const size_t name::expected_types;                      \
    const size_t name::output_type;                         \
    const size_t base_##name::type_index = commands::values::base_##name; \
    const size_t base_##name::context_types;                \
    const size_t base_##name::expected_types;               \
    const size_t base_##name::output_type;                  \
    
#define CHARACTER_PENALTY_STAT_FUNC(name) STAT_FUNC(name##_penalty)
    UNIQUE_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) const size_t name::type_index = commands::values::name;
    UNIQUE_RESOURCES_LIST
#undef STAT_FUNC
    
#define COMMON_STAT_IMPLEMENTATION(name, type)                               \
    struct object name::process(context* ctx) const {                        \
      auto c = ctx->current.get<core::type*>();                              \
      return object(double(c->current_stats.get(core::type##_stats::name))); \
    }                                                                        \
    \
    struct object base_##name::process(context* ctx) const {                 \
      auto c = ctx->current.get<core::type*>();                              \
      return object(double(c->stats.get(core::type##_stats::name)));         \
    }
    
#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, character)
#define CHARACTER_PENALTY_STAT_FUNC(name) STAT_FUNC(name##_penalty)
    CHARACTER_PENALTY_STATS_LIST
    BASE_CHARACTER_STATS_LIST
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, realm)
    BASE_REALM_STATS_LIST
    VASSAL_RESOURCE_INCOME_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, province)
    BASE_PROVINCE_STATS_LIST                  
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, city)
    BASE_CITY_STATS_LIST
    LIEGE_RESOURCE_INCOME_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, army)
    BASE_ARMY_STATS_LIST                      
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, troop)
    BASE_TROOP_STATS_LIST                     
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, hero_troop)
    BASE_HERO_TROOP_STATS_LIST                
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION

#define COMMON_STAT_IMPLEMENTATION(name, type)                                  \
  struct object name::process(context* ctx) const {                             \
    auto c = ctx->current.get<core::type*>();                                   \
    return object(double(c->current_hero_stats.get(core::hero_stats::name)));   \
  }                                                                             \
  \
  struct object base_##name::process(context* ctx) const {                      \
    auto c = ctx->current.get<core::type*>();                                   \
    return object(double(c->hero_stats.get(core::hero_stats::name)));           \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, character)
    BASE_HERO_STATS_LIST                      
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION

#define STAT_TYPE_CASE(name, type_name)                          \
  case object::type::type_name: {                                \
    auto ptr = ctx->current.get<core::type_name*>();             \
    val = ptr->current_stats.get(core::type_name##_stats::name); \
    break;                                                       \
  }
  
#define STAT_TYPE_CASE_BASE(name, type_name)                     \
  case object::type::type_name: {                                \
    auto ptr = ctx->current.get<core::type_name*>();             \
    val = ptr->stats.get(core::type_name##_stats::name);         \
    break;                                                       \
  }
  
#define STAT_TYPE_CASE2(name, type_name)                       \
  case object::type::type_name: {                              \
    auto ptr = ctx->current.get<core::type_name*>();           \
    val = ptr->current_hero_stats.get(core::hero_stats::name); \
    break;                                                     \
  }
  
#define STAT_TYPE_CASE2_BASE(name, type_name)                  \
  case object::type::type_name: {                              \
    auto ptr = ctx->current.get<core::type_name*>();           \
    val = ptr->hero_stats.get(core::hero_stats::name);         \
    break;                                                     \
  }

#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, realm)                   \
      STAT_TYPE_CASE(name, hero_troop)              \
      STAT_TYPE_CASE2(name, character)              \
      default: throw std::runtime_error("Function '" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }                                                 \
  \
  struct object base_##name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE_BASE(name, realm)              \
      STAT_TYPE_CASE_BASE(name, hero_troop)         \
      STAT_TYPE_CASE2_BASE(name, character)         \
      default: throw std::runtime_error("Function 'base_" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }
  
#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    HERO_FACTOR_STATS_LIST                    
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, city)                    \
      STAT_TYPE_CASE(name, province)                \
      STAT_TYPE_CASE(name, army)                    \
      STAT_TYPE_CASE(name, troop)                   \
      default: throw std::runtime_error("Function '" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }                                                 \
  \
  struct object base_##name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE_BASE(name, city)               \
      STAT_TYPE_CASE_BASE(name, province)           \
      STAT_TYPE_CASE_BASE(name, army)               \
      STAT_TYPE_CASE_BASE(name, troop)              \
      default: throw std::runtime_error("Function 'base_" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    TROOP_FACTOR_STATS_LIST                   
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, city)                    \
      STAT_TYPE_CASE(name, province)                \
      STAT_TYPE_CASE(name, character)               \
      default: throw std::runtime_error("Function '" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }                                                 \
  \
  struct object base_##name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE_BASE(name, city)               \
      STAT_TYPE_CASE_BASE(name, province)           \
      STAT_TYPE_CASE_BASE(name, character)          \
      default: throw std::runtime_error("Function 'base_" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST 
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, city)                    \
      STAT_TYPE_CASE(name, province)                \
      STAT_TYPE_CASE(name, realm)                   \
      default: throw std::runtime_error("Function '" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }                                                 \
  \
  struct object base_##name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE_BASE(name, city)               \
      STAT_TYPE_CASE_BASE(name, province)           \
      STAT_TYPE_CASE_BASE(name, realm)              \
      default: throw std::runtime_error("Function 'base_" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }                                                 \

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    SHARED_PROVINCE_CITY_REALM_STATS_LIST     
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, city)                    \
      STAT_TYPE_CASE(name, province)                \
      default: throw std::runtime_error("Function '" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }                                                 \
  \
  struct object base_##name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE_BASE(name, city)               \
      STAT_TYPE_CASE_BASE(name, province)           \
      default: throw std::runtime_error("Function 'base_" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    SHARED_PROVINCE_CITY_STATS_LIST           
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, hero_troop)              \
      STAT_TYPE_CASE(name, army)                    \
      default: throw std::runtime_error("Function '" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }                                                 \
  \
  struct object base_##name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE_BASE(name, hero_troop)         \
      STAT_TYPE_CASE_BASE(name, army)               \
      default: throw std::runtime_error("Function 'base_" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    SHARED_HERO_TROOP_ARMY_STATS_LIST         
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, troop)                   \
      STAT_TYPE_CASE2(name, character)              \
      default: throw std::runtime_error("Function '" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }                                                 \
  \
  struct object base_##name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE_BASE(name, troop)              \
      STAT_TYPE_CASE2_BASE(name, character)         \
      default: throw std::runtime_error("Function 'base_" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    SHARED_TROOP_HERO_STATS_LIST              
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, city)                    \
      STAT_TYPE_CASE(name, province)                \
      STAT_TYPE_CASE(name, realm)                   \
      STAT_TYPE_CASE(name, character)               \
      default: throw std::runtime_error("Function '" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }                                                 \
  \
  struct object base_##name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE_BASE(name, city)               \
      STAT_TYPE_CASE_BASE(name, province)           \
      STAT_TYPE_CASE_BASE(name, realm)              \
      STAT_TYPE_CASE_BASE(name, character)          \
      default: throw std::runtime_error("Function 'base_" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    RESOURCE_INCOME_STATS_LIST
    RESOURCE_INCOME_FACTOR_STATS_LIST
    BUILD_FACTOR_STATS_LIST
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, army)                    \
      STAT_TYPE_CASE(name, realm)                   \
      STAT_TYPE_CASE(name, character)               \
      default: throw std::runtime_error("Function '" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }                                                 \
  \
  struct object base_##name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE_BASE(name, army)               \
      STAT_TYPE_CASE_BASE(name, realm)              \
      STAT_TYPE_CASE_BASE(name, character)          \
      default: throw std::runtime_error("Function 'base_" #name "' receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }  

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    ARMY_FACTOR_STATS_LIST                    
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION

#undef STAT_TYPE_CASE
#undef STAT_TYPE_CASE2

#define STAT_TYPE_CASE(name, type_name)                          \
  case object::type::type_name: {                                \
    auto ptr = ctx->current.get<core::type_name*>();             \
    val = ptr->resources.get(core::type_name##_resources::name); \
    break;                                                       \
  }

#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object name::process(context* ctx) const { \
    double val = 0.0;                               \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, realm)                   \
      STAT_TYPE_CASE(name, character)               \
      default: throw std::runtime_error("Function " #name " receive object with wrong type"); \
    }                                               \
    return object(val);                             \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    RESOURCE_STATS_LIST
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#undef STAT_TYPE_CASE

#define COMMON_STAT_IMPLEMENTATION(name, type)                     \
  struct object name::process(context* ctx) const {                \
    auto c = ctx->current.get<core::type*>();                      \
    return object(double(c->resources.get(core::type##_resources::name))); \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, city)
    CITY_RESOURCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, army)
    ARMY_RESOURCE_STATS_LIST
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
    
#define COMMANDS_COMMON_PART(name) \
    const size_t name::type_index = commands::values::name; \
    const size_t name::context_types;     \
    const size_t name::expected_types;    \
    const size_t name::output_type;       \
    
#define CONDITION_COMMAND_FUNC(name)      \
    COMMANDS_COMMON_PART(name)            \
    void name::draw(context* ctx) const { \
      const auto obj = process(ctx); \
      /* заполняем draw_data */      \
      draw_data dd(ctx);             \
      dd.function_name = commands::names[type_index]; \
      dd.value = obj;                \
      dd.original = ctx->rvalue;     \
      /* передаем в функцию */       \
      ctx->draw(&dd);                \
    }                                \
    
    CHARACTER_GET_BOOL_NO_ARGS_COMMANDS_LIST
    REALM_GET_BOOL_NO_ARGS_COMMANDS_LIST
    
#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) \
    COMMANDS_COMMON_PART(name)            \
    void name::draw(context* ctx) const { \
      const auto obj = process(ctx); \
      /* заполняем draw_data */      \
      draw_data dd(ctx);             \
      dd.function_name = commands::names[type_index]; \
      dd.value = obj;                \
      dd.original = ctx->rvalue;     \
      /* передаем в функцию */       \
      ctx->draw(&dd);                \
    }                                \
    
    CHARACTER_GET_NUMBER_NO_ARGS_COMMANDS_LIST
    
#undef CONDITION_COMMAND_FUNC

#define CONDITION_ARG_COMMAND_FUNC(name, value_type_bit, constness, value_type) \
    COMMANDS_COMMON_PART(name)               \
    name::name(const interface* entity) noexcept : entity(entity) {} \
    name::~name() noexcept { entity->~interface(); }                 \
    void name::draw(context* ctx) const {    \
      const auto obj = process(ctx);         \
      const auto ent = entity->process(ctx); \
      /* заполняем draw_data */         \
      draw_data dd(ctx);                \
      dd.function_name = commands::names[type_index]; \
      dd.value = obj;                   \
      dd.arguments[0].first = "entity"; \
      dd.arguments[0].second = ent;     \
      /* передаем в функцию */          \
      ctx->draw(&dd);                   \
    }                                   \
    
    CHARACTER_GET_BOOL_ONE_ARG_COMMANDS_LIST
    
#undef CONDITION_ARG_COMMAND_FUNC

#define CONDITION_ARG_COMMAND_FUNC(name, unused1, unused2, type) \
    COMMANDS_COMMON_PART(name)               \
    name::name(const type &data) noexcept : data(data) {} \
    void name::draw(context* ctx) const {    \
      const auto obj = process(ctx);         \
      /* заполняем draw_data */         \
      draw_data dd(ctx);                \
      dd.function_name = commands::names[type_index]; \
      dd.value = obj;                   \
      dd.arguments[0].first = "data";   \
      dd.arguments[0].second = object(double(data));  \
      /* передаем в функцию */          \
      ctx->draw(&dd);                   \
    }                                   \
    
    REALM_GET_BOOL_EXISTED_ARG_COMMANDS_LIST
    
#undef CONDITION_ARG_COMMAND_FUNC

#define STAT_FUNC(name) \
    void name::draw(context* ctx) const { \
      const auto obj = process(ctx); \
      /* заполняем draw_data */      \
      draw_data dd(ctx);             \
      dd.function_name = commands::names[type_index]; \
      dd.value = ctx->rvalue;        \
      dd.original = obj;             \
      /* передаем в функцию */       \
      ctx->draw(&dd);                \
    }                                \
    \
    void base_##name::draw(context* ctx) const { \
      const auto obj = process(ctx); \
      /* заполняем draw_data */      \
      draw_data dd(ctx);             \
      dd.function_name = commands::names[type_index]; \
      dd.value = ctx->rvalue;        \
      dd.original = obj;             \
      /* передаем в функцию */       \
      ctx->draw(&dd);                \
    }
    
#define CHARACTER_PENALTY_STAT_FUNC(name) STAT_FUNC(name##_penalty)
    
    UNIQUE_STATS_LIST
    
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) \
    void name::draw(context* ctx) const { \
      const auto obj = process(ctx); \
      /* заполняем draw_data */      \
      draw_data dd(ctx);             \
      dd.function_name = commands::names[type_index]; \
      dd.value = ctx->rvalue;        \
      dd.original = obj;             \
      /* передаем в функцию */       \
      ctx->draw(&dd);                \
    }    
    
    UNIQUE_RESOURCES_LIST
    
#undef STAT_FUNC

  }
}
