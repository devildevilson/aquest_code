#include "logic.h"
#include "utils/thread_pool.h"
#include "utils/globals.h"
#include "core_context.h"
#include "utils/works_utils.h"
#include "game_time.h"
#include "ai/ai_system.h"
#include "utils/systems.h"

#include <stdexcept>

namespace devils_engine {
  namespace game {
    enum class gameplay_state {
      turn_advancing,
      ai_turn,
      player_turn,
      next_player,
    };
    
    static core::character* player = nullptr;
    void update_player(core::character* c) {
      player = c;
    }
    
    static std::atomic<gameplay_state> new_state = gameplay_state::turn_advancing;
    static gameplay_state current_state = gameplay_state::player_turn;
    void player_has_ended_turn() { // перед этим мы должны проверить сделал ли игрок все вещи (ответил на все эвенты, всеми походил)
      if (current_state != gameplay_state::player_turn) throw std::runtime_error("Current game state isnt player turn");
      new_state = gameplay_state::next_player;
    }
    
    bool current_player_turn() {
      return current_state == gameplay_state::player_turn;
    }
    
    bool player_end_turn() { //core::character* c
      // тут мы должны проверить все ли сделал игрок (резолв битвы (вобзможно какого события) и эвенты)
      // наверное нужно запилить функцию дополнительной проверки
      // передавать игрока из интерфейса не нужно
      // как тогда получить игрока? должен быть список игроков и текущий игрок
      ASSERT(player != nullptr);
      
      player_has_ended_turn();
      return true;
    }
    
    void advance_state() {
      if (current_state == new_state) return;
      current_state = new_state;
      
      // к сожалению видимо никак кроме последовательности игрок -> комп -> следующий ход
      // сделать не получится, нам нужно правильно и быстро обработать подсистемы
      // а это означает что дробить вычисления компа - это плохая идея
      
      switch (new_state) {
        case gameplay_state::turn_advancing: {
          auto pool = global::get<dt::thread_pool>();
          pool->submitbase([pool] () {
            PRINT("Thread compute turn advancing")
            auto ctx = global::get<systems::map_t>()->core_context;
            ctx->sort_characters();
            const size_t first_not_dead = ctx->first_not_dead_character();
            const size_t first_playable = ctx->first_playable_character();
            const size_t count_not_dead = ctx->characters_count() - first_not_dead;
            const size_t count_playable = ctx->characters_count() - first_playable;
            
            // статы у городов, провинций, фракций, армий и героев (возможно сначало нужно посчитать эвенты)
            
            utils::submit_works_async(pool, count_not_dead, [ctx, first_not_dead] (const size_t &start, const size_t &count) {
              for (size_t i = start; i < start+count; ++i) {
                const size_t index = i + first_not_dead;
                auto c = ctx->get_character(index);
                ASSERT(!c->is_dead());
                // нужно наверное добавить флажок необходимости обновляться
                
                std::this_thread::sleep_for(std::chrono::microseconds(10)); // пока что моделируем какую то бурную деятельность
              }
            });
            
            pool->compute();
            
            // должно сработать (текущий тред занят этой конкретной функцией)
            while (pool->working_count() != 1) { std::this_thread::sleep_for(std::chrono::microseconds(1)); }
            
            // вычислить эвенты (?)
            // вычислить рождения/смерти (тут же нужно посчитать наследство)
            // распространение техов/религии
            
            // делаем работу дальше 
            global::get<utils::calendar>()->advance_turn();
            
            new_state = gameplay_state::player_turn; // ?
          });
          
          break;
        }
        
        case gameplay_state::ai_turn: {
          auto pool = global::get<dt::thread_pool>();
          pool->submitbase([pool] () {
            PRINT("Thread compute ai turns")
            // тут мы должны вычислить ии и сложный луа ии
            // интеллект должен понять ситуацию вокруг, посмотреть дипломатию,
            // можно ли что-то построить и куда то сходить героями и армиями
            // оценить ситуацию можно и параллельно
            // а вот более конкретные действия требуют какой то последовательности
            // на такого рода последовательности у меня аллергия
            // можно как то все сделать параллельно? тут надо понять что все
            // поиск пути нужно сделать, мы вполне можем найти пути для всех армий заранее
            // (другое дело что нам возможно потребуется очень много памяти) 
            // нашел статейку которая кратко описывает как работает ии
            // основная вещь в том что ии большое количество времени находится в паузе
            // то есть не выполняет никаких действий или выполняет какую то одну легкую подсистему
            // существует множество подсистем с которыми взаимодействуют персонажи
            // некоторые из них выполняются чаще чем другие (например в европке дипломатия обновляется каждый месяц)
            // я так понимаю происходит распределение на основе весов + неких общих ограничителей?
            // мы вполне можем распределить веса параллельно, к весам нужно еще распределить нагрузку
            // подсистемы должны обладать неким коэффициентом вычислительной сложности, которая повлияет на развесовку 
            // (точнее на распределение нагрузки: более тяжелые персонажи с большей вероятностью получат вычислительное время)
            // (но при этом всего вычисляемых персонажей будет ограниченное количество)
            // какие то подсистемы можем посчитать параллельно, какие то нужно считать последовательно
            // (например передвижение войск должно быть более менее последовательно)
            // (но при этом передвижение войск можно считать менее часто, и не во всех странах идет сейчас война)
            // (параллельно можно определить какие то долгосрочные цели, найти супруга, сделать вклад в дипломатию (тут посчитаем отношения соседей))
            // каждому персонажу должно быть выдано вычислительное время, не каждый ход, скорее всего зависит и от максимального титула
            // то есть хотелки императора должны вычисляться чаще, у зависимых персонажей видимо меньше всего должно быть вычислительного времени (?)
            // (относительно меньше всего, так или иначе вычислять мы должны всех хотя бы по разу в год (?))
            // одно хорошо - у нас не реалтайм, где то рядом нужно посчитать луа ии
            // луа ии отличается тем что у нас будет гораздо больше опций по оптимизации и рационализации поведения персонажей
            // но при этом мы должны предоставить возможность какие то части моделировать с помощью с++ кода
            // (например оставить возможность с++ вычислять женитьбу, так как она чаще всего сопряжена с поиском среди всех персонажей)
            // как то так
            
            // вот хорошая статья про это https://www.rockpapershotgun.com/2016/11/11/crusader-kings-2-characters/
            
            // сейчас нужно сделать пока что одну подсистему которая должна отвечать за строительство
            
            auto ctx = global::get<systems::map_t>()->core_context;
            const size_t first_not_dead = ctx->first_not_dead_character();
            const size_t first_playable = ctx->first_playable_character();
            const size_t count_not_dead = ctx->characters_count() - first_not_dead;
            const size_t count_playable = ctx->characters_count() - first_playable;
            
            uint32_t systems_data[count_not_dead];
            uint32_t* systems_data_ptr = systems_data;
            
            const auto &systems = global::get<systems::map_t>()->ai_systems->get_subsystems();
            for (auto s : systems) {
              // по идее всегда должно быть true
              // скорее всего иное какое то серьезное исключение
              //if (!s->get_attrib(ai::sub_system::attrib_threadsave_check)) continue;
              ASSERT(s->get_attrib(ai::sub_system::attrib_threadsave_check));
              
              memset(systems_data_ptr, 0, sizeof(uint32_t) * count_not_dead);
              const size_t count = s->get_attrib(ai::sub_system::attrib_playable_characters) ? count_playable : count_not_dead;
              const size_t first = s->get_attrib(ai::sub_system::attrib_playable_characters) ? first_playable : first_not_dead;
              
              utils::submit_works_async(pool, count, [ctx, first, s, systems_data_ptr] (const size_t &start, const size_t &count) {
                for (size_t i = start; i < start+count; ++i) {
                  const size_t index = i + first;
                  auto c = ctx->get_character(index);
                  if (c->is_player()) continue;
                  ASSERT(!c->is_dead());
                  
                  systems_data_ptr[i] = s->check(c); // похоже что чек не проходит
                  if (systems_data_ptr[i] == SUB_SYSTEM_SKIP) continue;
                  // а зачем мне тогда массив? мне он нужен когда ai::sub_system::attrib_threadsave_process == false 
                  if (s->get_attrib(ai::sub_system::attrib_threadsave_process)) {
                    s->process(c, systems_data_ptr[i]);
                  }
                }
              });
              
              pool->compute();
              
              // должно сработать (текущий тред занят этой конкретной функцией)
              // кажется работает, осталось немного поработать над синхронизацией всего этого дела
              // мне нужно сделать так чтобы когда я буду заполнять данные в рендер у меня все не ломалось
              while (pool->working_count() != 1 || pool->tasks_count() != 0) { // pool->tasks_count() != 0 по идее это избыточно
                //PRINT("Thread waits")
                std::this_thread::sleep_for(std::chrono::microseconds(1)); 
              }
              
              // такая ситуация должна быть очень редкой
              // мы можем использовать это для запуска луа функции
              if (!s->get_attrib(ai::sub_system::attrib_threadsave_process)) {
                const size_t start = 0;
                for (size_t i = start; i < start+count; ++i) {
                  const size_t index = i + first;
                  if (systems_data_ptr[i] == SUB_SYSTEM_SKIP) continue;
                          
                  auto c = ctx->get_character(index);
                  if (c->is_player()) continue;
                  ASSERT(!c->is_dead());
                  
                  s->process(c, systems_data_ptr[i]);
                }
              }
            }
            
            new_state = gameplay_state::turn_advancing;
          });
          break;
        }
        
        case gameplay_state::player_turn: {
          // мы как то должны переключить стейт отсюда
          break;
        }
        
        case gameplay_state::next_player: {
          // выбираем следующего игрока (сейчас понятное дело никакого другого игрока нет)
          new_state = gameplay_state::ai_turn;
          break;
        }
      }
    }
  }
}
