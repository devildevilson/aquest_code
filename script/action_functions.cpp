#include "action_functions.h"

//#include "bin/core_structures.h"
#include "core/structures_header.h"
#include "re2/re2.h"
#include "utility.h"
#include "utils/magic_enum_header.h"
#include "utils/globals.h"
#include "utils/systems.h"
#include "core/context.h"

namespace devils_engine {
  namespace script {    
    template <typename T, typename STAT_T>
    void add_to_base(const target_t &t, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &stat_index) {
      assert(t.type == static_cast<uint32_t>(T::s_type)); assert(count == 1);
      assert(data[0].command_type == command_type::action); assert(stat_index < STAT_T::count);
      const auto [value, special_stat, number_type, compare_type] = get_num_from_data(&t, ctx, &data[0]);
      auto obj = reinterpret_cast<T*>(t.data);
      if (ctx->itr_func != nullptr) {
        const double original = obj->stats.get(stat_index);
        call_lua_func(&t, ctx, data, value, compare_type, special_stat, original, IGNORE_BLOCK);
        return;
      }
      obj->stats.add(stat_index, value);
    }
    
    void add_to_hero_base(const target_t &t, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &stat_index) {
      assert(t.type == static_cast<uint32_t>(core::character::s_type)); assert(count == 1);
      assert(data[0].command_type == command_type::action); assert(stat_index < core::hero_stats::count);
      const auto [value, special_stat, number_type, compare_type] = get_num_from_data(&t, ctx, &data[0]);
      auto obj = reinterpret_cast<core::character*>(t.data);
      if (ctx->itr_func != nullptr) {
        const double original = obj->hero_stats.get(stat_index);
        call_lua_func(&t, ctx, data, value, compare_type, special_stat, original, IGNORE_BLOCK);
        return;
      }
      obj->hero_stats.add(stat_index, value);
    }
    
    template <typename T>
    void add_to_resource(const target_t &t, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &stat_index) {
      assert(t.type == static_cast<uint32_t>(T::s_type)); assert(count == 1);
      assert(data[0].command_type == command_type::action); assert(stat_index < core::character_resources::count);
      const auto [value, special_stat, number_type, compare_type] = get_num_from_data(&t, ctx, &data[0]);
      auto obj = reinterpret_cast<T*>(t.data);
      if (ctx->itr_func != nullptr) {
        const double original = obj->resources.get(stat_index);
        call_lua_func(&t, ctx, data, value, compare_type, special_stat, original, IGNORE_BLOCK);
        return;
      }
      obj->resources.add(stat_index, value);
    }
    
#define ADD_TO_BASE_CASE(type, name) case core::structure::type: add_to_base<core::type, core::type##_stats::values>(t, ctx, count, data, core::type##_stats::name); break;
#define ADD_TO_BASE_CASE2(type, name) case core::structure::type: add_to_hero_base(t, ctx, count, data, core::hero_stats::name); break;
#define ADD_TO_BASE_CASE3(type, name) case core::structure::type: add_to_resource<core::type>(t, ctx, count, data, core::type##_resources::name); break;
#define DEFAULT_CASE(name) default: throw std::runtime_error(                  \
    "Bad input type '" +                                                       \
    std::string(magic_enum::enum_name(target_type)) +                          \
    "' for " + std::string(magic_enum::enum_name(action_function::add_##name)) \
  );
    
#define CHARACTER_PENALTY_STAT_FUNC(name) void add_##name##_penalty(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name##_penalty); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(character, name)    \
        default: throw std::runtime_error(                       \
          "Bad input type '" +                                   \
          std::string(magic_enum::enum_name(target_type)) +      \
          "' for " + std::string(magic_enum::enum_name(action_function::add_##name##_penalty))      \
        );                                   \
      }                                      \
    }
    
    CHARACTER_PENALTY_STATS_LIST
    
#undef CHARACTER_PENALTY_STAT_FUNC

// прибавка к базовым статам
#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(character, name)    \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    BASE_CHARACTER_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(realm, name)        \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    BASE_REALM_STATS_LIST
    VASSAL_RESOURCE_INCOME_FACTOR_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(province, name)     \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    BASE_PROVINCE_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(city, name)         \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    BASE_CITY_STATS_LIST
    LIEGE_RESOURCE_INCOME_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(army, name)         \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    BASE_ARMY_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE2(character, name)   \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    BASE_HERO_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(troop, name)        \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    BASE_TROOP_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(hero_troop, name)   \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    BASE_HERO_TROOP_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(province, name)     \
        ADD_TO_BASE_CASE(city, name)         \
        ADD_TO_BASE_CASE(army, name)         \
        ADD_TO_BASE_CASE(troop, name)        \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    TROOP_FACTOR_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(province, name)     \
        ADD_TO_BASE_CASE(city, name)         \
        ADD_TO_BASE_CASE(character, name)    \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(realm, name)        \
        ADD_TO_BASE_CASE(city, name)         \
        ADD_TO_BASE_CASE(province, name)     \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    SHARED_PROVINCE_CITY_REALM_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(city, name)         \
        ADD_TO_BASE_CASE(province, name)     \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    SHARED_PROVINCE_CITY_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(troop, name)        \
        ADD_TO_BASE_CASE(army, name)         \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    SHARED_HERO_TROOP_ARMY_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(troop, name)        \
        ADD_TO_BASE_CASE2(character, name)   \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    SHARED_TROOP_HERO_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(realm, name)        \
        ADD_TO_BASE_CASE(hero_troop, name)   \
        ADD_TO_BASE_CASE2(character, name)   \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    HERO_FACTOR_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(province, name)     \
        ADD_TO_BASE_CASE(city, name)         \
        ADD_TO_BASE_CASE(character, name)    \
        ADD_TO_BASE_CASE(realm, name)        \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    RESOURCE_INCOME_STATS_LIST
    RESOURCE_INCOME_FACTOR_STATS_LIST
    BUILD_FACTOR_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE(character, name)    \
        ADD_TO_BASE_CASE(realm, name)        \
        ADD_TO_BASE_CASE(army, name)         \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    ARMY_FACTOR_STATS_LIST
    
#undef STAT_FUNC

#define STAT_FUNC(name) void add_##name(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) \
    { assert(count == 1); assert(data[0].helper1 == action_function::add_##name); \
      const auto target_type = static_cast<core::structure>(t.type); \
      switch (target_type) {                 \
        ADD_TO_BASE_CASE3(character, name)   \
        ADD_TO_BASE_CASE3(realm, name)       \
        DEFAULT_CASE(name)                   \
      }                                      \
    }
    
    RESOURCE_STATS_LIST
    
#undef STAT_FUNC
    
    void add_trait(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      assert(t.type == static_cast<uint32_t>(core::character::s_type));
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].helper1 == action_function::add_trait);
      // по этой строке мы должны найти трейт и добавить его персонажу
      auto character = reinterpret_cast<core::character*>(t.data);
      
    }
    
    void add_flag(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].helper1 == action_function::add_flag);
      
      if (ctx->itr_func != nullptr) {
        sol::state_view state = ctx->itr_func->lua_state();
        sol::object obj;
        
        // тут может быть несколько входных данных
        if (data[0].number_type == number_type::array) {
          const size_t array_count = data[0].value;
          assert(array_count <= 2 && array_count != 0);
          auto table = state.create_table(0, 2);
          auto flag_data = reinterpret_cast<const script_data*>(data[0].data);
          const auto str = get_string_from_data(&t, ctx, &flag_data[0]);
          table["flag"] = str;
          if (array_count > 1) {
            const auto [value, special_stat, number_type, compare_type] = get_num_from_data(&t, ctx, &flag_data[1]);
            const size_t turns = value;
            table["time"] = turns;
          }
        } else {
          const auto str = get_string_from_data(&t, ctx, &data[0]);
          obj = sol::make_object(state, str);
        }
        
        // пока что оставим UINT16_MAX
        call_lua_func(&t, ctx, &data[0], obj, sol::nil, sol::nil);
        return;
      }
      
      // тут может быть несколько входных данных
      std::string_view str;
      size_t turns = SIZE_MAX;
      if (data[0].number_type == number_type::array) {
        const size_t array_count = data[0].value;
        assert(array_count <= 2 && array_count != 0);
        auto flag_data = reinterpret_cast<const script_data*>(data[0].data);
        str = get_string_from_data(&t, ctx, &flag_data[0]);
        if (array_count > 1) {
          const auto [value, special_stat, number_type, compare_type] = get_num_from_data(&t, ctx, &flag_data[1]);
          turns = value;
        }
      } else {
        str = get_string_from_data(&t, ctx, &data[0]);
      }
      
      // эдд флаг как раз пример того что нужно делать сложную систему огранизации инпута в луа функцию
      // во первых у нас явно может здесь быть как и массив данных, так и просто строка
      // вообще нам бы вернуть таблицу в том же виде в котором она была в конфиге скрипта
      // другое дело что у нас очень четкий порядок переменных, 
      // и скорее всего можно не запариваться над наименованиями переменных
      //if (ctx.itr_func != nullptr) { call_lua_func_value(t, ctx, data[0], val, special_stat); return; }
      
      // по идее строка должна остаться валидной
      const core::structure target_type = static_cast<core::structure>(t.type);
      switch (target_type) {
        // нужно сделать списки какие данные каких типов наследуют какие контейнеры
        // чтобы аккуратно делать вот такие функции
#define ADD_FLAG_CASE(name) case core::structure::name: { \
          auto obj = reinterpret_cast<core::name*>(t.data); \
          obj->add_flag(str, turns); \
          break; \
        }
        
        ADD_FLAG_CASE(army)
        ADD_FLAG_CASE(character)
        ADD_FLAG_CASE(city)
        ADD_FLAG_CASE(province)
        ADD_FLAG_CASE(realm)
        ADD_FLAG_CASE(titulus)
        ADD_FLAG_CASE(hero_troop)
        ADD_FLAG_CASE(war)
        
#undef ADD_FLAG_CASE
        
        default: throw std::runtime_error("Bad target type " + std::string(magic_enum::enum_name<core::structure>(target_type)));
      }
    }
    
    void marry(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      // по идее здесь уже должен по умолчанию приходить массив
      // какой размер? нужно указать на ком женитьба, кто патрон, ???
      // как быть с условиями женитьбы? то есть как посчитать очки хочет/не хочет?
      // хотя в цк3 приходит только персонаж на ком жениться
      // да это очень простое действие
      // мне нужно сделать значит проверку возможности женитьбы отдельную
      // и для нее сделать вывод полезной информации
      
      // самое важное - эта фукция будет вызывать еще и эвенты по цепочке 
      // utils::action_type::on_marriage, другое дело что нужно супер аккуратно вызвать только один раз
      // 
      
      assert(count == 1);
      
      // тут в принципе ничего особеного
      const auto consort_t = get_target_from_data(&t, ctx, &data[0]);
      
      assert(t.type == static_cast<uint32_t>(core::structure::character));
      assert(consort_t.type == static_cast<uint32_t>(core::structure::character));
      auto ch = reinterpret_cast<core::character*>(t.data);
      auto consort_ch = reinterpret_cast<core::character*>(consort_t.data);
      
      assert(!ch->is_married());
      assert(!consort_ch->is_married());
      
      if (ctx->itr_func != nullptr) {
        call_lua_func(&t, ctx, data, &consort_t, sol::nil, IGNORE_BLOCK);
        return;
      }
      
      ch->family.consort = consort_ch;
      consort_ch->family.consort = ch;
      
      // вообще по идее еще должны быть бонусы там престижа
      // где они указываются? их надо расчитывать
      // женитьба вообще требует выбрать 4 участников: жених, невеста, инициатор женитьбы, тот кто должен согласиться на нее
      // иногда люди эти пересекаются, но довольно часто нет, мне нужно еще на этапе проверки потенциальных условий, как то понять
      // имеет ли смысл дальше проверять интеракцию
    }
    
    void add_hook(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      assert(t.type == static_cast<uint32_t>(core::character::s_type));
      // тут нужно 4 входных данных: тип, на кого, какой секрет, сколько дней держится
      // таргет только персонаж
      // по крайней мере один входной будет в контексте
      // с хуками немног посложнее, их может быть очень много с учетом всех персонажей в игре
      // нужно как то оптимизировать все это дело
    }
    
    // могут ли экшоны вызываться параллельно? было бы неплохо чтобы да
    // 
    void start_war(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      // одна из самых важных функций, для старта войны нам нужно 
      // casus_belli, таргет, кто клаймат (то есть из-за кого (из-за чьих амбиций) мы начали войну)
      // и титулы за которые мы боремся, война стартуется между персонажами, что делать если персонаж умирает на войне?
      // если клаймат умирает во время войны, то война заканчивается (точнее если клейм не передается по наследству)
      // война длится чаще всего много лет и игра заключается в том чтобы сохранить жизнь челику до конца войны
      // хватит ли мне ходов до конца войны? точнее сколько ходов у меня будет средняя война? 
      // несколько лет это примерно 150-200 ходов, на такое количество ходов нужно расчитывать 
      // для обычной войны за титул с примерно равным противником, хотя это сильно зависит от присутствия союзников
      // в моем случае война поди будет уделом реалма, даже притом что зависит от клеймов персонажа
      // можно даже сделать государственный клейм, и например совет может решить стартануть войну по этому клейму
      // клаймат и таргет может быть как и персонажем так и реалмом?
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].number_type == number_type::array);
      auto array_data = reinterpret_cast<script_data*>(data[0].data);
      const size_t array_count = data[0].value;
      assert(array_count == 4); // кажется здесь может приходить только 4 штуки данных
      // что такое казус белли? по идее это тоже игровая сущность, которую можно задать
      // таргет должен быть реалмом, все войны хранятся там
      // тут еще нужно добавить сохранение в скоуп новой войны
      
      // тут у нас происходит очень банальная вещь: из контекста мы создаем объект war
      // и заполняем его тем что у нас здесь есть, и по идее все
      // это означает что у нас есть война, теперь на уровне игры нам нужно сделать так чтобы армии
      // сходились и происходили битвы, то есть теперь нужно обходить враждебные армии или 
      // если мы прямо в них хотим пойти, то связывать их боем
      
      const auto cb = get_target_from_data(&t, ctx, &array_data[0]);
      const auto target = get_target_from_data(&t, ctx, &array_data[1]);
      const auto claimat = get_target_from_data(&t, ctx, &array_data[2]);
      const size_t titles_count = array_data[2].value;
      auto titles_array = reinterpret_cast<script_data*>(array_data[2].data);
      
      // а дальше титулы, титулы задаются сразу пачкой, как эту пачку получить?
      // старт войны по кнопке игрока походу зависит от типа в цк3, там не используется явно 
      // start_war функция, она видимо используется неявно в интерфейсе, 
      // но я посмотрел и не могу толком сказать, цб в цк3 немного отличается от того что было в цк2
      // главным образом тем что добавлены дополнительные условия и эффекты
      // короче я пока что не могу понять как именно приходят титулы
      // вроде как обычным способом можно бороться только за один титул
      // (ну или за все титулы внутри титула - религиозная война)
      // скорее всего тут оставлена возможность указать несколько титулов 
      // для специальных скриптовых войн для определенных стран
      // тогда берем один титул
      
      assert(cb.type == static_cast<uint32_t>(core::casus_belli::s_type));
      assert(target.type == static_cast<uint32_t>(core::realm::s_type));
      assert(claimat.type == static_cast<uint32_t>(core::realm::s_type));
      
      auto cb_ptr = reinterpret_cast<core::casus_belli*>(cb.data);
      auto starter_ptr = reinterpret_cast<core::realm*>(t.data);
      auto target_ptr = reinterpret_cast<core::realm*>(target.data);
      auto claimat_ptr = reinterpret_cast<core::realm*>(claimat.data);
      
      assert(cb_ptr      != nullptr);
      assert(starter_ptr != nullptr);
      assert(target_ptr  != nullptr);
      assert(claimat_ptr != nullptr);
      
      auto core_ctx = global::get<systems::map_t>()->core_context;
      const size_t starter_token = t.token != SIZE_MAX ? t.token : core_ctx->get_realm_token(starter_ptr);
      const size_t target_token  = target.token != SIZE_MAX ? target.token : core_ctx->get_realm_token(target_ptr);
      
      if (ctx->itr_func != nullptr) {
        sol::state_view s = ctx->itr_func->lua_state();
        
        auto titles_t = s.create_table(titles_count);
        for (size_t i = 0; i < titles_count; ++i) {
          const auto &title_data = titles_array[i];
          const auto title = get_target_from_data(&t, ctx, &title_data);
          assert(title.type == static_cast<uint32_t>(core::titulus::s_type));
          auto title_ptr = reinterpret_cast<core::titulus*>(title.data);
          titles_t.add(sol::make_object(s, title_ptr));
        }
        
        auto t_data = s.create_table_with(
          "cb", sol::make_object(s, cb_ptr),
          "target", sol::make_object(s, target_ptr),
          "claimat", sol::make_object(s, claimat_ptr),
          "titles", titles_t
        );
        
        call_lua_func(&t, ctx, data, t_data, sol::nil, sol::nil);
        return;
      }
      
      const size_t war_token = core_ctx->create_war();
      auto war = core_ctx->get_war(war_token);
      // заполняем war, пихаем индекс войны в реалмы, тащемта это все
      // нужно еще добавить возможность в контекст пихнуть
      war->cb = cb_ptr;
      war->opener_realm = starter_ptr;
      war->target_realm = target_ptr;
      war->claimat = claimat_ptr;
      // запускать войну могут только лидеры? по идее да
      war->war_opener = starter_ptr->leader;
      war->target_character = target_ptr->leader;
      // войны было бы неплохо указывать сразу с кем война, так еще быстро можно проверить чтоб по 500 раз с одним и тем же не было войны
      if (const auto itr1 = starter_ptr->wars.find(target_token), itr2 = target_ptr->wars.find(starter_token); 
          itr1 != starter_ptr->wars.end() || itr2 != target_ptr->wars.end()) 
            throw std::runtime_error("The war between theese two is already exist");
      
      for (size_t i = 0; i < titles_count; ++i) {
        const auto &title_data = titles_array[i];
        const auto title = get_target_from_data(&t, ctx, &title_data);
        assert(title.type == static_cast<uint32_t>(core::titulus::s_type));
        auto title_ptr = reinterpret_cast<core::titulus*>(title.data);
        war->target_titles.push_back(title_ptr);
      }
      
      starter_ptr->wars.emplace(target_token, war_token);
      target_ptr->wars.emplace(starter_token, war_token);
      
      // если таргет будет safe_target, то в каждой функции нужно будет получать объект из контекста
      // насколько это плохо? подгружаем инфу в кеш и так много раз, это просто не нужно
      // имеет смысл держать скоуп в сейф состоянии, а потом при неспоредственном скрипте 
      // развернуть все эти вещи и что если их несуществует? по идее придется отменить эвент
      
      // индексы тоже не защищают меня от проблем, может быть ситуация пересоздания объекта 
      // а значит по индексу мы обратимся к неверному объекту, единственное что спасает явно
      // это генерация некоего токена для каждого объекта, для того чтобы обратиться к объекту мы запоминаем
      // и токен и индекс (или токен является индексом), где хранится токен? было бы неплохо использовать 
      // unordered_map для того чтобы получать объекты, но это целый рот бесполезных вычислений
      // я вот что подумал мы можем в токен задать индекс, но более умным способом, 
      // если бы у нас было представление о максимальном возможном количестве данного объекта
      // то есть там условно 10к армий железный максимум, то мы можем при переиспользовании слота увеличивать 
      // счетчик, индекс в этом случае будет (счетчик * макс + индекс) - это и будет нашим токеном
      // так мы можем легко проверить поменялись ли у нас данные в слоте
      
      // так же имеет смысл описать уведомления которые можно послать персонажам
      // где можно указать разные полезные данные, например при старте войны написать с кем война
    }
    
    void value(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].helper1 == action_function::value);
      assert(ctx->current_value.number_type == number_type::number);
      
      const auto [val, special_type, number_type, compare_type] = get_num_from_data(&t, ctx, &data[0]);
      // переменная в контексте ДОЛЖНА быть определена
      if (ctx->itr_func != nullptr) {
        call_lua_func(&t, ctx, data, val, compare_type, special_type, ctx->current_value.value, IGNORE_BLOCK);
        return;
      }
      ctx->current_value.value = val;
    }
    
    void add(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].helper1 == action_function::add);
      assert(ctx->current_value.number_type == number_type::number);
      
      const auto [val, special_type, number_type, compare_type] = get_num_from_data(&t, ctx, &data[0]);
      // переменная в контексте ДОЛЖНА быть определена
      if (ctx->itr_func != nullptr) {
        call_lua_func(&t, ctx, data, val, compare_type, special_type, ctx->current_value.value, IGNORE_BLOCK);
        return;
      }
      ctx->current_value.value += val;
    }
    
    void factor(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].helper1 == action_function::factor);
      assert(ctx->current_value.number_type == number_type::number);
      
      const auto [val, special_type, number_type, compare_type] = get_num_from_data(&t, ctx, &data[0]);
      // переменная в контексте ДОЛЖНА быть определена
      if (ctx->itr_func != nullptr) {
        call_lua_func(&t, ctx, data, val, compare_type, special_type, ctx->current_value.value, IGNORE_BLOCK);
        return;
      }
      ctx->current_value.value *= val;
    }
    
    void string(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].helper1 == action_function::string);
      // переменная в контексте ДОЛЖНА быть определена
      assert(ctx->current_value.number_type == number_type::string_view);
      
      // может быть плохая строчка, нужно сделать кучу проверок на валидность строки
      const auto val = get_raw_string_from_data(&t, ctx, &data[0]);
      if (ctx->itr_func != nullptr) {
        call_lua_func(&t, ctx, data, val, sol::nil, IGNORE_BLOCK);
        return;
      }
      ctx->current_value.data = val;
    }
    
    void object(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].helper1 == action_function::object);
      // переменная в контексте ДОЛЖНА быть определена
      assert(ctx->current_value.number_type == number_type::object);
      
      // тут нужно получить объект примерно по тому же принципу что и строку
      const auto target = get_target_from_data(&t, ctx, &data[0]);
      if (ctx->itr_func != nullptr) {
        call_lua_func(&t, ctx, data, &target, sol::nil, IGNORE_BLOCK);
        return;
      }
      
      ctx->current_value.helper2 = target.type;
      ctx->current_value.data = target.data;
      ctx->current_value.value = cast_to_double(target.token);
    }
    
    void save_as(const target_t &t, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action);
      assert(data[0].helper1 == action_function::save_as);
      
      // для сохранения достаточно указать имя переменной
      // но вообще мы можем захотеть сохранить еще какие нибудь числа, строки или объекты
      // тут можно вот как сделать: если два элемента массива, то это объект (разных типов) + строка
      // если один элемент массива только это только строка
      
      const size_t array_count = data[0].value;
      auto context_array = reinterpret_cast<script_data*>(data[0].data);
      // как понять тип? а ну мы можем в любом случае создавать массив
      if (array_count == 1) {
        auto str = get_string_from_data(&t, ctx, &context_array[0]);
        // куда то текущий таргет сохраняем
        return;
      }
      
      if (array_count == 3) {
        // в одном слоте точно лежит строка, а в другом может лежать любой тип
        // number_type использовать плохая идея, потому что он редко будет сопоставим с переменной
        auto str = get_string_from_data(&t, ctx, &context_array[0]);
        // во втором слоте будет лежать тип переменной
        const auto type = context_array[1].number_type;
        switch (type) {
          case number_type::number: {
            const auto [val, special_type, number_type, compare_type] = get_num_from_data(&t, ctx, &context_array[2]);
            
            break;
          }
          
          case number_type::string: {
            const auto str = get_string_from_data(&t, ctx, &context_array[2]);
            
            break;
          }
          
          case number_type::object: {
            const auto obj = get_target_from_data(&t, ctx, &context_array[2]);
            
            break;
          }
          
          default: assert(false);
        }
        
        return;
      }
      
      assert(false);
    }
    
/* =============================================    
                  START INIT
   ============================================= */
    
    // мне нужно наверное как то указать add_money для персонажа и для реалма
    // или не надо? как по другому отделять с помощью enum? да никак видимо

    void add_flag_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      // флаг мы можем добавить почти ко всему
      // 
      
      data->command_type = command_type::action;
      data->helper1 = action_function::add_flag;
      
      // на вход может податься только строка? число нет, контекст? контекст или строка
      // если строка не парсится через regex, то это флаг?
      
      if (obj.get_type() != sol::type::string || obj.get_type() != sol::type::table) throw std::runtime_error("Bad input obj for add_flag");
      
      if (obj.get_type() == sol::type::string) {
        string_input_init("add_flag", target_type, obj, data);
        // если здесь сложная строчка, то мы перепутаем ее с массивом по умолчанию
        return;
      }
      
      if (obj.get_type() == sol::type::table) {
        const sol::table t = obj.as<sol::table>();
        const auto flag_proxy = t["flag"]; 
        const bool valid_flag = flag_proxy.valid() && flag_proxy.get_type() == sol::type::string;
        const auto time_proxy = t["time"]; 
        const bool valid_time = flag_proxy.valid() && (flag_proxy.get_type() == sol::type::string || flag_proxy.get_type() == sol::type::number);
        // мы вылетим здесь если мы попытаемся сделать сложную строку
        // а хотя может это и к лучшему, в том плане что сложная строка у нас будет храниться вложенной в эту таблицу
        // тогда если в add_flag приходит array, то понятно что это
        if (!valid_flag) throw std::runtime_error("Bad add_flag input value type");
        
        script_data* flag_data = nullptr;
        script_data* turns_data = nullptr;
        if (valid_flag && valid_time) {
          data->number_type = number_type::array;
          data->value = 2;
          script_data* d = new script_data[2];
          data->data = d;
          flag_data = &d[0];
          turns_data = &d[1];
        } else {
          assert(valid_flag);
          assert(!valid_time);
          flag_data = data;
        }
        
        string_input_init("add_flag", target_type, flag_proxy, flag_data);
        if (valid_time) variable_input_init("add_flag", target_type, time_proxy, turns_data);
      }
      
      UNUSED_VARIABLE(target_type);
    }
    
    void add_hook_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      if (obj.get_type() != sol::type::table) throw std::runtime_error("Bad data for add_money command");
      
      const sol::table t = obj.as<sol::table>();
      // обязательно должны быть указаны: тип, таргет, секрет, и по желанию количество ходов
    }
    
    void marry_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      // тут должен быть контекст скорее всего который вернет таргет, таргет может быть только персонажем
      // скорее всего здесь не должно быть ни одного способа получить персонажа без контекста
    }
    
    void add_trait_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      // тут может быть id треита или мы можем взять его из контекста
      // откуда треит мы можем получить? нужна мапа со всеми такими объектами
    }
    
    void start_war_init(const uint32_t &, const sol::object &, script_data*) {
      // тут особо разночтений не будет, таргет, цб, клаймат, титулы
    }
    
    void value_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      data->command_type = command_type::action;
      data->helper1 = action_function::value;
      variable_input_init("value", target_type, obj, data);
    }
    
    void add_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      data->command_type = command_type::action;
      data->helper1 = action_function::add;
      variable_input_init("add", target_type, obj, data);
    }
    
    void factor_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      data->command_type = command_type::action;
      data->helper1 = action_function::factor;
      variable_input_init("factor", target_type, obj, data);
    }
    
    void string_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      data->command_type = command_type::action;
      data->helper1 = action_function::string;
      string_input_init("string", target_type, obj, data);
    }
    
    void object_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      data->command_type = command_type::action;
      data->helper1 = action_function::object;
      
      // тут нужно получить объект примерно по тому же принципу что и строку
      target_input_init("object", target_type, obj, data);
    }
    
    void save_as_init(const uint32_t &target_type, const sol::object &obj, script_data* data) {
      data->command_type = command_type::action;
      data->helper1 = action_function::save_as;
      
      if (obj.get_type() == sol::type::table) {
        const auto table = obj.as<sol::table>();
        const auto name_proxy = table["name"];
        if (!name_proxy.valid()) throw std::runtime_error("'save_as' function require 'name' string in table");
        
        const auto val_proxy = table["value"];
        const auto str_proxy = table["string"];
        const auto object_proxy = table["object"];
        
        size_t size = 0;
        script_data* array = nullptr;
        if (val_proxy.valid()) {
          size = 3;
          array = new script_data[size];
          string_input_init("save_as", target_type, name_proxy, &array[0]);
          array[1].number_type = number_type::number;
          variable_input_init("save_as", target_type, val_proxy, &array[2]);
        } else if (str_proxy.valid()) {
          size = 3;
          array = new script_data[size];
          string_input_init("save_as", target_type, name_proxy, &array[0]);
          array[1].number_type = number_type::string;
          string_input_init("save_as", target_type, str_proxy, &array[2]);
        } else if (object_proxy.valid()) {
          size = 3;
          array = new script_data[size];
          string_input_init("save_as", target_type, name_proxy, &array[0]);
          array[1].number_type = number_type::object;
          target_input_init("save_as", target_type, object_proxy, &array[2]);
        } else {
          const size_t table_size = table.size()-1;
          std::cout << "WARNING: 'save_as' ignored " << table_size << " values in init table!" << "\n";
          size = 1;
          array = new script_data[size];
          string_input_init("save_as", target_type, name_proxy, &array[0]);
        }
        
        data->number_type = number_type::array;
        data->value = size;
        data->data = array;
      } else {
        string_input_init("save_as", target_type, obj, data);
      }
    }
    
#define CHARACTER_PENALTY_STAT_FUNC(name) void add_##name##_penalty_init(const uint32_t &target_type, const sol::object &obj, script_data* data) { \
      if (target_type != UINT32_MAX && target_type != static_cast<uint32_t>(core::character::s_type)) \
        throw std::runtime_error(                            \
          "Bad input type '" +                               \
          std::string(magic_enum::enum_name<core::structure>(static_cast<core::structure>(target_type))) + \
          "' for " + std::string(magic_enum::enum_name(action_function::add_##name##_penalty))             \
        );                                                   \
      data->command_type = command_type::action;             \
      data->helper1 = action_function::add_##name##_penalty; \
      variable_input_init("add_"#name"_penalty", target_type, obj, data);    \
    }
    
    CHARACTER_PENALTY_STATS_LIST

#undef CHARACTER_PENALTY_STAT_FUNC

#define DEFAULT_STAT_FUNC(name, cond) void add_##name##_init(const uint32_t &target_type, const sol::object &obj, script_data* data) { \
      if (target_type != UINT32_MAX && (cond))            \
        throw std::runtime_error(                         \
          "Bad input type '" +                            \
          std::string(magic_enum::enum_name<core::structure>(static_cast<core::structure>(target_type))) + \
          "' for " + std::string(magic_enum::enum_name(action_function::add_##name))                       \
        );                                                \
      data->command_type = command_type::action;          \
      data->helper1 = action_function::add_##name;        \
      variable_input_init("add_"#name, target_type, obj, data); \
    }

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::character::s_type))
    BASE_CHARACTER_STATS_LIST
#undef STAT_FUNC
    
#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::province::s_type) && \
 target_type != static_cast<uint32_t>(core::city::s_type) && \
 target_type != static_cast<uint32_t>(core::character::s_type))
    SHARED_PROVINCE_CITY_CHARACTER_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::province::s_type) && \
 target_type != static_cast<uint32_t>(core::city::s_type) && \
 target_type != static_cast<uint32_t>(core::character::s_type) && \
 target_type != static_cast<uint32_t>(core::realm::s_type))
    RESOURCE_INCOME_STATS_LIST
    RESOURCE_INCOME_FACTOR_STATS_LIST
    BUILD_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::army::s_type) && \
 target_type != static_cast<uint32_t>(core::character::s_type) && \
 target_type != static_cast<uint32_t>(core::realm::s_type))
    ARMY_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::realm::s_type))
    BASE_REALM_STATS_LIST
    VASSAL_RESOURCE_INCOME_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::province::s_type) && \
 target_type != static_cast<uint32_t>(core::city::s_type) && \
 target_type != static_cast<uint32_t>(core::realm::s_type))
    SHARED_PROVINCE_CITY_REALM_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::province::s_type) && \
 target_type != static_cast<uint32_t>(core::city::s_type) && \
 target_type != static_cast<uint32_t>(core::army::s_type) && \
 target_type != static_cast<uint32_t>(core::troop::s_type))
    TROOP_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::province::s_type))
    BASE_PROVINCE_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::city::s_type))
    BASE_CITY_STATS_LIST
    LIEGE_RESOURCE_INCOME_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::army::s_type))
    BASE_ARMY_STATS_LIST
    SHARED_HERO_TROOP_ARMY_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::hero_troop::s_type))
    BASE_HERO_TROOP_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::troop::s_type))
    BASE_TROOP_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::province::s_type) && \
 target_type != static_cast<uint32_t>(core::city::s_type))
    SHARED_PROVINCE_CITY_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, target_type != static_cast<uint32_t>(core::character::s_type))
    BASE_HERO_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
  target_type != static_cast<uint32_t>(core::troop::s_type) && \
  target_type != static_cast<uint32_t>(core::character::s_type))
    SHARED_TROOP_HERO_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
  target_type != static_cast<uint32_t>(core::character::s_type) && \
  target_type != static_cast<uint32_t>(core::realm::s_type) && \
  target_type != static_cast<uint32_t>(core::hero_troop::s_type))
    HERO_FACTOR_STATS_LIST
#undef STAT_FUNC

#define STAT_FUNC(name) DEFAULT_STAT_FUNC(name, \
 target_type != static_cast<uint32_t>(core::character::s_type) && \
 target_type != static_cast<uint32_t>(core::realm::s_type))
    RESOURCE_STATS_LIST
#undef STAT_FUNC
    
#undef DEFAULT_STAT_FUNC
  }
}
