#include "realm_rights_checker.h"

#include "character.h"
#include "realm.h"
#include "titulus.h"

namespace devils_engine {
  namespace core {
    namespace rights {
      const realm* get_government(const character* invoker) {
        if (invoker->suzerain.valid()) return invoker->suzerain.get();
        assert(invoker->self.valid());
        if (invoker->is_independent()) return invoker->self->get_state().get();
        return invoker->self->liege.get(); // по идее льедж всегда указывает на государство или на селф льеджа
      }
      
      realm* get_government(character* invoker) {
        if (invoker->suzerain.valid()) return invoker->suzerain.get();
        assert(invoker->self.valid());
        if (invoker->is_independent()) return invoker->self->get_state().get();
        return invoker->self->liege.get(); // по идее льедж всегда указывает на государство или на селф льеджа
      }
      
      const titulus* get_duchy(const titulus* cur) {
        if (cur->type() > core::titulus::type::duke) return nullptr;
        if (cur->type() == core::titulus::type::duke) return cur;
        auto parent = cur->parent;
        while (parent != nullptr && parent->type() < core::titulus::type::duke) {
          parent = cur->parent;
        }
        return parent;
      }
      
      const titulus* get_kingdom(const titulus* cur) {
        if (cur->type() > core::titulus::type::king) return nullptr;
        if (cur->type() == core::titulus::type::king) return cur;
        auto parent = cur->parent;
        while (parent != nullptr && parent->type() < core::titulus::type::king) {
          parent = cur->parent;
        }
        return parent;
      }
      
      const titulus* get_empire(const titulus* cur) {
        if (cur->type() > core::titulus::type::imperial) return nullptr;
        if (cur->type() == core::titulus::type::imperial) return cur;
        auto parent = cur->parent;
        while (parent != nullptr && parent->type() < core::titulus::type::imperial) {
          parent = cur->parent;
        }
        return parent;
      }
      
      const titulus* get_top_title(const titulus* cur) {
        auto parent = cur->parent;
        while (parent != nullptr) {
          parent = cur->parent;
        }
        return parent;
      }
      
      titulus* get_duchy(titulus* cur) {
        if (cur->type() > core::titulus::type::duke) return nullptr;
        if (cur->type() == core::titulus::type::duke) return cur;
        auto parent = cur->parent;
        while (parent != nullptr && parent->type() < core::titulus::type::duke) {
          parent = cur->parent;
        }
        return parent;
      }
      
      titulus* get_kingdom(titulus* cur) {
        if (cur->type() > core::titulus::type::king) return nullptr;
        if (cur->type() == core::titulus::type::king) return cur;
        auto parent = cur->parent;
        while (parent != nullptr && parent->type() < core::titulus::type::king) {
          parent = cur->parent;
        }
        return parent;
      }
      
      titulus* get_empire(titulus* cur) {
        if (cur->type() > core::titulus::type::imperial) return nullptr;
        if (cur->type() == core::titulus::type::imperial) return cur;
        auto parent = cur->parent;
        while (parent != nullptr && parent->type() < core::titulus::type::imperial) {
          parent = cur->parent;
        }
        return parent;
      }
      
      titulus* get_top_title(titulus* cur) {
        auto parent = cur->parent;
        while (parent != nullptr) {
          parent = cur->parent;
        }
        return parent;
      }
      
      bool is_government_of(const realm* invoker, const realm* of) {
        // of - может быть какой то ветвью власти у вассала
        if (of->is_independent_realm()) return false;
        
        const auto &of_liege = of->liege->get_state();
        auto state = invoker->get_state();
        return state == of_liege;
      }
      
      bool is_realm_character(const character* c, const realm* of) {
        if (c->is_independent()) return false;
        const auto &of_state = of->get_state();
        auto state = get_government(c)->get_state();
        return state == of_state;
      }
      
      bool is_vassal_of(const character* c, const realm* of) {
        return is_vassal(c) && is_realm_character(c, of);
      }
      
      bool is_courtier_of(const character* c, const realm* of) {
        return !is_vassal(c) && is_realm_character(c, of);
      }
      
      bool is_prisoner(const realm* invoker, const character* c) {
        if (!c->is_prisoner()) return false;
        
        // вообще у каждой силы в государстве может быть своя тюрьма
        // и для того чтобы это работало так нужно будет добавлять механик
        // сейчас пока будем пользоваться гос тюрьмой
        auto state = invoker->get_state();
        return c->prison == state;
      }
      
      bool is_main_title(const realm* invoker, const titulus* title) {
        return invoker->main_title != nullptr && invoker->main_title == title;
      }
      
      bool is_title_type_same_as_main_title(const realm* invoker, const titulus* title) {
        return invoker->main_title != nullptr && invoker->main_title->type() == title->type();
      }
      
      bool is_pleb(const character* c) {
        return c->is_pleb();
      }
      
      bool is_noble(const character* c) {
        return c->is_noble();
      }
      
      bool is_vassal(const character* c) {
        const auto self = c->self.get();
        return self != nullptr && self->main_title != nullptr && !self->is_independent_realm();
      }
      
      bool has_landed_title(const character* c) {
        const auto r = c->self;
        if (r == nullptr) return false;
        // это ошибка
        ASSERT(r->is_self() && r->titles != nullptr);
        //if (r->titles == nullptr) return false;
        return r->has_landed_title();
      }
      
      bool is_considered_shunned(const character* c) {
        const auto self = c->self.get();
        const auto suzerain = c->suzerain.get();
        assert(size_t(self != nullptr) + size_t(suzerain != nullptr) == 1);
        const auto state = self != nullptr ? self->get_state() : suzerain->get_state();
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
        const auto self = c->self.get();
        const auto suzerain = c->suzerain.get();
        assert(size_t(self != nullptr) + size_t(suzerain != nullptr) == 1);
        const auto state = self != nullptr ? self->get_state() : suzerain->get_state();
        for (size_t i = 0; i < c->known_secrets.size(); ++i) {
          const auto &data = c->known_secrets[i];
          ASSERT(data.type < secret_types::count);
          const size_t right = state_rights::criminal_start + data.type;
          const bool is_criminal = state->get_state_mechanic(right);
          if (is_criminal) return true;
        }
        
        return false;
      }
      
      bool is_in_realm_of(const realm* minor, const realm* of) {
        // of может не быть государством, но тут требуется проверить господина или господина господина и проч
        auto main_realm = of->get_state();
        auto liege = minor->liege->get_state();
        while (liege != nullptr && liege != main_realm) {
          liege = liege->liege->get_state();
        }
        
        return liege == nullptr && liege == main_realm;
      }
      
      bool is_liege_or_above(const realm* liege, const realm* minor) {
        auto m_liege = minor->liege.get();
        while (m_liege != nullptr && m_liege != liege) {
          m_liege = m_liege->liege.get();
        }
        
        return m_liege == nullptr && m_liege == liege;
      }
      
      bool is_parent_or_above(const titulus* parent, const titulus* cur) {
        auto cur_parent = cur->parent;
        while (cur_parent != nullptr && cur_parent != parent) {
          cur_parent = cur_parent->parent;
        }
        
        return cur_parent != nullptr && cur_parent == parent;
      }
      
      bool is_election_candidate(const realm* r, const character* ch) {
        const bool is_elective = r->has_right(core::power_rights::elective);
        if (!is_elective) return false;
        
        const auto state = r->get_state();
        assert(state.valid());
        
        // вассал этого реалма или все таки стейта? зависит от того где у нас располагаются все кортиры и вассалы
        // думаю что имеет смысл расположить всех персонажей в одном месте чтобы было попроще
        const bool is_character_in_realm = is_realm_character(ch, state.get()); // r
        if (!is_character_in_realm) return false;
        
        // тут еще нужно проверить на пол персонажей
        
        const bool vassal = is_vassal(ch);
        
        const bool is_elected_from_vassals = r->has_right(core::power_rights::elected_from_vassals);
        if (is_elected_from_vassals) { return vassal; }
        
        const bool is_elected_from_court = r->has_right(core::power_rights::elected_from_court);
        if (is_elected_from_court) { return !vassal; }
        
        const bool is_elected_from_nobles = r->has_right(core::power_rights::elected_from_nobles);
        if (is_elected_from_nobles) { return is_noble(ch); }
        
        const bool is_elected_from_heroes = r->has_right(core::power_rights::elected_from_heroes);
        if (is_elected_from_heroes) { return ch->is_hero(); }
        
        const bool is_elected_from_electors = r->has_right(core::power_rights::elected_from_electors);
        if (is_elected_from_electors) { return is_elector_of(ch, r); }
        
        const bool is_elected_from_generals = r->has_right(core::power_rights::elected_from_generals);
        if (is_elected_from_generals) { return ch->is_general(); }
        
        const bool is_elected_from_priests = r->has_right(core::power_rights::elected_from_priests);
        if (is_elected_from_priests) { return ch->is_priest(); }
        
        const bool is_elected_from_statemans = r->has_right(core::power_rights::elected_from_statemans);
        if (is_elected_from_statemans) { return is_member_of(ch, state->get_state().get()); }
        
        const bool is_elected_from_councillors = r->has_right(core::power_rights::elected_from_councillors);
        if (is_elected_from_councillors) { return is_member_of(ch, state->get_council().get()); }
        
        const bool is_elected_from_magistrates = r->has_right(core::power_rights::elected_from_magistrates);
        if (is_elected_from_magistrates) { return is_member_of(ch, state->get_tribunal().get()); }
        
        const bool is_elected_from_assemblers = r->has_right(core::power_rights::elected_from_assemblers);
        if (is_elected_from_assemblers) { return is_member_of(ch, state->get_assembly().get()); }
        
        const bool is_elected_from_clergyman = r->has_right(core::power_rights::elected_from_clergyman);
        if (is_elected_from_clergyman) { return is_member_of(ch, state->get_clergy().get()); }
        
        const bool is_elected_from_liege_family = r->has_right(core::power_rights::elected_from_liege_family);
        if (is_elected_from_liege_family) { return state->leader->is_close_relative_of(ch); }
        
        const bool is_elected_from_liege_extended_family = r->has_right(core::power_rights::elected_from_liege_extended_family);
        if (is_elected_from_liege_extended_family) { return state->leader->is_extended_relative_of(ch); } // только расширенная?
        //if (is_elected_from_liege_extended_family) { return state->leader->is_close_or_extended_relative_of(ch); }
        
        const bool is_elected_from_liege_dynasty = r->has_right(core::power_rights::elected_from_liege_dynasty);
        if (is_elected_from_liege_dynasty) { return state->leader->is_blood_relative_of(ch); }
        
        return false;
      }
      
      bool is_ruler(const character* c) {
        const auto self = c->self.get();
        const auto suzerain = c->suzerain.get();
        assert(size_t(self != nullptr) + size_t(suzerain != nullptr) == 1);
        const auto state = self != nullptr ? self->get_state() : suzerain->get_state();
        return state->leader == c;
      }
      
      bool is_councilor(const character* c) {
        return c->is_council_member();
      }
      
      bool is_magistrate(const character* c) {
        return c->is_tribunal_member();
      }
      
      bool is_assembler(const character* c) {
        return c->is_assembly_member();
      }
      
      bool is_clergyman(const character* c) {
        return c->is_clergy_member();
      }
      
      bool is_council_elector(const character* c) {
        return c->is_council_elector();
      }
      
      bool is_tribunal_elector(const character* c) {
        return c->is_tribunal_elector();
      }
      
      bool is_assembly_elector(const character* c) {
        return c->is_assembly_elector();
      }
      
      bool is_clergy_elector(const character* c) {
        return c->is_clergy_elector();
      }
      
      bool is_member_of(const character* c, const realm* of) {
        if (of == nullptr) return false;
        // тут по идее простая проверка, указатели на то членами чего мы являемся должны быть перечисленны в персонаже
        if (of->leader == c) return true; // нужно добавить эту логику в скрипт
        if (of->heir == c) return true;
        
        for (const auto &r : c->realms) {
          if (r == of) return true;
        }
        
        return false;
      }
      
      bool is_elector_of(const character* c, const realm* of) {
        for (const auto &r : c->electorate) {
          if (r == of) return true;
        }
        
        return false;
      }
      
      uint32_t get_realm_type(const realm* r) {
        // возвратим число - тип чем является реалм
        if (r->is_council()) return character::council;
        if (r->is_tribunal()) return character::tribunal;
        if (r->is_assembly()) return character::assembly;
        if (r->is_clergy()) return character::clergy;
        return character::establishment;
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
              const bool is_state = target == invoker->get_state();
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
              const bool same_rel = invoker->get_dominant_religion() == target->leader->religion;
              const bool has_right = invoker->get_power_mechanic(core::power_rights::can_revoke_title_from_excommunicated);
              if (revoke && has_right && is_ex && same_rel) return true;
            }
            
            {
              // опять берем доминантную религию?
              const auto rel = invoker->get_dominant_religion();
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
        if (target->suzerain.valid() && target->suzerain->get_state() != invoker->get_state()) return false;
        const auto self_realm = target->self;
        if (self_realm.valid() && self_realm->liege->get_state() != invoker->get_state()) return false;
        
        const bool has_l_title = has_landed_title(target);
        const bool big_title_type = title->type() > titulus::type::baron;
        const bool can_give_this_title = big_title_type ? has_l_title : true;
        
        {
          const bool has_right = invoker->get_power_mechanic(core::power_rights::can_give_title_to_pleb);
          const bool pleb = is_pleb(target);
          if (!has_right && pleb) return false;
        }
        
        const bool same_rel = invoker->get_dominant_religion() == target->religion;
        
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
        
        const auto state = invoker->get_state();
        const auto self_realm = target->self;
        if (target->suzerain.valid() && target->suzerain->get_state() != state) return false;
        if (self_realm.valid() && self_realm->liege->get_state() != state) return false;
        
        const bool imprison = invoker->get_power_mechanic(core::power_rights::can_imprison);
        if (!imprison) return false;
        
        const bool imprison_freely = invoker->get_power_mechanic(core::power_rights::can_imprison_freely);
        if (imprison_freely) return true;
        
        const bool same_rel = invoker->get_dominant_religion() == target->religion;
        
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
        
        const auto state = invoker->get_state();
        const auto self_realm = target->self;
        if (target->suzerain.valid() && target->suzerain->get_state() != state) return false;
        if (self_realm.valid() && self_realm->liege->get_state() != state) return false;
        
        const bool execution_allowed = state->get_state_mechanic(state_rights::execution_allowed);
        if (!execution_allowed) return false;
        
        const bool can_execute = invoker->get_power_mechanic(power_rights::can_execute);
        if (!can_execute) return false;
        
        const bool can_execute_freely = invoker->get_power_mechanic(power_rights::can_execute_freely);
        if (can_execute_freely) return true;
        
        const bool same_rel = invoker->get_dominant_religion() == target->religion;
        
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
        
        const auto state = invoker->get_state();
        const auto self_realm = target->self;
        if (target->suzerain.valid() && target->suzerain->get_state() != state) return false;
        if (self_realm.valid() && self_realm->liege->get_state() != state) return false;
        
        const bool banishment_allowed = state->get_state_mechanic(state_rights::banishment_allowed);
        if (!banishment_allowed) return false;
        
        const bool can_banish = invoker->get_power_mechanic(power_rights::can_banish);
        if (!can_banish) return false;
        
        const bool can_banish_freely = invoker->get_power_mechanic(power_rights::can_banish_freely);
        if (can_banish_freely) return true;
        
        const bool same_rel = invoker->get_dominant_religion() == target->religion;
        
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
        
        const auto state = invoker->get_state();
        const auto self_realm = target->self;
        if (target->suzerain.valid() && target->suzerain->get_state() != state) return false;
        if (self_realm.valid() && self_realm->liege->get_state() != state) return false;
        
        const bool can_free = invoker->get_power_mechanic(power_rights::can_free_from_prison);
        if (!can_free) return false;
        
        const bool same_rel = invoker->get_dominant_religion() == target->religion;
        
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
        const auto state = get_government(invoker);
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
        const bool is_l = is_ruler(invoker);
        if (is_l) { return true; }
        
        const bool is_hero = invoker->is_hero();
        if (is_hero) {
          //const bool has_right = state->council->get_power_mechanic(power_rights::hero_can_get_this_status);
          const bool has_right = state->get_state_mechanic(state_rights::hero_can_become_a_general);
          if (has_right && gender_allowed) return true;
        }
        
        const bool is_priest = invoker->is_priest();
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
        const auto state = get_government(invoker);
        const bool male = invoker->is_male();
        const bool is_criminal = is_considered_criminal(invoker);
        if (is_criminal) return false;
        if (invoker->is_hero()) return false;
        
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
        const bool is_l = is_ruler(invoker);
        if (is_l) { return true; }
        
        const bool is_priest = invoker->is_priest();
        if (is_priest) {
          const bool has_right = state->get_state_mechanic(state_rights::priest_can_become_a_hero);
          if (has_right && gender_allowed) return true;
        }
        
        if (noble) return gender_allowed && state->get_state_mechanic(state_rights::noble_can_become_a_hero);
        if (!state->get_state_mechanic(state_rights::courtier_can_become_a_hero)) return false;
        
        return gender_allowed;
      }
      
      bool can_become_noble(const character* invoker) {
        // нубл или нет отпределяется за счет наличия или отсутствия династии
        // и вообще то говоря наверное у пристов не должно быть династии даже при начии титулов
        // хотя это открытый вопрос
        // короче говоря довольно сложно кого то как то ограничить от становления дворянином
        // не думаю что имеет смысл ограничивать
        return !invoker->is_noble();
      }
      
      // нужно разделить на технически может ли чел стать консулом и по праву
      bool can_become_stateman(const character* invoker) { return can_become_member(invoker, get_government(invoker)->get_state().get()); }
      bool can_become_councillor(const character* invoker) { 
        const auto realm = get_government(invoker)->get_state()->get_council();
        if (!realm.valid()) return false;
        return can_become_member(invoker, realm.get()); 
      }
      
      bool can_become_magistrate(const character* invoker) { 
        const auto realm = get_government(invoker)->get_state()->get_tribunal();
        if (!realm.valid()) return false;
        return can_become_member(invoker, realm.get()); 
      }
      
      bool can_become_assembler(const character* invoker) { 
        const auto realm = get_government(invoker)->get_state()->get_assembly();
        if (!realm.valid()) return false;
        return can_become_member(invoker, realm.get()); 
      }
      
      bool can_become_clergyman(const character* invoker) { 
        const auto realm = get_government(invoker)->get_state()->get_clergy();
        if (!realm.valid()) return false;
        return can_become_member(invoker, realm.get()); 
      }
      
      bool has_right_to_become_stateman(const character* invoker) {
        return has_right_to_become_member(invoker, get_government(invoker)->get_state().get());
      }
      
      bool has_right_to_become_councillor(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_council();
        if (!realm.valid()) return false;
        return has_right_to_become_member(invoker, realm.get());
      }
      
      bool has_right_to_become_magistrate(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_tribunal();
        if (!realm.valid()) return false;
        return has_right_to_become_member(invoker, realm.get());
      }
      
      bool has_right_to_become_assembler(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_assembly();
        if (!realm.valid()) return false;
        return has_right_to_become_member(invoker, realm.get());
      }
      
      bool has_right_to_become_clergyman(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_clergy();
        if (!realm.valid()) return false;
        return has_right_to_become_member(invoker, realm.get());
      }
      
      bool can_become_stateman_elector(const character* invoker) { return can_become_elector(invoker, get_government(invoker)->get_state().get()); }
      bool can_become_councillor_elector(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_council();
        if (!realm.valid()) return false;
        return can_become_elector(invoker, realm.get()); 
      }
      
      bool can_become_tribunal_elector(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_tribunal();
        if (!realm.valid()) return false;
        return can_become_elector(invoker, realm.get()); 
      }
      
      bool can_become_assembly_elector(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_assembly();
        if (!realm.valid()) return false;
        return can_become_elector(invoker, realm.get()); 
      }
      
      bool can_become_clergyman_elector(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_clergy();
        if (!realm.valid()) return false;
        return can_become_elector(invoker, realm.get()); 
      }
      
      bool has_right_to_become_stateman_elector(const character* invoker) {
        return has_right_to_become_elector(invoker, get_government(invoker)->get_state().get());
      }
      
      bool has_right_to_become_councillor_elector(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_council();
        if (!realm.valid()) return false;
        return has_right_to_become_elector(invoker, realm.get()); 
      }
      
      bool has_right_to_become_tribunal_elector(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_tribunal();
        if (!realm.valid()) return false;
        return has_right_to_become_elector(invoker, realm.get()); 
      }
      
      bool has_right_to_become_assembly_elector(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_assembly();
        if (!realm.valid()) return false;
        return has_right_to_become_elector(invoker, realm.get()); 
      }
      
      bool has_right_to_become_clergyman_elector(const character* invoker) {
        const auto realm = get_government(invoker)->get_state()->get_clergy();
        if (!realm.valid()) return false;
        return has_right_to_become_elector(invoker, realm.get());
      }
      
      bool can_become_member(const character* invoker, const realm* target) {
        const auto state = get_government(invoker)->get_state();
        uint32_t index = UINT32_MAX;
        if (target->is_state_independent_power()) index = character::establishment;
        if (target->is_council()) index = character::council;
        if (target->is_tribunal()) index = character::tribunal;
        if (target->is_assembly()) index = character::assembly;
        if (target->is_clergy()) index = character::clergy;
        assert(index != UINT32_MAX);
        
        // что делать с реалмом который abroad (например с папством?), по идее какие то челики могут стать его частью и быть не совсем частью текущего государства
        // нужно добавить указатель на другой реалм, указатель на другой реалм будет лежать в льедже (что собственно логично)
        // но в любом случае мы не можем быть частью чиновников какого то другого государства 
        if (state != target->get_state()) return false;
        if (invoker->realms[index].valid()) return false;
        // по идее эти проверки не нужны если есть проверка совпадают ли стейты
//         if (target->is_council()  && !state->council.valid())  return false;
//         if (target->is_tribunal() && !state->tribunal.valid()) return false;
//         if (target->is_assembly() && !state->assembly.valid()) return false;
//         if (target->is_clergy()   && !state->clergy.valid())   return false;
        
        return true;
      }
      
      bool has_right_to_become_member(const character* invoker, const realm* target) {
        if (!can_become_member(invoker, target)) return false;
        if (is_considered_criminal(invoker)) return false;
        
        const bool male = invoker->is_male();
        const bool man_has_right = target->get_power_mechanic(power_rights::man_can_become_a_member);
        const bool woman_has_right = target->get_power_mechanic(power_rights::woman_can_become_a_member);
        //assert(man_has_right || woman_has_right);
        
        const bool gender_allowed = man_has_right || woman_has_right ? (male ? man_has_right : woman_has_right) : male;
        //const bool gender_allowed = male ? man_has_right : woman_has_right;
        
        const auto state = target->get_state();
        const auto council = target->get_council();
        const auto tribunal = target->get_tribunal();
        const auto assembly = target->get_assembly();
        const auto clergy = target->get_clergy();
        
        //const bool get_status = state->council.valid() && state->council->get_power_mechanic(power_rights::);
        // тут мы должны проверить разные статусы
        const bool is_l = is_ruler(invoker);
        if (is_l) {
          const bool has_right = target->get_power_mechanic(power_rights::ruler_can_get_this_status);
          if (has_right) return true;
        }
        
        // кстати а как чел становится генералом? генерал поидее просто должность, может ли чел перестать быть генералом?
        // может по идее, да генерал должен быть скорее технической должностью
        const bool is_general = invoker->is_general();
        if (is_general) {
          const bool has_right = target->get_power_mechanic(power_rights::general_can_get_this_status); // наверное в этом случае такого права быть не должно?
          if (has_right) return true; // gender_allowed
        }
        
        const bool is_hero = invoker->is_hero();
        if (is_hero) {
          // это право стать членом в любой момент после получения статуса, должно ли оно тут учитываться? что на счет права получить членство?
          const bool has_right_to_get_status = target->get_power_mechanic(power_rights::hero_can_get_this_status);
          const bool has_right_to_become_member = target->get_power_mechanic(power_rights::hero_can_become_a_member);
          if (has_right_to_get_status || (has_right_to_become_member && gender_allowed)) return true; // гендер? get_status вне зависимости от гендера
        }
        
        const bool is_priest = invoker->is_priest();
        if (is_priest) {
          const bool has_right_to_get_status = target->get_power_mechanic(power_rights::priest_can_get_this_status);
          const bool has_right_to_become_member = target->get_power_mechanic(power_rights::priest_can_become_a_member);
          if (has_right_to_get_status || (has_right_to_become_member && gender_allowed)) return true; // гендер? get_status вне зависимости от гендера
        }
        
        if (state->leader == invoker) {
          const bool has_right = target->get_power_mechanic(power_rights::ruler_can_get_this_status);
          if (has_right) return true;
        }
        
        if (target != state && state->is_state_independent_power() && is_member_of(invoker, state.get())) {
          const bool has_right = target->get_power_mechanic(power_rights::stateman_can_get_this_status);
          if (has_right) return true;
        }
        
        if (target != council && council.valid() && is_member_of(invoker, council.get())) {
          const bool has_right_to_get_status = target->get_power_mechanic(power_rights::councillor_can_get_this_status);
          if (has_right_to_get_status) return true;  // gender_allowed
        }
        
        if (target != assembly && assembly.valid() && is_member_of(invoker, assembly.get())) {
          const bool has_right = target->get_power_mechanic(power_rights::assembler_can_get_this_status);
          if (has_right) return true;  // gender_allowed
        }
        
        if (target != tribunal && tribunal.valid() && is_member_of(invoker, tribunal.get())) {
          const bool has_right = target->get_power_mechanic(power_rights::magistrate_can_get_this_status);
          if (has_right) return true;  // gender_allowed
        }
        
        if (target != clergy && clergy.valid() && is_member_of(invoker, clergy.get())) {
          const bool has_right = target->get_power_mechanic(power_rights::clergyman_can_get_this_status);
          if (has_right) return true;  // gender_allowed
        }
        
        if (is_considered_shunned(invoker)) {
          const auto state = target->get_state();
          // какое право у шаннед?
          //const bool has_right = target->get_power_mechanic(power_rights::shunn);
        }
        
        const bool noble = is_noble(invoker);
        const bool noble_has_right = target->get_power_mechanic(power_rights::noble_can_become_a_member);
        const bool pleb_has_right = target->get_power_mechanic(power_rights::pleb_can_become_a_member);
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = target->get_power_mechanic(power_rights::vassal_can_become_a_member);
          if (noble) {
            if (has_right && noble_has_right && gender_allowed) return true;
            else return false;
          } else {
            assert(invoker->is_pleb());
            if (has_right && pleb_has_right && gender_allowed) return true;
            else return false;
          }
        }
        
        const bool courtier_has_right = target->get_power_mechanic(power_rights::courtier_can_become_a_member);
        if (!courtier_has_right) return false;
        
        if (noble) return noble_has_right && gender_allowed;
        return pleb_has_right && gender_allowed;
      }
      
      bool can_become_elector(const character* invoker, const realm* target) {
        const auto state = get_government(invoker)->get_state();
        uint32_t index = UINT32_MAX;
        if (target->is_state_independent_power()) index = character::establishment;
        if (target->is_council()) index = character::council;
        if (target->is_tribunal()) index = character::tribunal;
        if (target->is_assembly()) index = character::assembly;
        if (target->is_clergy()) index = character::clergy;
        assert(index != UINT32_MAX);
        
        // что делать с реалмом который abroad (например с папством?), по идее какие то челики могут стать его частью и быть не совсем частью текущего государства
        // нужно добавить указатель на другой реалм, указатель на другой реалм будет лежать в льедже (что собственно логично)
        // но в любом случае мы не можем быть частью чиновников какого то другого государства 
        if (state != target->get_state()) return false;
        // может ли мембер этого реалма быть электором? вообще может быть
        if (invoker->electorate[index].valid()) return false; // уже электор
        // по идее эти проверки не нужны если есть проверка совпадают ли стейты
//         if (target->is_council()  && !state->council.valid())  return false;
//         if (target->is_tribunal() && !state->tribunal.valid()) return false;
//         if (target->is_assembly() && !state->assembly.valid()) return false;
//         if (target->is_clergy()   && !state->clergy.valid())   return false;
        
        return true;
      }
      
      bool has_right_to_become_elector(const character* invoker, const realm* target) {
        if (!can_become_elector(invoker, target)) return false;
        if (is_considered_criminal(invoker)) return false; // по умолчанию криминал не может ничего
        
        const bool male = invoker->is_male();
        const bool man_has_right = target->get_power_mechanic(power_rights::man_can_become_an_elector);
        const bool woman_has_right = target->get_power_mechanic(power_rights::woman_can_become_an_elector);
        //assert(man_has_right || woman_has_right);
        
        // по умолчанию предпочтение отдается мужчине (чисто чтобы не вылетать на базовых проверках)
        const bool gender_allowed = man_has_right || woman_has_right ? (male ? man_has_right : woman_has_right) : male;
        //const bool gender_allowed = male ? man_can_become_a_elector : woman_can_become_a_elector;
        
        //const bool get_status = state->council.valid() && state->council->get_power_mechanic(power_rights::);
        // тут мы должны проверить разные статусы
        // вообще правитель может стать электором для консила например
//         const bool is_l = is_ruler(invoker);
//         if (is_l) {
//           const bool has_right = target->get_power_mechanic(power_rights::ruler_can_get_this_status);
//           if (has_right) return true;
//         }
//         
//         // кстати а как чел становится генералом? генерал поидее просто должность, может ли чел перестать быть генералом?
//         // может по идее, да генерал должен быть скорее технической должностью
//         // может ли генерал быть электором? в текущий момент нет, кто такой вообще генерал? взрослый человек с землей?
//         // нужно ли их количество как то ограничить?
//         const bool is_general = invoker->is_general();
//         if (is_general) {
//           const bool has_right = target->get_power_mechanic(power_rights::general_can_get_this_status); // наверное в этом случае такого права быть не должно?
//           if (has_right) return true; // gender_allowed
//         }
        
        const bool is_hero = invoker->is_hero();
        if (is_hero) {
          // это право стать членом в любой момент после получения статуса, должно ли оно тут учитываться? что на счет права получить членство?
          const bool has_right_to_become_member = target->get_power_mechanic(power_rights::hero_can_become_an_elector);
          if (has_right_to_become_member && gender_allowed) return true;
        }
        
        const bool is_priest = invoker->is_priest();
        if (is_priest) {
          const bool has_right_to_become_member = target->get_power_mechanic(power_rights::priest_can_become_an_elector);
          if (has_right_to_become_member && gender_allowed) return true;
        }
        
        if (is_considered_shunned(invoker)) {
          const auto state = target->get_state();
          // какое право у шаннед?
          //const bool has_right = target->get_power_mechanic(power_rights::shunn);
        }
        
        const bool noble = is_noble(invoker);
        const bool noble_has_right = target->get_power_mechanic(power_rights::noble_can_become_an_elector);
        const bool pleb_has_right = target->get_power_mechanic(power_rights::pleb_can_become_an_elector);
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = target->get_power_mechanic(power_rights::vassal_can_become_an_elector);
          if (noble) {
            if (has_right && noble_has_right && gender_allowed) return true;
            else return false;
          } else {
            assert(invoker->is_pleb());
            if (has_right && pleb_has_right && gender_allowed) return true;
            else return false;
          }
        }
        
        const bool courtier_has_right = target->get_power_mechanic(power_rights::courtier_can_become_an_elector);
        if (!courtier_has_right) return false;
        
        if (noble) return noble_has_right && gender_allowed;
        return pleb_has_right && gender_allowed;
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
        const auto state = get_government(invoker)->get_state();
        const auto r = state->get_tribunal();
        if (!r.valid()) return false;
        return can_apply_to(invoker, r.get());
      }
      
      bool can_apply_to_the_council(const character* invoker) {
        const auto state = get_government(invoker)->get_state();
        const auto r = state->get_council();
        if (!r.valid()) return false;
        return can_apply_to(invoker, r.get());
      }
      
      bool can_apply_to_the_assembly(const character* invoker) {
        const auto state = get_government(invoker)->get_state();
        const auto r = state->get_assembly();
        if (!r.valid()) return false;
        return can_apply_to(invoker, r.get());
      }
      
      bool can_apply_to_the_clergy(const character* invoker) {
        const auto state = get_government(invoker)->get_state();
        const auto clergy = state->get_clergy();
        if (!clergy.valid()) return false;
        return can_apply_to(invoker, clergy.get());
      }
      
      bool can_apply_to_the_state(const character* invoker) {
        const auto state = get_government(invoker)->get_state();
        return can_apply_to(invoker, state.get());
      }
      
      bool has_right_to_apply_to_the_tribunal(const character* invoker) {
        const auto state = get_government(invoker)->get_state();
        const auto r = state->get_tribunal();
        if (!r.valid()) return false;
        return can_apply_to(invoker, r.get());
      }
      
      bool has_right_to_apply_to_the_council(const character* invoker) {
        const auto state = get_government(invoker)->get_state();
        const auto r = state->get_council();
        if (!r.valid()) return false;
        return has_right_to_apply_to(invoker, r.get());
      }
      
      bool has_right_to_apply_to_the_assembly(const character* invoker) {
        const auto state = get_government(invoker)->get_state();
        const auto r = state->get_assembly();
        if (!r.valid()) return false;
        return can_apply_to(invoker, r.get());
      }
      
      bool has_right_to_apply_to_the_clergy(const character* invoker) {
        const auto state = get_government(invoker)->get_state();
        const auto clergy = state->get_clergy();
        if (!clergy.valid()) return false;
        return can_apply_to(invoker, clergy.get());
      }
      
      bool has_right_to_apply_to_the_state(const character* invoker) {
        const auto state = get_government(invoker)->get_state();
        return has_right_to_apply_to(invoker, state.get());
      }
      
      bool can_apply_to(const character* invoker, const realm* target) {
        // нужно проверить во первых чтобы таргет был в одном государстве с инвокером
        // что то еще? дальше по идее только права
        const auto state = get_government(invoker)->get_state();
        
        // а может быть все таки криминал просто не имеет прав пользоваться услугами там разными?
        // вообще правда, вылета не произойдет, поэтому наверное пусть перемещается в разряд прав
        // можем ли мы обратиться в какой то стейт вне текущего государства? да, по идее в религиозную организацию
        // как это сделать? нужно найти какого то иного хозяина среди стейтов, хотя если только в религиозную организацию
        // то можно проверить иначе, оставим это "на подумать"
        return state == target->get_state();
      }
      
      bool has_right_to_apply_to(const character* invoker, const realm* target) {
        if (!can_apply_to(invoker, target)) return false;
        if (is_considered_criminal(invoker)) return false;
        
        const auto state = target->get_state();
        if (state == nullptr) return false;
        //if (state->tribunal->get_power_mechanic(power_rights::)) return false;
        // по идее мы можем обратиться и к авторитарному правителю
        
        const bool member = is_member_of(invoker, target);
        const bool elector = is_elector_of(invoker, target); // может ли электор обладать инициативой?
        if (member) return target->get_power_mechanic(power_rights::initiative);
        if (elector) { // считаем что электор - это часть силы
          if (target->get_power_mechanic(power_rights::elector_can_apply)) return target->get_power_mechanic(power_rights::initiative);
        }
        
        // по идее из таргета мы можем получить все необходимое
        const auto council = target->get_council();
        const auto tribunal = target->get_tribunal();
        const auto assembly = target->get_assembly();
        const auto clergy = target->get_clergy();
        
        const bool male = invoker->is_male();
        
        const bool man_has_right = target->get_power_mechanic(power_rights::man_can_apply);
        const bool woman_has_right = target->get_power_mechanic(power_rights::woman_can_apply);
        // такого рода ошибка в таких функциях, будет честно говоря как издевательство, другое дело что ситуация 
        // когда никто не может подать в трибунал может существовать, но не с помощью гендерных законов
        // наверное нужно поставить в качестве дефолтного значения male
        //assert(man_has_right || woman_has_right);
        
        // можно ли сделать проверки при которых другой гендер ставится в менее выгодную позицию?
        // то есть примерно как в цк2? мы по идее можем отменить гендерность для конкретных статусов челиков
        const bool gender_allowed = man_has_right || woman_has_right ? (male ? man_has_right : woman_has_right) : male;
        
        // никакой дополнительной спецификации для генерала
        
        // осуждаемый может ли направить запрос?
        
        // герой и священник освобождены от проверок на гендер
        const bool is_hero = invoker->is_hero();
        if (is_hero) {
          const bool has_right = target->get_power_mechanic(power_rights::hero_can_apply);
          if (has_right) return true; // gender_allowed
        }
        
        const bool is_priest = invoker->is_priest();
        if (is_priest) {
          const bool has_right = target->get_power_mechanic(power_rights::priest_can_apply);
          if (has_right) return true; // gender_allowed
        }
        
        if (state->leader == invoker) {
          const bool has_right = target->get_power_mechanic(power_rights::ruler_can_apply);
          if (has_right) return true;
        }
        
        // еще мемберы и электоры могут направить запросы, нужно ли им проверять гендер?
        if (target != state && state->is_state_independent_power() && is_member_of(invoker, state.get())) {
          const bool has_right = target->get_power_mechanic(power_rights::stateman_can_apply);
          if (has_right) return true;
        }
        
        if (target != council && council.valid() && is_member_of(invoker, council.get())) {
          const bool has_right = target->get_power_mechanic(power_rights::councillor_can_apply);
          if (has_right) return true;
        }
        
        if (target != tribunal && tribunal.valid() && is_member_of(invoker, tribunal.get())) {
          const bool has_right = target->get_power_mechanic(power_rights::magistrate_can_apply);
          if (has_right) return true;
        }
        
        if (target != assembly && assembly.valid() && is_member_of(invoker, assembly.get())) {
          const bool has_right = target->get_power_mechanic(power_rights::assembler_can_apply);
          if (has_right) return true;
        }
        
        if (target != clergy && clergy.valid() && is_member_of(invoker, clergy.get())) {
          const bool has_right = target->get_power_mechanic(power_rights::clergyman_can_apply);
          if (has_right) return true;
        }
        
        const bool noble = is_noble(invoker);
        const bool noble_has_right = target->get_power_mechanic(power_rights::noble_can_apply);
        const bool pleb_has_right = target->get_power_mechanic(power_rights::pleb_can_apply);
        
        const bool vassal = is_vassal(invoker);
        if (vassal) {
          const bool has_right = target->get_power_mechanic(power_rights::vassal_can_apply);
          if (noble) {
            if (has_right && noble_has_right && gender_allowed) return true;
            else return false;
          } else {
            assert(invoker->is_pleb());
            if (has_right && pleb_has_right && gender_allowed) return true;
            else return false;
          }
        }
        
        const bool courtier_has_right = target->get_power_mechanic(power_rights::courtier_can_apply);
        if (!courtier_has_right) return false;
        
        if (noble) return noble_has_right && gender_allowed;
        return pleb_has_right && gender_allowed;
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
