#include "helper.h"
#include "utils/utility.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

using namespace devils_engine;

enum class game_state {
  loading,
  menu,
  create_map,
  map
};

int main(int argc, char const *argv[]) {
  set_application_path();
  
//   PRINT("first sleep")
//   std::this_thread::sleep_for(std::chrono::seconds(5));

  glfw_t glfw;

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
  
  game_state current_state = game_state::create_map;
  global::get<render::window>()->show();
  utils::frame_time frame_time;
  while (!global::get<render::window>()->close()) {
    frame_time.start();
    poll_events();
    const size_t time = frame_time.get();
    mouse_input(ent, time); 
    key_input(time);
    zoom_input(ent);
    next_nk_frame(time);
    camera::strategic(ent);
    input::update_time(time);
    
    switch (current_state) {
      case game_state::loading: {
        ASSERT(false);
        break;
      }
      
      case game_state::menu: {
        ASSERT(false);
        break;
      }
      
      case game_state::create_map: {
        // тут или ранее мы должны создать несколько генераторов
//         if (generators.empty()) {
//           generators = ;
//         }
        
        // рисуем интерфейс
        // если в данный момент мы генерируем карту 
        // то у нас должен быть интерфейс генерации 
        // (то есть прогресс бар с описанием что происходит)
        // для того чтобы это правильно сделать нужно будет 
        // вынести генерацию в отдельный поток
        // поэтому пока наверное не буду это делать
        
//         if (gen == nullptr) {
//           gen = new systems::generator;
//         }
//         
// //         sol::table table; // эта таблица не должна пересоздаваться каждый раз
//         const auto status = overlay::debug_generator(gen, &ctx, table);
//         
//         if (status == overlay::state::constructed_generator) {
//           gen->generate(&ctx, table);
//           rendering_mode(ctx.container, ctx.map, map::debug::properties::tile::plate_index, 1, 0); // должна быть функция от строки
//           //render::mode("plate_rendering");
//         }
//         
//         // если карта сгенерирована то мы должны перейти к следующему шагу
//         if (status == overlay::state::end) {
//           find_border_points(ctx.container, ctx.map, table);
//           current_state = game_state::map;
//           DELETE_PTR(ctx.random)
//           DELETE_PTR(ctx.noise)
//           DELETE_PTR(gen)
//           {
//             const size_t mem = ctx.container->compute_memory_size();
//             std::cout << "Container size: " << mem << " bytes (" << (float(mem)/1024.0f/1024.0f) << " mb)" << "\n";
//           }
//           
//           {
//             const size_t mem = ctx.map->memory_size();
//             std::cout << "Game map  size: " << mem << " bytes (" << (float(mem)/1024.0f/1024.0f) << " mb)" << "\n";
//           }
//         }
        
        if (creator == nullptr) {
          creator = setup_map_generator();
        }
        
        creator->generate();
        
        if (creator->finished()) {
          destroy_map_generator(&creator);
          find_border_points(global::get<map::generator::container>(), global::get<core::map>(), sol::table());
          generate_tile_connections(global::get<core::map>(), &pool);
          current_state = game_state::map;
        }
        
        break;
      }
      
      case game_state::map: {
        // когда игра загружена у нас должен быть список персонажей
        // по идее достаточно обойти список и вызвать функцию поведения
        break;
      }
    }
    
    const uint32_t picked_tile_index = cast_mouse_ray();
    global::get<render::tile_render>()->picked_tile(picked_tile_index);
    overlay::debug(picked_tile_index, global::get<map::generator::container>());
    
    // ошибки с отрисовкой непосредственно тайлов могут быть связаны с недостаточным буфером для индексов (исправил)
    // еще меня беспокоит то что игра занимает уже 270 мб оперативы 
    // (примерно 100 мб занимает вся информация о игровой карте (размер контейнера и размер данных в кор::мап))
    // (еще мегобайт 100 занимает луа (похоже что с этим бороться будет крайне сложно))
    // нужно каким то образом это сократить
    global::get<systems::render>()->update(global::get<render::container>());
    const size_t sync_time = global::get<render::window>()->flags.vsync() ? global::get<render::window>()->refresh_rate_mcs() : 0;
    sync(frame_time, sync_time);
  }

  return 0;
}
