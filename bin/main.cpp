#include "helper.h"
#include "utils/utility.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

using namespace devils_engine;

int main(int argc, char const *argv[]) {
  set_application_path();

  glfw_t glfw;

  const uint32_t thread_pool_size = std::max(std::thread::hardware_concurrency()-1, uint32_t(1));
  //const uint32_t threads_count = thread_pool_size+1;
  dt::thread_pool pool(thread_pool_size);

  input::data input_data;
  global::get(&input_data);
  yacs::world world;

  system_container_t systems;
  create_render_system(systems);
  create_map_container(systems);
  create_render_stages(systems);
  
  const float dist = 550.0f;
  const glm::vec3 default_camera_pos = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * dist;
  
  auto ent = world.create_entity();
  ent->add<components::transform>(default_camera_pos, glm::vec3(-1.0f, 0.0f, 0.0f));
  ent->add<components::camera>(ent);
  
  // какие энтити у нас будут? провинции?
  // провинции, города и другие статичные вещи наверное лучше сделать отдельными структурами
  // для чего нам может потребоваться энтити в этих случаях? особые свойства? это какие?
  // 
  
  keys_setup();
  
  {
    utils::time_log log("map generation");
    utils::random_engine_st engine;
    FastNoise noise(1);
    noise.SetNoiseType(FastNoise::Perlin);
    
    const map::generator_data gen_data{
      &engine,
      &noise,
      199,
      0.65f,
      0,    // не могу ничего адекватного получить из blur_plate_boundary_stress
      0.4f, // почти любые значения могут сгенерировать очень плохие взаимодейсвия плит
      4,
      7,
      0.3f,
      1.0f,
      1.0f,
      4000
    };
    
    // 5000 провинций - многовато
    
    map::generator_context context;
    context.data = &gen_data;
    context.map = global::get<core::map>();
    
    create_map_generator(systems, &pool, &context);
    
    global::get<systems::generator<map::generator_context>>()->generate(&context);
    
    const size_t mem = context.memory();
    PRINT_VAR("context memory  b", mem)
    PRINT_VAR("context memory mb", float(mem) / 1024.0f / 1024.0f)
  }

  global::get<render::window>()->show();
  utils::frame_time frame_time;
  while (!global::get<render::window>()->close()) {
    frame_time.start();
    poll_events();
    
    const size_t time = frame_time.get();
    
    mouse_input(ent, time);
    zoom_input(ent);
    next_nk_frame();
    camera::strategic(ent);
    const uint32_t picked_tile_index = cast_mouse_ray();
    overlay::debug(picked_tile_index);
    
    // теперь все гексы вычисляются в компут шейдере и кажется это происходит крайне быстро
    // теперь у меня есть все гексы в индексном инстансном буфере, нужно туда добавить какие нибудь дополнительные данные
    // точнее они наверное добавятся: мне нужно на гексах нарисовать границы (причем так чтобы они не мерцали), 
    // мне нужно нарисовать границу (по идее это просто цвет сверху), выделение выбраной провинции + разные типы отображения карты
    // (культуры, религии, дипломатия и проч)
    // нужно нарисовать объекты на тайле (деревья + особые объекты), заполним для этого инстансный буфер в компут шейдере
    // денсити может быть как больше нуля (денсити * макс_количество = объекты на тайле), 
    // так и меньше нуля (шанс появления от тайла к тайлу), у тайла есть самый важный параметр - id тайла, 
    // по которому мне нужно будет генерировать множество значений, осталось понять как генерировать точки на гексагоне
    // причем очень быстро, возможно их неплохо было бы как нибудь запоминать

    global::get<systems::render>()->update(global::get<render::container>());
    const size_t sync_time = global::get<render::window>()->flags.vsync() ? global::get<render::window>()->refresh_rate_mcs() : 0;
    sync(frame_time, sync_time);
  }

  return 0;
}
