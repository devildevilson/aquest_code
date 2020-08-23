#ifndef CHARACTER_H
#define CHARACTER_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include "utils/id.h"
#include "stats.h"
#include "utils/bit_field.h"
#include "data_parser.h"

#include <foonathan/memory/container.hpp> // vector, list, list_node_size
#include <foonathan/memory/memory_pool.hpp> // memory_pool
#include <foonathan/memory/smart_ptr.hpp> // allocate_unique
#include <foonathan/memory/static_allocator.hpp> // static_allocator_storage, static_block_allocator
#include <foonathan/memory/temporary_allocator.hpp> // temporary_allocator

// alias namespace foonathan::memory as memory for easier access
//#include <foonathan/memory/namespace_alias.hpp>

using namespace foonathan;

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
    using static_pool_t = memory::memory_pool<memory::node_pool, memory::static_block_allocator>;
    
    struct character;
    struct culture;
    struct religion;
    struct modificator;
    struct trait;
    struct event;
    struct faction;
    
    struct event_container {
      size_t time; // время (в ходах) когда флаг был добавлен
      std::array<utils::target_data, 8> event_stack; // размер стека получаем по валидным указателям
    };
    
    // трейты остаются после смерти персонажа
    template <size_t N>
    struct traits_container {
      size_t count;
      std::array<const trait*, N> container;
    };
    
    // модификаторы, эвенты и флаги можно будет удалить после смерти персонажа
    template <size_t N>
    struct modificators_container {
      size_t count;
      std::array<std::pair<const modificator*, size_t>, N> container;
    };
    
    template <size_t N>
    struct events_container {
      size_t count;
      std::array<std::pair<const event*, event_container>, N> container;
    };
    
    template <size_t N>
    struct flags_container {
      size_t count;
      std::array<size_t, N> container;
    };
    
    // это частично может быть удалено
    struct data_storage {
      static const size_t flags_set_storage_size = 4352u;
      static const size_t flags_set_max_size = 100;
      static const size_t modificators_map_storage_size = 4864u;
      static const size_t modificators_map_max_size = 131;
      static const size_t event_map_storage_size = 4352u;
      
      memory::static_allocator_storage<flags_set_storage_size> flags_set_storage;
      memory::static_allocator_storage<modificators_map_storage_size> modificators_map_storage;
      memory::static_allocator_storage<event_map_storage_size> event_map_storage;
      static_pool_t flags_set_static_pool;
      static_pool_t modificators_map_static_pool;
      static_pool_t event_map_static_pool;
      
      // не думаю что unordered_map и unordered_set - это хорошая идея
      // можно вполне использовать vector, так как элементов будет сравнительно мало
      memory::unordered_map<const modificator*, size_t, static_pool_t> modificators;
      memory::unordered_map<const event*, event_container, static_pool_t> events;
      memory::unordered_set<std::string_view, static_pool_t> flags;
      
      data_storage();
    };
    
    struct titulus {
      static const size_t events_container_size = 15;
      static const size_t flags_container_size = 25;
      
      enum class type { // скорее всего нужно добавить еще тип владения конкретным городом (для возможности давать этот титул родственникам например)
//         city, 
        baron, // это титул города! он играбельный тогда когда это главный титул в провинции
        duke,
        king,
        imperial,
//         special
      };
      
      // титул может быть формальным или реальным
      // у титулов существует иерархия
      // имперский титул, королевский титул, герцогский титул, титул барона
      // барон - самый нижний титул, остальные составляются из баронов, герцогств и королевств
      // еще нужно учесть что существует де-юре и де-факто владение титулами
      // де-юре - это земли в массиве childs, а де-факто - это владения персонажа для которого этот титул является определяющим
      // если титул не является главным, то де-факто - все земли у владельца титула
      //utils::id id; // тут опять же id не будет наверное
      enum type type;
      uint32_t count; // если 0 то это специальный титул
      union {
        titulus** childs; // реальный титул обладает определенным набором титулов нижнего уровня
        titulus* child;
        struct {
          uint32_t provinces[2]; // либо это баронский титул
        };
      };
      titulus* parent; // если мы создем титул, то может ли у текущего быть два титула верхнего уровня?
      character* owner; // у титула может быть только один владелец
      size_t name_str;
      size_t description_str;
      size_t adjective_str;
      titulus* next;
      titulus* prev;
      // герб
      
      // думаю что в титулах маленькие контейнеры потребуются
      events_container<events_container_size> events; // должно хватить
      flags_container<flags_container_size> flags; // флагов довольно много
      
      titulus(const enum type &t);
      titulus(const enum type &t, const uint32_t &count);
      ~titulus();
      bool is_formal() const;
      void set_child(const uint32_t &index, titulus* child);
      titulus* get_child(const uint32_t &index) const;
      void set_province(const uint32_t &province_index);
      uint32_t get_province() const;
      
      std::string_view full_name() const;
      std::string_view base_name() const;
      std::string_view ruler_title() const;
      std::string_view adjective() const;
      std::string_view form_of_address() const; 
    };
    
    struct dynasty {
      size_t name_str;
      std::vector<character*> characters;
      // герб
      // некие механики династии?
      // возможно какие то улучшения авторитета, уважение и влияния?
    };
    
    // нужно будет еще много чего добавить (аттрибуты, религию, двор, ваасалов, ...)
    // отношения вычисляются на основе перков и особенностей
    // как персонаж выглядит
    struct character {
      static const size_t traits_container_size = 100;
      static const size_t modificators_container_size = 75;
      static const size_t events_container_size = 30;
      static const size_t flags_container_size = 50;
      
      static bool is_cousin(const character* a, const character* b);
      static bool is_sibling(const character* a, const character* b);
      static bool is_full_sibling(const character* a, const character* b);
      static bool is_half_sibling(const character* a, const character* b);
      static bool is_relative(const character* a, const character* b);
      static bool is_bastard(const character* a);
      static bool is_concubine_child(const character* a);
      static bool is_concubine(const character* a, const character* b);
      
      struct family {
        // нужно еще указать реального отца и текущего
        // скорее всего должен быть еще какой нибудь статус, который при рождении выдается
        character* parents[2];
        character* grandparents[4];
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
        // друзья, враги, брачные узы, любовники (сколько выделять памяти?)
        character* friends[8]; // врядли будет больше 4-6
        character* rivals[8];  // тут тоже многовато 16
        // брак близкого родственника с человеком из другой династии (может быть довольно много, можно ограничить только текущим поколением)
        // если ограничить текущим поколением, то не нужно даже запоминать
        //character* marriage_ties[16];
        character* lovers[4];  // вряд ли больше 2-3
        
        relations();
      };
      
      stat_container stats[character_stats::count]; // по умолчанию
      stat_container current_stats[character_stats::count]; // пересчитаные статы от треитов, шмоток и модификаторов
      stat_container hero_stats[hero_stats::count];
      stat_container current_hero_stats[hero_stats::count];
      uint32_t name_number; // если в династии уже есть использованное имя, то тут нужно указать сколько раз встречалось
      float money; // деньги наследуются
      
      size_t born_date;
      size_t death_date; // причина?
      size_t name_str;
      size_t nickname_str; // как выдается ник?
      character* suzerain;
      
      struct family family;
      struct relations relations;
      titulus* titles; // титулы наследуются
      const struct culture* culture;
      const struct religion* religion;
      const struct religion* hidden_religion;
      struct faction* faction;
      
      utils::bit_field<1> data;
      
      // можем ли мы разделить трейты на группы? можем, сколько теперь выделять для каждой группы
      // в цк2 трейты разделены на группы, некоторые трейты замещают другие в одной группе
      // некоторые трейты не могут быть взяты из-за религии
      traits_container<traits_container_size> traits;
      // после смерти нам это не нужно
      modificators_container<modificators_container_size>* modificators; // по идее их меньше чем треитов
      events_container<events_container_size>* events; // должно хватить
      flags_container<flags_container_size>* flags; // флагов довольно много
      
      character();
      ~character();
      bool is_independent() const;
      bool is_male() const;
      bool is_hero() const;
      bool is_player() const;
      bool is_dead() const;
      
      void add_title(titulus* title);
      void remove_title(titulus* title);
      
      float stat(const uint32_t &index) const;
      void set_stat(const float &value);
      float hero_stat(const uint32_t &index) const;
      void set_hero_stat(const float &value);
      
      bool get_bit(const size_t &index); // тут по максимуму от того что останется
      void set_bit(const size_t &index, const bool value);
      
      bool has_flag(const std::string_view &str) const;
      void add_flag(const std::string_view &str);
      void remove_flag(const std::string_view &str);
      
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

#endif

