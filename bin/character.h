#ifndef CHARACTER_H
#define CHARACTER_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include "utils/id.h"
#include "stats.h"
#include "utils/realm_mechanics.h"
#include "utils/bit_field.h"
#include "data_parser.h"
#include "declare_structures.h"
#include "utils/linear_rng.h"
#include "parallel_hashmap/phmap.h"

// взаимоотношения между персонажами должны быть сделанны с помощью таблицы
// я думал использовать какую-нибудь базу данных 
// для того чтобы не писать самому всякие вещи связанные с сериализацией
// но я беспокоюсь на счет скорости таких бд, то есть мне нужно будет 
// готовить sql запрос для взаимодействия с бд, а это парсинг строки
// это вопервых, во вторых когда работает ии нужно будет обращаться к бд
// нужно чтобы ход не превращался в ход в вахе
// почекал sqlite (бд в памяти), там по идее довольно быстро парсится
// строка, нужно прикинуть как выглядит структура для персонажа

// все данные игры видимо крайне сложно уместить в 500 мб в принципе
// в цк2 я загрузился с 800 какого то года, и игра уже весит 1.5 гб
// что можно сделать с расходом памяти? 
// многие персонажи в игре мертвы и чем дальше тем больше мертвых персонажей
// по идее должен быть способ хранить мертвецов эффективно

namespace devils_engine {
  namespace core {
    struct event_container {
      size_t time; // время (в ходах) когда флаг был добавлен
      std::array<utils::target_data, 8> event_stack; // размер стека получаем по валидным указателям
      event_container() : time(SIZE_MAX) {}
    };
    
    // трейты остаются после смерти персонажа
    template <size_t N>
    struct traits_container {
      size_t count;
      std::array<const trait*, N> container;
      
      traits_container() : count(0), container{nullptr} {}
      bool has(const trait* t) const {
        for (uint32_t i = 0; i < count; ++i) {
          if (container[i] == t) return true;
        }
        
        return false;
      }
      
      void add(const trait* t) {
        if (has(t)) return;
        ASSERT(count < N);
        container[count] = t;
        ++count;
      }
      
      void remove(const trait* t) {
        for (uint32_t i = 0; i < count; ++i) {
          if (container[i] == t) {
            --count;
            std::swap(container[i], container[count]);
            break;
          }
        }
      }
    };
    
    // модификаторы, эвенты и флаги можно будет удалить после смерти персонажа
    template <size_t N>
    struct modificators_container {
      size_t count;
      std::array<std::pair<const modificator*, size_t>, N> container;
      
      modificators_container() : count(0), container{std::make_pair(nullptr, 0)} {}
      bool has(const modificator* m) const {
        for (uint32_t i = 0; i < count; ++i) {
          if (container[i].first == m) return true;
        }
        
        return false;
      }
      
      void add(const modificator* m, const size_t &turn) {
        if (has(m)) return;
        ASSERT(count < N);
        container[count] = std::make_pair(m, turn);
        ++count;
      }
      
      void remove(const modificator* m) {
        for (uint32_t i = 0; i < count; ++i) {
          if (container[i].first == m) {
            --count;
            std::swap(container[i], container[count]);
            break;
          }
        }
      }
    };
    
    template <size_t N>
    struct events_container {
      size_t count;
      std::array<std::pair<const event*, event_container>, N> container;
      
      events_container() : count(0), container{std::make_pair(nullptr, event_container())} {}
      bool has(const event* e) const {
        for (uint32_t i = 0; i < count; ++i) {
          if (container[i].first == e) return true;
        }
        
        return false;
      }
      
      void add(const event* e, const event_container &cont) {
        if (has(e)) return;
        ASSERT(count < N);
        container[count] = std::make_pair(e, cont);
        ++count;
      }
      
      void remove(const event* e) {
        for (uint32_t i = 0; i < count; ++i) {
          if (container[i].first == e) {
            --count;
            std::swap(container[i], container[count]);
            break;
          }
        }
      }
    };
    
    template <size_t N>
    struct flags_container {
      size_t count;
      std::array<size_t, N> container;
      
      flags_container() : count(0), container{0} {}
      bool has(const size_t &f) const {
        for (uint32_t i = 0; i < count; ++i) {
          if (container[i] == f) return true;
        }
        
        return false;
      }
      
      void add(const size_t &f) {
        if (has(f)) return;
        ASSERT(count < N);
        container[count] = f;
        ++count;
      }
      
      void remove(const size_t &f) {
        for (uint32_t i = 0; i < count; ++i) {
          if (container[i] == f) {
            --count;
            std::swap(container[i], container[count]);
            break;
          }
        }
      }
    };
    
    // это частично может быть удалено
//     struct data_storage {
//       static const size_t flags_set_storage_size = 4352u;
//       static const size_t flags_set_max_size = 100;
//       static const size_t modificators_map_storage_size = 4864u;
//       static const size_t modificators_map_max_size = 131;
//       static const size_t event_map_storage_size = 4352u;
//       
//       memory::static_allocator_storage<flags_set_storage_size> flags_set_storage;
//       memory::static_allocator_storage<modificators_map_storage_size> modificators_map_storage;
//       memory::static_allocator_storage<event_map_storage_size> event_map_storage;
//       static_pool_t flags_set_static_pool;
//       static_pool_t modificators_map_static_pool;
//       static_pool_t event_map_static_pool;
//       
//       // не думаю что unordered_map и unordered_set - это хорошая идея
//       // можно вполне использовать vector, так как элементов будет сравнительно мало
//       memory::unordered_map<const modificator*, size_t, static_pool_t> modificators;
//       memory::unordered_map<const event*, event_container, static_pool_t> events;
//       memory::unordered_set<std::string_view, static_pool_t> flags;
//       
//       data_storage();
//     };
    
    struct titulus {
      static const structure s_type = structure::titulus;
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      
      enum class type { // скорее всего нужно добавить еще тип владения конкретным городом (для возможности давать этот титул родственникам например)
        city,  // в цк2 существует титул города
        baron, // в цк2 титул города и баронский титул - это разные вещи, но кажется они каким то образом зависимы друг от друга
        duke,
        king,
        imperial,
//         special,
        
        count
      };
      
      // титул может быть формальным или реальным
      // у титулов существует иерархия
      // имперский титул, королевский титул, герцогский титул, титул барона
      // барон - самый нижний титул, остальные составляются из баронов, герцогств и королевств
      // еще нужно учесть что существует де-юре и де-факто владение титулами
      // де-юре - это земли в массиве childs, а де-факто - это владения персонажа для которого этот титул является определяющим
      // если титул не является главным, то де-факто - все земли у владельца титула
      
      std::string id;
      enum type type;
      uint32_t count; // если 0 то это специальный титул
      union {
        titulus** childs; // реальный титул обладает определенным набором титулов нижнего уровня
        titulus* child;
        struct city* city;         // титул города
        struct province* province; // баронский титул
//         struct {
//           uint32_t provinces[2]; // либо это баронский титул
//         };
      };
      titulus* parent; // если мы создем титул, то может ли у текущего быть два титула верхнего уровня?
      faction* owner;  // у титула может быть только один владелец (фракция титулы наследует и передает)
      size_t name_str;
      size_t description_str;
      size_t adjective_str;
      titulus* next;
      titulus* prev;
      render::color_t main_color;    // цвет будет использоваться на глобальной карте для страны
      render::color_t border_color1; // думаю двух цветов достаточно, нужно посмотреть как сделано в цк2
      render::color_t border_color2;
      uint32_t heraldy;
      
      // думаю что в титулах маленькие контейнеры потребуются
      //events_container<events_container_size> events; // должно хватить
      //flags_container<flags_container_size> flags; // флагов довольно много
      phmap::flat_hash_map<const event*, event_container> events;
      phmap::flat_hash_map<std::string_view, size_t> flags;
      
      titulus();
      titulus(const enum type &t);
      titulus(const enum type &t, const uint32_t &count);
      ~titulus();
      bool is_formal() const;
      void set_child(const uint32_t &index, titulus* child);
      titulus* get_child(const uint32_t &index) const;
      void set_province(struct province* province);
      struct province* get_province() const;
      void set_city(struct city* city);
      struct city* get_city() const;
      
      void create_children(const uint32_t &count);
      
      std::string_view full_name() const;
      std::string_view base_name() const;
      std::string_view ruler_title() const;
      std::string_view adjective() const;
      std::string_view form_of_address() const; 
    };
    
    // как династию создать, нужно указать id?
    struct dynasty {
      size_t name_str;
      std::vector<character*> characters;
      // герб (нужно выбрать тип щита, темплейт, изображения на темплейт, изображения сверху, цвета для всего этого, можно ли за один раз нарисовать?)
      // некие механики династии?
      // возможно какие то улучшения авторитета, уважение и влияния?
      // ии династии
      
      
    };
    
    // нужно будет еще много чего добавить (аттрибуты, религию, двор, ваасалов, ...)
    // отношения вычисляются на основе перков и особенностей
    // как персонаж выглядит
    // нужно придумать способ ссылаться именно на этого перса при генерации, индекс? 
    // похоже самый адекватный способ (с id не понятно че делать, он нужен только при генерации)
    // можно еще добавить хронику - эвенты заполняют историю мира
    struct character {
      static const structure s_type = structure::character;
      static const size_t traits_container_size = 100;
      static const size_t modificators_container_size = 75;
      static const size_t events_container_size = 30; // поди больше нужен
      static const size_t flags_container_size = 50;
      
      using state = utils::xoshiro256plusplus::state;
      
      static bool is_cousin(const character* a, const character* b);
      static bool is_sibling(const character* a, const character* b);
      static bool is_full_sibling(const character* a, const character* b);
      static bool is_half_sibling(const character* a, const character* b);
      static bool is_relative(const character* a, const character* b);
      static bool is_bastard(const character* a);
      static bool is_concubine_child(const character* a);
      static bool is_concubine(const character* a, const character* b);
      
      // придется создавать несколько фракции: собственно персонаж,
      // совет (парламент), суд, элективный огран, все это для того чтобы 
      // правильно передать титулы, в цк2 настройки наследования были у титулов
      // мне нужно наверное сделать наследование по фракции 
      // суд по идее может отобрать титул, где то же он должен храниться
      enum faction_type {
        self,
        council,
        tribunal,
        elective,
        faction_type_count
      };
      
      struct family {
        // нужно еще указать реального отца и текущего
        // скорее всего должен быть еще какой нибудь статус, который при рождении выдается
        character* real_parents[2];
        character* parents[2];
//         character* grandparents[4]; // дедов можем получить из отцов
        character* children;
        character* next_sibling;
        character* prev_sibling;
        character* consort; // как запомнить мертвых жен?
        character* previous_consorts;
        character* owner;
        character* concubines;
        struct dynasty* blood_dynasty;
        struct dynasty* dynasty;
        
        family();
      };
      
      struct relations {
        static const size_t max_game_friends = 8;
        static const size_t max_game_rivals = 8;
        static const size_t max_game_lovers = 4;
        
        // друзья, враги, брачные узы, любовники (сколько выделять памяти?)
        std::array<character*, max_game_friends> friends; // врядли будет больше 4-6
        std::array<character*, max_game_rivals> rivals;  // тут тоже многовато 16
        // брак близкого родственника с человеком из другой династии (может быть довольно много, можно ограничить только текущим поколением)
        // если ограничить текущим поколением, то не нужно даже запоминать
        //character* marriage_ties[16];
        std::array<character*, max_game_lovers> lovers;  // вряд ли больше 2-3
        
        relations();
      };
      
      std::array<stat_container, character_stats::count> stats; // по умолчанию
      std::array<stat_container, character_stats::count> current_stats; // пересчитаные статы от треитов, шмоток и модификаторов
      std::array<stat_container, hero_stats::count> hero_stats;
      std::array<stat_container, hero_stats::count> current_hero_stats;
      uint32_t name_number; // если в династии уже есть использованное имя, то тут нужно указать сколько раз встречалось
//       float money; // деньги наследуются
      
      int64_t born_day;
      int64_t death_day; // причина?
      size_t name_str;
      size_t nickname_str; // как выдается ник? 
      character* suzerain; // текущий персонаж при ждворе этого
      faction* imprisoner; // мы можем сидеть во фракционной тюрьме (государственная тюрьма)
      character* next_prisoner;
      character* prev_prisoner;
      character* next_courtier;
      character* prev_courtier;
      
      // здесь еще должны добавиться указатели на данные героя и армии
      // героем персонаж может стать если выполнит определенные условия (статы + может что то еще)
      // армия у персонажа может появиться если он нанят как полководец или правитель
      // как создать армию верно? мне нужно выделить память и найти свободный армейский слот
      struct hero_troop* troop; // как подтвердить владение?
      struct army* army;        // как подтвердить владение?
      
      struct family family;
      struct relations relations;
      const struct culture* culture;
      const struct religion* religion;
      const struct religion* hidden_religion;
      // в элективных монархиях существует фракция персонажа, фракция элективного государства (в этой фракции титулы передаются к избранному наследнику),
      // по идее нужно будет создать еще фракцию совета, там временно будут храниться отобранные титулы
      std::array<struct faction*, faction_type_count> factions; // если здесь есть указатели на совет и на суд, то персонаж состоит в совете или в суде
      // совет строго относится к текущему правителю (то есть если даже избирательная система у вассалов, то совет набирается значит из избранных вассалов)
      
      utils::bit_field<1> data;
      state rng_state; // нужно ли передать в луа?
      
      // можем ли мы разделить трейты на группы? можем, сколько теперь выделять для каждой группы
      // в цк2 трейты разделены на группы, некоторые трейты замещают другие в одной группе
      // некоторые трейты не могут быть взяты из-за религии
      //traits_container<traits_container_size> traits;
      // после смерти нам это не нужно
      //modificators_container<modificators_container_size>* modificators;
      //events_container<events_container_size>* events;
      //flags_container<flags_container_size>* flags;
      
      phmap::flat_hash_set<const trait*> traits;
      // модификаторы, затронутые эвенты и флаги не нужны после смерти
      phmap::flat_hash_map<const modificator*, size_t> modificators;
      phmap::flat_hash_map<const event*, event_container> events;
      // нужно ли как то оформять флаги? нет, это будут строго внутренние вещи
      phmap::flat_hash_map<std::string_view, size_t> flags;
      //phmap::flat_hash_set<std::string_view> flags; // а может вообще без хода?
      
      character(const bool male, const bool dead);
      ~character();
      bool is_independent() const;
      bool is_prisoner() const;
      bool is_married() const;
      bool is_male() const;
      bool is_hero() const;
      bool is_player() const;
      bool is_dead() const;
      bool has_dynasty() const;
      bool is_ai_playable() const;
      bool is_troop_owner() const; // было бы неплохо оставить указатель после смерти, чтобы разместить там известных членов партии для истории
      bool is_army_owner() const;
      
      void set_dead();
      void make_hero();
      void make_player();
      
      void add_title(titulus* title);
      void remove_title(titulus* title);
      titulus* get_main_title() const;
      
      void add_vassal(character* vassal);
      void remove_vassal(character* vassal);
      
      void add_courtier(character* courtier);
      void add_courtier_raw(character* courtier);
      void remove_courtier(character* courtier);
      character* get_last_courtier() const;
      
      void add_prisoner(character* prisoner);
      void remove_prisoner(character* prisoner);
      
      void add_concubine(character* concubine);
      void add_concubine_raw(character* concubine);
      void remove_concubine(character* concubine);
      
//       void add_child(character* child); // ребенок должен быть чей то
      void add_child_raw(character* child); // возможно будет только эта функция 
//       void remove_child(character* child); // врядли вообще когда нибудь я этим воспользуюсь
      
      // это работа я так понимаю с базовыми статами, текущие обновляются в другом месте
      // может ли вообще потребоваться из луа что то изменять?
      float stat(const uint32_t &index) const;
      void set_stat(const uint32_t &index, const float &value);
      float add_to_stat(const uint32_t &index, const float &value);
      float hero_stat(const uint32_t &index) const;
      void set_hero_stat(const uint32_t &index, const float &value);
      float add_to_hero_stat(const uint32_t &index, const float &value);
      
      bool get_bit(const size_t &index) const; // тут по максимуму от того что останется
      bool set_bit(const size_t &index, const bool value);
      
//       bool has_flag(const size_t &flag) const;
//       void add_flag(const size_t &flag);
//       void remove_flag(const size_t &flag);
      
      size_t has_flag(const std::string_view &flag) const;
      void add_flag(const std::string_view &flag, const size_t &turn);
      void remove_flag(const std::string_view &flag);
      
      bool has_trait(const trait* t) const;
      void add_trait(const trait* t);
      void remove_trait(const trait* t);
      
      bool has_modificator(const modificator* m) const;
      void add_modificator(const modificator* m, const size_t &turn);
      void remove_modificator(const modificator* m);
      
      bool has_event(const event* e) const;
      void add_event(const event* e, const event_container &cont);
      void remove_event(const event* e);
      
      uint64_t get_random();
      
      // откуда это брать? по идее это задано по умолчанию, но при этом нужно как то локализацию сделать
      std::string_view object_pronoun() const;
      std::string_view poss_pronoun() const;
      std::string_view reflexive_pronoun() const;
      std::string_view subject_pronoun() const;
      std::string_view gender() const;
      std::string_view man_woman() const;
      std::string_view boy_girl() const;
      std::string_view boy_man_girl_woman() const;
      std::string_view spouse_type() const;
      std::string_view parent_type() const;
      std::string_view child_type() const;
      std::string_view master_gender() const;
      std::string_view lad_gender() const;
      std::string_view lord_gender() const;
      std::string_view king_gender() const;
      std::string_view emperor_gender() const;
      std::string_view patriarch_gender() const;
      std::string_view sibling_gender() const;
      
      // эти штуки нужно составлять из разных строк, по идее тут нужно возвращать обычный std string
      // я уже забыл что есть что
      std::string_view first_name() const;
      std::string_view first_name_with_nick() const;
      std::string_view title_str() const;
      std::string_view titled_name() const;
      std::string_view titled_name_with_nick() const;
      std::string_view titled_first_name() const;
      std::string_view titled_first_name_with_nick() const;
      std::string_view titled_first_name_no_regnal() const;
      std::string_view dyn_name() const;
      std::string_view full_name() const;
      std::string_view best_name() const;
      std::string_view best_name_no_regnal() const;
    };
    
    struct faction {
      static const structure s_type = structure::faction;
      
      character* leader;    // текущий глава фракции (то есть например король) (его может не быть если это особая фракция)
      character* heir;      // персонаж наследник, к нему отойдут все титулы (это может быть элективный наследник)
      // деньги нужно передать если это личная фракция

      // всякие технические вещи
      // господин и вассалы наследуются
      faction* liege;    // фракция сюзерена
      faction* state;    // фракция государства (либо элективный орган, либо фракция персонажа (то есть указатель на самого себя))
      faction* council;  // фракция парламента (кто входит в парламент? по идее указывается у конкретного персонажа)
      faction* tribunal; // фракция суда (кто входит в суд?)
      
      faction* vassals;
      faction* next_vassal; // персонажи с одним сюзереном
      faction* prev_vassal;
      
      titulus* titles; // если персонаж избран то у персонажа появляются титулы непосредственно государства
      titulus* main_title;
      
      // по идее придворные тоже наследуются
      character* courtiers;
      character* prisoners; // в тюрьму мы сажаем непосредственно персонажей
      // глобальные характеристики
      std::array<stat_container, faction_stats::count> stats; // кому уходят характеристики? по идее если это не личная фракция, то всем
      // статы должны пересчитываться после наследования? я пока не очень понимаю как тут вообще статы будут
      // принятые законы (законы по категориям, причем первые две категории всегда про наследование)
      // права жителей фракции (скорее всего едины с законами) (состоят из прав религии и локальных прав, что то еще?) (более менее сделано)
      utils::bit_field_32<utils::realm_mechanics::bit_container_size> mechanics; // это по сути и будут нашими законами
      // конкретные законы и их группировка задается в структуре
      
      faction();
      
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline void set_mechanic(const size_t &index, const bool value) { mechanics.set(index, value); }
      
      // нужно выделить 4 состояния: государство независимого персонажа, государство зависимого персонажа, элективное государство и зависимое элективное государство
      inline bool is_independent() const { return liege == nullptr; }
      inline bool is_state() const { return state == this; }      
      inline bool is_council() const { return council == this; }
      inline bool is_tribunal() const { return tribunal == this; }
      inline bool is_self() const { return !is_state() && !is_council() && !is_tribunal(); }
      
      void succession();
      void usurped_by_self();
      void usurped_by_council();
      void create_state();
      void create_council();
      void create_tribunal();
      
      void add_title(titulus* title);
      void remove_title(titulus* title);
      titulus* get_last_title() const;
      
      void add_vassal(faction* vassal);
      void add_vassal_raw(faction* vassal);
      void remove_vassal(faction* vassal);
      faction* get_last_vassal() const;
      
      void add_prisoner(character* prisoner);
      void add_prisoner_raw(character* prisoner);
      void remove_prisoner(character* prisoner);
      character* get_last_prisoner() const;
    };
  }
}

// нам нужно нарисовать портреты всех персонажей
// портретов будет довольно много + 
// желательно чтобы не было лагов при открывании панельки персонажа
// детей (а значит братьев и сестер) может быть реально сильно больше чем 64
// особенно если персонаж бессмертен
// возможно имеет смысл хранить информацию всю в династии
// точнее она и так будет храниться в династии, нужно только ее оттуда подгрузить
// в персонаже должен хранится указатель на династию
// а из династии мы загрузим все данные о семье
// так же бы сделать и с титулами и с вассалами и видимо со двором
// вот и получается база данных
// у персонажа: сюзерен, династия родная, династия текущая, родители + аттрибуты и проч
// жену получаем из переменной consort, а наложниц ищем в таблице по той же переменной
// когда наложница умирает, то нужно убрать ее из поиска
// а вот мерных жен нужно учитывать
// дружеские отношения? 

// титулов строго ограниченное число + не может быть один титул у удвух персонажей
// а значит мы можем положить такого рода данные в список по указателям

// я не очень понимаю как будет выглядеть ход
// мне нужно определить как то персонажа который будет ходить
// причем будет ли это именно персонаж либо нужно какую то фракцию организовать? нет
// нужен быстрый доступ к провинциям под непосредственным контролем, к войскам
// не через титулы... в начале хода можно для всех персонажей пересчитать войска
// а во время хода обойти провинции и проверить что да как
// то есть я думаю что может быть и не надо что то придумывать
// короче нужно просто обновлять список с персонажами

// после титулов можно создавать персонажей
// что такое персонаж: это объект обладающий рядом титулов (реальный титул дает этому персонажу право хода)
// и обладающий рядом прав, зависящих от религии, культуры (?) и текущего политического строя
// например право наследования, право передачи титула, право занимать должности (женщины в основном ограничены в этом праве) и прочие
// (еще должны быть какие то экономические права (дань), что то еще?)
// у персонажа должны быть ряд характеристик важные из которых это прежде всего
// количество земель под непосредственным контролем и количество вассалов
// число зависит от строя, характеристик персонажа (???), культурных особенностей и религии (возможно)
// мне с одной стороны не нравится изменение количества от характеристик персонажа
// с другой стороны иначе это может превратиться в выфармливание нужной религии и культуры
// персонажей будет сильно больше чем титулов, почти всем владельцам реальных титулов нужно придумать 
// двор, семью, некоторых сторонних персонажей (главу религии, например)
// семья должна быть продумана почти у всех на несколько поколений в прошлое (желательно)
// у семьи должна быть какая то история
// сначало нужно задать: титулы, базовые права, характеристики (?), семью
// часть двора будет состоять из вассалов и семьи, еще некоторую нужно сгенерировать
// ко всему прочему нужно сгенерировать немного формальных титулов

// для работы с советом и судом у нас должны быть десижены
// у нас еще есть должности в государстве

#endif

