#ifndef WAR_H
#define WAR_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include "declare_structures.h"
#include "utils/structures_utils.h"

// война, тут прежде всего нужно указать из-за чего война,
// кто стартанул и против кого воюем, клаймат, участников
// титулы которые мы получим
// что то еще? то как идет война указывается в казус белли
// где указывать войны? в реалме, нужно выполнять проверки
// на то сохранился ли альянс между участниками 
// и могут ли они продолжать конкретную войну

namespace devils_engine {
  namespace core {
    // какое время жизни у этой структуры? в конце войны удаляем структуру после вызова эвентов?
    // структуру создаем в эффектах, основной страх заключается в том что появится возможность на спавнить кучу войн без контрольно
    // когда удалять? война окончена + если война стала невалидной 
    struct war : public utils::flags_container, public utils::events_container {
      static const core::structure s_type = core::structure::war;
      
      const casus_belli* cb; // можно ли создать казус белли по ходу игры? вряд ли
      // если персонаж умирает то при элективном государстве война продолжается,
      // в ином случае война заканчивается
      const character* war_opener;
      const character* target_character;
      const realm* opener_realm;
      const realm* target_realm;
      // может ли быть государственный клейм? скорее да
      // главное не забыть что при элективном государстве self и elective realm - это разные вещи
      const realm* claimat;
      // титулов может быть много, а вот челиков на войне не очень
      // для челиков наверное можно сделать array
      std::vector<const titulus*> target_titles;
      std::vector<const realm*> attackers;
      std::vector<const realm*> defenders;
      
      // естественно конец и начало войны вызывают какие то скрипты
      // также война может быть таргетом в скрипте
      
      // возможно тут еще нужно указать какие то конкретные вещи из казус белли
      
      war();
    };
  }
}

#endif
