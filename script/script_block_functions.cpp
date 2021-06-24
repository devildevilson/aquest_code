#include "script_block_functions.h"

//#include "bin/core_structures.h"
#include "core/structures_header.h"

namespace devils_engine {
  namespace script {
    template <typename T>
    void call_lua_func(const sol::function *f, const struct target &t, const context &ctx, const script_data &data, const struct target &new_t) {
      if (f == nullptr) return;
      
      const auto &func = *f;
      const sol::state_view state = func.lua_state();
      // нужно передать название
      const auto name = magic_enum::enum_name(static_cast<T>(data.helper1));
      // нужно отправить таргета
      // возможно нужно передать данные, какие? размер массива или число или указатель на объект
      // причем нужно отправить вычисленное значение? к вычесленному значению нужно отправить 
      // тип сравнения и дополнительный тип переменной (например указать что это месячный инком)
      sol::object target_obj;
      sol::object new_target_obj;
      // желательно отдельный таргет тайп, с другой стороны таргет всегда объект мира
      switch (static_cast<core::structure>(t.type)) {
        // откуда брать луа стейт? должен быть видимо в контексте
        case core::structure::army: target_obj = sol::make_object(state, reinterpret_cast<core::army*>(t.data)); break;
        case core::structure::character: target_obj = sol::make_object(state, reinterpret_cast<core::character*>(t.data)); break;
        case core::structure::city: target_obj = sol::make_object(state, reinterpret_cast<core::city*>(t.data)); break;
        case core::structure::province: target_obj = sol::make_object(state, reinterpret_cast<core::province*>(t.data)); break;
        case core::structure::realm: target_obj = sol::make_object(state, reinterpret_cast<core::realm*>(t.data)); break;
        case core::structure::titulus: target_obj = sol::make_object(state, reinterpret_cast<core::titulus*>(t.data)); break;
        default: throw std::runtime_error("Not implemented yet");
      }
      
      switch (static_cast<core::structure>(new_t.type)) {
        // откуда брать луа стейт? должен быть видимо в контексте
        case core::structure::army: new_target_obj = sol::make_object(state, reinterpret_cast<core::army*>(new_t.data)); break;
        case core::structure::character: new_target_obj = sol::make_object(state, reinterpret_cast<core::character*>(new_t.data)); break;
        case core::structure::city: new_target_obj = sol::make_object(state, reinterpret_cast<core::city*>(new_t.data)); break;
        case core::structure::province: new_target_obj = sol::make_object(state, reinterpret_cast<core::province*>(new_t.data)); break;
        case core::structure::realm: new_target_obj = sol::make_object(state, reinterpret_cast<core::realm*>(new_t.data)); break;
        case core::structure::titulus: new_target_obj = sol::make_object(state, reinterpret_cast<core::titulus*>(new_t.data)); break;
        default: break;
      }
      // нужно еще передать результат сравнения
      if (new_t.data == nullptr) func(name, target_obj, data.value, sol::nil, sol::nil, sol::nil);
      else func(name, target_obj, data.value, sol::nil, sol::nil, new_target_obj);
    }
    
    int32_t light_condition(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      // тут мы должны обойти массив скриптовых данных
      // и выйти как только получим false
      assert(count == 1);
      assert(data[0].command_type == command_type::condition_script_block);
      assert(data[0].number_type == number_type::array);
      
      const size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      for (size_t i = 0; i < block_size; ++i) {
        const auto &command = block_data[i];
        assert(command.command_type == command_type::condition || command.command_type == command_type::condition_script_block);
        const uint32_t func_index = command.helper1;
        
        switch (command.command_type) {
          case command_type::condition: {
            const auto func = condition_functions[func_index];
            // имеет ли смысл тут передавать количество другое нежели чем 1?
            // неуверен
            const bool ret = func(target, ctx, 1, &command);
            if (!ret) return FALSE_BLOCK;
            
            break;
          }
          
          case command_type::condition_script_block: {
            const auto func = condition_block_functions[func_index];
            const int32_t ret = func(target, ctx, 1, &command);
            // может прийти IGNORE_BLOCK
            if (ret == FALSE_BLOCK) return FALSE_BLOCK;
            
            break;
          }
          
          default: assert(false);
        }
        
      }
      
      return TRUE_BLOCK;
    }
    
    // функция оказась проще чем я думал
    int32_t condition(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      // тут мы должны обойти массив скриптовых данных
      // и выйти как только получим false
      assert(count == 1);
      assert(data[0].command_type == command_type::condition_script_block);
      assert(data[0].number_type == number_type::array);
      
      // иногда нам нужна возможность выключить это дело, можно использовать мутабле переменную
      // ctx у нас уникальный для каждого запуска функции, да и потом в параллельном вызове мы не будем это дело использовать
      // но это пол правды, так то нужно будет собирать данные для контекста
      call_lua_func<condition_block_function::value>(ctx.itr_func, target, ctx, data[0], { UINT32_MAX, nullptr });
      
      return light_condition(target, ctx, count, data);
    }
    
    // по умолчанию мы не проверяем блок кондишона его не должно быть
    // как узнать кондишон блок? command.helper1 == block_function::condition
    int32_t action(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action_script_block);
      assert(data[0].number_type == number_type::array);
      
      call_lua_func<action_block_function::value>(ctx.itr_func, target, ctx, data[0], { UINT32_MAX, nullptr });
      
      const size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      for (size_t i = 0; i < block_size; ++i) {
        const auto &command = block_data[i];
        assert(command.command_type == command_type::action || command.command_type == command_type::action_script_block);
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
            // тут по идее не нужен ret
            (void)ret;
            break;
          }
          
          default: assert(false);
        }
      }
      
      return TRUE_BLOCK;
    }
    
    int32_t condition_or(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::condition_script_block);
      assert(data[0].number_type == number_type::array);
      const size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      for (size_t i = 0; i < block_size; ++i) {
        const auto &command = block_data[i];
        assert(command.command_type == command_type::condition || command.command_type == command_type::condition_script_block);
        const uint32_t func_index = command.helper1;
        switch (command.command_type) {
          case command_type::condition: {
            const auto func = condition_functions[func_index];
            // имеет ли смысл тут передавать количество другое нежели чем 1?
            // неуверен
            const bool ret = func(target, ctx, 1, &command);
            if (ret) return TRUE_BLOCK;
            
            break;
          }
          
          case command_type::condition_script_block: {
            const auto func = condition_block_functions[func_index];
            const int32_t ret = func(target, ctx, 1, &command);
            if (ret == TRUE_BLOCK) return TRUE_BLOCK;
            
            break;
          }
          
          default: assert(false);
        }
      }
      
      return FALSE_BLOCK;
    }
    
    int32_t common_block(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action_script_block || data[0].command_type == command_type::condition_script_block);
      assert(data[0].number_type == number_type::array);
      const size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      for (size_t i = 0; i < block_size; ++i) {
        const auto &command = block_data[i];
        const uint32_t func_index = command.helper1;
        switch (command.command_type) {
          case command_type::condition: {
            const auto func = condition_functions[func_index];
            // имеет ли смысл тут передавать количество другое нежели чем 1?
            // неуверен
            const bool ret = func(target, ctx, 1, &command);
            if (ret) return TRUE_BLOCK;
            
            break;
          }
          
          case command_type::action: {
            const auto func = action_functions[func_index];
            func(target, ctx, 1, &command);
            break;
          }
          
          case command_type::action_script_block: {
            const auto func = action_block_functions[func_index];
            const int32_t ret = func(target, ctx, 1, &command);
            if (ret == TRUE_BLOCK) return TRUE_BLOCK;
            
            break;
          }
          
          case command_type::condition_script_block: {
            const auto func = condition_block_functions[func_index];
            const int32_t ret = func(target, ctx, 1, &command);
            if (ret == TRUE_BLOCK) return TRUE_BLOCK;
            
            break;
          }
          
          default: assert(false);
        }
      }
      
      return FALSE_BLOCK;
    }
    
    int32_t block_default(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action_script_block || data[0].command_type == command_type::condition_script_block);
      assert(data[0].number_type == number_type::array);
      const size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      
      // в этом случае сначала нужно найти блок кондишона
      // он должен быть первым
      if (block_size == 0) return TRUE_BLOCK;
      
      if (block_data[0].command_type == command_type::condition_script_block && block_data[0].helper1 == condition_block_function::condition) {
        const uint32_t func_index = block_data[0].helper1;
        const auto func = condition_block_functions[func_index];
        const int32_t ret = func(target, ctx, 1, &block_data[0]);
        // если здесь false то нам нужно вернуть особое состояние, по которому мы сможем проигнорировать текущий блок скрипта
        // нужно в таком случае вернуть int32_t, по идее имеет смысл возвращать состояния только в блоках скриптов
        if (ret == FALSE_BLOCK) return IGNORE_BLOCK;
      }
      
      return condition(target, ctx, block_size-1, &block_data[1]);
    }
    
    int32_t block_or(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(count == 1);
      assert(data[0].command_type == command_type::action_script_block || data[0].command_type == command_type::condition_script_block);
      assert(data[0].number_type == number_type::array);
      const size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      
      // в этом случае сначала нужно найти блок кондишона
      // он должен быть первым
      if (block_size == 0) return TRUE_BLOCK;
      
      if (block_data[0].command_type == command_type::condition_script_block && block_data[0].helper1 == condition_block_function::condition) {
        const uint32_t func_index = block_data[0].helper1;
        const auto func = condition_block_functions[func_index];
        const int32_t ret = func(target, ctx, 1, &block_data[0]);
        // если здесь false то нам нужно вернуть особое состояние, по которому мы сможем проигнорировать текущий блок скрипта
        // нужно в таком случае вернуть int32_t, по идее имеет смысл возвращать состояния только в блоках скриптов
        if (ret == FALSE_BLOCK) return IGNORE_BLOCK;
      }
      
      return condition_or(target, ctx, block_size-1, &block_data[1]);
    }
    
    // в условных штуках бессмысленно делать рандом
    int32_t random_vassal(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      // тут наверное приходит реалм
      assert(target.type == static_cast<uint32_t>(core::realm::s_type));
      // откуда мы берем случайное число?
      auto realm = reinterpret_cast<core::realm*>(target.data);
      //const uint64_t num = character->get_random();
      
      auto vassal = realm->vassals;
      auto next_vassal = vassal != nullptr ? vassal->next_vassal : nullptr;
      size_t vassals_count = size_t(vassal != nullptr);
      std::array<core::realm*, 256> vassals_array;
      memset(vassals_array.data(), 0, vassals_array.size() * sizeof(vassals_array[0]));
      vassals_array[0] = vassal;
      while (vassal != next_vassal) {
        vassals_array[vassals_count] = next_vassal;
        ++vassals_count;
        assert(vassals_count < 256);
        next_vassal = next_vassal->next_vassal;
      }
      
      assert(count == 1);
      assert(data[0].command_type == command_type::action_script_block);
      assert(data[0].number_type == number_type::array);
      
      const size_t block_size = data[0].value;
      auto block_data = reinterpret_cast<script_data*>(data[0].data);
      if (block_size == 0) return TRUE_BLOCK;
      
      struct target t = {static_cast<uint32_t>(core::realm::s_type), nullptr};
      // проверяем условие а для этого было бы неплохо в массив сложить всех вассалов
      if (block_data[0].command_type == command_type::condition_script_block && block_data[0].helper1 == condition_block_function::condition) {
        const uint32_t func_index = block_data[0].helper1;
        const auto func = condition_block_functions[func_index];
        size_t vassals_count_tmp = vassals_count;
        while (vassals_count_tmp != 0) {
          const uint64_t num = ctx.rnd->next();
          size_t index = vassals_count * utils::rng_normalize(num);
          auto vassal = vassals_array[index];
          if (vassal == nullptr) continue;
          
          struct target tmp = {static_cast<uint32_t>(core::realm::s_type), vassal};
          //const int32_t ret = func(tmp, ctx, 1, &block_data[0]);
          const int32_t ret = light_condition(tmp, ctx, 1, &block_data[0]);
          if (ret == TRUE_BLOCK) {
            t = tmp;
            break;
          }
          
          --vassals_count_tmp;
          vassals_array[index] = nullptr;
        }
        
        // если у нас есть вассал подходящий по описанию, то мы данные этого блока можем вывести в луа функцию
        // и если нет тоже
        
        // тут нужно отправить и таргет и вассала
        call_lua_func<action_block_function::value>(ctx.itr_func, target, ctx, data[0], t);
        
        if (t.data == nullptr) return IGNORE_BLOCK;
      } else {
        // условия нет, берем любого вассала
        const uint64_t num = ctx.rnd->next();
        size_t index = vassals_count * utils::rng_normalize(num);
        auto vassal = vassals_array[index];
        assert(vassal != nullptr);
        
        t = {static_cast<uint32_t>(core::realm::s_type), vassal};
        
        call_lua_func<action_block_function::value>(ctx.itr_func, target, ctx, data[0], t);
      }
      
      return action(t, ctx, block_size-1, &block_data[1]);
    }
    
    int32_t self_realm(const struct target &target, const context &ctx, const uint32_t &count, const script_data* data) {
      assert(target.type == static_cast<uint32_t>(core::character::s_type));
      auto character = reinterpret_cast<core::character*>(target.data);
      auto r = character->realms[core::character::self];
      if (r == nullptr) throw std::runtime_error("Character does not have a self realm");
      
      const struct target t = {static_cast<uint32_t>(core::realm::s_type), r};
      return common_block(t, ctx, count, data);
    }
    
    void condition_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      if (obj.get_type() != sol::type::table) throw std::runtime_error("Bad input data for condition block");
      
      data->command_type = command_type::condition_script_block;
      data->number_type = number_type::array;
      if (data->helper1 == UINT16_MAX) data->helper1 = condition_block_function::condition;
      
      std::vector<script_data> datas;
      const sol::table input = obj.as<sol::table>();
      for (const auto &pair : input) {
        // тут должны быть в качестве id строки
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (const auto e = magic_enum::enum_cast<condition_function::value>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = condition_init_functions[index];
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<condition_block_function::value>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = condition_block_init_functions[index];
            assert(pair.second.get_type() == sol::type::table);
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          throw std::runtime_error("Could not parse script key " + std::string(str));
        }
        
        if (pair.first.get_type() == sol::type::number) {
          // тут по идее должны быть строго таблицы
          datas.emplace_back();
          assert(pair.second.get_type() == sol::type::table);
          condition_init(target_data, pair.second, &datas.back());
          continue;
        }
        
        throw std::runtime_error("Bad script function type");
      }
      
      assert(!datas.empty());
      
      data->value = datas.size();
      auto ptr = new script_data[datas.size()];
      data->data = ptr;
      for (size_t i = 0; i < datas.size(); ++i) {
        ptr[i] = std::move(datas[i]);
      }
    }
    
    void condition_or_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      if (obj.get_type() != sol::type::table) throw std::runtime_error("Bad input data for condition block");
      
      data->command_type = command_type::condition_script_block;
      data->number_type = number_type::array;
      if (data->helper1 == UINT16_MAX) data->helper1 = condition_block_function::condition;
      
      std::vector<script_data> datas;
      const sol::table input = obj.as<sol::table>();
      for (const auto &pair : input) {
        // тут должны быть в качестве id строки
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (const auto e = magic_enum::enum_cast<condition_function::value>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = condition_init_functions[index];
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<condition_block_function::value>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = condition_block_init_functions[index];
            assert(pair.second.get_type() == sol::type::table);
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          throw std::runtime_error("Could not parse script key " + std::string(str));
        }
        
        if (pair.first.get_type() == sol::type::number) {
          // тут по идее должны быть строго таблицы
          datas.emplace_back();
          assert(pair.second.get_type() == sol::type::table);
          condition_or_init(target_data, pair.second, &datas.back());
          continue;
        }
        
        throw std::runtime_error("Bad script function type");
      }
      
      assert(!datas.empty());
      
      PRINT_VAR("cond datas size", datas.size())
      data->value = datas.size();
      auto ptr = new script_data[datas.size()];
      data->data = ptr;
      for (size_t i = 0; i < datas.size(); ++i) {
        ptr[i] = std::move(datas[i]);
      }
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
      
      std::vector<script_data> datas;
      const sol::table input = obj.as<sol::table>();
      for (const auto &pair : input) {
        // тут должны быть в качестве id строки
        if (pair.first.get_type() == sol::type::string) {
          const auto str = pair.first.as<std::string_view>();
          if (const auto e = magic_enum::enum_cast<action_function::value>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = action_init_functions[index];
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          if (const auto e = magic_enum::enum_cast<action_block_function::value>(str); e) {
            datas.emplace_back();
            const auto index = e.value();
            const auto func = action_block_init_functions[index];
            assert(pair.second.get_type() == sol::type::table);
            func(target_data, pair.second, &datas.back());
            continue;
          }
          
          throw std::runtime_error("Could not parse script key " + std::string(str));
        }
        
        if (pair.first.get_type() == sol::type::number) {
          // тут по идее должны быть строго таблицы
          datas.emplace_back();
          assert(pair.second.get_type() == sol::type::table);
          action_init(target_data, pair.second, &datas.back());
          continue;
        }
        
        throw std::runtime_error("Bad script function type");
      }
      
      assert(!datas.empty());
      
      PRINT_VAR("acti datas size", datas.size())
      data->value = datas.size();
      auto ptr = new script_data[datas.size()];
      data->data = ptr;
      for (size_t i = 0; i < datas.size(); ++i) {
        ptr[i] = std::move(datas[i]);
      }
    }
    
    void random_vassal_init(const uint32_t &target_data, const sol::object &obj, script_data* data) {
      // тут должен быть только экшон
      data->helper1 = action_block_function::random_vassal;
      action_init(target_data, obj, data);
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
