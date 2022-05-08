#include "logic.h"

#include "utils/thread_pool.h"
#include "utils/globals.h"
#include "utils/works_utils.h"
#include "utils/systems.h"
#include "utils/game_context.h"
#include "utils/deferred_tasks.h"
#include "utils/game_context.h"

#include "core/context.h" 

#include "ai/ai_system.h"

#include "game_time.h"

#include <stdexcept>

namespace devils_engine {
  namespace game {
    
    namespace turn_states_events {
#define STATE(x) static constexpr dsml::State<struct x##_> x{};
#define TURN_STATE_FUNC(name) STATE(name)
      TURN_STATES_LIST
#undef TURN_STATE_FUNC
#undef STATE

#define EVENT(x) static constexpr dsml::Event<struct x##_> x{};
#define TURN_EVENT_FUNC(name) EVENT(name)
      TURN_EVENTS_LIST
#undef TURN_EVENT_FUNC
#undef EVENT
    }

    // как по итогу выглядит ход ИИ? у нас существует пачка решений, интеракций и ивентов + войска + строительство
    // всем этим нужно последовательно воспользоваться, возможно какую то часть или все вместе для отдельных ии передадим в луа
    // 
    
    static void initial_state_action(turn_status &ts);
    static void preparing_turn_action(turn_status &ts);
    static void player_turn_action(turn_status &ts);
    static void ai_turn_action(turn_status &ts);

    // работа с состояниями должна быть в основном потоке
    struct turn_status::state_decl {
      auto operator()() const noexcept {
        using namespace dsml::literals;
        using namespace dsml::operators;
        using namespace turn_states_events;
        
        const auto display = [] (const std::string_view& msg) {
          return [msg] () { std::cout << msg << "\n"; };
        };
        
        const auto set_state = [] (const enum state s) {
          return [s] (turn_status &status) { status.state = s; };
        };
        
        const auto next_player = [] () -> bool {
          // тут проверим если остались у нас еще игроки
          // но пока что будет только один
          return false;
        };
        
        return dsml::make_transition_table(
          dsml::initial_state + dsml::on_entry / (display("initial_state"), set_state(turn_status::state::initial), initial_state_action),
          dsml::initial_state + advance = preparing_turn,
          
          preparing_turn + dsml::on_entry / (display("preparing_turn"), set_state(turn_status::state::preparing_turn), preparing_turn_action),
          preparing_turn + advance = player_turn,
          
          player_turn + dsml::on_entry / (display("player_turn"), set_state(turn_status::state::player_turn), player_turn_action),
          player_turn + advance [ next_player ] = player_turn,
          player_turn + advance [!next_player ] = ai_turn,
          
          ai_turn + dsml::on_entry / (display("ai_turn"), set_state(turn_status::state::ai_turn), ai_turn_action),
          ai_turn + turn_states_events::end_turn = preparing_turn
        );
      }
    };
    
    turn_status::turn_status() : player_index(0), current_player(nullptr), m_sm{*this} {} // using namespace turn_states_events; m_sm.process_event(advance);
    void turn_status::set_state(const enum state s) { std::unique_lock<std::mutex> lock(mutex); state = s; }
    enum turn_status::state turn_status::get_state() const { std::unique_lock<std::mutex> lock(mutex); return state; }
    
    bool can_end_turn() {
      // когда я могу сделать ход? когда у меня все пендинг битвы резолвлены
      // ивенты? некоторые ивенты по идее блокируют следующий ход
      // что-то еще?
      
      
    }
    
    void end_turn() {
      using namespace turn_states_events;
      
      auto game_ctx = global::get<systems::core_t>()->game_ctx;
      game_ctx->turn_status->m_sm.process_event(advance);
      
      // у нас почти идертичные вещи будут для битвы, но при этом состояние карты нужно сохранить
      // но в битве потребуется несколько новых состояний
      // да и в общем то все, большая часть вычислений будет в функциях ивентов
    }
    
    static void initial_state_action(turn_status &ts) {
      global::get<utils::deferred_tasks>()->add([&ts] (const size_t &) -> bool {
        using namespace turn_states_events;
        if (ts.current_player == nullptr) return false;
        ts.m_sm.process_event(advance);
        return true;
      });
    }
    
    // что конкретно проиходит в состояниях? 
    static void preparing_turn_action(turn_status &ts) {
      // тут наверное нужно сначало добавить в deferred_tasks, там проверить указан ли персонаж для текущего игрока
      // если не указан то мы ждем пока игрок выберет себе персонажа, использую для этого состояние initial
      auto pool = global::get<dt::thread_pool>();
      pool->submitbase([&ts] () {
        // статус использовать тут не будем, деферред таскс вызовет его в основном потоке
        // увеличение хода на 1 
        // + перерасчет статов (+ ресурсы (хп, ход, деньги)) 
        // + нужно обработать смерти (сортировка по старшинству)
        // + обработка строительства везде 
        // + обработка ивентов (посчитать время возникновения)
        // + сезоны 
        // + ???
        
        global::get<utils::deferred_tasks>()->add([&ts] (const size_t &) -> bool {
          using namespace turn_states_events;
          ts.m_sm.process_event(advance);
          return true;
        });
      });
    }
    
    static void player_turn_action(turn_status &ts) {
      // меняем текущего игрока который делает ход и видимо просто ждем
      // надо ли указатель обнулить когда начинается ход компов? желательно, как сделать?
      // обнулить его в ai_turn_action
      // откуда брать игроков? где то у меня должен храниться список всех игроков, в game_ctx?
      ts.player_index = ts.player_index % 1;
      
      ts.current_player = global::get<systems::core_t>()->game_ctx->player_character;
      
      ++ts.player_index;
    }
    
    // как сделать последовательность? например интеракции поди нужно выполнять последовательно
    static void ai_turn_action(turn_status &ts) {
      ts.current_player = nullptr;
      auto pool = global::get<dt::thread_pool>();
      pool->submitbase([&ts] () {
        // решения, интеракции, ивенты + войска + строительство
        // сами интеракции и решения делаются безусловно последовательно, 
        // но нужно посчитать потентиал и кондитион + ai_will_do, после этого
        // добавим эти штуки в контейнер, беда в том что у нас может поменяться 
        // состояние ИИ в это время, но с другой стороны это поди не так часто происходить будет
        // было бы неплохо отслеживать изменение состояния персонажа
        
        global::get<utils::deferred_tasks>()->add([&ts] (const size_t &) -> bool {
          ts.m_sm.process_event(turn_states_events::end_turn);
          return true;
        });
      });
    }

//     enum class gameplay_state {
//       turn_advancing,
//       ai_turn,
//       player_turn,
//       next_player,
//     };
//     
//     //static core::character* player = nullptr;
//     void update_player(core::character* c) {
//       auto game_ctx = global::get<systems::core_t>()->game_ctx;
//       game_ctx->player_character = c;
//     }
//     
//     core::character* get_player() {
//       auto game_ctx = global::get<systems::core_t>()->game_ctx;
//       return game_ctx->player_character;
//     }
//     
//     static std::atomic<gameplay_state> new_state = gameplay_state::turn_advancing;
//     static gameplay_state current_state = gameplay_state::player_turn;
//     void player_has_ended_turn() { // перед этим мы должны проверить сделал ли игрок все вещи (ответил на все эвенты, всеми походил)
//       if (current_state != gameplay_state::player_turn) throw std::runtime_error("Current game state isnt player turn");
//       new_state = gameplay_state::next_player;
//     }
//     
//     bool current_player_turn() {
//       return current_state == gameplay_state::player_turn;
//     }
//     
//     bool player_end_turn() { //core::character* c
//       // тут мы должны проверить все ли сделал игрок (резолв битвы (вобзможно какого события) и эвенты)
//       // наверное нужно запилить функцию дополнительной проверки
//       // передавать игрока из интерфейса не нужно
//       // как тогда получить игрока? должен быть список игроков и текущий игрок
//       auto game_ctx = global::get<systems::core_t>()->game_ctx;
//       ASSERT(game_ctx->player_character != nullptr);
//       
//       player_has_ended_turn();
//       return true;
//     }
    
//     void advance_state() {
//       if (current_state == new_state) return;
//       current_state = new_state;
//       
//       // к сожалению видимо никак кроме последовательности игрок -> комп -> следующий ход
//       // сделать не получится, нам нужно правильно и быстро обработать подсистемы
//       // а это означает что дробить вычисления компа - это плохая идея
//       
//       switch (new_state) {
//         case gameplay_state::turn_advancing: {
//           auto pool = global::get<dt::thread_pool>();
//           pool->submitbase([pool] () {
//             PRINT("Thread compute turn advancing")
//             auto ctx = global::get<systems::map_t>()->core_context;
//             ctx->sort_characters();
// //             const size_t first_playable = ctx->first_playable_character();
//             const size_t count_not_dead = ctx->living_characters_count();
// //             const size_t count_playable = ctx->characters_count() - first_playable;
//             
//             // статы у городов, провинций, фракций, армий и героев (возможно сначало нужно посчитать эвенты)
//             
//             utils::submit_works_async(pool, count_not_dead, [ctx] (const size_t &start, const size_t &count) {
//               for (size_t i = start; i < start+count; ++i) {
//                 const size_t index = i;
//                 auto c = ctx->get_living_character(index);
//                 ASSERT(!c->is_dead());
//                 // нужно наверное добавить флажок необходимости обновляться
//                 
//                 std::this_thread::sleep_for(std::chrono::microseconds(10)); // пока что моделируем какую то бурную деятельность
//               }
//             });
//             
//             pool->compute();
//             
//             // должно сработать (текущий тред занят этой конкретной функцией)
//             //while (pool->working_count() != 1) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }
//             utils::async_wait(pool);
//             
//             // вычислить эвенты (?)
//             // вычислить рождения/смерти (тут же нужно посчитать наследство)
//             // распространение техов/религии
//             
//             // делаем работу дальше 
//             global::get<utils::calendar>()->advance_turn();
//             
//             new_state = gameplay_state::player_turn; // ?
//           });
//           
//           break;
//         }
//         
//         case gameplay_state::ai_turn: {
//           auto pool = global::get<dt::thread_pool>();
//           pool->submitbase([pool] () {
//             PRINT("Thread compute ai turns")
//             // тут мы должны вычислить ии и сложный луа ии
//             // интеллект должен понять ситуацию вокруг, посмотреть дипломатию,
//             // можно ли что-то построить и куда то сходить героями и армиями
//             // оценить ситуацию можно и параллельно
//             // а вот более конкретные действия требуют какой то последовательности
//             // на такого рода последовательности у меня аллергия
//             // можно как то все сделать параллельно? тут надо понять что все
//             // поиск пути нужно сделать, мы вполне можем найти пути для всех армий заранее
//             // (другое дело что нам возможно потребуется очень много памяти) 
//             // нашел статейку которая кратко описывает как работает ии
//             // основная вещь в том что ии большое количество времени находится в паузе
//             // то есть не выполняет никаких действий или выполняет какую то одну легкую подсистему
//             // существует множество подсистем с которыми взаимодействуют персонажи
//             // некоторые из них выполняются чаще чем другие (например в европке дипломатия обновляется каждый месяц)
//             // я так понимаю происходит распределение на основе весов + неких общих ограничителей?
//             // мы вполне можем распределить веса параллельно, к весам нужно еще распределить нагрузку
//             // подсистемы должны обладать неким коэффициентом вычислительной сложности, которая повлияет на развесовку 
//             // (точнее на распределение нагрузки: более тяжелые персонажи с большей вероятностью получат вычислительное время)
//             // (но при этом всего вычисляемых персонажей будет ограниченное количество)
//             // какие то подсистемы можем посчитать параллельно, какие то нужно считать последовательно
//             // (например передвижение войск должно быть более менее последовательно)
//             // (но при этом передвижение войск можно считать менее часто, и не во всех странах идет сейчас война)
//             // (параллельно можно определить какие то долгосрочные цели, найти супруга, сделать вклад в дипломатию (тут посчитаем отношения соседей))
//             // каждому персонажу должно быть выдано вычислительное время, не каждый ход, скорее всего зависит и от максимального титула
//             // то есть хотелки императора должны вычисляться чаще, у зависимых персонажей видимо меньше всего должно быть вычислительного времени (?)
//             // (относительно меньше всего, так или иначе вычислять мы должны всех хотя бы по разу в год (?))
//             // одно хорошо - у нас не реалтайм, где то рядом нужно посчитать луа ии
//             // луа ии отличается тем что у нас будет гораздо больше опций по оптимизации и рационализации поведения персонажей
//             // но при этом мы должны предоставить возможность какие то части моделировать с помощью с++ кода
//             // (например оставить возможность с++ вычислять женитьбу, так как она чаще всего сопряжена с поиском среди всех персонажей)
//             // как то так
//             
//             // вот хорошая статья про это https://www.rockpapershotgun.com/2016/11/11/crusader-kings-2-characters/
//             
//             // сейчас нужно сделать пока что одну подсистему которая должна отвечать за строительство
//             
//             auto ctx = global::get<systems::map_t>()->core_context;
//             const size_t count_not_dead = ctx->living_characters_count();
//             const size_t count_playable = ctx->living_playable_characters_count();
//             
//             std::vector<uint32_t> systems_data(count_not_dead); // msvc не хочет работать если используется статик массив
//             uint32_t* systems_data_ptr = systems_data.data();
//             
//             const auto &systems = global::get<systems::map_t>()->ai_systems->get_subsystems();
//             for (auto s : systems) {
//               // по идее всегда должно быть true
//               // скорее всего иное какое то серьезное исключение
//               //if (!s->get_attrib(ai::sub_system::attrib_threadsave_check)) continue;
//               ASSERT(s->get_attrib(ai::sub_system::attrib_threadsave_check));
//               
//               memset(systems_data_ptr, 0, sizeof(uint32_t) * count_not_dead);
//               //const size_t count = s->get_attrib(ai::sub_system::attrib_playable_characters) ? count_playable : count_not_dead;
//               
//               if (s->get_attrib(ai::sub_system::attrib_playable_characters)) {
//                 const size_t count = count_playable;
//                 utils::submit_works_async(pool, count, [ctx, s, systems_data_ptr] (const size_t &start, const size_t &count) {
//                   for (size_t i = start; i < start+count; ++i) {
//                     const size_t index = i;
//                     auto c = ctx->get_living_playable_character(index);
//                     if (c->is_player()) continue;
//                     ASSERT(!c->is_dead());
//                     
//                     systems_data_ptr[i] = s->check(c); // похоже что чек не проходит
//                     if (systems_data_ptr[i] == SUB_SYSTEM_SKIP) continue;
//                     // а зачем мне тогда массив? мне он нужен когда ai::sub_system::attrib_threadsave_process == false 
//                     if (s->get_attrib(ai::sub_system::attrib_threadsave_process)) {
//                       s->process(c, systems_data_ptr[i]);
//                     }
//                   }
//                 });
//                 
//                 pool->compute();
//               } else {
//                 const size_t count = count_not_dead;
//                 utils::submit_works_async(pool, count, [ctx, s, systems_data_ptr] (const size_t &start, const size_t &count) {
//                   for (size_t i = start; i < start+count; ++i) {
//                     const size_t index = i;
//                     auto c = ctx->get_living_character(index);
//                     if (c->is_player()) continue;
//                     ASSERT(!c->is_dead());
//                     
//                     systems_data_ptr[i] = s->check(c); // похоже что чек не проходит
//                     if (systems_data_ptr[i] == SUB_SYSTEM_SKIP) continue;
//                     // а зачем мне тогда массив? мне он нужен когда ai::sub_system::attrib_threadsave_process == false 
//                     if (s->get_attrib(ai::sub_system::attrib_threadsave_process)) {
//                       s->process(c, systems_data_ptr[i]);
//                     }
//                   }
//                 });
//                 
//                 pool->compute();
//               }
//               
//               // должно сработать (текущий тред занят этой конкретной функцией)
//               // кажется работает, осталось немного поработать над синхронизацией всего этого дела
//               // мне нужно сделать так чтобы когда я буду заполнять данные в рендер у меня все не ломалось
// //               while (pool->working_count() != 1 || pool->tasks_count() != 0) { // pool->tasks_count() != 0 по идее это избыточно
// //                 //PRINT("Thread waits")
// //                 std::this_thread::sleep_for(std::chrono::microseconds(1)); 
// //               }
//               
//               utils::async_wait(pool);
//               
//               // такая ситуация должна быть очень редкой
//               // мы можем использовать это для запуска луа функции
//               if (!s->get_attrib(ai::sub_system::attrib_threadsave_process)) {
//                 if (s->get_attrib(ai::sub_system::attrib_playable_characters)) {
//                   const size_t count = count_playable;
//                   const size_t start = 0;
//                   for (size_t i = start; i < start+count; ++i) {
//                     const size_t index = i;
//                     if (systems_data_ptr[i] == SUB_SYSTEM_SKIP) continue;
//                             
//                     auto c = ctx->get_living_playable_character(index);
//                     if (c->is_player()) continue;
//                     ASSERT(!c->is_dead());
//                     
//                     s->process(c, systems_data_ptr[i]);
//                   }
//                 } else {
//                   const size_t count = count_not_dead;
//                   const size_t start = 0;
//                   for (size_t i = start; i < start+count; ++i) {
//                     const size_t index = i;
//                     if (systems_data_ptr[i] == SUB_SYSTEM_SKIP) continue;
//                             
//                     auto c = ctx->get_living_character(index);
//                     if (c->is_player()) continue;
//                     ASSERT(!c->is_dead());
//                     
//                     s->process(c, systems_data_ptr[i]);
//                   }
//                 }
//               }
//             }
//             
//             new_state = gameplay_state::turn_advancing;
//           });
//           break;
//         }
//         
//         case gameplay_state::player_turn: {
//           // мы как то должны переключить стейт отсюда
//           break;
//         }
//         
//         case gameplay_state::next_player: {
//           // выбираем следующего игрока (сейчас понятное дело никакого другого игрока нет)
//           new_state = gameplay_state::ai_turn;
//           break;
//         }
//       }
//     }
    
    // в этих функциях еще нужно будет какую то инфу по сети передавать
    
    static std::string get_error_str_run_decision(const core::compiled_decision* d) {
      auto game_ctx = global::get<systems::core_t>()->game_ctx;
      if (game_ctx->turn_status->state != turn_status::state::player_turn) 
        return "Could not run decision " + d->d->id + ": current turn state is not player turn state";
      //return "Could not run decision " + d->d->id + ": currently another player makes turn";
      if (d->character != game_ctx->player_character) return "Could not run another character decision " + d->d->id;
      if (d->used) return "Could not run used decision " + d->d->id + ", get available decision list again";
      
      return std::string();
    }
    
    static std::string get_error_str_run_interaction(const core::compiled_interaction* i) {
      auto game_ctx = global::get<systems::core_t>()->game_ctx;
      if (game_ctx->turn_status->state != turn_status::state::player_turn) return "Could not run interaction " + i->native->id + ": current turn state is not player turn state";
      //return "Could not run interaction " + i->native->id + ": currently another player makes turn";
      if (i->character != game_ctx->player_character) return "Could not run another character interaction " + i->native->id;
      
      return std::string();
    }
    
    // может строку с ошибкой возвращать?
    bool can_run_decision(const core::compiled_decision* d, std::string &err_str) {
      // как проверить кто сейчас ходит? состояние хода нужно где то запомнить
      // + у нас может быть несколько игроков, значит может быть ситуация 
      // когда ходит НЕ тот игрок для которого мы пытаемся вызвать решение
      // значит первое условие: должен быть ход игрока, второе условие игрок должен совпадать с тем что находится в решении
      auto str = get_error_str_run_decision(d);
      if (str.empty()) return true;
      err_str = std::move(str);
      return false;
    }
    
    bool run_decision(core::compiled_decision* d) {
      const auto &str = get_error_str_run_decision(d);
      if (!str.empty()) throw std::runtime_error(str);
      return d->run();
    }
    
    bool can_send_interaction(const core::compiled_interaction* i, std::string &err_str) {
      auto str = get_error_str_run_interaction(i);
      if (str.empty()) return true;
      err_str = std::move(str);
      return false;
    }
    
    bool send_interaction(core::compiled_interaction* i) {
      const auto &str = get_error_str_run_interaction(i);
      if (!str.empty()) throw std::runtime_error(str);
      return i->send();
    }
    
    static std::string get_error_str_construct_building(const core::city* c, const uint32_t &building_index) {
      auto game_ctx = global::get<systems::core_t>()->game_ctx;
      if (building_index >= c->type->buildings_count) return "Invalid building index";
      if (game_ctx->turn_status->state != turn_status::state::player_turn) 
        return "Could not construct building " + c->type->buildings[building_index]->id + " in city " + c->title->id + ": current turn state is not player turn state";
      //return "Could not run interaction " + i->native->id + ": currently another player makes turn";
      if (c->get_direct_owner() != game_ctx->player_character) return "Could not construct building " + c->type->buildings[building_index]->id + " in city " + c->title->id + " owned by another character";
      // проверка?
      
      return std::string();
    }
    
    bool can_construct_building(const core::city* c, const uint32_t &building_index, std::string &err_str) {
      auto str = get_error_str_construct_building(c, building_index);
      if (str.empty()) return true;
      err_str = std::move(str);
      return false;
    }
    
    bool construct_building(core::city* c, const uint32_t &building_index) {
      const auto &str = get_error_str_construct_building(c, building_index);
      if (!str.empty()) throw std::runtime_error(str);
      // текущий ход игрока
      //return c->start_build(building_index);
    }
    
    bool can_move_unit(const core::army* a, const uint32_t &end_tile_index, std::string &err_str) {
      // когда мы не можем заставить юнит передвигаться? 
      // когда текущее состояние хода != ход игрока
      // когда ходит другой игрок чем владелец армии
      // когда армия находится в неверном состаянии
      // ???
    }
    
    bool move_unit(core::army* a, const uint32_t &end_tile_index) {
      //a->find_path(end_tile_index);
      // тут скорее мы вызываем адванс чем ищем путь, путь мы можем найти где угодно
      a->advance();
    }
    
    bool can_move_unit(const core::hero_troop* h, const uint32_t &end_tile_index, std::string &err_str) {
      
    }
    
    bool move_unit(core::hero_troop* h, const uint32_t &end_tile_index) {
      
    }
  
    bool can_attack(const core::army* a, const core::city* ec) {
      // может атаковать когда?
      // когда текущее состояние хода == ход игрока
      // когда ходит тот же игрок что и владелец армии
      // когда армия находится в верном состоянии
      // когда юнит противника находится в верном состоянии
      // когда юнит противника находится рядом на один тайл (но это не строгая проверка, если что то не так то мы должны проигнорировать)
      
      // дополнительные условия про то может ли эта конкретная армия вообще нападать?
    }
    
    bool can_attack(const core::army* a, const core::army* ea) {
      
    }
    
    bool can_attack(const core::army* a, const core::hero_troop* eh) {
      
    }
    
    bool can_attack(const core::hero_troop* h, const core::city* ec) {
      
    }
    
    bool can_attack(const core::hero_troop* h, const core::army* ea) {
      
    }
    
    bool can_attack(const core::hero_troop* h, const core::hero_troop* eh) {
      
    }
    
    // что конкретно происходит при нападении? у юнитов меняется стейт и мы ждем подтверждения игрока на какое то действие
    // какие действия? по крайней мере 3 исхода именно движковых: стартануть битву (загрузить поле боя и проч), авторасчет, отступить
    // + я бы хотел добавить "удерживание", вызов командира на бой, ??? + было бы неплохо какие то пользовательские взаимодействия сделать
    // (например каст заклинания перед битвой, на ходе противника ему будет предложенно вступить в битву с заклинанием, или отступить)
    // таким образом атака просто меняет состояния юнитов + нужно получить ответ от самого игрока
    bool attack(core::army* a, core::city* ec) {
      
    }
    
    bool attack(core::army* a, core::army* ea) {
      
    }
    
    bool attack(core::army* a, core::hero_troop* eh) {
      
    }
    
    bool attack(core::hero_troop* h, core::city* ec) {
      
    }
    
    bool attack(core::hero_troop* h, core::army* ea) {
      
    }
    
    bool attack(core::hero_troop* h, core::hero_troop* eh) {
      
    }
  }
}
