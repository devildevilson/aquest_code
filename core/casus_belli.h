#ifndef DEVILS_ENGINE_CORE_CASUS_BELLI_H
#define DEVILS_ENGINE_CORE_CASUS_BELLI_H

#include <string>
#include <cstdint>
#include <cstddef>
#include "declare_structures.h"
#include "utils/bit_field.h"
#include "utils/constexpr_funcs.h"
#include "script/header.h"
#include "script/condition_commands_macro.h"

#include "casus_belli_data_macro.h"
  
namespace devils_engine {
  namespace core {
    struct casus_belli {
      static const core::structure s_type = core::structure::casus_belli;
      
      enum class flags {
        // casus_belli_data_macro.h
#define CASUS_BELLI_FLAG_FUNC(name) name,
        CASUS_BELLI_FLAGS_LIST
#undef CASUS_BELLI_FLAG_FUNC

        count
      };
      
      std::string id;
      
      // casus_belli_data_macro.h
#define CASUS_BELLI_NUMBER_FUNC(name) float name;
      CASUS_BELLI_NUMBERS_LIST
#undef CASUS_BELLI_NUMBER_FUNC

      size_t truce_turns;
      utils::bit_field<static_cast<size_t>(flags::count)> flags_field;
      // картиночка
      // diplo_view_region - меняет провинции котолрые подсвечиваются при выборе этого цб
      // check_de_jure_tier - сканирует (?) все де юре королевства на предмет владений у вассалов?
      
      // в казус белли у нас добавится куча скриптов
      // can_use (разные скоупы), is_valid (разные скоупы), ai_will_do, on_add, on_add_post, 
      // on_success, on_success_post, on_fail, on_fail_post, on_reverse_demand, on_reverse_demand_post, 
      // on_attacker_leader_death, on_defender_leader_death, on_thirdparty_death, on_invalidation
      // можно ли запускать скрипты от казус белли? скорее всего нужно запускать от войны
      script::string name_script; // скрипты зависят от непосредственно войны
      script::string war_name_script;
      script::condition can_use_script; // условия для того чтобы персонаж мог воспользоваться этим казус белли
      script::condition is_valid; // этот скрипт в цк2 мог взываться от персонажа и от титула
      script::number ai_will_do;
      script::number authority_cost_script;
      script::number esteem_cost_script;
      script::number influence_cost_script;
      script::number money_cost_script;
      // тут обходим эффекты
      script::effect on_add; // при добавлении участника
      script::effect on_add_post;
      script::effect on_success; // успешное завершение войны (для нападающего)
      script::effect on_success_post; // должно вызваться после разрушения войны для кого? возможно придется делать списки
      script::effect on_fail; // не успешное завершение войны
      script::effect on_fail_post;
      script::effect on_reverse_demand; // выдвинуты обратные требования (то есть победа защиты?)
      script::effect on_reverse_demand_post;
      script::effect on_attacker_leader_death;
      script::effect on_defender_leader_death;
      script::effect on_thirdparty_death;
      // если is_valid = false, а также при конце войны, когда тип конца войны инвалид
      // 
      script::effect on_invalidation;
      
      casus_belli();
      
      inline bool get_flag(const enum flags &flag) const { return flags_field.get(static_cast<size_t>(flag)); }
      inline bool set_flag(const enum flags &flag, const bool value) { return flags_field.set(static_cast<size_t>(flag), value); }
      
      // какие данные мы ожидаем здесь?
      bool can_use(const realm* root, const realm* target, const titulus* title) const;
      float authority_cost(const realm* root, const realm* target, const titulus* title) const;
      float esteem_cost(const realm* root, const realm* target, const titulus* title) const;
      float influence_cost(const realm* root, const realm* target, const titulus* title) const;
      float money_cost(const realm* root, const realm* target, const titulus* title) const;
      float ai_probability(const realm* root, const realm* target, const titulus* title) const;
      std::string_view name(const realm* root, const realm* target, const titulus* title) const;
      std::string_view war_name(const realm* root, const realm* target, const titulus* title) const;
      
#define CASUS_BELLI_FLAG_FUNC(name) bool name() const;
      CASUS_BELLI_GET_BOOL_NO_ARGS_COMMANDS_LIST
#undef CASUS_BELLI_FLAG_FUNC

#define CASUS_BELLI_NUMBER_FUNC(name) double get_##name() const;
      CASUS_BELLI_GET_NUMBER_NO_ARGS_COMMANDS_LIST
#undef CASUS_BELLI_NUMBER_FUNC
    };
    
    bool validate_casus_belli(const size_t &index, const sol::table &table);
    void parse_casus_belli(core::casus_belli* casus_belli, const sol::table &table);
  }
}

#endif
