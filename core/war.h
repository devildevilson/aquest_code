#ifndef WAR_H
#define WAR_H

#include <cstddef>
#include <cstdint>
#include <vector>

// война, тут прежде всего нужно указать из-за чего война,
// кто стартанул и против кого воюем, клаймат, участников
// титулы которые мы получим
// что то еще? то как идет война указывается в казус белли
// где указывать войны? в реалме, нужно выполнять проверки
// на то сохранился ли альянс между участниками 
// и могут ли они продолжать конкретную войну

namespace devils_engine {
  namespace core {
    struct casus_belli;
    struct character;
    struct realm;
    struct titulus;
    
    struct war {
      const casus_belli* cb;
      const character* war_opener;
      const character* target_character;
      const realm* opener_realm;
      const realm* target_realm;
      const character* claimat;
      // титулов может быть много, а вот челиков на войне не очень
      // для челиков наверное можно сделать array
      std::vector<const titulus*> target_titles;
      std::vector<const realm*> attackers;
      std::vector<const realm*> defenders;
    };
  }
}

#endif
