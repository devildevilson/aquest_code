#include "helper.h"

using namespace devils_engine;

int main(int argc, char const *argv[]) {
  UNUSED_VARIABLE(argc);
  UNUSED_VARIABLE(argv);
  
  set_application_path();

  glfw_t glfw;

  // насколько адекватно создавать луа стейт для каждого потока?
  const uint32_t thread_pool_size = std::max(std::thread::hardware_concurrency()-1, uint32_t(1));
  dt::thread_pool pool(thread_pool_size);
  global::get(&pool);
  utils::deffered_tasks tasks;
  global::get(&tasks);

  systems::core_t base_systems;
  systems::map_t map_systems;
  systems::battle_t battle_systems;
  systems::encounter_t encounter_systems;
  
  global::get(&base_systems);
  global::get(&map_systems);
  global::get(&battle_systems);
  global::get(&encounter_systems);

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
  
  input::block();

  utils::main_menu_loading_state main_menu_loading_state;
  utils::main_menu_state main_menu_state;
  utils::map_creation_loading_state map_creation_loading_state;
  utils::map_creation_state map_creation_state;
  utils::map_creation_generation_state map_creation_generation_state;
  utils::world_map_loading_state world_map_loading_state;
  utils::world_map_state world_map_state;
  utils::battle_loading_state battle_loading_state;
  utils::battle_state battle_state;
  utils::encounter_loading_state encounter_loading_state;
  utils::encounter_state encounter_state;

  // по всей видимости луа занимает реально много памяти
  // что то с этим сделать вряд ли можно (интересно есть ли какие нибудь жесткие ограничения на андроиде?)
  // вроде как жестких ограничений нет, смогу ли я сделать приложение меньше чем 500 мб?
  // это хороший ориентир для приложений без луа, но с ним получается как то слишком многопамяти =(

//   const float dist = 550.0f;
//   const glm::vec3 default_camera_pos = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * dist;

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
  
  // частично сделал генерацию, нужно сделать текстурки + базовые отряды + инпут
  // битвы должны быть в несколько фаз: фаза подготовки (расставляем отряды),
  // фаза битвы (собственно делаем боевые действия)
  // еще нужно сделать геройскую битву, ее можно оформить примерно как даркест данжеон
  // то есть прямой пол с текстуркой земли + стенка с текстуркой удаленного ландшафта
  // на полу гексагональные тайлы, выглядит очень похоже на героев, осталось понять какое количество тайлов сделать, но это потом
  // в битвах я остановился на варианте задавать приказы на этапе хода преказы выполняются, 
  // значит нужно как то особенно рендерить этот промежуток между ходами, в это время должны меняться состояния 
  // юнитов в отрядах, отряды должны вступать в битву, самое главное стараться исполнить приказ в любом случае
  // в таком случае очень важна очередность хода + нужно что то придумать с отрядами с разной скоростью, 
  // скорее всего мне либо нужно по ктрл задавать конкретный путь, либо как то так забалансить чтобы они не слишком дальше ходили в отличие от основных отрядов
  // но поиск пути по ктрл довольно полезная хрень, если не весь доступный ход был использован то можно пометить остаток хода куда может сходить юнит
  // 
  
  // нужно сделать что? ... отряд (какой то базовый рендер отряда) + управление отрядом
  // управление меняется в зависимости от текущего quest_state, а значит наверное 
  // логику нужно перенести туда, более менее переписал инпут, теперь разный инпут в разных состояниях
  // чет я подумал что бессмысленно перетаскивать логику в quest_state, исправил парочку вещей
  // теперь по идее нужно сделать войска, чего мне не хватает? рендер войск + выделение
  // 
  
  // мне нужно придумать способ свободно отключать рендеринг какой то части пайплайна
  
  // рандом в генераторе нужно организовать как строку из 16 символов
  // тип: deadbeafdeadbeaf
  
  // судя по тому видосу из процджама мне нужно сделать интерфейс получше
  // возможно стоит сейчас им в принципе заняться, загрузить иконки,
  // попробовать подобрать цвета для интерфейса, как то оформить поля и кнопки
  // нужно вести список того что мне нужно в каких то документах
  
  // короч я более менее понял что нужно чтобы сделать загрузку в битву
  // было бы неплохо сейчас набрасывать основные механики в игру
  // это значит что нужно сделать: интерфейс для персонажа, переходы от одного к другому,
  // интерфейс титула, (и еще куча других интерфейсов), генерацию строк
  
  // начал переписывать функции генератора на луа, хотел вообще изначально сделать генерацию в луа
  // но чему то решил сделать пока в с++, переписанные функции луа по известным причинам значительно
  // медленее чем с++ аналоги (с оптимизациями generate_plates занимает 13 секунд)
  // есть ли хоть какая то возможность ускорить это дело?
  
  // несколько очень важных изменений: теперь весь интерфейс одной функцией у меня
  // в том числе не существует явных переходов между одним интерфейсом и другим
  // все это дело задается с помощью разных таблиц и игрового состояния
  // это избавляет меня от придумывания сложных состояний и отделяет интерфейс
  // от других частей программы, остается вопрос с инпутом, если нет явных переходов
  // то и явных ограничений на инпут тоже нет + интерфейс рисуется полностью
  // а значит и менюшка поверх геймплея рисуется в это же время, без необходимости
  // заводить отдельную структуру меню, логика меню будет собственно в этой функции,
  // короче что делать с инпутом? собственно два варианта: либо всю логику перетащить
  // в луа, либо только часть логики, мы можем перетащить всю логику, для этого
  // нам нужно сделать выделение, приказ на передвижение и проч
  // полностью перенести уже не получится, перемещение камеры контролируется 
  // довольно сложно... ну хотя сложность заключается в основном в том что
  // мне нужно еще положение курсора получать, в общем пока что фиг с ним
  // что нужно сделать сейчас так это кнопки, лучше конечно сделать 
  // просто тупа все эвенты чтобы в интерфейсе можно было поймать, 
  // единственное разраничение что на одну кнопку можно несколько эвентов повесить 
  // по состояниям приложения
  
  // инпут в луа, что может пойти не так? логику реально проще в луа накатать, ко всему прочему
  // функция будет вызвана лишь один раз за кадр, значит не сильно ударит по производительности
  // что нужно в качестве инпута? кнопки мыши, выделение, передвижение камеры на при прикосновении к краям,
  // передвижение камеры по кнопке, скорость передвижения камеры вообще регулируется из настроек
  // и было бы неплохо не давать эту возможность в луа, еще нужно сделать работу с выделением:
  // отменить выделение (ну и перевыделить), задать путь, пройти по пути, выбрать отряд или строение
  // (появится окошко с интерфейсом), просмотреть выделение (хотя может быть вообще работа с выделением)
  // (то есть по шифту добавить, по ктрл убрать и проч)
  // где сделать доступ к индексу тайла?

  const std::array<utils::quest_state*, utils::quest_state::count> game_states = {
    &main_menu_loading_state,
    &main_menu_state,
    &map_creation_loading_state,
    &map_creation_state,
    &map_creation_generation_state,
    &world_map_loading_state,
    &world_map_state,
    // эти стейты должны генерить карту, сохранения? в любом случае будут в этом стейте
    &battle_loading_state,
    &battle_state,
    &encounter_loading_state,
    &encounter_state
  };
  
#ifndef _NDEBUG
  for (size_t i = 0; i < game_states.size(); ++i) {
    assert(game_states[i]->current_state() == i);
  }
  
  assert(game_states[utils::quest_state::main_menu_loading]->current_state() == utils::quest_state::main_menu_loading);
  assert(game_states[utils::quest_state::world_map]->current_state() == utils::quest_state::world_map);
  assert(game_states[utils::quest_state::world_map_generator]->current_state() == utils::quest_state::world_map_generator);
#endif
  
  // теперь у меня есть структура с игровым контекстом
  // нам нужно использовать тамошние переменные
  auto game_ctx = base_systems.game_ctx;
  game_ctx->state = utils::quest_state::main_menu_loading;

  //uint32_t current_game_state_index = utils::quest_state::main_menu_loading;
  //uint32_t current_game_state_index = utils::quest_state::battle;
  utils::quest_state* current_game_state = game_states[game_ctx->state];
  utils::quest_state* previous_game_state = nullptr;
  bool loading = true;
  current_game_state->enter(previous_game_state);

//   std::future<void> rendering_future;
  global::get<render::window>()->show();
  utils::frame_time frame_time;
  while (!global::get<render::window>()->close() && !game_ctx->quit_state()) {
    frame_time.start();
    const size_t time = frame_time.get();
    input::update_time(time);
    poll_events();
    if (global::get<render::window>()->close()) break;
    
    manage_states(game_states, &current_game_state, &previous_game_state, game_ctx, time);
    
    // нужно сделать управление для разных типов карт, по идее лучше переопределить функцию у quest_state
    auto camera = camera::get_camera();
    float ray_tile_dist = 100000.0f;
    const uint32_t tile_index = cast_mouse_ray(ray_tile_dist);
    const uint32_t battle_tile_index = get_casted_battle_map_tile();
    //mouse_input(camera, time, tile_index);
    //key_input(time, 0, loading);
    zoom_input(camera);
    next_nk_frame(time);
    camera::strategic(camera);
    game_ctx->traced_tile_dist = ray_tile_dist;
    game_ctx->traced_tile_index = tile_index;
    
    render::update_selection();
    
    if (camera != nullptr) camera->update(time);
    
//     PRINT_VEC3("camera pos", camera->current_pos())
    
    //base_systems.interface_container->draw(time);
    {
      //utils::time_log log("Interface updation");
      base_systems.interface_container->update(time);
    }
    
    update(time);
    tasks.update(time);
    
    if (game_ctx->reload_interface) {
      base_systems.reload_interface();
      basic_interface_functions(base_systems);
      game_ctx->reload_interface = false;
    }
    
    if (game_ctx->state == utils::quest_state::world_map) {
      //auto c = game_ctx->player_character;
      //test_decision(c);
    }
    
    // в текущем виде занимает в среднем от 20-50 мкс, 
    // я так подозреваю что проблемы начнутся только от 1000 юнитов
    // вообще обновлять юниты нужно от отрядов, так как только 
    // в этом случае можно рассчитать конкретное положение
    if (!loading && current_game_state == &battle_state) {
      static size_t counter = 0;
      static size_t time_counter = 0;
      static auto t_point = std::chrono::steady_clock::now();
      
      auto start = std::chrono::steady_clock::now();
      auto battle = global::get<systems::battle_t>();
      auto ctx = battle->context;
      auto map = battle->map;
      
      const float insc_radius = (glm::sqrt(3.0f) / 2.0f) * 1.0f;
      //const float dist = 1.0f * glm::sin(glm::radians(45.0f));
      const float dist = (insc_radius) * glm::sqrt(2.0f);
      const glm::vec3 fwd = glm::vec3(0.0f, 0.0f, -1.0f);
      const glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
      // чет не понимаю почему все равно за пределы тайла выходит отряд
      // но вроде при 0.3f выглядит более менее
      const float tile_side_offset = 0.3f * (dist * 2.0f);
      const float box_side = (dist * 2.0f) - 2.0f * tile_side_offset;
      const float box_half_side = box_side / 2.0f;
      
      const size_t troop_count = ctx->get_entity_count<battle::troop>();
      for (size_t i = 0; i < troop_count; ++i) {
        auto troop = ctx->get_entity<battle::troop>(i);
        // нужно выяснить что происходит вокруг + прикинуть где должны находиться юниты
        const auto tile_pos = map->get_tile_pos(troop->tile_index);
        // по идее юнитов мы можем расположить в радиусе 1.0 (примерно)
        // нужно сделать построение в окружности, максимальный вписанный квадрат в окружность, которая вписана в гекс
        // вроде более менее сделал (но пока что средне по удачности), 
        // теперь нужно сделать выделение и перемещение войск
        
        const glm::vec3 box[] = {
          //tile_pos + ( fwd) * (box_half_side - tile_side_offset) + (-right) * (box_half_side - tile_side_offset),
          tile_pos + ( fwd) * box_half_side + (-right) * box_half_side,
          tile_pos + ( fwd) * box_half_side + ( right) * box_half_side,
          tile_pos + (-fwd) * box_half_side + (-right) * box_half_side,
          tile_pos + (-fwd) * box_half_side + ( right) * box_half_side,
        };
        
        size_t row_counter = 0;
        size_t column_counter = 0;
        const float offset_column = box_side / 4.0f; // здесь обязательно box_side / (max_column - 1)
        const float offset_row = box_side / 3.0f;    // здесь обязательно box_side / (max_row - 1)
        for (size_t i = troop->unit_offset; i < troop->unit_offset + troop->unit_count; ++i) {
          auto unit = ctx->get_entity<battle::unit>(i);
          const auto pos = box[0] + (-fwd) * (offset_row * row_counter) + right * (offset_column * column_counter);
          unit->set_pos(glm::vec4(pos, 1.0f));
          unit->set_dir(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
          
          ++column_counter;
          if (column_counter >= 5) {
            if (row_counter == 0) {
              const auto dist = glm::distance(pos, box[1]);
              ASSERT(dist < EPSILON);
            }
            
            ++row_counter;
            column_counter = 0;
          }
          
          unit->update(time);
        }
      }
      const size_t unit_count = ctx->get_entity_count<battle::unit>();
//       for (size_t i = 0; i < unit_count; ++i) {
//         auto unit = ctx->get_entity<battle::unit>(i);
//         unit->update(time);
//       }
      
      auto end = std::chrono::steady_clock::now() - start;
      auto mcs = std::chrono::duration_cast<std::chrono::microseconds>(end).count();
      //PRINT_VAR("20 unit state time: ", mcs)
      ++counter;
      time_counter += mcs;
      
//       if (time_counter > ONE_SECOND) {
//         const size_t avg_time = time_counter / counter;
//         PRINT_VAR(std::to_string(unit_count) + " unit state avg time", avg_time)
//         time_counter = 0;
//         counter = 0;
//       }
      
      auto end2 = std::chrono::steady_clock::now() - t_point;
      auto mcs2 = std::chrono::duration_cast<std::chrono::microseconds>(end2).count();
      if (mcs2 > ONE_SECOND) {
        const size_t avg_time = time_counter / counter;
        PRINT_VAR(std::to_string(unit_count) + " unit state avg time", avg_time)
        time_counter = 0;
        counter = 0;
        t_point = std::chrono::steady_clock::now();
      }
      
//       if (battle_tile_index != UINT32_MAX) {
//         PRINT_VAR("battle_tile_index", battle_tile_index)
//       }
    }
    
    // существует какая то утечка (около 100 мб), которая не регистрируется санитизиром
    // она появляется при перезагрузке карты (то есть игра загружает 100 мб, перезагружаем карту, игра отдает примерно 40-60 мб, и снова загружает 100 мб)
    // (и так каждую перезагрузку, с чем связано не понимаю, но кажется что игра правильно выгружает эти лишние аллокации при закрытии приложения)
    const size_t sync_time = global::get<render::window>()->flags.vsync() ? global::get<render::window>()->refresh_rate_mcs() : 0;
    sync(frame_time, sync_time); // rendering_future
  }
  
  game_ctx->quit_game();
  
  pool.compute();
  pool.wait();

  return 0;
}
