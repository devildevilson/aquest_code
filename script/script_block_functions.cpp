#include "script_block_functions.h"

//#include "bin/core_structures.h"
#include "core/structures_header.h"
#include "utility.h"
#include "re2/re2.h"
#include "utils/magic_enum_header.h"

namespace devils_engine {
  namespace script {
    static const RE2 regex_dot_matcher(dot_matcher);
    static const RE2 regex_colon_matcher(colon_matcher);
    
    // так не получится скорее всего
//     int32_t call_common_block_with_lua(const struct target_t &t, const struct target_t &new_t, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &block_type) {
//       change_prev_target cpt(ctx, t);
//       int32_t ret = INT32_MAX;
//       switch (block_type) {
//         case CONDITION: {
//           {
//             turn_off_function off(ctx);
//             ret = common_block(new_t, ctx, 1, &rvalue_data[1], block_type);
//           }
//           
//           ASSERT(ret != INT32_MAX);
//           
//           if (ctx->itr_func != nullptr) {
//             call_lua_func(&t, ctx, data, &new_t, sol::nil, sol::make_object(ctx->itr_func->lua_state(), bool(ret)));
//             common_block(new_t, ctx, 1, &rvalue_data[1], block_type);
//           }
//           
//           break;
//         }
//         
//         case ACTION: {
//           call_lua_func(&t, ctx, data, &new_t, sol::nil, sol::nil);
//           ret = common_block(new_t, ctx, 1, &rvalue_data[1], block_type);
//           break;
//         }
//         
//         default: assert(false);
//       }
//     }
    
    int32_t rvalue_script_block(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &block_type) {
      assert(count == 1);
      assert(data[0].command_type == command_type::rvalue);
      assert(data[0].number_type == number_type::array);
      assert(size_t(data[0].value) == 2);
      // тут нужно что то вывести
      auto rvalue_data = reinterpret_cast<const script_data*>(data[0].data);
      const auto t = get_target_from_data(&target, ctx, &rvalue_data[0]);
      change_prev_target cpt(ctx, target);
      int32_t ret = INT32_MAX;
      switch (block_type) {
        case CONDITION: {
          {
            turn_off_function off(ctx);
            ret = common_block(t, ctx, 1, &rvalue_data[1], block_type);
          }
          
          ASSERT(ret != INT32_MAX);
          
          if (ctx->itr_func != nullptr) {
            call_lua_func(&target, ctx, data, &t, sol::nil, ret);
            common_block(t, ctx, 1, &rvalue_data[1], block_type);
          }
          
          break;
        }
        
        case ACTION: {
          call_lua_func(&target, ctx, data, &t, sol::nil, IGNORE_BLOCK);
          ret = common_block(t, ctx, 1, &rvalue_data[1], block_type);
          break;
        }
        
        default: assert(false);
      }
      
      return ret;
    }
    
    int32_t light_condition(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      // тут мы должны обойти массив скриптовых данных
      // и выйти как только получим false
      const bool not_interface = ctx->itr_func == nullptr;
      
      increase_nesting n(ctx);
      for (size_t i = 0; i < count; ++i) {
        const auto &command = data[i];
        assert(command.command_type == command_type::condition || 
               command.command_type == command_type::condition_script_block || 
               command.command_type == command_type::general_script_block || 
               command.command_type == command_type::rvalue);
        const uint32_t func_index = command.helper1;
        
        switch (command.command_type) {
          case command_type::condition: {
            const auto func = condition_functions[func_index];
            // имеет ли смысл тут передавать количество другое нежели чем 1? неуверен
            const bool ret = func(target, ctx, 1, &command);
            if (not_interface && !ret) return FALSE_BLOCK;
            
            break;
          }
          
          case command_type::condition_script_block: {
            const auto func = condition_block_functions[func_index];
            const int32_t ret = func(target, ctx, 1, &command);
            // может прийти IGNORE_BLOCK
            if (not_interface && ret == FALSE_BLOCK) return FALSE_BLOCK;
            
            break;
          }
          
          case command_type::general_script_block: {
            const auto func = general_block_functions[func_index];
            const int32_t ret = func(target, ctx, 1, &command, CONDITION);
            if (not_interface && ret == FALSE_BLOCK) return FALSE_BLOCK;
            break;
          }
          
          case command_type::rvalue: {
            const int32_t ret = rvalue_script_block(target, ctx, 1, &command, CONDITION);
            if (not_interface && ret == FALSE_BLOCK) return FALSE_BLOCK;
            break;
          }
          
          default: assert(false);
        }
      }
      
      return TRUE_BLOCK;
    }
    
    int32_t light_condition_or(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      const bool not_interface = ctx->itr_func == nullptr;
      
      increase_nesting n(ctx);
      for (size_t i = 0; i < count; ++i) {
        const auto &command = data[i];
        assert(command.command_type == command_type::condition || 
               command.command_type == command_type::condition_script_block || 
               command.command_type == command_type::general_script_block || 
               command.command_type == command_type::rvalue);
        const uint32_t func_index = command.helper1;
        
        switch (command.command_type) {
          case command_type::condition: {
            const auto func = condition_functions[func_index];
            // имеет ли смысл тут передавать количество другое нежели чем 1?
            // неуверен
            const bool ret = func(target, ctx, 1, &command);
            if (not_interface && ret) return TRUE_BLOCK;
            
            break;
          }
          
          case command_type::condition_script_block: {
            const auto func = condition_block_functions[func_index];
            const int32_t ret = func(target, ctx, 1, &command);
            // может прийти IGNORE_BLOCK
            if (not_interface && ret == TRUE_BLOCK) return TRUE_BLOCK;
            
            break;
          }
          
          case command_type::general_script_block: {
            const auto func = general_block_functions[func_index];
            const int32_t ret = func(target, ctx, 1, &command, CONDITION);
            if (not_interface && ret == TRUE_BLOCK) return TRUE_BLOCK;
            break;
          }
          
          case command_type::rvalue: {
            const int32_t ret = rvalue_script_block(target, ctx, 1, &command, CONDITION);
            if (not_interface && ret == TRUE_BLOCK) return TRUE_BLOCK;
            break;
          }
          
          default: assert(false);
        }
      }
      
      return FALSE_BLOCK;
    }
    
    int32_t light_action(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      increase_nesting n(ctx);
      for (size_t i = 0; i < count; ++i) {
        const auto &command = data[i];
        assert(command.command_type == command_type::action || 
               command.command_type == command_type::action_script_block || 
               command.command_type == command_type::general_script_block || 
               command.command_type == command_type::rvalue);
        const uint32_t func_index = command.helper1;
        switch (command.command_type) {
          case command_type::action: {
            const auto func = action_functions[func_index];
            func(target, ctx, 1, &command);
            break;
          }
          
          case command_type::action_script_block: {
            const auto func = action_block_functions[func_index];
            const int32_t ret = func(target, ctx, 1, &command);
            (void)ret; // тут по идее не нужен ret
            break;
          }
          
          case command_type::general_script_block: {
            const auto func = general_block_functions[func_index];
            const int32_t ret = func(target, ctx, 1, &command, ACTION);
            (void)ret;
            break;
          }
          
          case command_type::rvalue: {
            rvalue_script_block(target, ctx, 1, &command, ACTION);
            break;
          }
          
          default: assert(false);
        }
      }
      
      return TRUE_BLOCK;
    }
    
    // функция оказась проще чем я думал
    int32_t condition(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      // тут мы должны обойти массив скриптовых данных
      // и выйти как только получим false
      assert(count == 1);
      assert(data[0].command_type == command_type::condition_script_block);
      assert(data[0].helper1 == condition_block_function::condition);
      assert(data[0].number_type == number_type::array);
      
      int32_t ret = INT32_MAX;
      
      const size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<const script_data*>(data[0].data);
      {
        turn_off_function off(ctx);
        assert(ctx->itr_func == nullptr);
        ret = light_condition(target, ctx, block_size, block_data);
      }
      
      ASSERT(ret != INT32_MAX);
      
      if (ctx->itr_func != nullptr) {
        // иногда нам нужна возможность выключить это дело, можно использовать мутабле переменную
        // ctx у нас уникальный для каждого запуска функции, да и потом в параллельном вызове мы не будем это дело использовать
        // но это пол правды, так то нужно будет собирать данные для контекста
        const auto nil = target_t();
        call_lua_func(&target, ctx, &data[0], &nil, sol::nil, ret);
        light_condition_or(target, ctx, block_size, block_data);
      }
    }
    
    int32_t AND(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::condition_script_block);
      assert(data[0].number_type == number_type::array);
      assert(data[0].helper1 == condition_block_function::AND);
      
      size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      if (block_size == 0) return TRUE_BLOCK; // таких ситуаций по идее не должно быть
      
      int32_t ret = INT32_MAX;
      do {
        turn_off_function off(ctx);
        if (block_data[0].command_type == command_type::condition_script_block && block_data[0].helper1 == condition_block_function::condition) {
          assert(block_data[0].number_type == number_type::array);
          const uint32_t cond_size = block_data[0].value;
          auto cond_data = reinterpret_cast<const script_data*>(block_data[0].data);
          const int32_t cond_ret = light_condition(target, ctx, cond_size, cond_data);
          
          ret = cond_ret == FALSE_BLOCK ? IGNORE_BLOCK : ret;
          if (cond_ret == FALSE_BLOCK) break;
          block_size = block_size-1;
          block_data = &block_data[1];
        }
      
        ret = light_condition(target, ctx, block_size, block_data);
      } while (false);
      
      ASSERT(ret != INT32_MAX);
      // мы можем тут получить IGNORE_BLOCK, как правильно на это реагировать? отправить в ретюрн валью nil, видимо будет выглядеть так в итоге
      
      if (ctx->itr_func != nullptr) {
        size_t block_size = data[0].value;
        auto block_data = reinterpret_cast<script_data*>(data[0].data);
        const auto nil = target_t();
        call_lua_func(&target, ctx, &data[0], &nil, sol::nil, ret);
        light_condition_or(target, ctx, block_size, block_data);
      }
      
      return ret;
    }
    
    int32_t OR(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::condition_script_block);
      assert(data[0].number_type == number_type::array);
      assert(data[0].helper1 == condition_block_function::OR);
      
      size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      if (block_size == 0) return TRUE_BLOCK;
      
      int32_t ret = INT32_MAX;
      do {
        turn_off_function off(ctx);
        if (block_data[0].command_type == command_type::condition_script_block && block_data[0].helper1 == condition_block_function::condition) {
          assert(block_data[0].number_type == number_type::array);
          const uint32_t cond_size = block_data[0].value;
          auto cond_data = reinterpret_cast<const script_data*>(block_data[0].data);
          const int32_t cond_ret = light_condition(target, ctx, cond_size, cond_data);
          
          ret = cond_ret == FALSE_BLOCK ? IGNORE_BLOCK : ret;
          if (cond_ret == FALSE_BLOCK) break;
          block_size = block_size-1;
          block_data = &block_data[1];
        }
      
        ret = light_condition_or(target, ctx, block_size, block_data);
      } while (false);
      
      ASSERT(ret != INT32_MAX);
      // мы можем тут получить IGNORE_BLOCK, как правильно на это реагировать? отправить в ретюрн валью nil, видимо будет выглядеть так в итоге
      
      if (ctx->itr_func != nullptr) {
        size_t block_size = data[0].value;
        auto block_data = reinterpret_cast<script_data*>(data[0].data);
        const auto nil = target_t();
        call_lua_func(&target, ctx, &data[0], &nil, sol::nil, ret);
        light_condition_or(target, ctx, block_size, block_data);
      }
      
      return ret;
    }
    
    int32_t inv_block_return(const int32_t &ret) {
      return ret == TRUE_BLOCK ? FALSE_BLOCK : (ret == FALSE_BLOCK ? TRUE_BLOCK : ret);
    }
    
    int32_t NAND(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::condition_script_block);
      assert(data[0].number_type == number_type::array);
      assert(data[0].helper1 == condition_block_function::NAND);
      
      size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      if (block_size == 0) return TRUE_BLOCK;
      
      int32_t ret = INT32_MAX;
      do {
        turn_off_function off(ctx);
        if (block_data[0].command_type == command_type::condition_script_block && block_data[0].helper1 == condition_block_function::condition) {
          assert(block_data[0].number_type == number_type::array);
          const uint32_t cond_size = block_data[0].value;
          auto cond_data = reinterpret_cast<const script_data*>(block_data[0].data);
          const int32_t cond_ret = light_condition(target, ctx, cond_size, cond_data);
          
          ret = cond_ret == FALSE_BLOCK ? IGNORE_BLOCK : ret;
          if (cond_ret == FALSE_BLOCK) break;
          block_size = block_size-1;
          block_data = &block_data[1];
        }
      
        const int32_t block_ret = light_condition(target, ctx, block_size, block_data);
        ret = inv_block_return(block_ret);
      } while (false);
      
      ASSERT(ret != INT32_MAX);
      // мы можем тут получить IGNORE_BLOCK, как правильно на это реагировать? отправить в ретюрн валью nil, видимо будет выглядеть так в итоге
      
      if (ctx->itr_func != nullptr) {
        size_t block_size = data[0].value;
        auto block_data = reinterpret_cast<script_data*>(data[0].data);
        const auto nil = target_t();
        call_lua_func(&target, ctx, &data[0], &nil, sol::nil, ret);
        light_condition(target, ctx, block_size, block_data);
      }
      
      return ret;
    }
    
    int32_t NOR(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::condition_script_block);
      assert(data[0].number_type == number_type::array);
      assert(data[0].helper1 == condition_block_function::NOR);
      
      size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      if (block_size == 0) return TRUE_BLOCK;
      
      // для того чтобы получить осмысленную функцию, нам придется обойти кондишены
      int32_t ret = INT32_MAX;
      do {
        turn_off_function off(ctx);
        if (block_data[0].command_type == command_type::condition_script_block && block_data[0].helper1 == condition_block_function::condition) {
          assert(block_data[0].number_type == number_type::array);
          const uint32_t cond_size = block_data[0].value;
          auto cond_data = reinterpret_cast<const script_data*>(block_data[0].data);
          const int32_t cond_ret = light_condition(target, ctx, cond_size, cond_data);
          
          ret = cond_ret == FALSE_BLOCK ? IGNORE_BLOCK : ret;
          if (cond_ret == FALSE_BLOCK) break;
          block_size = block_size-1;
          block_data = &block_data[1];
        }
      
        const int32_t block_ret = light_condition_or(target, ctx, block_size, block_data);
        ret = inv_block_return(block_ret);
      } while (false);
      
      ASSERT(ret != INT32_MAX);
      // мы можем тут получить IGNORE_BLOCK, как правильно на это реагировать? отправить в ретюрн валью nil, видимо будет выглядеть так в итоге
      
      if (ctx->itr_func != nullptr) {
        size_t block_size = data[0].value;
        auto block_data = reinterpret_cast<script_data*>(data[0].data);
        const auto nil = target_t();
        call_lua_func(&target, ctx, &data[0], &nil, sol::nil, ret);
        light_condition_or(target, ctx, block_size, block_data);
      }
      
      return ret;
    }
    
    // по умолчанию мы не проверяем блок кондишона его не должно быть
    // как узнать кондишон блок? command.helper1 == block_function::condition
    int32_t action(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action_script_block);
      assert(data[0].number_type == number_type::array);
      
      const auto nil = target_t();
      call_lua_func(&target, ctx, &data[0], &nil, sol::nil, IGNORE_BLOCK);
      
      const size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<const script_data*>(data[0].data);
      return light_action(target, ctx, block_size, block_data);
    }
    
    // можно по идее тут добавить константу по которой мы будем делить то что происходит 
    // темплейт не получится, тому что нужен рантайм
    int32_t common_block(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &block_type) {
      // должны ли мы что то отправить в луа функцию? по идее нет, этот блок будем использовать как технический
      //call_lua_func<general_block_function::value>(ctx->itr_func, target, ctx, data[0], { UINT32_MAX, nullptr });
      assert(count == 1);
      assert(data[0].number_type == number_type::array);
      const size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<const script_data*>(data[0].data);
      if (block_type == CONDITION) return light_condition(target, ctx, block_size, block_data);
      return light_action(target, ctx, block_size, block_data);
    }
    
    // в условных штуках бессмысленно делать рандом
    int32_t random_vassal(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      // тут наверное приходит реалм
      assert(target.type == static_cast<uint32_t>(core::realm::s_type));
      assert(count == 1);
      assert(data[0].command_type == command_type::action_script_block);
      assert(data[0].helper1 == action_block_function::random_vassal);
      // откуда мы берем случайное число?
      auto realm = reinterpret_cast<core::realm*>(target.data);
      //const uint64_t num = character->get_random();
      
      if (data[0].number_type == number_type::lvalue) throw std::runtime_error("Cannot call random_vassal function as lvalue");
      
      // добавил токены и застрял с их реализацией, нужно ли мне всех вассалов оформлять как токены?
      // как обычно основная беда заключается в том что я могу захотеть сохранить указатель где нибудь в интерфейсе
      // но тут в скриптах у меня есть полный контроль над тем что я делаю, поэтому тут можно наверное и более опасно пользоваться тем что имею
      // мне и так и сяк нужно инвалидировать указатели на реалмы среди других реалмов после условного удаления
      // 
      
      // по идее этого не должно быть
      const size_t block_size = data[0].value;
      if (block_size == 0) return TRUE_BLOCK;
      // тут опять же проблема как в функции has_vassal
      if (realm->vassals == nullptr) {
        const auto nil = target_t();
        call_lua_func(&target, ctx, &data[0], &nil, sol::nil, IGNORE_BLOCK);
        return FALSE_BLOCK;
      }
      
      auto vassal = realm->vassals;
//       auto next_vassal = vassal != nullptr ? vassal->next_vassal : nullptr;
//       size_t vassals_count = size_t(vassal != nullptr);
      size_t vassals_count = 0;
      // вассалы вассалов здесь по идее не учитываются, поэтому 256 скорее всего достаточно
      // вообще не особ удачная идея все спихнуть в массив - это долго для случаев без условий
      // но с другой стороны функция будет запускаться редко
      const size_t maximum_vassals = 256;
      std::array<core::realm*, maximum_vassals> vassals_array = {};
      memset(vassals_array.data(), 0, vassals_array.size() * sizeof(vassals_array[0]));
//       vassals_array[0] = vassal;
//       while (vassal != next_vassal) {
//         vassals_array[vassals_count] = next_vassal;
//         ++vassals_count;
//         assert(vassals_count < maximum_vassals);
//         next_vassal = next_vassal->next_vassal;
//       }
      
      while (vassal != nullptr) {
        vassals_array[vassals_count] = vassal;
        ++vassals_count;
        assert(vassals_count < maximum_vassals);
        vassal = vassal->next_vassal;
      }
      
      assert(count == 1);
      assert(data[0].command_type == command_type::action_script_block);
      assert(data[0].number_type == number_type::array);
      
      auto block_data = reinterpret_cast<const script_data*>(data[0].data);
      
      target_t t;
      // проверяем условие а для этого было бы неплохо в массив сложить всех вассалов
      if (block_data[0].command_type == command_type::condition_script_block && block_data[0].helper1 == condition_block_function::condition) {
        sol::function* f = nullptr;
        std::swap(ctx->itr_func, f);
        const auto tmp = ctx->prev;
        ctx->prev = target;
        const size_t cond_size = block_data[0].value;
        auto cond_data = reinterpret_cast<const script_data*>(block_data[0].data);
        size_t vassals_count_tmp = vassals_count;
        while (vassals_count_tmp != 0) {
          const uint64_t num = ctx->rnd->next();
          size_t index = vassals_count * utils::rng_normalize(num);
          auto vassal = vassals_array[index];
          if (vassal == nullptr) continue;
          
          // тут я вообще могу найти токен, но нужен ли он мне? а вот при задании данных из вне
          // и при сохранении в скоуп токен обязателен для проверки
          // в struct target он в основном нужен для добавления в разные структуры 
          // и проверки валидности указателя на этапе получения из контекста
          target_t tmp(vassal);
          //const int32_t ret = func(tmp, ctx, 1, &block_data[0]);
          
          const int32_t ret = light_condition(tmp, ctx, cond_size, cond_data);
          if (ret == TRUE_BLOCK) {
            t = tmp;
            break;
          }
          
          --vassals_count_tmp;
          vassals_array[index] = nullptr;
        }
        
        std::swap(ctx->itr_func, f);
        // если у нас есть вассал подходящий по описанию, то мы данные этого блока можем вывести в луа функцию
        // и если нет тоже
        
        // тут нужно отправить и таргет и вассала
        call_lua_func(&target, ctx, &data[0], &t, sol::nil, IGNORE_BLOCK);
        
        if (t.data == nullptr) return IGNORE_BLOCK;
        
        const auto ret = light_action(t, ctx, block_size-1, &block_data[1]);
        ctx->prev = tmp;
        return ret;
      }
      
      // условия нет, берем любого вассала
      const uint64_t num = ctx->rnd->next();
      size_t index = vassals_count * utils::rng_normalize(num);
      auto current_vassal = vassals_array[index];
      assert(current_vassal != nullptr);
      
      t = target_t(current_vassal);
      
      call_lua_func(&target, ctx, &data[0], &t, sol::nil, IGNORE_BLOCK);
      const auto tmp = ctx->prev;
      ctx->prev = target;
      const auto ret = light_action(t, ctx, block_size, block_data);
      ctx->prev = tmp;
      return ret;
    }
    
    // кстати хороший вопрос как тут именно работает функция
    // нам поди нужно обойти условия в любом случае для того чтобы нарисовать их в интерфейсе
    int32_t has_vassal(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::condition_script_block);
      assert(data[0].helper1 == condition_block_function::has_vassal);
      assert(target.type == static_cast<uint32_t>(core::realm::s_type));
      assert(data[0].number_type == number_type::array);
      
      auto r = reinterpret_cast<core::realm*>(target.data);
      auto vassal = r->vassals;
      
      // здесь нужно вывести что мы хотя бы здесь были
      if (vassal == nullptr) {
        const auto nil = target_t();
        call_lua_func(&target, ctx, &data[0], &nil, sol::nil, FALSE_BLOCK);
        return FALSE_BLOCK;
      }
      
      const size_t array_count = data[0].value;
      auto array_data = reinterpret_cast<const script_data*>(data[0].data);
      
      core::realm* valid_vassal = nullptr;
      change_prev_target cpt(ctx, target);
      {
        turn_off_function off(ctx);
        while (vassal != nullptr && valid_vassal == nullptr) {
          const struct target_t t(vassal);
          const auto ret = light_condition(t, ctx, array_count, array_data);
          valid_vassal = ret == TRUE_BLOCK ? vassal : nullptr;
          
          vassal = vassal->next_vassal;
        }
      }
      
      const int32_t ret = valid_vassal != nullptr ? TRUE_BLOCK : FALSE_BLOCK;
      if (ctx->itr_func != nullptr) {
        const struct target_t nil;
        const struct target_t t(valid_vassal);
        const struct target_t final_t = valid_vassal != nullptr ? t : nil;
        call_lua_func(&target, ctx, &data[0], &final_t, sol::nil, ret);
        // тут обойдем кондишоны для верного вассала, а если нет верного вассала? тогда для первого?
        const struct target_t first(vassal);
        light_condition(valid_vassal != nullptr ? t : first, ctx, array_count, array_data);
      }
      
      return ret;
    }
    
    int32_t every_vassal(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action_script_block);
      assert(data[0].helper1 == action_block_function::every_vassal);
      assert(target.type == static_cast<uint32_t>(core::realm::s_type));
      assert(data[0].number_type == number_type::array);
      
      auto r = reinterpret_cast<core::realm*>(target.data);
      auto vassal = r->vassals;
      
      // здесь нужно вывести что мы хотя бы здесь были
      if (vassal == nullptr) {
        const auto nil = target_t();
        call_lua_func(&target, ctx, &data[0], &nil, sol::nil, IGNORE_BLOCK);
        return FALSE_BLOCK;
      }
      
      const size_t array_count = data[0].value;
      auto array_data = reinterpret_cast<const script_data*>(data[0].data);
      
      change_prev_target cpt(ctx, target);
      while (ctx->itr_func == nullptr && vassal != nullptr) {
        const struct target_t t = target_t(vassal);
        light_action(t, ctx, array_count, array_data);
        
        vassal = vassal->next_vassal;
      }
      
      // достаточно пройти лишь один раз для интерфейса
      if (ctx->itr_func != nullptr) {
        const struct target_t t = target_t(vassal);
        call_lua_func(&target, ctx, &data[0], &t, sol::nil, IGNORE_BLOCK);
        light_action(t, ctx, array_count, array_data);
      }
      
      return TRUE_BLOCK;
    }
    
    int32_t self_realm(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &block_type) {
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      assert(count == 1);
      assert(data[0].command_type == command_type::general_script_block);
      assert(data[0].helper1 == general_block_function::self_realm);
      auto character = reinterpret_cast<core::character*>(target.data);
      auto r = character->realms[core::character::self];
      
      const struct target_t nil;
      const struct target_t t(r);
      
      if (data[0].number_type == number_type::lvalue) {
        script_data val = r == nullptr ? nil : t;
        ctx->current_value = std::move(val);
#ifndef _NDEBUG
        if (r == nullptr) assert(ctx->current_value.helper2 == UINT16_MAX);
#endif
        return ctx->current_value.helper2 == UINT16_MAX ? FALSE_BLOCK : TRUE_BLOCK;
      }
      
      if (r == nullptr) {
        call_lua_func(&target, ctx, data, &nil, sol::nil, FALSE_BLOCK);
        return FALSE_BLOCK;
      }
      
      change_prev_target cpt(ctx, target);
      int32_t ret = INT32_MAX;
      switch (block_type) {
        case CONDITION: {
          {
            turn_off_function off(ctx);
            ret = common_block(t, ctx, 1, &data[0], block_type);
          }
          
          ASSERT(ret != INT32_MAX);
          
          if (ctx->itr_func != nullptr) {
            call_lua_func(&target, ctx, data, &t, sol::nil, ret);
            common_block(t, ctx, 1, &data[0], block_type);
          }
          
          break;
        }
        
        case ACTION: {
          call_lua_func(&target, ctx, data, &t, sol::nil, IGNORE_BLOCK);
          ret = common_block(t, ctx, 1, &data[0], block_type);
          break;
        }
        
        default: assert(false);
      }
      
      ctx->current_value = script_data(t); // если r == nullptr, то мы должны как то задать ошибку
      return ret;
    }
    
    int32_t prev(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &block_type) {
      assert(count == 1);
      assert(data[0].command_type == command_type::general_script_block);
      assert(data[0].helper1 == general_block_function::prev);
      
      const struct target_t t = ctx->prev;
      
      if (data[0].number_type == number_type::lvalue) {
        const struct target_t nil;
        script_data val = t.data == nullptr ? nil : t;
        ctx->current_value = std::move(val);
#ifndef _NDEBUG
        if (t.data == nullptr) assert(ctx->current_value.helper2 == UINT16_MAX);
#endif
        return ctx->current_value.helper2 == UINT16_MAX ? FALSE_BLOCK : TRUE_BLOCK;
      }
      
      if (t.data == nullptr) {
        const target_t nil;
        call_lua_func(&target, ctx, data, &nil, sol::nil, FALSE_BLOCK);
        return FALSE_BLOCK;
      }
      
      change_prev_target cpt(ctx, target);
      int32_t ret = INT32_MAX;
      switch (block_type) {
        case CONDITION: {
          {
            turn_off_function off(ctx);
            ret = common_block(t, ctx, 1, &data[0], block_type);
          }
          
          ASSERT(ret != INT32_MAX);
          
          if (ctx->itr_func != nullptr) {
            call_lua_func(&target, ctx, data, &t, sol::nil, ret);
            common_block(t, ctx, 1, &data[0], block_type);
          }
          
          break;
        }
        
        case ACTION: {
          call_lua_func(&target, ctx, data, &t, sol::nil, IGNORE_BLOCK);
          ret = common_block(t, ctx, 1, &data[0], block_type);
          break;
        }
        
        default: assert(false);
      }
      
      ctx->current_value = script_data(t);
      return ret;
    }
    
    int32_t root(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &block_type) {
      assert(count == 1);
      assert(data[0].command_type == command_type::general_script_block);
      assert(data[0].helper1 == general_block_function::root);
      
      //const struct target t = {ctx->array_data[0].helper2, ctx->array_data[0].data};
      const struct target_t t = ctx->root;
      assert(t.data != nullptr);
      
      if (data[0].number_type == number_type::lvalue) {
        script_data val = t;
        ctx->current_value = std::move(val);
        return TRUE_BLOCK;
      }
      
      change_prev_target cpt(ctx, target);
      int32_t ret = INT32_MAX;
      switch (block_type) {
        case CONDITION: {
          {
            turn_off_function off(ctx);
            ret = common_block(t, ctx, 1, &data[0], block_type);
          }
          
          ASSERT(ret != INT32_MAX);
          
          if (ctx->itr_func != nullptr) {
            call_lua_func(&target, ctx, data, &t, sol::nil, ret);
            common_block(t, ctx, 1, &data[0], block_type);
          }
          
          break;
        }
        
        case ACTION: {
          call_lua_func(&target, ctx, data, &t, sol::nil, IGNORE_BLOCK);
          ret = common_block(t, ctx, 1, &data[0], block_type);
          break;
        }
        
        default: assert(false);
      }
      
      ctx->current_value = script_data(t);
      return ret;
    }
    
    int32_t current(const struct target_t &target, context* ctx, const uint32_t &count, const script_data* data, const uint32_t &block_type) {
      assert(count == 1);
      assert(data[0].command_type == command_type::general_script_block);
      assert(data[0].helper1 == general_block_function::current);
      
      if (data[0].number_type == number_type::lvalue) {
        script_data val = target;
        ctx->current_value = std::move(val);
        return TRUE_BLOCK;
      }
      
      const target_t nil;
      int32_t ret = INT32_MAX;
      switch (block_type) {
        case CONDITION: {
          {
            turn_off_function off(ctx);
            ret = common_block(target, ctx, 1, &data[0], block_type);
          }
          
          ASSERT(ret != INT32_MAX);
          
          if (ctx->itr_func != nullptr) {
            call_lua_func(&target, ctx, data, &nil, sol::nil, ret);
            common_block(target, ctx, 1, &data[0], block_type);
          }
          
          break;
        }
        
        case ACTION: {
          call_lua_func(&target, ctx, data, &nil, sol::nil, IGNORE_BLOCK);
          ret = common_block(target, ctx, 1, &data[0], block_type);
          break;
        }
        
        default: assert(false);
      }
      
      ctx->current_value = script_data(target);
      return ret;
    }
    
    
/* =============================================    
                  START INIT
   ============================================= */
    

    bool check_valid_rvalue(const std::string_view &str) {
      const bool has_colon = RE2::PartialMatch(str, regex_colon_matcher);
      const bool has_dot = RE2::PartialMatch(str, regex_dot_matcher);
      return has_colon || has_dot;
    }
    
    void rvalue_script_block_init(const uint32_t &target_data, const std::string_view &str, const sol::object &obj, script_data* data, const uint32_t &block_type) {
      if (obj.get_type() != sol::type::table) throw std::runtime_error("Lua table is only allowed data for rvalue " + std::string(str));
      
      data->command_type = command_type::rvalue;
      data->number_type = number_type::array;
      data->value = 2;
      auto rvalue_data = new script_data[2];
      data->data = rvalue_data;
      
      const bool has_colon = RE2::PartialMatch(str, regex_colon_matcher);
      make_lvalue(str, has_colon, &rvalue_data[0]);
      common_block_init(target_data, obj, &rvalue_data[1], block_type);
    }
    
    void find_condition_block(const uint32_t &target_data, const sol::table &input, std::vector<script_data> &datas) {
      assert(datas.empty());
      const auto str = magic_enum::enum_name(condition_block_function::condition);
      for (const auto &pair : input) {
        if (pair.first.get_type() != sol::type::string) continue;
        const auto key = pair.first.as<std::string_view>();
        if (key != str) continue;
        
        datas.emplace_back();
        assert(pair.second.get_type() == sol::type::table);
        condition_init(target_data, pair.second, &datas.back());
        break;
      }
    }
    
    void find_condition_block_and_throw(const uint16_t &cond_func_index, const sol::table &input) {
      const auto str = magic_enum::enum_name(condition_block_function::condition);
      for (const auto &pair : input) {
        if (pair.first.get_type() != sol::type::string) continue;
        const auto key = pair.first.as<std::string_view>();
        if (key != str) continue;
        
        throw std::runtime_error("Could not make nested condition block in " + std::string(magic_enum::enum_name(static_cast<condition_block_function::values>(cond_func_index))));
      }
    }
    
    void condition_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      if (obj.get_type() != sol::type::table) throw std::runtime_error("Bad input data for condition block");
      
      data->command_type = command_type::condition_script_block;
      data->number_type = number_type::array;
      if (data->helper1 == UINT16_MAX) data->helper1 = condition_block_function::condition;
      
      std::vector<script_data> datas;
      const sol::table input = obj.as<sol::table>();
      find_condition_block_and_throw(data->helper1, input); // кондитион в кондитион не должно быть
      
      for (const auto &pair : input) {
        // тут должны быть в качестве id строки
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (const auto e = magic_enum::enum_cast<condition_function::values>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = condition_init_functions[index];
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<condition_block_function::values>(str); e) {
            const auto index = e.value();
            if (index == condition_block_function::condition) continue;
            
            datas.emplace_back();
            const auto func = condition_block_init_functions[index];
            assert(pair.second.get_type() == sol::type::table);
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<general_block_function::values>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = general_block_init_functions[index];
            assert(pair.second.get_type() == sol::type::table);
            func(target_data, pair.second, &datas.back(), CONDITION);
            continue;
          }
          
          if (check_valid_rvalue(str)) {
            datas.emplace_back();
            rvalue_script_block_init(target_data, str, pair.second, &datas.back(), CONDITION);
            continue;
          }
          
          throw std::runtime_error("Could not parse script key " + std::string(str));
        }
        
        if (pair.first.get_type() == sol::type::number) {
          // тут по идее должны быть строго таблицы
          assert(pair.second.get_type() == sol::type::table);
          if (pair.second.as<sol::table>().empty()) continue;
          datas.emplace_back();
          AND_init(target_data, pair.second, &datas.back());
          continue;
        }
        
        throw std::runtime_error("Bad script function type");
      }
      
      data->value = 0;
      //assert(!datas.empty());
      if (datas.empty()) return; // может быть пусто
      
      data->value = datas.size();
      auto ptr = new script_data[datas.size()];
      data->data = ptr;
      for (size_t i = 0; i < datas.size(); ++i) {
        ptr[i] = std::move(datas[i]);
      }
    }
    
    void main_condition_init(const uint32_t &target_data, const sol::object &obj, script_data* data, const function_init_p default_block_func) {
      if (obj.get_type() != sol::type::table) throw std::runtime_error("Bad input data for condition block");
      
      data->command_type = command_type::condition_script_block;
      data->number_type = number_type::array;
      if (data->helper1 == UINT16_MAX) data->helper1 = condition_block_function::AND;
      
      // вообще то нам нужно инициализировать все функции по какой то иерархии
      // например кондитион блок должен быть всегда первым
      // если кондитион блок обязан быть первым это правило, то иерархия не обязательна 
      // и можно оставить пока что случайный порядок
      
      const auto cond_name = magic_enum::enum_name(condition_block_function::condition);
      
      std::vector<script_data> datas;
      const sol::table input = obj.as<sol::table>();
      find_condition_block(target_data, input, datas);
      
      for (const auto &pair : input) {
        // тут должны быть в качестве id строки
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          
          if (const auto e = magic_enum::enum_cast<condition_function::values>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = condition_init_functions[index];
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<condition_block_function::values>(str); e) {
            assert(pair.second.get_type() == sol::type::table);
            const auto index = e.value();
            if (index == condition_block_function::condition) continue;
            
            datas.emplace_back();
            const auto func = condition_block_init_functions[index];
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<general_block_function::values>(str); e) {
            assert(pair.second.get_type() == sol::type::table);
            datas.emplace_back();
            const auto index = e.value();
            const auto func = general_block_init_functions[index];
            func(target_data, pair.second, &datas.back(), CONDITION);
            continue;
          }
          
          // rvalue, это строка такая же как и лвалуе
          // может ли это быть таблица? это по крайней мере неудобно
          if (check_valid_rvalue(str)) {
            datas.emplace_back();
            rvalue_script_block_init(target_data, str, pair.second, &datas.back(), CONDITION);
            continue;
          }
          
          throw std::runtime_error("Could not parse script rtoken " + std::string(str));
        }
        
        if (pair.first.get_type() == sol::type::number) {
          // тут по идее должны быть строго таблицы
          assert(pair.second.get_type() == sol::type::table);
          // у меня может быть пустая таблица, в этом случае нужно проигнорировать эту конструкцию
          if (pair.second.as<sol::table>().empty()) continue;
          
          datas.emplace_back();
          (*default_block_func)(target_data, pair.second, &datas.back());
          continue;
        }
        
        throw std::runtime_error("Bad script function type");
      }
      
      data->value = 0;
      //assert(!datas.empty());
      if (datas.empty()) return; // может быть пусто
      
      data->value = datas.size();
      auto ptr = new script_data[datas.size()];
      data->data = ptr;
      for (size_t i = 0; i < datas.size(); ++i) {
        ptr[i] = std::move(datas[i]);
      }
    }
    
    void AND_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      data->helper1 = condition_block_function::AND;
      main_condition_init(target_data, obj, data, AND_init);
    }
    
    void OR_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      data->helper1 = condition_block_function::OR;
      main_condition_init(target_data, obj, data, OR_init);
    }
    
    void NAND_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      data->helper1 = condition_block_function::NAND;
      main_condition_init(target_data, obj, data, NAND_init);
    }
    
    void NOR_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      data->helper1 = condition_block_function::NOR;
      main_condition_init(target_data, obj, data, NOR_init);
    }
    
    void block_default_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      
    }
    
    void block_or_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      
    }
    
    void action_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      if (obj.get_type() != sol::type::table) throw std::runtime_error("Bad input data for condition block");
      
      data->command_type = command_type::action_script_block;
      data->number_type = number_type::array;
      if (data->helper1 == UINT16_MAX) data->helper1 = action_block_function::action;
      
      const auto cond_name = magic_enum::enum_name(condition_block_function::condition);
      
      std::vector<script_data> datas;
      const sol::table input = obj.as<sol::table>();
      find_condition_block(target_data, input, datas);
      
      for (const auto &pair : input) {
        // тут должны быть в качестве id строки
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (const auto e = magic_enum::enum_cast<action_function::values>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = action_init_functions[index];
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<action_block_function::values>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = action_block_init_functions[index];
            assert(pair.second.get_type() == sol::type::table);
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<general_block_function::values>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = general_block_init_functions[index];
            assert(pair.second.get_type() == sol::type::table);
            func(target_data, pair.second, &datas.back(), ACTION);
            continue;
          }
          
          if (str == cond_name) continue;
          
          if (check_valid_rvalue(str)) {
            datas.emplace_back();
            rvalue_script_block_init(target_data, str, pair.second, &datas.back(), ACTION);
            continue;
          }
          
          throw std::runtime_error("Could not parse script key " + std::string(str));
        }
        
        if (pair.first.get_type() == sol::type::number) {
          // тут по идее должны быть строго таблицы
          assert(pair.second.get_type() == sol::type::table);
          if (pair.second.as<sol::table>().empty()) continue;
          datas.emplace_back();
          action_init(target_data, pair.second, &datas.back());
          continue;
        }
        
        throw std::runtime_error("Bad script function type");
      }
      
      data->value = 0;
      //assert(!datas.empty());
      if (datas.empty()) return; // может быть пусто
      
      //PRINT_VAR("acti datas size", datas.size())
      data->value = datas.size();
      auto ptr = new script_data[datas.size()];
      data->data = ptr;
      for (size_t i = 0; i < datas.size(); ++i) {
        ptr[i] = std::move(datas[i]);
      }
    }
    
    void random_vassal_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      // таргет дата бывает сложно определить если приходится вычислять rvalue, нужно добавить специальное значение для этого
      if (target_data != UINT32_MAX && target_data != static_cast<uint32_t>(core::realm::s_type)) throw std::runtime_error("random_vassal can be taken only from realm");
      // тут должен быть только экшон
      data->helper1 = action_block_function::random_vassal;
      action_init(static_cast<uint32_t>(core::realm::s_type), obj, data);
    }
    
    void has_vassal_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      if (target_data != UINT32_MAX && target_data != static_cast<uint32_t>(core::realm::s_type)) throw std::runtime_error("random_vassal can be taken only from realm");
      data->helper1 = condition_block_function::has_vassal;
      condition_init(static_cast<uint32_t>(core::realm::s_type), obj, data);
    }
    
    void every_vassal_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      if (target_data != UINT32_MAX && target_data != static_cast<uint32_t>(core::realm::s_type)) throw std::runtime_error("random_vassal can be taken only from realm");
      data->helper1 = action_block_function::every_vassal;
      action_init(static_cast<uint32_t>(core::realm::s_type), obj, data);
    }
    
    void common_block_init(const uint32_t &target_data, const sol::object &obj, script_data* data, const uint32_t &block_type) {
      if (obj.get_type() != sol::type::table) throw std::runtime_error("Bad input data for condition block");
      
      data->command_type = command_type::general_script_block;
      data->number_type = number_type::array;
      if (data->helper1 == UINT16_MAX) data->helper1 = general_block_function::common_block;
      
      std::vector<script_data> datas;
      const sol::table input = obj.as<sol::table>();
      find_condition_block(target_data, input, datas);
      
      for (const auto &pair : input) {
        // тут должны быть в качестве id строки
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (const auto e = magic_enum::enum_cast<action_function::values>(str); e) {
            if (block_type != ACTION) throw std::runtime_error("Could not initialize condition script block with effect function " + std::string(str));
            datas.emplace_back();
            const auto index = e.value();
            const auto func = action_init_functions[index];
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<action_block_function::values>(str); e) {
            if (block_type != ACTION) throw std::runtime_error("Could not initialize condition script block with effect function " + std::string(str));
            datas.emplace_back();
            const auto index = e.value();
            const auto func = action_block_init_functions[index];
            assert(pair.second.get_type() == sol::type::table);
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<condition_function::values>(str); e) {
            if (block_type != CONDITION) throw std::runtime_error("Could not initialize effect script block with condition function " + std::string(str));
            datas.emplace_back();
            const auto index = e.value();
            const auto func = condition_init_functions[index];
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<condition_block_function::values>(str); e) {
            const auto index = e.value();
            if (index == condition_block_function::condition) continue;
            if (block_type != CONDITION) throw std::runtime_error("Could not initialize effect script block with condition function " + std::string(str));
            datas.emplace_back();
            const auto func = condition_block_init_functions[index];
            assert(pair.second.get_type() == sol::type::table);
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<general_block_function::values>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = general_block_init_functions[index];
            assert(pair.second.get_type() == sol::type::table);
            func(target_data, pair.second, &datas.back(), block_type);
            continue;
          }
          
          // нужно еще проверить rvalue, то есть примерно та же строчка, что и для lvalue
          if (check_valid_rvalue(str)) {
            datas.emplace_back();
            rvalue_script_block_init(target_data, str, pair.second, &datas.back(), block_type);
            continue;
          }
          
          throw std::runtime_error("Could not parse script key " + std::string(str));
        }
        
        if (pair.first.get_type() == sol::type::number) {
          // тут по идее должны быть строго таблицы
          assert(pair.second.get_type() == sol::type::table);
          if (pair.second.as<sol::table>().empty()) continue;
          datas.emplace_back();
          common_block_init(target_data, pair.second, &datas.back(), block_type);
          continue;
        }
        
        throw std::runtime_error("Bad script function type");
      }
      
      data->value = 0;
      //assert(!datas.empty());
      if (datas.empty()) return; // может быть пусто
      
      //PRINT_VAR("acti datas size", datas.size())
      data->value = datas.size();
      auto ptr = new script_data[datas.size()];
      data->data = ptr;
      for (size_t i = 0; i < datas.size(); ++i) {
        ptr[i] = std::move(datas[i]);
      }
    }
    
    void self_realm_init(const uint32_t &target_data, const sol::object &obj, script_data* data, const uint32_t &block_type) {
      if (target_data != UINT32_MAX && target_data != static_cast<uint32_t>(core::character::s_type)) throw std::runtime_error("self_realm can be taken only from character");
      data->helper1 = general_block_function::self_realm;
      common_block_init(static_cast<uint32_t>(core::realm::s_type), obj, data, block_type);
    }
    
    void prev_init(const uint32_t &target_data, const sol::object &obj, script_data* data, const uint32_t &block_type) {
      data->helper1 = general_block_function::prev;
      common_block_init(target_data, obj, data, block_type);
    }
    
    void root_init(const uint32_t &target_data, const sol::object &obj, script_data* data, const uint32_t &block_type) {
      data->helper1 = general_block_function::root;
      common_block_init(target_data, obj, data, block_type);
    }
    
    void current_init(const uint32_t &target_data, const sol::object &obj, script_data* data, const uint32_t &block_type) {
      data->helper1 = general_block_function::current;
      common_block_init(target_data, obj, data, block_type);
    }
  }
}



// по идее мы взяли рандомного вассала
// с другой стороны нас скорее всего будет интересовать уже скомпилированный скрипт
// где рандомный вассал уже будет известен, да и вообще все данные уже будут известны
// иначе вассал будет при каждом обращении разный
// как быть? на самом деле все что нужно это создать массив куда положить все данные
// с третьей стороны я могу легко обойти с небольшими изменениями условия
// и рандомного вассала не нужно использовать в условиях
// в условиях должен быть "если есть хотя бы один вассал"
// какие изменения нужно сделать? тут на самом деле все довольно просто
// блочные функции должны принимать функцию луа, и в эту функцию передавать
// что запустили, с какими входными данными и какой результат
// осталось только понять как сохранить рандомного вассала, по большому счету мне нужно число
// мне нужно обойти решение или эвент заранее ... хотя я чет думаю что в решениях я не найду
// рандомного юнита, если это так то это все значительно упрощает, нашел рандом со 100% шансом,
// не то что я хотел, но вот я что подумал, как вообще работает показ решения игроку или ии,
// мы нажимаем правой кнопкой и начинаем считать условия? а не долго ли?
// нужно как то по группам разделить, и тогда будет полегче с выводом десижонов
// самые очевидные группы это религия, м/ж, тюряга
// для того чтобы рандомность поддержать завесте структуру маленькую
// из которой мы берем следующее число, так как у нас строго задана последовательность
// вызовов, где то рядом можно будет положить функцию луа и 
// видимо нужно сделать версию без примения эффектов

// скорее всего у меня должна быть функция которая вернут список решений для таргета
// например void get_decisions(char, target), где char - это текущий персонаж интерфейса,
// target - то к чему мы пытаемся получить десижоны

// есть необходимость посчитать число с помощью скрипта, примерно как это было в цк
// то есть для всех функций которые принимают или число или строку мы можем указать таблицу
// в которой есть некие модификаторы + функция value (или base?) 
// которая задает значение для функции в зависимости от модификаторов, эти модификаторы
// могут изменять значение функции в зависимости от условий, может получится сильно вложенная
// структура - никаких проблем, добавятся функции value, factor, add, 
// модификатор - это таблицы через запятую, должен быть указан изначальный value
