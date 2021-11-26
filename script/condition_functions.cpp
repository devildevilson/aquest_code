#include "condition_functions.h"

#include "context.h"
#include "core/structures_header.h"
#include "core.h"

namespace devils_engine {
  namespace script {
    struct object is_ai::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(!c->is_player());
    }
    
    struct object is_player::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_player());
    }
    
    struct object is_dead::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_dead());
    }
    
    struct object is_alive::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(!c->is_dead());
    }
    
    struct object is_independent::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_independent());
    }
    
    struct object is_vassal::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(!c->is_independent() && c->suzerain.get() == nullptr);
    }
    
    struct object is_courtier::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->suzerain.valid());
    }
    
    struct object is_pleb::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(!c->has_dynasty()); // только наличие династии отделяет дворянина от плебея? скорее всего
    }
    
    struct object is_noble::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->has_dynasty());
    }
    
    struct object is_priest::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(core::character::is_priest(c));
    }
    
    struct object is_male::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_male());
    }
    
    struct object is_female::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(!c->is_male());
    }
    
    struct object is_hero::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(core::character::is_hero(c));
    }
    
    struct object is_prisoner::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_prisoner());
    }
    
    struct object is_married::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_married());
    }
    
    struct object is_sick::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(core::character::is_sick(c));
    }
    
    struct object is_in_war::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      // по идее тот персонаж в войне, который обладает собственным реалмом и в реалме можно найти войну
      assert(false);
      return object(false);
    }
    
    struct object is_in_society::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      // будут ли у меня такие же сообщества как в цк? было бы неплохо
      assert(false);
      return object(false);
    }
    
    struct object is_clan_head::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      // глава мусульманского клана
      assert(false);
      return object(false);
    }
    
    struct object is_religious_head::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      // глава религии, как проверить? мы должны с кем то сравнить, пока не понимаю как сделать
      assert(false);
      return object(false);
    }
    
    struct object is_father_alive::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      auto ptr = c->get_father();
      // отца может и не быть в этом случае он считается мертвым, возможно нужно добавить проверку определен ли отец вообще
      // что с реальным отцом? не понятно кстати как соотносятся эти вещи в цк
      // раскрытия секрета или публичного признания, тайные родители должны стать основными или как то иначе?
      // не понятно на самом деле
      return object(ptr != nullptr && !ptr->is_dead());
    }
    
    struct object is_mother_alive::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      auto ptr = c->get_mother();
      // матери может и не быть в этом случае она считается мертвой, возможно нужно добавить проверку определена ли мать вообще
      // что с реальной матью? не понятно кстати как соотносятся эти вещи в цк
      return object(ptr != nullptr && !ptr->is_dead());
    }
    
    struct object is_establishment_member::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_establishment_member());
    }
    
    struct object is_council_member::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_council_member());
    }
    
    struct object is_tribunal_member::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_tribunal_member());
    }
    
    struct object is_assembly_member::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_assembly_member());
    }
    
    struct object is_clergy_member::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_clergy_member());
    }
    
    struct object is_elector::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_elector());
    }
    
    struct object is_establishment_elector::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_establishment_elector());
    }
    
    struct object is_council_elector::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_council_elector());
    }
    
    struct object is_tribunal_elector::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_tribunal_elector());
    }
    
    struct object is_assembly_elector::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_assembly_elector());
    }
    
    struct object is_clergy_elector::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->is_clergy_elector());
    }
    
    struct object is_among_most_powerful_vassals::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      // как проверить? нужно посчитать рейтинг силы для всех вассалов? из чего он складывается?
      assert(false);
      return object(false);
    }
    
    struct object can_change_religion::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      // что это значит? а я знаю, должен быть религиозный треит, который блочит смену религии
      return object(false);
    }
    
    struct object can_call_crusade::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      // кто может созвать священную войну? только глава религии? скорее всего у самой религии еще должно быть парочку настроек
      // я подумал что возможно имеет смысл обыграть священность в трейте, тип трейт позволяет как то влиять на религию специальным людям
      return object(false);
    }
    
    struct object can_grant_title::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      auto r = c->self.get(); // по крайней мере должен быть собственный реалм?
      // нужно проверить права персонажа в государстве
      return object(false);
    }
    
    struct object can_marry::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(core::character::can_marry(c));
    }
    
    struct object has_dynasty::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->has_dynasty());
    }
    
    struct object has_self_realm::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->self.valid());
    }
    
    struct object has_owner::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(c->family.owner != nullptr);
    }
    
    // как определять какая внешняя функция нужна?
    struct object age::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      return object(double(c->get_age()));
    }
    
    struct object has_culture::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      const auto obj = entity->process(ctx);
      auto ent = obj.get<core::culture*>();
      return object(c->culture == ent);
    }
    
    struct object has_culture_group::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      const auto obj = entity->process(ctx);
      auto ent = obj.get<core::culture_group*>();
      return object(c->culture->group == ent);
    }
    
    struct object has_religion::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      const auto obj = entity->process(ctx);
      auto ent = obj.get<core::religion*>();
      return object(c->religion == ent);
    }
    
    struct object has_religion_group::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      const auto obj = entity->process(ctx);
      auto ent = obj.get<core::religion_group*>();
      return object(c->religion->group == ent);
    }
    
    struct object has_trait::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      const auto obj = entity->process(ctx);
      auto ent = obj.get<core::trait*>();
      return object(c->has_trait(ent));
    }
    
    struct object has_modificator::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      const auto obj = entity->process(ctx);
      auto ent = obj.get<core::modificator*>();
      return object(c->has_modificator(ent));
    }
    
    struct object has_flag::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      const auto obj = entity->process(ctx);
      const auto str = obj.get<std::string_view>();
      const bool val = c->has_flag(str);
      return object(val);
    }
    
    struct object has_title::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      if (!c->self.valid()) return object(false);
      
      const auto obj = entity->process(ctx);
      auto ent = obj.get<core::titulus*>();
      for (auto t = c->self->titles; t != nullptr; t = utils::ring::list_next<utils::list_type::titles>(t, c->self->titles)) {
        if (t == ent) return object(true);
      }
      
      return object(false);
    }
    
#define STAT_FUNC(name) \
    const size_t name::type_index = commands::values::name; \
    const size_t base_##name::type_index = commands::values::base_##name;
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
      return object(double(c->current_stats.get(core::type##_stats::name))); \
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
    return object(double(c->current_hero_stats.get(core::hero_stats::name)));   \
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
    
#define CONDITION_COMMAND_FUNC(name) \
    const size_t name::type_index = commands::values::name; \
    void name::draw(context* ctx) const { \
      const auto obj = process(ctx); \
      /* заполняем draw_data */      \
      draw_data dd(ctx);             \
      dd.function_name = commands::names[commands::name]; \
      dd.value = obj;                \
      /* передаем в функцию */       \
      ctx->draw_function(&dd);       \
    }                                \
    
    CONDITION_COMMANDS_LIST
    
#undef CONDITION_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) \
    const size_t name::type_index = commands::values::name;          \
    name::name(const interface* entity) noexcept : entity(entity) {} \
    void name::draw(context* ctx) const {    \
      const auto obj = process(ctx); \
      const auto ent = entity->process(ctx); \
      /* заполняем draw_data */      \
      draw_data dd(ctx);             \
      dd.function_name = commands::names[commands::name]; \
      dd.value = obj;                \
      dd.arguments[0].first = "entity"; \
      dd.arguments[0].second = ent;  \
      /* передаем в функцию */       \
      ctx->draw_function(&dd);       \
    }                                \
    
    CHARACTER_CONDITION_STRING_CHECK_COMMANDS_LIST
    
#undef CONDITION_COMMAND_FUNC

#define STAT_FUNC(name) \
    void name::draw(context* ctx) const { \
      const auto obj = process(ctx); \
      /* заполняем draw_data */      \
      draw_data dd(ctx);             \
      dd.function_name = commands::names[commands::name]; \
      dd.value = obj;                \
      /* передаем в функцию */       \
      ctx->draw_function(&dd);       \
    }                                \
    \
    void base_##name::draw(context* ctx) const { \
      const auto obj = process(ctx); \
      /* заполняем draw_data */      \
      draw_data dd(ctx);             \
      dd.function_name = commands::names[commands::name]; \
      dd.value = obj;                \
      /* передаем в функцию */       \
      ctx->draw_function(&dd);       \
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
      dd.function_name = commands::names[commands::name]; \
      dd.value = obj;                \
      /* передаем в функцию */       \
      ctx->draw_function(&dd);       \
    }    
    
    UNIQUE_RESOURCES_LIST
    
#undef STAT_FUNC

  }
}
