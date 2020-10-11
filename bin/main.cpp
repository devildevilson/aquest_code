#include "helper.h"
#include "utils/utility.h"
#include <filesystem>

using namespace devils_engine;

int main(int argc, char const *argv[]) {
  set_application_path();

//   PRINT("first sleep")
//   std::this_thread::sleep_for(std::chrono::seconds(5));

  glfw_t glfw;

  // насколько адекватно создавать луа стейт для каждого потока?
  const uint32_t thread_pool_size = std::max(std::thread::hardware_concurrency()-1, uint32_t(1));
  //const uint32_t threads_count = thread_pool_size+1;
  dt::thread_pool pool(thread_pool_size);
  global::get(&pool);

  systems::core_t base_systems;
  systems::map_t map_systems;
  systems::battle_t battle_systems;
  systems::encouter_t encounter_systems;

  base_systems.create_utility_systems();
  utils::settings settings;
  settings.load_settings();
  global::get(&settings);
  create_render_system(base_systems);
  setup_callbacks();
  base_systems.create_render_stages();
  base_systems.create_interface();
  // базовые функции интерфейса?
  // ну мы должны получить какую то таблицу с указанием откуда че загружать
  basic_interface_functions(base_systems);
  
  settings.graphics.find_video_mode();

  global::get(&base_systems);
  global::get(&map_systems);
  global::get(&battle_systems);
  global::get(&encounter_systems);

  utils::main_menu_state main_menu_state;
  utils::map_creation_state map_creation_state;
  utils::world_map_state world_map_state;
  utils::battle_state battle_state;
  utils::encounter_state encounter_state;

  render::updater upd;
  global::get(&upd);
  yacs::world world;

  // по всей видимости луа занимает реально много памяти
  // что то с этим сделать вряд ли можно (интересно есть ли какие нибудь жесткие ограничения на андроиде?)
  // вроде как жестких ограничений нет, смогу ли я сделать приложение меньше чем 500 мб?
  // это хороший ориентир для приложений без луа, но с ним получается как то слишком многопамяти =(

  const float dist = 550.0f;
  const glm::vec3 default_camera_pos = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * dist;

//   auto ent = world.create_entity();
//   ent->add<components::transform>(default_camera_pos, glm::vec3(-1.0f, 0.0f, 0.0f));
//   ent->add<components::camera>(ent); // тут нужно использовать алгоритм тини (?) : camera_pos += (end_pos - camera_pos) * CONST
  components::camera camera(default_camera_pos);
//   global::get(&camera);
  // так камера будет плавно подъезжать к end_pos в зависимости от константы

  // какие энтити у нас будут? провинции?
  // провинции, города и другие статичные вещи наверное лучше сделать отдельными структурами
  // для чего нам может потребоваться энтити в этих случаях? особые свойства? это какие?
  // не уверен зачем тут может потребоваться энтити
  // проблема в том что тут довольно серьезную оптимизацию нужно предпринять по уменьшению занимаемой памяти
  // да и ко всему прочему очень мало данных у каждой энтити
  
  //keys_setup();

  // по идее хорошим дизайном будет запускать интерфейс без условно всегда
  // просто следить чтобы там во время появлялись нужные функции интерфейса
  // нужно также подчистить интерфейс
  // проблема в том что мне часто нужно определенное возвращемое значение
  // для каждого "экрана" (главное меню, загрузка, генерация, игра, битва, геройская битва)
  // мне скорее всего потребуется некий уникальный набор правил для интерфейсов
  // + ко всему мне нужно чистить интерфейсы, то есть нужно таблицу интерфейсов держать отдельно
  // и скорее тот интерфейс который у меня сейчас - это надстройка над контейнером

  // судя по всему довольно уверенно наступает момент когда мне требуется загружать изображения
  // как их грузить? достаточно очевидно что мне нужны изображения для всех аспектов игры
  // грузим мы их наверное тоже через луа (да вообще нахрена что то еще если есть луа?)
  // нам требуется собрать описания, заранее посчитать сколько всего нужно создать
  // посчитать размер + ко всему нужно уметь грузить все в несколько итераций
  // перед сменой состояния нам нужно будет все не нужное удалить
//   map_systems.setup_map_generator();
//   setup_map_generator(map_systems);

  const std::vector<std::string> base_interfaces = { // как передать данные?
    "main_menu",
    "",
    // должен быть указатель на персонажа, вообще наверное нет,
    // должен быть указатель на какого то помошника (там должен быть более простой доступ ко всем функциям)
    // например контейнер для поиска жен, и прочее, как его передать?
    "player_interface",
    "battle_interface",
    "encounter_interface"
  };

  const std::array<utils::quest_state*, utils::quest_state::count> game_states = {
    &main_menu_state,
    &map_creation_state,
    &world_map_state,
    &battle_state,
    &encounter_state
  };

  uint32_t current_game_state_index = utils::quest_state::main_menu;
  utils::quest_state* current_game_state = game_states[current_game_state_index];
  bool loading = true;
  current_game_state->enter();

  global::get<render::window>()->show();
  utils::frame_time frame_time;
  while (!global::get<render::window>()->close() && !base_systems.menu->m_quit_game) {
    frame_time.start();
    const size_t time = frame_time.get();
    input::update_time(time);
    poll_events();
    if (global::get<render::window>()->close()) break;
    mouse_input(&camera, time);
    key_input(time, current_game_state_index, loading);
    zoom_input(&camera);
    next_nk_frame(time);
    camera::strategic(&camera);
    
    camera.update(time);

    // здесь наверное будет обновлять интерфейс (где главное меню?)
//     if (base_systems.menu->exist()) input::block();
    base_systems.interface_container->draw(time);
//     if (base_systems.menu->exist()) input::unblock();

    if (loading) {
      if (current_game_state->load()) {
        loading = false;
        base_systems.interface_container->close_all();
      }
    } else {
      current_game_state->update(time);
      if (current_game_state->next_state() != UINT32_MAX) {
        const uint32_t index = current_game_state->next_state();
        current_game_state->clean(); // ождается что здесь мы почистим все ресурсы доконца и обратно вернем next_state к UINT32_MAX
        current_game_state = game_states[index];
        current_game_state->enter();
        loading = true;
        current_game_state_index = index;
      }
    }

    // ошибки с отрисовкой непосредственно тайлов могут быть связаны с недостаточным буфером для индексов (исправил)
    // еще меня беспокоит то что игра занимает уже 270 мб оперативы
    // (примерно 100 мб занимает вся информация о игровой карте (размер контейнера и размер данных в кор::мап))
    // (еще мегобайт 100 занимает луа (похоже что с этим бороться будет крайне сложно))
    // нужно каким то образом это сократить
    map_systems.lock_map();
    base_systems.render_slots->update(global::get<render::container>());
    const size_t sync_time = global::get<render::window>()->flags.vsync() ? global::get<render::window>()->refresh_rate_mcs() : 0;
    sync(frame_time, sync_time);
  }

  return 0;
}
