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

  //components::camera camera(default_camera_pos); // будем использовать эту камеру только в ворлд мап
  //camera.zoom_add(100.0f);
  //global::get(&camera);
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
  
  // переходим к городам и армиям, можно еще подумать насчет гербов, как их делать? 
  // смешивать цвета в фрагментом шейдере, кажется самый нормальный способ нарисовать пару гербов
  // так ли необходимо сделать возможность выделения их мышкой?
  
  // армии, с чего начать? во первых их нужно рисовать, во вторых нужно взаимодействовать с ними
  // кстати нужно ли делать какую то возможность действовать героями на армии? например
  // определенно нужно сделать возможность вызвать полководца на дуэль при каких то условиях
  // как то поднасрать армии? было бы неплохо, как должен выглядеть интерфейс?
  // я полагаю что можно стащить идею из вахи, то есть снизу плашка с отрядами или партией героя
  // я до сих не очень понимаю ограничивать ли размер армии или нет, армии отличаются по владельцам
  // и мы должны легко определять кто перед нами, противник или вассал или союзник
  // это мы можем делать с помощью знамен + к знаменам было бы неплохо пришпилить какой то опознавательный цвет
  // короч нужны армии + знамя над ним, знамя нужно рисовать с другим первым слоем, 
  // нужно прилепить на кнопку home переход к столице, 
  // знамена, гербы, города имеет смысл рисовать только при приближении
  // передвижение армии было бы неплохо как то анимировать, такое чувство что нужно заводить отдельный массив
  // для армий еще, там тип позиция, текстурка, герб (герб как бы отдельно нарисуем)
  // как нибудь сделать так чтобы не заводить еще буферы, а то и так слишком много
  
  // нужно сделать выделение: выделять я могу героев, армии, города, строения (персонажей? или это иначе делается)
  // выделение нескольких сущностей за раз? выделение дает возможность также нарисовать какой то интерфейс
  // для интерфейса нужно отправить какой то указатель в интерфейс, обджект
  // нужен ли в этом деле какой то помошник? вряд ли, у нас определенно должны быть 
  // какие то функции для взаимодействия с данными внутри указателя
  // например армии: просмотреть инфу по отрядам, выделить новую армию (указать место для создания новой армии?),
  // передать отряды из армии в армию
  
  // то есть у нас по крайней мере должно быть выделение 2 армии, я так понимаю этот случай делается 
  // чем то вроде эвентов, то есть на разные типы взаимодействия разные функции, для этого в свою очередь нужно
  // сделать множество функции с базовыми штуками, тип: передача одного отряда, и разные дургие штуки
  // возможно нужно сделать здесь же поиск пути, причем вот тут как раз нужно некое хранилище внешнее
  // при удержании правой кнопки мыши мы ищем путь в это хранилище, и при нажатии пихаем новый путь в 
  // армию или героя, 
  
  // на самом деле первое что нужно сделать это столновение 2-ух армий и генерацию карты?
  // или нет? поиск пути? нужно сначала сделать выделение и движение армии
  // выделение? выделить по идее мы можем одну структуру, но несколько армий (это еще тоже та еще проблема)
  // если несколько армий выделяем то и поисков пути должно быть несколько =(
  // искать лучше бы уникально для всех армий, но большинству армий не нужны допонительные штуки для синхронизации
  // как для игрока, может сделать максимальный размер выделения 256 или что то вроде?
  // для игрока добавлять задания в очередь, как выделять много армий? зажатая левая кнопка мыши 
  // выделяет область на экране, область на экране - это по идее тот же фрустум, только с чуточку другой позицией
  // как эту позицию найти? это центр выделенной области? или это из позиции камеры деформированный фрустум?
  // находим две точки на сфере (?), строим по ним бокс, тестируем
  
  // я тут подумал о том как сделать неразведанную территорию, было бы круто сделать так чтобы был не виден ландшафт
  // для этого нужны какие то другие тайлы, для других тайлов нужны иные стенки тайлов, пересчитывать стенки каждый раз
  // совершенно не желательно, я подумал что можно рисовать цилиндры для всех тайлов 
  // (есть правда небольшая проблема как сделать разный цвет у вершины целиндра и нижней части, можно рисовать по частям)
  // при такой отрисовке нужно использовать инстансный буфер (в буферах хранить не индексы вершин, а только инстансы)
  // другое дело что так нужно будет опять делить рендер на пентагоны и гексагоны (ну эт меньшая проблема)
  
  // что теперь? нужно положить найденные армии в селектор, по идее для вещей в селекторе нужно нарисовать какое то выделение
  // у меня есть пара идей, теперь когда у нас выбраны армии нужно как то их перемещать, по идее мы выбираем тайл 
  // зажимаем правую кнопку и уже должен искаться путь в отдельном потоке, причем нужно прекращать предыдущий поиск
  // и начинать следующий при смене тайла, когда мы кликаем нас перестает волновать состояние поисковика 
  // по идее мы можем это сделать через флажок
  
  // перейдем к генерации битв: это довольно сложная в плане кодовой базы механика,
  // мне нужно сделать обычную битву, осаду + энкаунтер героя,
  // все это генерируется из данных вокруг персонажей и армий (биомы, постройки, собственно сами армии)
  // ко всему прочему мне обязательно нужно как то избавиться от ненужных данных карты,
  // для этого мне нужно сохранить состояние глобальной карты (по идее это просто обычное сохранение игры)
  // (можно даже спросить у игрока, нужно ли сохранить это дело на диск?) 
  // от карты то в общем то остаются только геральдики (+ системные какие то картинки, иконки)
  // загрузка битвы и генерация карты для этой битвы, это одно и тоже? было бы неплохо сделать сохранение и там
  // генерация опять работает пошагово, как получить нужную инфу? в принципе генератор не особо отличается от предыдущего
  // инфу мы можем получить на первом шаге, а дальше сохраняем и начинаем собственно генерацию
  // короч видимо придется делать особый первый шаг
  
  // мне нужно придумать способ свободно отключать рендеринг какой то части пайплайна

//   const std::vector<std::string> base_interfaces = { // как передать данные?
//     "main_menu",
//     "",
//     // должен быть указатель на персонажа, вообще наверное нет,
//     // должен быть указатель на какого то помошника (там должен быть более простой доступ ко всем функциям)
//     // например контейнер для поиска жен, и прочее, как его передать?
//     "player_interface",
//     "battle_interface",
//     "encounter_interface"
//   };

  const std::array<utils::quest_state*, utils::quest_state::count> game_states = {
    &main_menu_state,
    &map_creation_state,
    &world_map_state,
    // эти стейты должны генерить карту, сохранения? в любом случае будут в этом стейте
    &battle_state,
    &encounter_state
  };

  //uint32_t current_game_state_index = utils::quest_state::main_menu;
  uint32_t current_game_state_index = utils::quest_state::battle;
  utils::quest_state* current_game_state = game_states[current_game_state_index];
  utils::quest_state* previous_game_state = nullptr;
  bool loading = true;
//   current_game_state->enter();
  
//   std::vector<void*> map_buffer(5000, nullptr);

  global::get<render::window>()->show();
  utils::frame_time frame_time;
  while (!global::get<render::window>()->close() && !base_systems.menu->m_quit_game) {
    frame_time.start();
    const size_t time = frame_time.get();
    input::update_time(time);
    poll_events();
    if (global::get<render::window>()->close()) break;

    if (loading) {
      if (current_game_state->load(previous_game_state)) {
        loading = false;
        base_systems.interface_container->close_all();
      }
    } else {
      current_game_state->update(time);
      if (current_game_state->next_state() != UINT32_MAX) {
        const uint32_t index = current_game_state->next_state();
        // ождается что здесь мы почистим все ресурсы доконца и обратно вернем next_state к UINT32_MAX
        // это не работает для перехода от карты к битве и обратно
        // мне либо делать так же как в героях (то есть не чистить память мира)
        // либо нужно придумать иной способ взаимодействия между стейтами
        // 
        previous_game_state = current_game_state;
//         current_game_state->clean();
        current_game_state = game_states[index];
//         current_game_state->enter(); // здесь мы должны подготовить системы к использованию
        loading = true;
        current_game_state_index = index;
      }
    }
    
    if (!loading && current_game_state == &world_map_state) {
//       static bool first = true;
//       if (first) {
//         const size_t city_count = map_systems.core_context->get_entity_count<core::city>();
//         for (size_t i = 0; i < city_count; ++i) {
//           const auto city = map_systems.core_context->get_entity<core::city>(i);
//           const auto &tile_data = render::unpack_data(map_systems.map->get_tile(city->tile_index));
//           const glm::vec4 pos = map_systems.map->get_point(tile_data.center);
//           union convert {
//             void* user_data;
//             core::map::user_data data;
//           };
//           convert c;
//           c.data.index = i;
//           c.data.type = core::map::user_data::type::city;
//           
//           const core::map::object obj{
//             {
//               pos,
//               glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)
//             },
//             c.user_data
//           };
//           
//           const size_t add_ret = map_systems.map->add_object(obj);
//           ASSERT(add_ret != SIZE_MAX);
//         }
//         first = false;
//       }
//       
//       const auto buffers = global::get<render::buffers>();
//       const auto &mat = buffers->get_matrix();
//       const auto frustum = utils::compute_frustum(mat);
//       {
//         utils::time_log tl("frustum culling");
//         const int32_t ret = map_systems.map->frustum_culling(frustum, map_buffer);
//         if (ret < 0) {
//           // нехвататет памяти в буффере
//           throw std::runtime_error("increase buffer");
//         }
//       }
//       
//       //PRINT_VAR("frustum culling res", ret)
//       
//       const auto opt = global::get<render::tile_optimizer>();
//       const auto tile_buffer = opt->tiles_index_buffer();
//       const auto indirect_buffer = opt->indirect_buffer();
//       auto device = global::get<systems::core_t>()->graphics_container->device;
//       yavf::Buffer staging(device, yavf::BufferCreateInfo::buffer(tile_buffer->info().size, VK_BUFFER_USAGE_TRANSFER_DST_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
//       
//       auto task = device->allocateTransferTask();
//       task->begin();
//       task->copy(tile_buffer, &staging); // копировать буфер понятное дело лучше не здесь
//       task->end();
//       task->start();
//       task->wait();
//       
//       auto index_arr = reinterpret_cast<uint32_t*>(staging.ptr());
//       auto indirect_data = reinterpret_cast<struct render::tile_optimizer::indirect_buffer*>(indirect_buffer->ptr());
      
      // в общем это рабочий вариант, но прикол в том что я могу делать это в гпу
      // но мне нужно наверное все же делать это в отдельном шейдере с другим набором данных
      // мне нужно решить несколько проблем: посчитать рейкастинг, сделать выделение
      // данные какие? мне нужно нарисовать города, структуры, гербы, 
      // то есть нужно создать еще один буффер с тайлами, в этих тайлах 
      // указать индекс стуктур, индекс герба, дороги, 
//       const uint32_t tiles_count = indirect_data->tiles_command.indexCount;
//       PRINT_VAR("tiles count", tiles_count / (PACKED_TILE_INDEX_COEF+1))
//       for (uint32_t i = 0; i < tiles_count; i += PACKED_TILE_INDEX_COEF+1) {
//         const uint32_t tile_index = index_arr[i] / PACKED_TILE_INDEX_COEF;
//         auto city = core::get_tile_city(tile_index);
//       }
      
      // кажется работает, можно сделать примерно тоже самое для городов (хотя нужно ли, мы можем работать с городами через тайл)
      // и для армий с героями (с ними таже история, зачем находить пересечение с ними если тайл очень близко)
      // теперь когда у нас есть вменяемые выделения, мне нужно делать всякие интерфейсы с инфой о вещах которые я выделяю
      // интерфейс города, интерфейс героя и армии, интерфейс персонажа (скорее всего немного интерфейса мы спиздим у цк2)
      // героям и армиям нужно будет делать поиск пути, а значит нужно хранить путь, путь это набор индексов тайлов
      // то есть по идее это множество массивов с индексами, массивы скорее всего будут не очень большими (100-200 размер)
      // но нужно быть готовым к тому что нужно будет выделать много памяти для этогоНаверное будет неплохо выделять 
      // память кусками для пути (то есть struct { uint32_t path_part[200]; void* next; }, как то так)
      
      const auto opt = global::get<render::tile_optimizer>();
      const auto indirect_buffer = opt->indirect_buffer();
      auto indirect_data = reinterpret_cast<struct render::tile_optimizer::indirect_buffer*>(indirect_buffer->ptr());
      // это не работает так как я хочу, спинлок вылетает в шейдерах
//       if (indirect_data->padding2[2] != UINT32_MAX) {
//         PRINT_VAR("heraldy dist ", glm::uintBitsToFloat(indirect_data->padding1[2]))
//         PRINT_VAR("heraldy index", indirect_data->padding2[2])
//       }
      
      // кажется работает, если строить фрустум получается гораздо точнее, 
      // но слишком много вычислений (?), у меня дополнительно должна быть логика 
      // замены выделения, по идее это означает что я должен как то дать понять игроку что он выделяет
      // но при этом не снимать старого выделения, как быть если выделение есть но при этом не выделено ничего?
      // по идее нужно чистить выделение, верно? или чистить выделение только при клике? для мобилок
      // нормально будет если мы будем чистить при клике (неплохо было бы иметь возможность поменять поведение выделения)
      // в этот селектион должны попадать еще строения (?), после чего мне его нужно обойти 
      // и выбрать только то с чем я могу взаимодействовать, игрок может выбрать все свои армии и героев,
      // но только по одной штуке города (армии противника неплохо было бы выделять все чтобы посмотреть сколько всего сил)
      // свои армии и армии противника нельзя выделить одновременно, свои армии в приоритете
      // (неплохо было бы иметь возможность поменять приоритеты в настройках)
      // довольно легко добавить индексы умноженные на некий коэффициент + тип того что выделили,
      // надеюсь что это будет работать быстро, нужно подсократить количество условий
      // как подсветить выделяемые вещи лучем? хороший вопрос, кажется достаточно добавить бит к индексу
      // было бы неплохо запихнуть как можно больше разных вещей в один шейдер, об этом позже
      
      // серьезно можно умножить индекс на количество типов + непосредственно тип и так хранить
      // типов сколько? армия+герой, структура, ??? (неплохо было бы разделить друг от друга эти вещи)
      // с некоторыми структурами взаимодействовать нельзя
      
      float min_dist = 10000.0f;
      uint32_t min_dist_heraldy_index = UINT32_MAX;
      
      const uint32_t selection_count = indirect_data->heraldies_command.indexCount;
      const auto selection_buffer = opt->structures_index_buffer();
      const auto selection_data = reinterpret_cast<uint32_t*>(selection_buffer->ptr());
//       if (selection_count != 0) {
//         PRINT_VAR("selection_count", selection_count)
        for (uint32_t i = 0 ; i < selection_count; ++i) {
          if (selection_data[i] > INT32_MAX) {
            const float tmp = -glm::uintBitsToFloat(selection_data[i]);
            if (tmp < min_dist) {
              min_dist = tmp;
              min_dist_heraldy_index = selection_data[i+1];
            }
            
            ++i;
            continue;
          }
          
          PRINT_VAR(std::to_string(i), selection_data[i])
  //         const uint32_t army_gpu_index = selection_data[i];
          // тут по идее нужно использовать unordered_map, в который мы положим индексы армии и указатели на армию
          global::get<utils::objects_selector>()->add(utils::objects_selector::unit::type::army, selection_data[i]);
          // хотя лучше просто армии по слотам расположить
          // вот у нас селектион, перемещение камеры по карте нужно сделать тогда приближением к краям + нужно сделать рендер курсора?
          // хотя наверное можно обойтись просто ограничением мышки
        }
        
        //PRINT_VAR("count", global::get<utils::objects_selector>()->count)
//       }
      
      if (min_dist_heraldy_index != UINT32_MAX) {
        PRINT_VAR("heraldy dist ", min_dist)
        PRINT_VAR("heraldy index", min_dist_heraldy_index)
      }
      
      // в моей игре еще нужно будет сделать видимость на карте, это довольно несложно сделать с помощью битового массива
      // не просто видимость, 3 типа: территория не разведана, территория разведана, территория видима
      // как сделать правильно? у меня есть с этим небольшая проблема, заключается в том что мне нужно нарисовать стенки тайлов
      // неплохо было бы рисовать динамические стенки, для этого нужно проверять соседей у всех видимых тайлов 
      // либо собрать все ребра на карте (по идее их столько же сколько точек), и дополнительно вычислить еще и ребра
    }
    
    auto camera = get_camera();
    const uint32_t tile_index = cast_mouse_ray();
    mouse_input(camera, time, tile_index);
    key_input(time, current_game_state_index, loading);
    zoom_input(camera);
    next_nk_frame(time);
    camera::strategic(camera);
    
    if (camera != nullptr) camera->update(time);
    
//     PRINT_VEC3("camera pos", camera->current_pos())
    
    base_systems.interface_container->draw(time);
    
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
    if (tile_index != UINT32_MAX) {
      global::get<render::tile_highlights_render>()->add(tile_index, render::make_color(0.9f, 0.9f, 0.0f, 0.5f));
      if (current_game_state == &world_map_state && !loading) {
        auto nuklear_ctx = &global::get<interface::context>()->ctx;
        // нужно проверить чтобы не было пересчения с окнами
        // перечение с тултипом, как его избежать? нужно проверить пересечение со всеми окнами ДО ЭТОГО
        // нужно тогда самостоятельно задать название окна
        const bool condition = !nk_window_is_any_hovered(nuklear_ctx) || nk_window_is_active(nuklear_ctx, "tile_window");
        if (condition) tile_func(base_systems.interface_container->moonnuklear_ctx, tile_index);
      }
    }
    
    draw_army_path();

    // ошибки с отрисовкой непосредственно тайлов могут быть связаны с недостаточным буфером для индексов (исправил)
    // еще меня беспокоит то что игра занимает уже 270 мб оперативы
    // (примерно 100 мб занимает вся информация о игровой карте (размер контейнера и размер данных в кор::мап))
    // (еще мегобайт 100 занимает луа (похоже что с этим бороться будет крайне сложно))
    // нужно каким то образом это сократить (по итогу все это дело занимает сейчас 500 мб, и 1 гб при генерации)
    lock_rendering();
    base_systems.render_slots->update(global::get<render::container>());
    const size_t sync_time = global::get<render::window>()->flags.vsync() ? global::get<render::window>()->refresh_rate_mcs() : 0;
    sync(frame_time, sync_time);
  }
  
  pool.compute();
  pool.wait();

  return 0;
}
