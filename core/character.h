#ifndef DEVILS_ENGINE_CORE_CHARACTER_H
#define DEVILS_ENGINE_CORE_CHARACTER_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include "parallel_hashmap/phmap.h"

#include "declare_structures.h"
#include "stats.h"
#include "stat_data.h"

#include "utils/bit_field.h"
#include "utils/linear_rng.h"
#include "utils/structures_utils.h"
#include "utils/list.h"
#include "utils/static_stack.h"
#include "utils/static_vector.h"
#include "utils/sol.h"
#include "utils/handle.h"

#include "script/get_scope_commands_macro.h"
#include "script/condition_commands_macro.h"

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

// так ли нужен отряд героя на ворлд мапе? хороший вопрос, вот я что думаю
// герой нужен для разведки и каких то приключений, нужно добавить всяких полезных штук для героя
// на карте так чтобы герои нужны были более менее всегда + нужно почитать что люди думают про 
// героев в тотал варе

// может быть задавать динамически? имеет смысл по крайней мере задать все эти значения в каком нибудь конфиге
#define CHARACTER_ADULT_AGE 16

namespace devils_engine {
  namespace utils {
    class world_serializator;
  }
  
  namespace core {
    // как династию создать, нужно указать id?
    // кстати ход может быть оформлен по династии (то есть династии ходят друг за другом)
    struct dynasty {
      static const structure s_type = structure::dynasty;
      
      size_t name_str;
      character* characters;
      // герб (нужно выбрать тип щита, темплейт, изображения на темплейт, изображения сверху, цвета для всего этого, можно ли за один раз нарисовать?)
      // некие механики династии?
      // возможно какие то улучшения авторитета, уважение и влияния?
      // ии династии
      
      
    };
    
    // нужно ли выделять в отдельную структуру (все равно потом унаследуемся от этого)?
    struct hero {
      character* leader;
      character* companions; // резерв?
      utils::stats_container<hero_stats::values> hero_stats;
      utils::stats_container<hero_stats::values> current_hero_stats;
      utils::handle<core::hero_troop> troop;
      
      hero();
    };
    
    // нужно будет еще много чего добавить (аттрибуты, религию, двор, ваасалов, ...)
    // отношения вычисляются на основе перков и особенностей
    // как персонаж выглядит
    // нужно придумать способ ссылаться именно на этого перса при генерации, индекс? 
    // похоже самый адекватный способ (с id не понятно че делать, он нужен только при генерации)
    // можно еще добавить хронику - эвенты заполняют историю мира
    // тут имеет смысл еще добавить листы с религией, культурой и династией, возможно лист с тайной религией
    struct character : 
      public utils::flags_container, 
      public utils::events_container, 
      public utils::modificators_container,
      public utils::traits_container, 
      public utils::hooks_container,
      public utils::ring::list<character, utils::list_type::prisoners>,
      public utils::ring::list<character, utils::list_type::courtiers>,
      public utils::ring::list<character, utils::list_type::concubines>,
      public utils::ring::list<character, utils::list_type::victims>,
      public utils::ring::list<character, utils::list_type::hero_companions>,
      public utils::ring::list<character, utils::list_type::culture_member>,
      public utils::ring::list<character, utils::list_type::dynasty_member>,
      public utils::ring::list<character, utils::list_type::believer>,
      public utils::ring::list<character, utils::list_type::secret_believer>,
      public utils::ring::list<character, utils::list_type::father_line_siblings>,
      public utils::ring::list<character, utils::list_type::mother_line_siblings>,
      // члены политических сил, как отделить электоров? еще запихивать листы? еще 80 байт лол
      public utils::ring::list<character, utils::list_type::statemans>, // этого скорее всего не будет, в качестве стейта выступает либо собственный реалм, либо один из коллективных
      public utils::ring::list<character, utils::list_type::councilors>,
      public utils::ring::list<character, utils::list_type::magistrates>,
      public utils::ring::list<character, utils::list_type::assemblers>,
      public utils::ring::list<character, utils::list_type::clergymans>,
      // электорат политических сил, он будет выбирать членов или главу
      public utils::ring::list<character, utils::list_type::state_electors>,
      public utils::ring::list<character, utils::list_type::council_electors>,
      public utils::ring::list<character, utils::list_type::tribunal_electors>,
      public utils::ring::list<character, utils::list_type::assembly_electors>,
      public utils::ring::list<character, utils::list_type::clergy_electors>
    {
      static const structure s_type = structure::character;
      // предварительные размеры
      static const size_t traits_container_size = 25;
      static const size_t modificators_container_size = 25;
      static const size_t events_container_size = 10;
      static const size_t flags_container_size = 25;
      
      using state = utils::xoshiro256plusplus::state;
      
      // придется создавать несколько фракции: собственно персонаж,
      // совет (парламент), суд, элективный огран, все это для того чтобы 
      // правильно передать титулы, в цк2 настройки наследования были у титулов
      // мне нужно наверное сделать наследование по фракции 
      // суд по идее может отобрать титул, где то же он должен храниться
      enum faction_type {
        establishment,
        council,
        tribunal,
        assembly,
        clergy,
        faction_type_count
      };
      
      struct family {
        // нужно еще указать реального отца и текущего
        // скорее всего должен быть еще какой нибудь статус, который при рождении выдается
        character* real_parents[2];
        character* parents[2];
//         character* grandparents[4]; // дедов можем получить из отцов
        character* children;
//         character* next_sibling;
//         character* prev_sibling;
        character* consort; // как запомнить мертвых жен?
//         character* previous_consorts;
        utils::static_stack<character*, 16> consorts; // предыдущие жены, здесь не должно быть текущей
        character* owner;
        character* concubines;
        struct dynasty* blood_dynasty;
        struct dynasty* dynasty;
        
        family() noexcept;
      };
      
      // отношения должны быть сложнее, скорее нужно список расширить и сделать единственным
      struct relations {
        enum friendship_level : int32_t {
          nemesis = -3,
          rival,
          foe,
          opponent,
          pal,
          mate,
          best_friend
        };
        
        enum love_level : int32_t {
          bete_noire = -3,
          hate,
          dislike,
          neutral,
          sympathy,
          lover,
          soulmate
        };
        
        struct data {
          int32_t friendship;
          int32_t love;
        };
        
        static const size_t max_game_acquaintance = 64;
        
        std::array<std::pair<character*, data>, max_game_acquaintance> acquaintances;
        
        relations() noexcept;
        
        bool is_acquaintance(character* c, int32_t* friendship = nullptr, int32_t* love = nullptr) const;
        bool add_acquaintance(character* c, int32_t friendship_level, int32_t love_level);
        bool remove_acquaintance(character* c);
        void remove_all_neutral();
      };
      
      // секрет, что это такое? секреты - это некоторые характеристики отношений между персонажами
      // и некоторые характеристики треитов или модификаторов (?), их конечное количество и проверяются они
      // по параметрам персонажа, вообще проверки можно сделать полностью в луа и соответственно 
      // дать возможность задать модерам все эти вещи, но пока оставим так
      // в этой структуре зададим те секреты которые уже стали известны публике
      struct secret {
        uint32_t type;
        const struct character* character;
        const struct trait* trait;
        
        inline secret() : type(UINT32_MAX), character(nullptr), trait(nullptr) {}
      };
      
      struct relation {
        uint32_t type;
        utils::handle<core::war> war;
        utils::handle<core::realm> realm; 
        // например жена, или супруг ребенка, вообще уз может быть несколько (до бесконечности)
        // как быть? опять массив? кто дает гарантии? только семья? близкие родственники?
        // нужно начать с того чтобы определить кто есть кто, все близкие родственники могут оказаться причиной возникновения дипломатии
        character* related_character;
      };
      
      utils::stats_container<character_stats::values, true> stats;
      utils::stats_container<character_stats::values, true> current_stats;
      utils::stats_container<hero_stats::values> hero_stats;
      utils::stats_container<hero_stats::values> current_hero_stats;
      utils::stats_container<character_resources::values> resources;
      utils::static_vector<secret, 16> known_secrets;
      
      uint32_t name_number; // если в династии уже есть использованное имя, то тут нужно указать сколько раз встречалось
      
      int64_t born_day;
      int64_t death_day; // причина?
      // что имя, что никнейм нам нужны скомпилированные для поиска
      // для никнейма наверное нужно сделать отдельную структуру, и там могут храниться дополнительные данные, например за что этот никнейм дали
      std::string name_table_id; // теперь нам не обязательно хранить индексы
      std::string nickname_table_id;
      uint32_t name_index;
      uint32_t nickname_index; // как выдается ник? 
      utils::handle<realm> suzerain; // может ли быть при дворе государства?
      utils::handle<realm> imprisoner; // мы можем сидеть во фракционной тюрьме (государственная тюрьма)
      character* victims; // если вообще я буду делать удаление персонажей, то может удалиться первая жертва, вообще наверное мы можем предусмотреть это в деструкторе
      character* killer;
      
      // здесь еще должны добавиться указатели на данные героя и армии
      // героем персонаж может стать если выполнит определенные условия (статы + может что то еще)
      // армия у персонажа может появиться если он нанят как полководец или правитель
      // как создать армию верно? мне нужно выделить память и найти свободный армейский слот
      // геройский отряд создаем только в момент выхода из города (или в эвентах?)
      // нам также нужно где то здесь перечислить текущих спутников героя, у героя еще должен быть инвентарь
      // нужно сделать драг'н'дроп
      utils::handle<core::hero_troop> troop;
      utils::handle<core::army> army;
      
      struct family family;
      struct relations relations;
      struct culture* culture;
      struct religion* religion;
      struct religion* secret_religion;
      // в элективных монархиях существует фракция персонажа, фракция элективного государства (в этой фракции титулы передаются к избранному наследнику),
      // по идее нужно будет создать еще фракцию совета, там временно будут храниться отобранные титулы
      utils::handle<realm> self;
      std::array<utils::handle<realm>, faction_type_count> realms; // если здесь есть указатели на совет и на суд, то персонаж состоит в совете или в суде
      // совет строго относится к текущему правителю (то есть если даже избирательная система у вассалов, то совет набирается значит из избранных вассалов)
      // вообще указатели на эти вещи есть и у реалма, но как быстро проверить куда входит персонаж?
      std::array<utils::handle<realm>, faction_type_count> electorate; // self - игнорируем
      
      utils::bit_field<64> data;
      uint64_t static_state;
      state rng_state; // нужно ли передать в луа? нет
      
      // можем ли мы разделить трейты на группы? можем, сколько теперь выделять для каждой группы
      // в цк2 трейты разделены на группы, некоторые трейты замещают другие в одной группе
      // некоторые трейты не могут быть взяты из-за религии
      
      // как тут сделать отношения и дипломатию? отношения сделал неудачно - лучше пусть просто будет список всех типов отношений, который и будем хранить 
      // в массиве, дипломатия должна быть здесь потому что она собственно только здесь и имеет смысл, как ее устроить? слева указатель на персонажа,
      // справа тип, война, персонаж, сколько всего может быть дипломатий? нужно ли смешивать дипломатию и отношения? не думаю что хорошая идея
      
      
      character(const bool male, const bool dead);
      ~character();
      
      // нужна функция которая вернет текущий "главный" (больший ранг титула) реалм лидером которого является персонаж
      // ну и проверить если таковой имеется
      utils::handle<core::realm> get_leader_of_realm() const;
      
//       character* get_father() const; // может вернуть nullptr
//       character* get_mother() const;
      
      void set_dead();
      void make_hero();
      void make_player();
      void make_excommunicated();
      void make_not_excommunicated();
      void make_general();
      void fire_general();
      
      void add_title(titulus* title);
      void remove_title(titulus* title);
      titulus* get_main_title() const;
      
      void add_vassal(character* vassal);
      void remove_vassal(character* vassal);
      
      void add_prisoner(character* prisoner);
      void remove_prisoner(character* prisoner);
      
      void add_concubine(character* concubine);
      void add_concubine_raw(character* concubine);
      void remove_concubine(character* concubine);
      
//       void add_child(character* child); // ребенок должен быть чей то
      void add_child_raw(character* child); // возможно будет только эта функция 
//       void remove_child(character* child); // врядли вообще когда нибудь я этим воспользуюсь (как быть с незаконно рожденными? должны же мы как то обращаться к этому ребенку у не своего отца?)
      
      // наверное не нужно
      bool get_bit(const size_t &index) const; // тут по максимуму от того что останется
      bool set_bit(const size_t &index, const bool value);
      
//       bool has_flag(const std::string_view &flag) const;
      void add_flag(const std::string_view &flag, const size_t &turn);
      void remove_flag(const std::string_view &flag);
      
//       bool has_trait(const trait* t) const;
      void add_trait(const trait* t);
      void remove_trait(const trait* t);
      
//       bool has_modificator(const modificator* m) const;
      void add_modificator(const modificator* m, const utils::modificators_container::modificator_data &data);
      void remove_modificator(const modificator* m);
      
      bool has_event(const event* e) const;
      void add_event(const event* e, utils::events_container::event_data &&data);
      void remove_event(const event* e);
      
      uint64_t get_random();
      
      uint32_t compute_age() const;
      
#define GET_SCOPE_COMMAND_FUNC(name, a, b, type) type get_##name() const;
      CHARACTER_GET_SCOPE_COMMANDS_LIST
#undef GET_SCOPE_COMMAND_FUNC

#define CONDITION_COMMAND_FUNC(name) bool name() const;
      CHARACTER_GET_BOOL_NO_ARGS_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC

// надо бы тут гет использовать
#define CONDITION_COMMAND_FUNC(name) double get_##name() const;
      CHARACTER_GET_NUMBER_NO_ARGS_COMMANDS_LIST
#undef CONDITION_COMMAND_FUNC

#define CONDITION_ARG_COMMAND_FUNC(name, value_type_bit, constness, value_type) bool name(const value_type) const;
      CHARACTER_GET_BOOL_ONE_ARG_COMMANDS_LIST
#undef CONDITION_ARG_COMMAND_FUNC

      core::culture* get_culture() const;
      const core::culture_group* get_culture_group() const;
      core::religion* get_religion() const;
      const core::religion_group* get_religion_group() const;
      
      // откуда это брать? по идее это задано по умолчанию, но при этом нужно как то локализацию сделать
      // стрингвью брать не удастся... или нет? не знаю как работает std::string_view в sol
      // по хорошему тут мы можем возвращать константные строки для локализации и тогда std::string_view встают нормально
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
      std::string_view hero_gender() const;
      std::string_view wizard_gender() const;
      std::string_view duke_gender() const;
      std::string_view count_gender() const;
      std::string_view heir_gender() const;
      std::string_view prince_gender() const;
      std::string_view baron_gender() const;
      
      // эти штуки нужно составлять из разных строк, по идее тут нужно возвращать обычный std string
      // я уже забыл что есть что, эти строки составляются из строк полученных из локализации
      // поэтому тут должны быть обычные строки, но скорее всего эти строчки я буду составлять в lua
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
    
    // может сначало зарегистрировать персонажа
    // а потом присвоить табличку в индекс
    // нужно ли регистрировать сразу нескольких персонажей? это было бы полезно в том числе не только для персонажей
    size_t add_character(const sol::table &table);
    size_t register_character();
    size_t register_characters(const size_t &count);
    void set_character(const size_t &index, const sol::table &table);
    bool validate_character(const size_t &index, const sol::table &table);
    bool validate_character_and_save(const size_t &index, sol::this_state lua, const sol::table &table, utils::world_serializator* container);
    void parse_character(core::character* character, const sol::table &table);
    void parse_character_goverment(core::character* character, const sol::table &table);
    
    void update_character_stats(core::character* character);
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
