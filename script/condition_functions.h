#ifndef DEVILS_ENGINE_SCRIPT_CONDITION_FUNCTIONS_H
#define DEVILS_ENGINE_SCRIPT_CONDITION_FUNCTIONS_H

#include <cstdint>
#include "condition_commands_macro.h"
#include "core/stats.h"
#include "interface.h"
#include "object.h"

namespace devils_engine {
  namespace script {
    struct context;
    
#define CONDITION_COMMAND_FUNC(name) \
    class name final : public interface {                               \
    public:                                                             \
      static const size_t type_index;                                   \
      static const size_t context_types = object::type_bit::character;  \
      static const size_t expected_types = 0;                           \
      static const size_t output_type = object::type_bit::boolean;      \
      struct object process(context* ctx) const override;               \
      void draw(context* ctx) const override;                           \
    };                                                                  \
    
    CONDITION_COMMANDS_LIST 
    
#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) \
    class name final : public interface {                               \
    public:                                                             \
      static const size_t type_index;                                   \
      static const size_t context_types = object::type_bit::character;  \
      static const size_t expected_types = 0;                           \
      static const size_t output_type = object::type_bit::boolean;      \
      name(const interface* entity) noexcept;                           \
      struct object process(context* ctx) const override;               \
      void draw(context* ctx) const override;                           \
    private:                                                            \
      const interface* entity;                                          \
    };                                                                  \
    
    CHARACTER_CONDITION_STRING_CHECK_COMMANDS_LIST
    
#undef CONDITION_COMMAND_FUNC

#define COMMON_STAT_FUNC(name, expected_context_bits, output_type_bits) \
    class name final : public interface {                        \
    public:                                                      \
      static const size_t type_index;                            \
      static const size_t context_types = expected_context_bits; \
      static const size_t expected_types = 0;                    \
      static const size_t output_type = output_type_bits;        \
      struct object process(context* ctx) const override;        \
      void draw(context* ctx) const override;                    \
    };                                                           \
    \
    class base_##name final : public interface {                        \
    public:                                                      \
      static const size_t type_index;                            \
      static const size_t context_types = expected_context_bits; \
      static const size_t expected_types = 0;                    \
      static const size_t output_type = output_type_bits;        \
      struct object process(context* ctx) const override;        \
      void draw(context* ctx) const override;                    \
    };                                                           \
    
#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::character, object::type_bit::number)
#define CHARACTER_PENALTY_STAT_FUNC(name) STAT_FUNC(name##_penalty)
    
    CHARACTER_PENALTY_STATS_LIST
    BASE_CHARACTER_STATS_LIST
    
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::realm, object::type_bit::number)
    BASE_REALM_STATS_LIST                     
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::province, object::type_bit::number)
    BASE_PROVINCE_STATS_LIST                  
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city, object::type_bit::number)
    BASE_CITY_STATS_LIST                      
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::army, object::type_bit::number)
    BASE_ARMY_STATS_LIST                      
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::troop, object::type_bit::number)
    BASE_TROOP_STATS_LIST                     
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::character, object::type_bit::number)
    BASE_HERO_STATS_LIST                      
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::hero_troop, object::type_bit::number)
    BASE_HERO_TROOP_STATS_LIST                
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::realm | object::type_bit::hero_troop | object::type_bit::character, object::type_bit::number)
    HERO_FACTOR_STATS_LIST                    
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province | object::type_bit::army | object::type_bit::troop, object::type_bit::number)
    TROOP_FACTOR_STATS_LIST                   
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province | object::type_bit::character, object::type_bit::number)
    SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST 
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city| object::type_bit::province | object::type_bit::realm, object::type_bit::number)
    SHARED_PROVINCE_CITY_REALM_STATS_LIST     
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province, object::type_bit::number)
    SHARED_PROVINCE_CITY_STATS_LIST           
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::hero_troop | object::type_bit::army, object::type_bit::number)
    SHARED_HERO_TROOP_ARMY_STATS_LIST         
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::troop | object::type_bit::character, object::type_bit::number)
    SHARED_TROOP_HERO_STATS_LIST              
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province | object::type_bit::realm | object::type_bit::character, object::type_bit::number)
    RESOURCE_INCOME_STATS_LIST
#undef STAT_FUNC
    
#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province | object::type_bit::realm | object::type_bit::character, object::type_bit::number)
    RESOURCE_INCOME_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city | object::type_bit::province | object::type_bit::realm | object::type_bit::character, object::type_bit::number)
    BUILD_FACTOR_STATS_LIST                   
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::character | object::type_bit::realm | object::type_bit::army, object::type_bit::number)
    ARMY_FACTOR_STATS_LIST                    
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city, object::type_bit::number)
    LIEGE_RESOURCE_INCOME_STATS_LIST          
#undef STAT_FUNC
    
#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::realm, object::type_bit::number)
    VASSAL_RESOURCE_INCOME_FACTOR_STATS_LIST  
#undef STAT_FUNC

#undef COMMON_STAT_FUNC

#define COMMON_STAT_FUNC(name, expected_context_bits, output_type_bits) \
    class name final : public interface {                        \
    public:                                                      \
      static const size_t type_index;                            \
      static const size_t context_types = expected_context_bits; \
      static const size_t expected_types = 0;                    \
      static const size_t output_type = output_type_bits;        \
      struct object process(context* ctx) const override;        \
      void draw(context* ctx) const override;                    \
    };                                                           \

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::character | object::type_bit::realm, object::type_bit::number)
    RESOURCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::city, object::type_bit::number)
    CITY_RESOURCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::army, object::type_bit::number)
    ARMY_RESOURCE_STATS_LIST
#undef STAT_FUNC

#undef COMMON_STAT_FUNC
  }
}

#endif
