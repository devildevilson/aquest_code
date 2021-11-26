#ifndef DEVILS_ENGINE_CORE_REALM_RIGHTS_CHECKER_H
#define DEVILS_ENGINE_CORE_REALM_RIGHTS_CHECKER_H

#include "declare_structures.h"

namespace devils_engine {
  namespace core {
    // тут мы проверяем можем ли мы это сделать по законам государства, но в игре должна быть возможность
    // сделать все эти вещи нарушив законы, в этом случае мы должны применить какие то штрафы для ии,
    // а значит эти функции не должны особо ни на что влиять и быть просто шорткатами к сложным проверкам прав
    // но при этом есть некоторые абсолютные ограничения типа если титул еще не создан, то мы ничего особо сделать и не можем
    // теперь я разделил права института и общие государственные права
    namespace rights {
      const realm* get_state(const realm* invoker);
      const realm* get_government(const realm* invoker);
      const religion* get_dominant_religion(const realm* invoker);
      bool is_government_of(const realm* invoker, const realm* of);
      bool is_prisoner(const realm* invoker, const character* c);
      bool is_main_title(const realm* invoker, const titulus* title);
      bool is_title_type_same_as_main_title(const realm* invoker, const titulus* title);
      bool is_pleb(const character* c);
      bool is_noble(const character* c);
      bool is_vassal(const character* c);
      bool has_landed_title(const character* c);
      bool is_considered_shunned(const character* c);
      bool is_considered_criminal(const character* c);
      
      bool is_liege(const character* c);
      bool is_councilor(const character* c);
      bool is_magistrate(const character* c);
      bool is_assembler(const character* c);
      bool is_clergyman(const character* c);
      bool is_council_elector(const character* c);
      bool is_tribunal_elector(const character* c);
      bool is_assembly_elector(const character* c);
      bool is_clergy_elector(const character* c);
      
      bool can_force_to_return_a_title(const realm* invoker, const realm* target, const titulus* title);
      bool can_revoke_title(const realm* invoker, const realm* target, const titulus* title);
      bool can_give_title(const realm* invoker, const character* target, const titulus* title);
      // это должно позволить нам сменить закон какой нибудь в реалме
      bool can_interfere_in_realm(const realm* invoker, const realm* target);
      bool can_imprison(const realm* invoker, const character* target);
      bool can_execute(const realm* invoker, const character* target);
      bool can_banish(const realm* invoker, const character* target);
      bool can_free(const realm* invoker, const character* target);
      
      bool can_declare_war(const realm* invoker);
      bool can_enact_laws(const realm* invoker);
      bool has_the_veto_right(const realm* invoker);
      // титулы могут уходить в пользу кого то другого, как проверить?
      bool vassal_titles_goes_back_to_the_liege(const realm* invoker);
      // эти функции для того чтобы самостоятельно стать тем или иным "класс"
      // тут мы должны проверить что доступно в текущем государтве
      bool can_become_general(const character* invoker);
      bool can_become_hero(const character* invoker);
      bool can_become_councillor(const character* invoker);
      bool can_become_magistrate(const character* invoker);
      bool can_become_elector(const character* invoker);
      bool can_become_clergyman(const character* invoker);
      bool can_become_liege(const character* invoker);
      bool can_become_noble(const character* invoker);
      // эти профессии требуют дополнительных игровых механик
      bool can_become_diplomat(const character* invoker);
      bool can_become_spymaster(const character* invoker);
      bool can_become_merchant(const character* invoker);
      bool can_apply_to_the_tribunal(const character* invoker);
      bool can_apply_to_the_council(const character* invoker);
      bool can_apply_to_the_assembly(const character* invoker);
      bool can_apply_to_the_clergy(const character* invoker);
      bool can_apply_to_the_state(const character* invoker);
      
      bool can_appoint_an_elector(const realm* invoker, const character* target);
      bool can_appoint_a_councillor(const realm* invoker, const character* target);
      bool can_appoint_a_magistrate(const realm* invoker, const character* target);
      bool can_appoint_a_heir(const realm* invoker, const character* target);
      bool can_appoint_a_noble(const realm* invoker, const character* target);
      bool can_appoint_a_hero(const realm* invoker, const character* target);
      bool can_appoint_a_general(const realm* invoker, const character* target);
      bool can_appoint_a_clergyman(const realm* invoker, const character* target);
      bool can_appoint_a_diplomat(const realm* invoker, const character* target);
      bool can_appoint_a_spymaster(const realm* invoker, const character* target);
      bool can_appoint_a_merchant(const realm* invoker, const character* target);
      
      // у нас еще существует несколько законов, но не особо важны и большинство из них мы можем проверить отдельно
    }
  }
}

#endif
