#include "realm_rights_checker.h"

#include "character.h"
#include "realm.h"
#include "titulus.h"

namespace devils_engine {
  namespace core {
    namespace rights {
      // тут мы должны получить государство, которое управляет челиком или его личную вотчину?
      const realm* get_state(const realm* invoker) {
        auto state = invoker;
        if (invoker->is_assembly() || 
            invoker->is_council()  ||
            invoker->is_tribunal() ||
            invoker->is_clergy()) 
          state = invoker->state.get();
        
        ASSERT(state != nullptr);
        return state;
      }
      
      const realm* get_government(const realm* invoker) {
        if (invoker->is_independent()) return get_state(invoker);
        return invoker->liege.get(); // по идее льедж всегда указывает на государство или на селф льеджа
      }
      
      const religion* get_dominant_religion(const realm* invoker) {
        const auto state = get_state(invoker);
        return state->clergy.valid() ? state->clergy->dominant_religion : state->dominant_religion;
      }
      
      bool is_government_of(const realm* invoker, const realm* of) {
        // of - может быть какой то ветвью власти у вассала
        if (of->is_independent()) return false;
        
        const auto &of_liege = get_state(of->liege.get());
        auto state = get_state(invoker);
        return state == of_liege;
      }
      
      bool is_prisoner(const realm* invoker, const character* c) {
        if (!c->is_prisoner()) return false;
        
        // вообще у каждой силы в государстве может быть своя тюрьма
        // и для того чтобы это работало так нужно будет добавлять механик
        // сейчас пока будем пользоваться гос тюрьмой
        auto state = get_state(invoker);
        return c->imprisoner == state;
      }
      
      bool is_main_title(const realm* invoker, const titulus* title) {
        return invoker->main_title != nullptr && invoker->main_title == title;
      }
      
      bool is_title_type_same_as_main_title(const realm* invoker, const titulus* title) {
        return invoker->main_title != nullptr && invoker->main_title->type() == title->type();
      }
      
      bool is_pleb(const character* c) {
        return !c->has_dynasty();
      }
      
      bool is_noble(const character* c) {
        return c->has_dynasty();
      }
      
      bool is_vassal(const character* c) {
        const auto self = c->realms[character::self].get();
        return self != nullptr && self->main_title != nullptr;
      }
      
      bool has_landed_title(const character* c) {
        const auto r = c->realms[core::character::self];
        if (r == nullptr) return false;
        // это ошибка
        ASSERT(r->is_self() && r->titles != nullptr);
        //if (r->titles == nullptr) return false;
        return r->has_landed_title();
      }
      
      bool is_considered_shunned(const character* c) {
        const auto self = c->realms[character::self].get();
        const auto suzerain = c->suzerain.get();
        assert(size_t(self != nullptr) + size_t(suzerain != nullptr) == 1);
        const auto state = self == nullptr ? get_state(self) : get_state(suzerain);
        for (size_t i = 0; i < c->known_secrets.size(); ++i) {
          const auto &data = c->known_secrets[i];
          ASSERT(data.type < secret_types::count);
          const size_t right = state_rights::shunned_start + data.type;
          const bool is_shunned = state->get_state_mechanic(right);
          if (is_shunned) return true;
        }
        
        return false;
      }
      
      bool is_considered_criminal(const character* c) {
        const auto self = c->realms[character::self].get();
        const auto suzerain = c->suzerain.get();
        assert(size_t(self != nullptr) + size_t(suzerain != nullptr) == 1);
        const auto state = self == nullptr ? get_state(self) : get_state(suzerain);
        for (size_t i = 0; i < c->known_secrets.size(); ++i) {
          const auto &data = c->known_secrets[i];
          ASSERT(data.type < secret_types::count);
          const size_t right = state_rights::criminal_start + data.type;
          const bool is_criminal = state->get_state_mechanic(right);
          if (is_criminal) return true;
        }
        
        return false;
      }
      
      bool is_liege(const character* c) {
        const auto self = c->realms[character::self].get();
        const auto suzerain = c->suzerain.get();
        assert(size_t(self != nullptr) + size_t(suzerain != nullptr) == 1);
        const auto state = self == nullptr ? get_state(self) : get_state(suzerain);
        return state->leader == c;
      }
      
      bool is_councilor(const character* c) {
        return c->realms[character::council].valid();
      }
      
      bool is_magistrate(const character* c) {
        return c->realms[character::tribunal].valid();
      }
      
      bool is_assembler(const character* c) {
        return c->realms[character::assembly].valid();
      }
      
      bool is_clergyman(const character* c) {
        return c->realms[character::clergy].valid();
      }
      
      bool is_council_elector(const character* c) {
        return c->electorate[character::council].valid();
      }
      
      bool is_tribunal_elector(const character* c) {
        return c->electorate[character::tribunal].valid();
      }
      
      bool is_assembly_elector(const character* c) {
        return c->electorate[character::assembly].valid();
      }
      
      bool is_clergy_elector(const character* c) {
        return c->electorate[character::clergy].valid();
      }
      
      bool can_force_to_return_a_title(const realm* invoker, const realm* target, const titulus* title) {
        // тут нужно запомнить кому принадлежал титул в прошлом
      }
      
      bool can_revoke_title(const realm* invoker, const realm* target, const titulus* title) {
        if (invoker == nullptr || target == nullptr || title == nullptr) return false;
        if (invoker == target) return false;
        if (title->owner != target) return false;
        
//         if (invoker->is_independent()) {
//           if (invoker->is_self()) {
            // это глава государства, государство авторитарного типа
            // а зачем мне нужен отдельно ревок титул? у кого я могу отнимать титул в этом случае?
            bool revoke = invoker->get_power_mechanic(core::power_rights::can_revoke_title);
            if (!revoke) return false;
            {
              const bool is_state = target == get_state(invoker);
              const bool not_a_main_title_type = !is_title_type_same_as_main_title(target, title);
              const bool has_right = invoker->get_power_mechanic(core::power_rights::can_revoke_title_from_state);
              if (revoke && is_state && has_right && not_a_main_title_type) return true;
            }
            
            if (!is_government_of(invoker, target)) return false;
            // если хозяин таргета находится в тюряге, то мы можем отнять титул если доступно следующее
            {
              const bool in_prison = is_prisoner(invoker, target->leader);
              const bool has_right = invoker->get_power_mechanic(core::power_rights::can_revoke_title_from_criminal);
              if (revoke && has_right && in_prison) return true;
            }
            
            // тут нам нужно сделать институт религии, в котором можно будет проверить все эти вещи
            {
              const bool is_ex = target->leader->is_excommunicated();
              const bool same_rel = get_dominant_religion(invoker) == target->leader->religion;
              const bool has_right = invoker->get_power_mechanic(core::power_rights::can_revoke_title_from_excommunicated);
              if (revoke && has_right && is_ex && same_rel) return true;
            }
            
            {
              // опять берем доминантную религию?
              const auto rel = get_dominant_religion(invoker);
              const auto another_rel = rel != target->leader->religion;
              const bool has_right = invoker->get_power_mechanic(core::power_rights::can_revoke_title_from_infidel);
              if (revoke && has_right && another_rel) return true;
            }
            
            // какой ревок мы можем сделать в ином случае? никакой?
            return false;
//           } else if (invoker->is_council()) {
//             
//           } else if (invoker->is_tribunal()) {
//             
//           }
//         }
        
        // как проверить что могут делать ваcсалы? вассалы могут обратиться к вышестоящим органам
        // за тем чтобы те что нибудь сделали, а их права мы можем проверить здесь
      }
      
      bool can_give_title(const realm* invoker, const character* target, const titulus* title) {
        if (invoker == nullptr || target == nullptr || title == nullptr) return false;
        if (title->owner == nullptr) return false;
        // вообще неплохо было бы сделать так чтобы сила могла раздать только те титулы которые у нее есть
        // наверное пусть так и будет
        if (title->owner != invoker) return false;
        
        // а зачем мне нужен отдельно ревок титул? у кого я могу отнимать титул в этом случае?
        const bool give = invoker->get_power_mechanic(core::power_rights::can_give_title);
        if (!give) return false;
        if (target->suzerain.valid() && get_state(target->suzerain.get()) != get_state(invoker)) return false;
        const auto self_realm = target->realms[character::self];
        if (self_realm.valid() && get_state(self_realm->liege.get()) != get_state(invoker)) return false;
        
        const bool has_l_title = has_landed_title(target);
        const bool big_title_type = title->type() > titulus::type::baron;
        const bool can_give_this_title = big_title_type ? has_l_title : true;
        
        {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_give_title_to_pleb);
          const bool pleb = is_pleb(target);
          if (!has_right && pleb) return false;
        }
        
        const bool same_rel = get_dominant_religion(invoker) == target->religion;
        
        // тут нам нужно сделать институт религии, в котором можно будет проверить все эти вещи
        if (same_rel && target->is_excommunicated()) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_give_title_to_excommunicated);
          return can_give_this_title && has_right;
        }
        
        if (!same_rel) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_give_title_to_infidel);
          return can_give_this_title && has_right;
        }
        
        return true;
      }
      
      bool can_interfere_in_realm(const realm* invoker, const realm* target) {
        if (invoker == nullptr || target == nullptr) return false;
        if (!is_government_of(invoker, target)) return false;
        
        const bool has_right = invoker->get_power_mechanic(core::power_rights::can_interfere_in_vassal_realm);
        return has_right;
      }
      
      bool can_imprison(const realm* invoker, const character* target) {
        if (invoker == nullptr || target == nullptr) return false;
        if (target->is_prisoner()) return false;
        
        const auto state = get_state(invoker);
        const auto self_realm = target->realms[character::self];
        if (target->suzerain.valid() && get_state(target->suzerain.get()) != state) return false;
        if (self_realm.valid() && get_state(self_realm->liege.get()) != state) return false;
        
        const bool imprison = invoker->get_power_mechanic(core::power_rights::can_imprison);
        if (!imprison) return false;
        
        const bool imprison_freely = invoker->get_power_mechanic(core::power_rights::can_imprison_freely);
        if (imprison_freely) return true;
        
        const bool same_rel = get_dominant_religion(invoker) == target->religion;
        
        if (same_rel && target->is_excommunicated()) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_imprison_excommunicated);
          if (has_right) return true;
        }
        
        if (!same_rel) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_imprison_infidel);
          if (has_right) return true;
        }
        
        // должно быть известное преступление, как сделать преступление? это явно должен быть какой то модификатор (?)
        // или хук? нужно сделать что то такое, задал массив с известными секретами, теперь как определить какой секрет 
        // является преступлением? зависит от текущих государственных законов
        if (is_considered_criminal(target)) return true;
        
        return false;
      }
      
      bool can_execute(const realm* invoker, const character* target) {
        if (invoker == nullptr || target == nullptr) return false;
        if (!target->is_prisoner()) return false;
        
        const auto state = get_state(invoker);
        const auto self_realm = target->realms[character::self];
        if (target->suzerain.valid() && get_state(target->suzerain.get()) != state) return false;
        if (self_realm.valid() && get_state(self_realm->liege.get()) != state) return false;
        
        const bool execution_allowed = state->get_state_mechanic(state_rights::execution_allowed);
        if (!execution_allowed) return false;
        
        const bool can_execute = invoker->get_power_mechanic(power_rights::can_execute);
        if (!can_execute) return false;
        
        const bool can_execute_freely = invoker->get_power_mechanic(power_rights::can_execute_freely);
        if (can_execute_freely) return true;
        
        const bool same_rel = get_dominant_religion(invoker) == target->religion;
        
        if (same_rel && target->is_excommunicated()) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_execute_excommunicated);
          if (has_right) return true;
        }
        
        if (!same_rel) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_execute_infidel);
          if (has_right) return true;
        }
        
        if (is_considered_criminal(target)) return true;
        
        return false;
      }
      
      bool can_banish(const realm* invoker, const character* target) {
        if (invoker == nullptr || target == nullptr) return false;
        
        const auto state = get_state(invoker);
        const auto self_realm = target->realms[character::self];
        if (target->suzerain.valid() && get_state(target->suzerain.get()) != state) return false;
        if (self_realm.valid() && get_state(self_realm->liege.get()) != state) return false;
        
        const bool banishment_allowed = state->get_state_mechanic(state_rights::banishment_allowed);
        if (!banishment_allowed) return false;
        
        const bool can_banish = invoker->get_power_mechanic(power_rights::can_banish);
        if (!can_banish) return false;
        
        const bool can_banish_freely = invoker->get_power_mechanic(power_rights::can_banish_freely);
        if (can_banish_freely) return true;
        
        const bool same_rel = get_dominant_religion(invoker) == target->religion;
        
        if (same_rel && target->is_excommunicated()) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_banish_excommunicated);
          if (has_right) return true;
        }
        
        if (!same_rel) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_banish_infidel);
          if (has_right) return true;
        }
        
        const bool can_banish_shunned = invoker->get_power_mechanic(core::power_rights::can_banish_shunned);
        if (can_banish_shunned && is_considered_shunned(target)) return true;
        
        if (is_considered_criminal(target)) return true;
        
        return false;
      }
      
      bool can_free(const realm* invoker, const character* target) {
        if (invoker == nullptr || target == nullptr) return false;
        if (!target->is_prisoner()) return false;
        
        const auto state = get_state(invoker);
        const auto self_realm = target->realms[character::self];
        if (target->suzerain.valid() && get_state(target->suzerain.get()) != state) return false;
        if (self_realm.valid() && get_state(self_realm->liege.get()) != state) return false;
        
        const bool can_free = invoker->get_power_mechanic(power_rights::can_free_from_prison);
        if (!can_free) return false;
        
        const bool same_rel = get_dominant_religion(invoker) == target->religion;
        
        if (same_rel && target->is_excommunicated()) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_free_excommunicated_from_prison);
          if (!has_right) return false;
        }
        
        if (!same_rel) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_free_infidel_from_prison);
          if (!has_right) return false;
        }
        
        if (is_considered_criminal(target)) {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_free_criminal_from_prison);
          if (!has_right) return false;
        }
        
        return true;
      }
      
      bool can_declare_war(const realm* invoker) {
        return invoker->get_power_mechanic(power_rights::can_declare_war);
      }
      
      bool can_enact_laws(const realm* invoker) {
        return invoker->get_power_mechanic(power_rights::can_enact_laws);
      }
      
      bool has_the_veto_right(const realm* invoker) {
        return invoker->get_power_mechanic(power_rights::has_the_veto_right);
      }
      
      bool vassal_titles_goes_back_to_the_liege(const realm* invoker) {
        
      }
      
      bool can_become_general(const character* invoker) {
        const auto state = get_government(invoker->realms[character::self].get());
        const bool male = invoker->is_male();
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (invoker->is_general()) return false;
        
        const bool gender_allowed = male ? 
          state->get_state_mechanic(state_rights::man_can_become_a_general) : 
          state->get_state_mechanic(state_rights::woman_can_become_a_general);
          
        const bool noble = is_noble(invoker);
        
        // вообще должно еще влиять shunned, тип для шаннед существует дополнительный закон позволяющий что то или нет
        // с другой стороны это прям серьезная сегрегация, ну хотя она существовала так то, пока что дадим возможность уволить шаннед
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = state->get_state_mechanic(state_rights::vassal_can_become_a_general);
          if (has_right && gender_allowed) return true;
        }
        
        // льежу прост по умолчанию дать возможность становится генералом?
        const bool is_l = is_liege(invoker);
        if (is_l) { return true; }
        
        const bool is_hero = core::character::is_hero(invoker);
        if (is_hero) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::hero_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::hero_can_become_a_general);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_priest = core::character::is_priest(invoker);
        if (is_priest) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::priest_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_become_a_general);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_become_a_general);
        if (!state->get_state_mechanic(state_rights::courtier_can_become_a_general)) return false;
        
        return gender_allowed;
      }
      
      bool can_become_hero(const character* invoker) {
        const auto state = get_government(invoker->realms[character::self].get());
        const bool male = invoker->is_male();
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (core::character::is_hero(invoker)) return false;
        
        const bool gender_allowed = male ? 
          state->get_state_mechanic(state_rights::man_can_become_a_hero) : 
          state->get_state_mechanic(state_rights::woman_can_become_a_hero);
          
        const bool noble = is_noble(invoker);
        
        // вообще должно еще влиять shunned, тип для шаннед существует дополнительный закон позволяющий что то или нет
        // с другой стороны это прям серьезная сегрегация, ну хотя она существовала так то, пока что дадим возможность уволить шаннед
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = state->get_state_mechanic(state_rights::vassal_can_become_a_hero);
          if (has_right && gender_allowed) return true;
        }
        
        // льежу прост по умолчанию дать возможность становится генералом?
        const bool is_l = is_liege(invoker);
        if (is_l) { return true; }
        
        const bool is_priest = core::character::is_priest(invoker);
        if (is_priest) {
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_become_a_hero);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_become_a_hero);
        if (!state->get_state_mechanic(state_rights::courtier_can_become_a_hero)) return false;
        
        return gender_allowed;
      }
      
      bool can_become_councillor(const character* invoker) {
        const auto state = get_government(invoker->realms[character::self].get());
        const bool male = invoker->is_male();
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (!state->council.valid()) return false;
        if (invoker->realms[character::council].valid()) return false;
        
        const bool noble = is_noble(invoker);
        
        const bool man_can_become_a_councillor = state->get_state_mechanic(state_rights::man_can_become_a_councillor);
        const bool woman_can_become_a_councillor = state->get_state_mechanic(state_rights::woman_can_become_a_councillor);
        assert(man_can_become_a_councillor || woman_can_become_a_councillor);
        
        // можно ли сделать проверки при которых другой гендер ставится в менее выгодную позицию?
        // то есть примерно как в цк2? мы по идее можем отменить гендерность для конкретных статусов челиков
        const bool gender_allowed = male ? man_can_become_a_councillor : woman_can_become_a_councillor;
        
        //const bool get_status = state->council.valid() && state->council->get_power_mechanic(power_rights::);
        // тут мы должны проверить разные статусы
        // кстати а как чел становится генералом? генерал поидее просто должность, может ли чел перестать быть генералом?
        // может по идее, да генерал должен быть скорее технической должностью
        const bool is_general = invoker->is_general();
        if (is_general) {
          const bool has_right = state->council->get_power_mechanic(power_rights::general_can_get_this_status);
          //const bool has_right = state->get_state_mechanic(state_rights::general);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_l = is_liege(invoker);
        if (is_l) {
          const bool has_right = state->council->get_power_mechanic(power_rights::liege_can_get_this_status);
          if (has_right) return true;
        }
        
        const bool is_hero = core::character::is_hero(invoker);
        if (is_hero) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::hero_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::hero_can_become_a_councillor);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_priest = core::character::is_priest(invoker);
        if (is_priest) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::priest_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_become_a_councillor);
          if (has_right && gender_allowed) return true;
        }
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = state->get_state_mechanic(state_rights::vassal_can_become_a_councillor);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_become_a_councillor);
        if (!state->get_state_mechanic(state_rights::courtier_can_become_a_councillor)) return false;
        
        return gender_allowed;
      }
      
      bool can_become_magistrate(const character* invoker) {
        const auto state = get_government(invoker->realms[character::self].get());
        const bool male = invoker->is_male();
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (!state->tribunal.valid()) return false;
        if (invoker->realms[character::tribunal].valid()) return false;
        
        const bool noble = is_noble(invoker);
        
        const bool man_can_become_a_magistrate = state->get_state_mechanic(state_rights::man_can_become_a_magistrate);
        const bool woman_can_become_a_magistrate = state->get_state_mechanic(state_rights::woman_can_become_a_magistrate);
        assert(man_can_become_a_magistrate || woman_can_become_a_magistrate);
        
        // можно ли сделать проверки при которых другой гендер ставится в менее выгодную позицию?
        // то есть примерно как в цк2? мы по идее можем отменить гендерность для конкретных статусов челиков
        const bool gender_allowed = male ? man_can_become_a_magistrate : woman_can_become_a_magistrate;
        
        //const bool get_status = state->council.valid() && state->council->get_power_mechanic(power_rights::);
        // тут мы должны проверить разные статусы
        // кстати а как чел становится генералом? генерал поидее просто должность, может ли чел перестать быть генералом?
        // может по идее, да генерал должен быть скорее технической должностью
        const bool is_general = invoker->is_general();
        if (is_general) {
          const bool has_right = state->tribunal->get_power_mechanic(power_rights::general_can_get_this_status);
          //const bool has_right = state->get_state_mechanic(state_rights::general);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_l = is_liege(invoker);
        if (is_l) {
          const bool has_right = state->tribunal->get_power_mechanic(power_rights::liege_can_get_this_status);
          if (has_right) return true;
        }
        
        const bool is_hero = core::character::is_hero(invoker);
        if (is_hero) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::hero_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::hero_can_become_a_magistrate);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_priest = core::character::is_priest(invoker);
        if (is_priest) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::priest_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_become_a_magistrate);
          if (has_right && gender_allowed) return true;
        }
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = state->get_state_mechanic(state_rights::vassal_can_become_a_magistrate);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_become_a_magistrate);
        if (!state->get_state_mechanic(state_rights::courtier_can_become_a_magistrate)) return false;
        
        return gender_allowed;
      }
      
      bool can_become_elector(const character* invoker) {
        
      }
      
      bool can_become_clergyman(const character* invoker) {
        const auto state = get_government(invoker->realms[character::self].get());
        const bool male = invoker->is_male();
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (!state->clergy.valid()) return false;
        if (invoker->realms[character::clergy].valid()) return false;
        
        const bool noble = is_noble(invoker);
        
        const bool man_can_become_a_clergyman = state->get_state_mechanic(state_rights::man_can_become_a_clergyman);
        const bool woman_can_become_a_clergyman = state->get_state_mechanic(state_rights::woman_can_become_a_clergyman);
        assert(man_can_become_a_clergyman || woman_can_become_a_clergyman);
        
        // можно ли сделать проверки при которых другой гендер ставится в менее выгодную позицию?
        // то есть примерно как в цк2? мы по идее можем отменить гендерность для конкретных статусов челиков
        const bool gender_allowed = male ? man_can_become_a_clergyman : woman_can_become_a_clergyman;
        
        //const bool get_status = state->council.valid() && state->council->get_power_mechanic(power_rights::);
        // тут мы должны проверить разные статусы
        // кстати а как чел становится генералом? генерал поидее просто должность, может ли чел перестать быть генералом?
        // может по идее, да генерал должен быть скорее технической должностью
        const bool is_general = invoker->is_general();
        if (is_general) {
          const bool has_right = state->tribunal->get_power_mechanic(power_rights::general_can_get_this_status);
          //const bool has_right = state->get_state_mechanic(state_rights::general);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_l = is_liege(invoker);
        if (is_l) {
          const bool has_right = state->tribunal->get_power_mechanic(power_rights::liege_can_get_this_status);
          if (has_right) return true;
        }
        
        const bool is_hero = core::character::is_hero(invoker);
        if (is_hero) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::hero_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::hero_can_become_a_clergyman);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_priest = core::character::is_priest(invoker);
        if (is_priest) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::priest_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_become_a_clergyman);
          if (has_right && gender_allowed) return true;
        }
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = state->get_state_mechanic(state_rights::vassal_can_become_a_clergyman);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_become_a_clergyman);
        if (!state->get_state_mechanic(state_rights::courtier_can_become_a_clergyman)) return false;
        
        return gender_allowed;
      }
      
      bool can_become_liege(const character* invoker) {
        // тут что? 
        (void)invoker;
        return true;
      }
      
      bool can_become_noble(const character* invoker) {
        // нубл или нет отпределяется за счет наличия или отсутствия династии
        // и вообще то говоря наверное у пристов не должно быть династии даже при начии титулов
        // хотя это открытый вопрос
        // короче говоря довольно сложно кого то как то ограничить от становления дворянином
        // не думаю что имеет смысл ограничивать
        (void)invoker;
        return true;
      }
      
      bool can_become_diplomat(const character* invoker) {
        (void)invoker;
        return false;
      }
      
      bool can_become_spymaster(const character* invoker) {
        (void)invoker;
        return false;
      }
      
      bool can_become_merchant(const character* invoker) {
        (void)invoker;
        return false;
      }
      
      bool can_apply_to_the_tribunal(const character* invoker) {
        const auto state = get_government(invoker->realms[character::self].get());
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (!state->tribunal.valid()) return false;
        //if (state->tribunal->get_power_mechanic(power_rights::)) return false;
        // по идее мы можем обратиться и к авторитарному правителю
        
        const bool member = is_magistrate(invoker);
        const bool elector = is_tribunal_elector(invoker);
        if ((member || elector)) return state->tribunal->get_power_mechanic(power_rights::initiative);
        
        const bool male = invoker->is_male();
        const bool noble = is_noble(invoker);
        
        const bool man_has_right = state->get_state_mechanic(state_rights::man_can_apply_to_the_tribunal);
        const bool woman_has_right = state->get_state_mechanic(state_rights::woman_can_apply_to_the_tribunal);
        // такого рода ошибка в таких функциях, будет честно говоря как издевательство, другое дело что ситуация 
        // когда никто не может подать в трибунал может существовать, но не с помощью гендерных законов
        // наверное нужно поставить в качестве дефолтного значения male
        //assert(man_has_right || woman_has_right);
        
        // можно ли сделать проверки при которых другой гендер ставится в менее выгодную позицию?
        // то есть примерно как в цк2? мы по идее можем отменить гендерность для конкретных статусов челиков
        const bool gender_allowed = man_has_right || woman_has_right ? (male ? man_has_right : woman_has_right) : male;
        
        // никакой дополнительной спецификации для генерала
        
        const bool is_hero = core::character::is_hero(invoker);
        if (is_hero) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::hero_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::hero_can_apply_to_the_tribunal);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_priest = core::character::is_priest(invoker);
        if (is_priest) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::priest_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_apply_to_the_tribunal);
          if (has_right && gender_allowed) return true;
        }
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = state->get_state_mechanic(state_rights::vassal_can_apply_to_the_tribunal);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_apply_to_the_tribunal);
        if (!state->get_state_mechanic(state_rights::courtier_can_apply_to_the_tribunal)) return false;
        
        return gender_allowed;
      }
      
      bool can_apply_to_the_council(const character* invoker) {
        const auto state = get_government(invoker->realms[character::self].get());
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (!state->council.valid()) return false;
        //if (state->tribunal->get_power_mechanic(power_rights::)) return false;
        // по идее мы можем обратиться и к авторитарному правителю
        
        const bool member = is_councilor(invoker);
        const bool elector = is_council_elector(invoker);
        if ((member || elector)) return state->council->get_power_mechanic(power_rights::initiative);
        
        const bool male = invoker->is_male();
        const bool noble = is_noble(invoker);
        
        const bool man_has_right = state->get_state_mechanic(state_rights::man_can_apply_to_the_council);
        const bool woman_has_right = state->get_state_mechanic(state_rights::woman_can_apply_to_the_council);
        // такого рода ошибка в таких функциях, будет честно говоря как издевательство, другое дело что ситуация 
        // когда никто не может подать в трибунал может существовать, но не с помощью гендерных законов
        // наверное нужно поставить в качестве дефолтного значения male
        //assert(man_has_right || woman_has_right);
        
        // можно ли сделать проверки при которых другой гендер ставится в менее выгодную позицию?
        // то есть примерно как в цк2? мы по идее можем отменить гендерность для конкретных статусов челиков
        const bool gender_allowed = man_has_right || woman_has_right ? (male ? man_has_right : woman_has_right) : male;
        
        // никакой дополнительной спецификации для генерала
        
        const bool is_hero = core::character::is_hero(invoker);
        if (is_hero) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::hero_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::hero_can_apply_to_the_council);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_priest = core::character::is_priest(invoker);
        if (is_priest) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::priest_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_apply_to_the_council);
          if (has_right && gender_allowed) return true;
        }
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = state->get_state_mechanic(state_rights::vassal_can_apply_to_the_council);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_apply_to_the_council);
        if (!state->get_state_mechanic(state_rights::courtier_can_apply_to_the_council)) return false;
        
        return gender_allowed;
      }
      
      bool can_apply_to_the_assembly(const character* invoker) {
        const auto state = get_government(invoker->realms[character::self].get());
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (!state->assembly.valid()) return false;
        //if (state->tribunal->get_power_mechanic(power_rights::)) return false;
        // по идее мы можем обратиться и к авторитарному правителю
        
        const bool member = is_assembler(invoker);
        const bool elector = is_assembly_elector(invoker);
        if ((member || elector)) return state->assembly->get_power_mechanic(power_rights::initiative);
        
        const bool male = invoker->is_male();
        const bool noble = is_noble(invoker);
        
        const bool man_has_right = state->get_state_mechanic(state_rights::man_can_apply_to_the_assembly);
        const bool woman_has_right = state->get_state_mechanic(state_rights::woman_can_apply_to_the_assembly);
        // такого рода ошибка в таких функциях, будет честно говоря как издевательство, другое дело что ситуация 
        // когда никто не может подать в трибунал может существовать, но не с помощью гендерных законов
        // наверное нужно поставить в качестве дефолтного значения male
        //assert(man_has_right || woman_has_right);
        
        // можно ли сделать проверки при которых другой гендер ставится в менее выгодную позицию?
        // то есть примерно как в цк2? мы по идее можем отменить гендерность для конкретных статусов челиков
        const bool gender_allowed = man_has_right || woman_has_right ? (male ? man_has_right : woman_has_right) : male;
        
        // никакой дополнительной спецификации для генерала
        
        const bool is_hero = core::character::is_hero(invoker);
        if (is_hero) {
          const bool has_right = state->get_state_mechanic(state_rights::hero_can_apply_to_the_assembly);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_priest = core::character::is_priest(invoker);
        if (is_priest) {
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_apply_to_the_assembly);
          if (has_right && gender_allowed) return true;
        }
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = state->get_state_mechanic(state_rights::vassal_can_apply_to_the_assembly);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_apply_to_the_assembly);
        if (!state->get_state_mechanic(state_rights::courtier_can_apply_to_the_assembly)) return false;
        
        return gender_allowed;
      }
      
      bool can_apply_to_the_clergy(const character* invoker) {
        const auto state = get_government(invoker->realms[character::self].get());
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (!state->clergy.valid()) return false;
        //if (state->tribunal->get_power_mechanic(power_rights::)) return false;
        // по идее мы можем обратиться и к авторитарному правителю
        
        const bool member = is_assembler(invoker);
        const bool elector = is_assembly_elector(invoker);
        if ((member || elector)) return state->clergy->get_power_mechanic(power_rights::initiative);
        
        const bool male = invoker->is_male();
        const bool noble = is_noble(invoker);
        
        const bool man_has_right = state->get_state_mechanic(state_rights::man_can_apply_to_the_clergy);
        const bool woman_has_right = state->get_state_mechanic(state_rights::woman_can_apply_to_the_clergy);
        // такого рода ошибка в таких функциях, будет честно говоря как издевательство, другое дело что ситуация 
        // когда никто не может подать в трибунал может существовать, но не с помощью гендерных законов
        // наверное нужно поставить в качестве дефолтного значения male
        //assert(man_has_right || woman_has_right);
        
        // можно ли сделать проверки при которых другой гендер ставится в менее выгодную позицию?
        // то есть примерно как в цк2? мы по идее можем отменить гендерность для конкретных статусов челиков
        const bool gender_allowed = man_has_right || woman_has_right ? (male ? man_has_right : woman_has_right) : male;
        
        // никакой дополнительной спецификации для генерала
        
        const bool is_hero = core::character::is_hero(invoker);
        if (is_hero) {
          const bool has_right = state->get_state_mechanic(state_rights::hero_can_apply_to_the_clergy);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_priest = core::character::is_priest(invoker);
        if (is_priest) {
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_apply_to_the_clergy);
          if (has_right && gender_allowed) return true;
        }
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = state->get_state_mechanic(state_rights::vassal_can_apply_to_the_clergy);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_apply_to_the_clergy);
        if (!state->get_state_mechanic(state_rights::courtier_can_apply_to_the_clergy)) return false;
        
        return gender_allowed;
      }
      
      bool can_apply_to_the_state(const character* invoker) {
        // в стейт мы тоже можем обратиться, более того мы еще и частью стейта можем быть
        // например если стейт коллективный
        const auto state = get_state(invoker->realms[character::self].get());
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (state == nullptr) return false;
        //if (state->tribunal->get_power_mechanic(power_rights::)) return false;
        // по идее мы можем обратиться и к авторитарному правителю
        
        const bool member = is_assembler(invoker);
        const bool elector = is_assembly_elector(invoker);
        if ((member || elector)) return state->clergy->get_power_mechanic(power_rights::initiative);
        
        const bool male = invoker->is_male();
        const bool noble = is_noble(invoker);
        
        const bool man_has_right = state->get_state_mechanic(state_rights::man_can_apply_to_the_clergy);
        const bool woman_has_right = state->get_state_mechanic(state_rights::woman_can_apply_to_the_clergy);
        // такого рода ошибка в таких функциях, будет честно говоря как издевательство, другое дело что ситуация 
        // когда никто не может подать в трибунал может существовать, но не с помощью гендерных законов
        // наверное нужно поставить в качестве дефолтного значения male
        //assert(man_has_right || woman_has_right);
        
        // можно ли сделать проверки при которых другой гендер ставится в менее выгодную позицию?
        // то есть примерно как в цк2? мы по идее можем отменить гендерность для конкретных статусов челиков
        const bool gender_allowed = man_has_right || woman_has_right ? (male ? man_has_right : woman_has_right) : male;
        
        // никакой дополнительной спецификации для генерала
        
        const bool is_hero = core::character::is_hero(invoker);
        if (is_hero) {
          const bool has_right = state->get_state_mechanic(state_rights::hero_can_apply_to_the_clergy);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_priest = core::character::is_priest(invoker);
        if (is_priest) {
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_apply_to_the_clergy);
          if (has_right && gender_allowed) return true;
        }
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = state->get_state_mechanic(state_rights::vassal_can_apply_to_the_clergy);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_apply_to_the_clergy);
        if (!state->get_state_mechanic(state_rights::courtier_can_apply_to_the_clergy)) return false;
        
        return gender_allowed;
      }
      
      bool can_appoint_an_elector(const realm* invoker, const character* target) {
        
      }
      
      bool can_appoint_a_councillor(const realm* invoker, const character* target) {
        
      }
      
      bool can_appoint_a_magistrate(const realm* invoker, const character* target) {
        
      }
      
      bool can_appoint_a_heir(const realm* invoker, const character* target) {
        
      }
      
      bool can_appoint_a_noble(const realm* invoker, const character* target) {
        
      }
      
      bool can_appoint_a_hero(const realm* invoker, const character* target) {
        
      }
      
      bool can_appoint_a_general(const realm* invoker, const character* target) {
        
      }
      
      bool can_appoint_a_clergyman(const realm* invoker, const character* target) {
        
      }
      
      bool can_appoint_a_diplomat(const realm* invoker, const character* target) {
        
      }
      
      bool can_appoint_a_spymaster(const realm* invoker, const character* target) {
        
      }
      
      bool can_appoint_a_merchant(const realm* invoker, const character* target) {
        
      }
    }
  }
}
