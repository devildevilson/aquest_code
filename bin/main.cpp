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

  input::data input_data;
  global::get(&input_data);
  render::mode_container mode_container;
  setup_rendering_modes(mode_container);
  global::get<const render::mode_container>(&mode_container);
  render::updater upd;
  global::get(&upd);
  yacs::world world;

  system_container_t systems;
  create_render_system(systems);
  create_map_container(systems);
  create_render_stages(systems);
  create_interface(systems);
  
  // по всей видимости луа занимает реально много памяти
  // что то с этим сделать вряд ли можно (интересно есть ли какие нибудь жесткие ограничения на андроиде?)
  // вроде как жестких ограничений нет, смогу ли я сделать приложение меньше чем 500 мб? 
  // это хороший ориентир для приложений без луа, но с ним получается как то слишком многопамяти =(
//   sol::state lua;
//   lua.open_libraries(sol::lib::base);
//   sol::table table = lua.create_table();
//   set_default_values(lua, table);
  
  const float dist = 550.0f;
  const glm::vec3 default_camera_pos = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * dist;
  
  auto ent = world.create_entity();
  ent->add<components::transform>(default_camera_pos, glm::vec3(-1.0f, 0.0f, 0.0f));
  ent->add<components::camera>(ent);
  
  // какие энтити у нас будут? провинции?
  // провинции, города и другие статичные вещи наверное лучше сделать отдельными структурами
  // для чего нам может потребоваться энтити в этих случаях? особые свойства? это какие?
  // не уверен зачем тут может потребоваться энтити
  // проблема в том что тут довольно серьезную оптимизацию нужно предпринять по уменьшению занимаемой памяти
  // да и ко всему прочему очень мало данных у каждой энтити
  
//   std::cout << "Current path is " << std::filesystem::current_path() << '\n';
  
  keys_setup();
  
//   map::generator::context ctx;
//   ctx.pool = &pool;
//   ctx.map = global::get<core::map>();
// //   ctx.noise = new FastNoise(1);             // это удалится после создания
// //   ctx.random = new utils::random_engine_st; // это удалится
//   ctx.container = global::get<map::generator::container>();
//   ctx.noise = nullptr;
//   ctx.random = nullptr;
  
//   systems::generator* gen = nullptr;
  map::creator* creator = nullptr;
  
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
  
  game_state new_state = game_state::create_map; // нужно для загрузчика
  game_state current_state = game_state::create_map;
  game_state old_state = game_state::create_map;
  //game_state current_state = game_state::map;
  global::get<render::window>()->show();
  utils::frame_time frame_time;
  while (!global::get<render::window>()->close()) {
    frame_time.start();
    const size_t time = frame_time.get();
    input::update_time(time);
    poll_events();
    mouse_input(ent, time); 
    key_input(time);
    zoom_input(ent);
    next_nk_frame(time);
    camera::strategic(ent);
    
    switch (current_state) {
      case game_state::loading: {
        ASSERT(false);
        // лоадинг может быть от карты к битве, от карты к столкновению, от битвы к карте, от столкновения к карте
        // переход от карты означает что мы должны сохранить все данные карты в каком нибудь адекватном виде
        // и загрузить только то что нужно непосредственно в битве (это анимации, биомы, генерация карты, расстановка войск)
        // нужно выделить все данные используемые на карте и их удалить после сериализации (в том числе core::map)
        // тут мы должны получить от чего к чему переходим и какие то данные этих переходов
        // переход от создания карты не должен переключать нас на экран загрузки, идеально сделать отдельный рендер для интерфейса
        // нужно попытаться сделать этот переход с функцией post_generation_work
        
        if (old_state == game_state::create_map && new_state == game_state::map) {
          // нужно положить post_generation_work в треад пул вместе с контейнером (можно в контейнере указывать когда конец когда начало)
          // нужно запустить функцию отрисовки интерфейса, 
          // причем перед этим нужно либо загрузить новую картинку, 
          // либо подсказать интерфейсу что использовать заглушку не нужно
        }
        
        if (old_state == game_state::map && new_state == game_state::battle) { // начало битвы
          
        }
        
        if (old_state == game_state::map && new_state == game_state::encounter) {
          
        }
        
        if (old_state == game_state::battle && new_state == game_state::map) { // конец битвы
          
        }
        
        if (old_state == game_state::encounter && new_state == game_state::map) {
          
        }
        
        if (old_state == game_state::map && new_state == game_state::menu) { // выходим в главное меню
          
        }
        
        if (old_state == game_state::battle && new_state == game_state::menu) {
          
        }
        
        if (old_state == game_state::encounter && new_state == game_state::menu) {
          
        }
        
        // загрузка игры (можем ли мы сохранить игру посреди битвы? вообще наверное можем, но там нужно дожидаться окончания анимаций)
        if (old_state == game_state::menu && new_state == game_state::map) {
          
        }
        
        break;
      }
      
      case game_state::menu: {
        ASSERT(false);
        break;
      }
      
      case game_state::create_map: {
        if (creator == nullptr) {
          creator = setup_map_generator();
        }
        
        creator->generate();
        
        if (creator->back_to_menu()) {
          destroy_map_generator(&creator);
        }
        
        if (creator->finished()) {
          post_generation_work(creator, systems); // эти вещи тоже вполне можно сделать асинхронно
          destroy_map_generator(&creator);
          current_state = game_state::map;
        }
        
        break;
      }
      
      case game_state::map: {
//         const uint32_t picked_tile_index = cast_mouse_ray();
//         global::get<render::tile_render>()->picked_tile(picked_tile_index); // тайл теперь нужно пикать с учетом высоты тайлов
//         overlay::debug(picked_tile_index, global::get<map::generator::container>());
        
        update(time);
        break;
      }
      
      case game_state::battle: {
        ASSERT(false);
        break;
      }
      
      case game_state::encounter: {
        ASSERT(false);
        break;
      }
    }
    
    // ошибки с отрисовкой непосредственно тайлов могут быть связаны с недостаточным буфером для индексов (исправил)
    // еще меня беспокоит то что игра занимает уже 270 мб оперативы 
    // (примерно 100 мб занимает вся информация о игровой карте (размер контейнера и размер данных в кор::мап))
    // (еще мегобайт 100 занимает луа (похоже что с этим бороться будет крайне сложно))
    // нужно каким то образом это сократить
    auto s = global::get<core::map>()->status();
    if (s == core::map::status::valid) global::get<core::map>()->set_status(core::map::status::rendering);
    global::get<systems::render>()->update(global::get<render::container>());
    const size_t sync_time = global::get<render::window>()->flags.vsync() ? global::get<render::window>()->refresh_rate_mcs() : 0;
    sync(frame_time, sync_time);
  }

  return 0;
}
