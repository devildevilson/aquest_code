#include "action_functions.h"

#include "core.h"
#include "context.h"
#include "core/structures_header.h"
#include "core/declare_structures_table.h"
#include "utils/magic_enum_header.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "core/context.h"

// все экшоны вполне можно засунуть в массив отложенного выполнения
// для этого достаточно собрать данные примерно как на отрисовку
// и рядом расположить функцию
// с чем это может помочь? во первых с возможными последствиями позднего кода "root.base_military"
// во вторых собрать эффекты можно в мультипотоке, применять их в таком виде гораздо проще
// но это означает выделение памяти каждый запуск выделение памяти только для эффектов

namespace devils_engine {
  namespace script {
#define ADD_FLAG_CASE(type_id) \
    case object::type::type_id: { \
      auto ptr = ctx->current.get<core::type_id*>(); \
      ptr->add_flag(flag_str, time_val); \
      break; \
    }
    
    const size_t add_flag::type_index = commands::values::add_flag;
    add_flag::add_flag(const interface* flag, const interface* time) noexcept : flag(flag), time(time) {}
    add_flag::~add_flag() noexcept { flag->~interface(); if (time != nullptr) time->~interface(); }
    struct object add_flag::process(context* ctx) const {
      const auto flag_obj = flag->process(ctx);
      const auto flag_str = flag_obj.get<std::string_view>();
      size_t time_val = SIZE_MAX;
      if (time != nullptr) {
        const auto time_obj = time->process(ctx);
        time_val = time_obj.ignore() ? time_val : time_obj.get<double>();
      }

      switch (ctx->current.get_type()) {
#define ADD_FLAG_TYPE_FUNC(type_name) ADD_FLAG_CASE(type_name)
      ADD_FLAG_TYPES_LIST
#undef ADD_FLAG_TYPE_FUNC
        default: throw std::runtime_error("Function 'add_flag' recieve invalid type " + std::string(core::structure_data::names[ctx->current.type]));
      }
      
      return object();
    }
    
    void add_flag::draw(context* ctx) const {
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
    
    const size_t add_hook::type_index = commands::values::add_hook;
    add_hook::add_hook(const interface* hook) noexcept : hook(hook) {}
    add_hook::~add_hook() noexcept { hook->~interface(); }
    struct object add_hook::process(context* ctx) const {
      auto c = ctx->current.get<core::character*>();
      const auto hook_obj = hook->process(ctx);
      // тут вообще то говоря должен быть: сам хук, тип хука (сильный слабый, или сам по себе хук должен это определять?), таргет
      return object();
    }
    
    void add_hook::draw(context*) const {
      
    }
    
    const size_t add_trait::type_index = commands::values::add_trait;
    add_trait::add_trait(const interface* entity) noexcept : entity(entity) {}
    add_trait::~add_trait() noexcept { entity->~interface(); }
    struct object add_trait::process(context * ctx) const {
      auto c = ctx->current.get<core::character*>();
      const auto ent_obj = entity->process(ctx);
      const auto trait = ent_obj.get<core::trait*>();
      c->add_trait(trait);
      return object();
    }
    
    void add_trait::draw(context * ctx) const {
      const auto ent_obj = entity->process(ctx);
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      dd.value = ent_obj;
      ctx->draw(&dd);
    }
    
    const size_t marry::type_index = commands::values::marry;
    marry::marry(const interface* character) noexcept : character(character) {}
    marry::~marry() noexcept { character->~interface(); }
    struct object marry::process(context * ctx) const {
      const auto char_obj = character->process(ctx);
      auto c = ctx->current.get<core::character*>();
      auto consort = char_obj.get<core::character*>();
      assert(c->family.consort == nullptr);
      assert(consort->family.consort == nullptr);
      
      c->family.consort = consort;
      consort->family.consort = c;
      
      // должны вызваться функции on_action, и там скорее всего скрипт поймет куда отправить освободившегося персонажа
      
      return object();
    }
    
    void marry::draw(context * ctx) const {
      const auto char_obj = character->process(ctx);
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      dd.value = char_obj;
      ctx->draw(&dd);
    }
    
    const size_t divorce::type_index = commands::values::divorce;
    struct object divorce::process(context * ctx) const {
      auto c = ctx->current.get<core::character*>();
      if (c->family.consort != nullptr) { // нужно ли вылетать если не с кем разводиться
        auto consort = c->family.consort;
        assert(c == consort->family.consort);
        c->family.consort = nullptr;
        consort->family.consort = nullptr;
        // должны вызваться функции on_action
      }
      
      return object();
    }
    
    void divorce::draw(context * ctx) const {
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      ctx->draw(&dd);
    }
    
    const size_t start_war::type_index = commands::values::start_war;
    start_war::start_war(const interface* target, const interface* cb, const interface* claimant, const interface* titles, const interface* attacker_realm, const interface* defender_realm) noexcept : 
      target_script(target), 
      cb_script(cb), 
      claimant_script(claimant), 
      titles_script(titles),
      attacker_realm(attacker_realm),
      defender_realm(defender_realm)
    {}
    start_war::~start_war() noexcept {
      target_script->~interface();
      cb_script->~interface();
      claimant_script->~interface();
      for (auto t = titles_script; t != nullptr; t = t->next) { t->~interface(); }
      if (attacker_realm != nullptr) attacker_realm->~interface();
      if (defender_realm != nullptr) defender_realm->~interface();
    }
    
    struct object start_war::process(context* ctx) const {
      const auto target_obj = target_script->process(ctx);
      const auto cb_obj = cb_script->process(ctx);
      const auto claimant_obj = claimant_script->process(ctx);
      
      auto current = ctx->current.get<core::character*>();
      auto target = target_obj.get<core::character*>(); // кто таргет? по идее мы (персонаж) объявляем войну персонажу
      auto cb = cb_obj.get<core::casus_belli*>();
      auto claimant = claimant_obj.get<core::character*>();
      
      // на какой реалм мы нападем? мы нападаем на госреалм если персонаж во главе государства
      // титулы должны быть либо у него, либо у госреалма
      // какой реалм нападает? можем ли мы объявить войну от реалма совета? вообще возможно
      // в таком случае должен быть явно указан скрипт чтобы получить нужный реалм
      
      // все таки скорее всего нападать нужно на персонажа, а тот уже разберется чем пользоваться
      // да и вообще вся дипломатия должна быть в персонаже, к сожалению в реалме делать все взаимодействия неудобно
      
      auto realm_attacker = current->self;
      if (current->realms[core::character::establishment].valid() && 
          current->realms[core::character::establishment]->is_state_independent_power() &&
          current->realms[core::character::establishment]->leader == current) {
        realm_attacker = current->realms[core::character::establishment];
      }
      
      if (attacker_realm != nullptr) {
        const auto obj = attacker_realm->process(ctx);
        realm_attacker = obj.get<utils::handle<core::realm>>();
      }
      
      auto realm_defender = target->self;
      if (target->realms[core::character::establishment].valid() && 
          target->realms[core::character::establishment]->is_state_independent_power() &&
          target->realms[core::character::establishment]->leader == target) {
        realm_defender = target->realms[core::character::establishment];
      }
      
      if (defender_realm != nullptr) {
        change_scope cs(ctx, target_obj, ctx->current);
        const auto obj = defender_realm->process(ctx);
        realm_defender = obj.get<utils::handle<core::realm>>();
      }
      
      // возможно потребуется больше проверок? возможно потребуется проверка ведут ли эти два персонажа войну друг с другом
      // нужно проверить не отвалился ли за это время хендл
      if (const auto itr = realm_attacker->relations.find(realm_defender); 
          itr != realm_attacker->relations.end() && (itr->second.relation_type == core::diplomacy::war_attacker || itr->second.relation_type == core::diplomacy::war_defender)) {
        throw std::runtime_error("War between this realms is already exists");
      }
      
      if (const auto itr = realm_defender->relations.find(realm_attacker); 
          itr != realm_defender->relations.end() && (itr->second.relation_type == core::diplomacy::war_attacker || itr->second.relation_type == core::diplomacy::war_defender)) {
        throw std::runtime_error("War between this realms is already exists");
      }
      
      auto core_ctx = global::get<systems::map_t>()->core_context;
      auto war_h = core_ctx->create_war();
      
      war_h->cb = cb;
      war_h->war_opener = current;
//       war_h->opener_realm = realm_attacker;
      war_h->target_character = target;
//       war_h->target_realm = realm_defender;
      war_h->claimant = claimant;
      for (auto t = titles_script; t != nullptr; t = t->next) {
        const auto title_obj = t->process(ctx);
        auto title = title_obj.get<core::titulus*>();
        war_h->target_titles.push_back(title);
      }
      
      core::realm::relation r;
      r.relation_type = core::diplomacy::war_attacker;
      r.war = war_h;
      realm_attacker->relations.emplace(realm_defender, r);
      r.relation_type = core::diplomacy::war_defender;
      realm_defender->relations.emplace(realm_attacker, r);
      
      // стартуем on_action
      
      return object();
    }
    
    // при изменении process изменить и это !!!!!!!!
    void start_war::draw(context* ctx) const {
      const auto target_obj = target_script->process(ctx);
      const auto cb_obj = cb_script->process(ctx);
      const auto claimant_obj = claimant_script->process(ctx);
      
      auto current = ctx->current.get<core::character*>();
      auto target = target_obj.get<core::character*>();
      
      auto realm_attacker = current->self;
      if (current->realms[core::character::establishment].valid() && 
          current->realms[core::character::establishment]->is_state_independent_power() &&
          current->realms[core::character::establishment]->leader == current) {
        realm_attacker = current->realms[core::character::establishment];
      }
      
      if (attacker_realm != nullptr) {
        const auto obj = attacker_realm->process(ctx);
        realm_attacker = obj.get<utils::handle<core::realm>>();
      }
      
      auto realm_defender = target->self;
      if (target->realms[core::character::establishment].valid() && 
          target->realms[core::character::establishment]->is_state_independent_power() &&
          target->realms[core::character::establishment]->leader == target) {
        realm_defender = target->realms[core::character::establishment];
      }
      
      if (defender_realm != nullptr) {
        change_scope cs(ctx, target_obj, ctx->current);
        const auto obj = defender_realm->process(ctx);
        realm_defender = obj.get<utils::handle<core::realm>>();
      }
      
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      dd.set_arg(0, "attacker", ctx->current);
      dd.set_arg(1, "attacker_realm", realm_attacker);
      dd.set_arg(2, "target", target_obj);
      dd.set_arg(3, "target_realm", realm_defender);
      dd.set_arg(4, "cb", cb_obj);
      dd.set_arg(5, "claimant", claimant_obj);
      const size_t start = 6;
      size_t counter = start;
      for (auto t = titles_script; t != nullptr; t = t->next) {
        const auto title_obj = t->process(ctx);
        if (counter >= draw_data::arguments_count) throw std::runtime_error("Too many target titles for war, maximum is " + std::to_string(draw_data::arguments_count - start));
        dd.set_arg(counter, "title", title_obj);
        ++counter;
      }
      
      ctx->draw(&dd);
    }
    
//     const size_t end_war::type_index = commands::values::end_war;
    const size_t end_war::context_types;
    const size_t end_war::output_type;
    end_war::end_war(const size_t &type) noexcept : type(type) {}
    struct object end_war::process(context* ctx) const {
      auto war = ctx->current.get<utils::handle<core::war>>();
      // как оканчиваются войны? мы должны запустить скрипт у казус белли
      // с какими данными мы это дело запускаем? только война в качестве рута? хороший вопрос
      // в цк2 передается персонаж (точнее несколько через ROOT, FROM и проч структуры)
      // я могу получить персонажа пройдясь по функциям смены скоупа, 
      // мне нужно закончить вычисление этого скрипта и где то чуть позже вычислить on_success
      // может быть это эвент? возможно кстати, такой же примерно как смерть персонажа
      //war->cb->on_success.compute();
      // то есть нужно вызвать on_war_end что то такое
    }
    
    void end_war::draw(context* ctx) const {
      
    }
    
//     const size_t add_attacker::type_index = commands::values::add_attacker;
    const size_t add_attacker::context_types;
    const size_t add_attacker::output_type;
    add_attacker::add_attacker(const interface* character) noexcept : character(character) {}
    add_attacker::~add_attacker() noexcept { character->~interface(); }
    struct object add_attacker::process(context* ctx) const {
      const auto obj = character->process(ctx);
      auto c = obj.get<core::character*>();
      auto w = ctx->current.get<utils::handle<core::war>>();
      // да, нападают то персонажи друг на друга
      //w->attackers.push_back(c);
    }
    
    void add_attacker::draw(context* ctx) const {
      
    }
    
    const size_t imprison::type_index = commands::values::imprison;
    const size_t imprison::context_types;
    const size_t imprison::expected_types;
    const size_t imprison::output_type;
    imprison::imprison(const interface* target, const interface* realm) noexcept : target(target), realm(realm) {}
    imprison::~imprison() noexcept { target->~interface(); if (realm != nullptr) realm->~interface(); }
    struct object imprison::process(context * ctx) const {
      const auto target_obj = target->process(ctx);
      auto current = ctx->current.get<core::character*>();
      auto t = target_obj.get<core::character*>();
      assert(!t->is_prisoner());
      if (realm != nullptr) {
        const auto realm_obj = realm->process(ctx);
        auto h = realm_obj.get<utils::handle<core::realm>>();
        // текущий персонаж должен иметь какое то отношение к реалму
        // блин я тут подумал что персонаж может иметь отношение и к совету хозяина и к совету своего реалма
        // это все? может ли хозяин иметь отношение к совету своих вассалов? ну он какое то отношение так или иначе имеет
        // думаю что это одно из ограничений игры
        if (!h->include(current)) throw std::runtime_error("Character that not belongs to the realm is trying to imprison");
        h->add_prisoner(t);
      } else {
        // вообще у нас потенциально может заключать в тюрьму чел у которого нет собственного реалма
        current->self->add_prisoner(t);
      }
      return object();
    }
    
    void imprison::draw(context * ctx) const {
      auto current = ctx->current.get<core::character*>();
      object realm_obj;
      if (realm != nullptr) {
        realm_obj = realm->process(ctx);
      } else {
        realm_obj = current->self;
      }
      const auto target_obj = target->process(ctx);
      
      draw_data dd(ctx);
      dd.function_name = commands::names[type_index];
      dd.arguments[0].first = "target";
      dd.arguments[0].second = target_obj;
      dd.arguments[1].first = "realm";
      dd.arguments[1].second = realm_obj;
      ctx->draw(&dd);
    }
    
#undef ADD_FLAG_CASE
#undef ADD_FLAG_INVALID_CASE
    
    // рисовать ли откуда мы это число получили?
#define COMMON_STAT_FUNC(name, expected_context_bits, expected_type_bits)     \
    const size_t add_##name::type_index = commands::values::add_##name;       \
    add_##name::add_##name(const interface* value) noexcept : value(value) {} \
    add_##name::~add_##name() noexcept { value->~interface(); }               \
    void add_##name::draw(context* ctx) const {                               \
      const auto val = value->process(ctx);                                   \
      draw_data dd(ctx);                                                      \
      dd.function_name = commands::names[type_index];                         \
      dd.value = val;                                                         \
      ctx->draw(&dd);                                                         \
    }                                                                         \
    
#define STAT_FUNC(name) COMMON_STAT_FUNC(name, object::type_bit::character, object::type_bit::number)
#define CHARACTER_PENALTY_STAT_FUNC(name) STAT_FUNC(name##_penalty)
    
    UNIQUE_STATS_RESOURCES_LIST
    
#undef STAT_FUNC
#undef CHARACTER_PENALTY_STAT_FUNC
#undef COMMON_STAT_FUNC

#define COMMON_STAT_IMPLEMENTATION(name, type)                               \
    struct object add_##name::process(context* ctx) const {                  \
      auto c = ctx->current.get<core::type*>();                              \
      const auto val = value->process(ctx);                                  \
      c->stats.add(core::type##_stats::name, val.get<double>());             \
      return object();                                                       \
    }                                                                        \
    
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
  struct object add_##name::process(context* ctx) const {                       \
    auto c = ctx->current.get<core::type*>();                                   \
    const auto val = value->process(ctx);                                       \
    c->hero_stats.add(core::hero_stats::name, val.get<double>());               \
    return object();                                                            \
  }                                                                             \

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, character)
    BASE_HERO_STATS_LIST                      
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION

#define STAT_TYPE_CASE(name, type_name)                          \
  case object::type::type_name: {                                \
    auto ptr = ctx->current.get<core::type_name*>();             \
    ptr->stats.add(core::type_name##_stats::name, val);          \
    break;                                                       \
  }
  
#define STAT_TYPE_CASE2(name, type_name)                       \
  case object::type::type_name: {                              \
    auto ptr = ctx->current.get<core::type_name*>();           \
    ptr->hero_stats.add(core::hero_stats::name, val);          \
    break;                                                     \
  }

#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object add_##name::process(context* ctx) const { \
    const auto obj = value->process(ctx);           \
    const double val = obj.get<double>();           \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, realm)                   \
      STAT_TYPE_CASE(name, hero_troop)              \
      STAT_TYPE_CASE2(name, character)              \
      default: throw std::runtime_error("Function 'add_"#name"' receive object with wrong type"); \
    }                                               \
    return object();                                \
  }                                                 \
  
#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    HERO_FACTOR_STATS_LIST                    
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object add_##name::process(context* ctx) const { \
    const auto obj = value->process(ctx);           \
    const double val = obj.get<double>();           \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, city)                    \
      STAT_TYPE_CASE(name, province)                \
      STAT_TYPE_CASE(name, army)                    \
      STAT_TYPE_CASE(name, troop)                   \
      default: throw std::runtime_error("Function 'add_"#name"' receive object with wrong type"); \
    }                                               \
    return object();                                \
  }                                                 \

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    TROOP_FACTOR_STATS_LIST                   
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object add_##name::process(context* ctx) const { \
    const auto obj = value->process(ctx);           \
    const double val = obj.get<double>();           \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, city)                    \
      STAT_TYPE_CASE(name, province)                \
      STAT_TYPE_CASE(name, character)               \
      default: throw std::runtime_error("Function 'add_"#name"' receive object with wrong type"); \
    }                                               \
    return object();                                \
  }                                                 \

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST 
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object add_##name::process(context* ctx) const { \
    const auto obj = value->process(ctx);           \
    const double val = obj.get<double>();           \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, city)                    \
      STAT_TYPE_CASE(name, province)                \
      STAT_TYPE_CASE(name, realm)                   \
      default: throw std::runtime_error("Function 'add_"#name"' receive object with wrong type"); \
    }                                               \
    return object();                                \
  }                                                 \

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    SHARED_PROVINCE_CITY_REALM_STATS_LIST     
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object add_##name::process(context* ctx) const { \
    const auto obj = value->process(ctx);           \
    const double val = obj.get<double>();           \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, city)                    \
      STAT_TYPE_CASE(name, province)                \
      default: throw std::runtime_error("Function 'add_"#name"' receive object with wrong type"); \
    }                                               \
    return object();                                \
  }                                                 \

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    SHARED_PROVINCE_CITY_STATS_LIST           
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object add_##name::process(context* ctx) const { \
    const auto obj = value->process(ctx);           \
    const double val = obj.get<double>();           \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, hero_troop)              \
      STAT_TYPE_CASE(name, army)                    \
      default: throw std::runtime_error("Function 'add_"#name"' receive object with wrong type"); \
    }                                               \
    return object();                                \
  }                                                 \

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    SHARED_HERO_TROOP_ARMY_STATS_LIST         
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object add_##name::process(context* ctx) const { \
    const auto obj = value->process(ctx);           \
    const double val = obj.get<double>();           \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, troop)                   \
      STAT_TYPE_CASE2(name, character)              \
      default: throw std::runtime_error("Function 'add_"#name"' receive object with wrong type"); \
    }                                               \
    return object();                                \
  }                                                 \

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    SHARED_TROOP_HERO_STATS_LIST              
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object add_##name::process(context* ctx) const { \
    const auto obj = value->process(ctx);           \
    const double val = obj.get<double>();           \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, city)                    \
      STAT_TYPE_CASE(name, province)                \
      STAT_TYPE_CASE(name, realm)                   \
      STAT_TYPE_CASE(name, character)               \
      default: throw std::runtime_error("Function 'add_"#name"' receive object with wrong type"); \
    }                                               \
    return object();                                \
  }                                                 \

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    RESOURCE_INCOME_STATS_LIST
    RESOURCE_INCOME_FACTOR_STATS_LIST
    BUILD_FACTOR_STATS_LIST
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object add_##name::process(context* ctx) const { \
    const auto obj = value->process(ctx);           \
    const double val = obj.get<double>();           \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, army)                    \
      STAT_TYPE_CASE(name, realm)                   \
      STAT_TYPE_CASE(name, character)               \
      default: throw std::runtime_error("Function 'add_"#name"' receive object with wrong type"); \
    }                                               \
    return object();                                \
  }                                                 \

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    ARMY_FACTOR_STATS_LIST                    
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION

#undef STAT_TYPE_CASE
#undef STAT_TYPE_CASE2

#define STAT_TYPE_CASE(name, type_name)                          \
  case object::type::type_name: {                                \
    auto ptr = ctx->current.get<core::type_name*>();             \
    ptr->resources.add(core::type_name##_resources::name, val);  \
    break;                                                       \
  }

#define COMMON_STAT_IMPLEMENTATION(name)            \
  struct object add_##name::process(context* ctx) const { \
    const auto obj = value->process(ctx);           \
    const double val = obj.get<double>();           \
    switch (ctx->current.get_type()) {              \
      STAT_TYPE_CASE(name, realm)                   \
      STAT_TYPE_CASE(name, character)               \
      default: throw std::runtime_error("Function 'add_"#name"' receive object with wrong type"); \
    }                                               \
    return object();                                \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name)
    RESOURCE_STATS_LIST
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
#undef STAT_TYPE_CASE

#define COMMON_STAT_IMPLEMENTATION(name, type)                     \
  struct object add_##name::process(context* ctx) const {          \
    auto c = ctx->current.get<core::type*>();                      \
    const auto obj = value->process(ctx);                          \
    const double val = obj.get<double>();                          \
    c->resources.add(core::type##_resources::name, val);           \
    return object();                                               \
  }

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, city)
    CITY_RESOURCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) COMMON_STAT_IMPLEMENTATION(name, army)
    ARMY_RESOURCE_STATS_LIST
#undef STAT_FUNC

#undef COMMON_STAT_IMPLEMENTATION
  }
}
