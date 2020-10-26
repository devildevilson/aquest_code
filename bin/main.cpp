#include "helper.h"
#include "utils/utility.h"
#include <filesystem>

using namespace devils_engine;

// было бы неплохо еще хранить дополнительную инфу (описание?)
// ну или можно сделать то же самое через несколько функций
// возможно это предпочтительнее
struct tile_info {
  render::biome_data_t* biome;
  float height;
  core::province* province;
  core::city* city;
};

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
  const auto tile_func = basic_interface_functions(base_systems);
  
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

  components::camera camera(default_camera_pos); // будем использовать эту камеру только в ворлд мап
  // для других состояний нам нужны другие камеры

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
  
  // потихоньку переходим к геймплею
  // нужно сделать города и какие то постройки: снабдить какой то иконкой (гербом)
  // нужно сделать юнитов на глобальной карте: непосредственно юнита и знамя, чтобы было легче выбирать
  // нужно сделать движение юнитов: выделение юнита и чекать куда я могу передвинуть модельку
  // интерфейс для всего этого
  // с чего начать? постройки... для этого нужно сделать рейтрейсинг нормальный, а для этого нужно сделать 
  // 3д иерархию, в чем прикол? нужно проверить многоугольник, 8 треугольников
  // более менее работает
  // теперь когда у нас есть выделенный тайл можно начать делать постройки и города
  // что нам нужно для этого, было бы неплохо сделать вид сверху и вид спереди
  // где вид сверху мы видим при определенном удалении, а вид спереди пришпилен к оси как спрайты в думе
  // так, нужно сделать буффер с данными, заполнить его, и еще иметь возможность переделать его в случае чего
  // как изменить? пока что думаю так оставить

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
        current_game_state->enter(); // здесь мы должны подготовить системы к использованию
        loading = true;
        current_game_state_index = index;
      }
    }
    
    // сделал, теперь наконец алгортим учитывает все нововведения
    // все остальное зависит от того где мы применяем функцию
    // теперь нужно вывести какую нибудь инфу
    // инфу о тайле через интерфейс как ее получить? 
    // на самом деле у тайла инфа только о высоте и биоме остальная инфа у провинции
    // мне нужно не делать рейкастинг если я задеваю какое нибудь окно
    // но при этом мне нужно рейкастить сквозь окно инфы о тайле (можно ли как то так сделать?)
    // не понятно как сделать вызов функции, то ли вызывать стандартным способом то ли вызывать отдельную функцию каждый кадр
    // функция каждый кадр - удобнее, но для этого придется придумать особый загрузчик для такой функции
    // в общем так или иначе мне нужно делать особую функцию
    const uint32_t tile_index = cast_mouse_ray();
    if (tile_index != UINT32_MAX) {
      global::get<render::tile_highlights_render>()->add(tile_index);
      if (current_game_state == &world_map_state && !loading) {
        auto nuklear_ctx = &global::get<interface::context>()->ctx;
        // нужно проверить чтобы не было пересчения с окнами
        // перечение с тултипом, как его избежать? нужно проверить пересечение со всеми окнами ДО ЭТОГО
        // нужно тогда самостоятельно задать название окна
        const bool condition = !nk_window_is_any_hovered(nuklear_ctx) || nk_window_is_active(nuklear_ctx, "tile_window");
        if (condition) tile_func(base_systems.interface_container->moonnuklear_ctx, tile_index);
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
