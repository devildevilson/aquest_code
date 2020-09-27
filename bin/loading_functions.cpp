#include "loading_functions.h"

#include "utils/utility.h"
#include "utils/thread_pool.h"
#include "utils/globals.h"
#include "map.h"
#include "figures.h"
#include "utils/works_utils.h"
#include "utils/serializator_helper.h"
#include "core_context.h"
#include "utils/systems.h"
#include "utils/string_container.h"
#include "utils/progress_container.h"
#include "logic.h"
#include "map_creator.h"
#include "utils/main_menu.h"
#include "render/targets.h"
#include "render/stages.h"
#include "render/yavf.h"
#include "game_time.h"

namespace devils_engine {
  namespace systems {
    void advance_progress(utils::progress_container* prog, std::string str) {
      prog->set_hint2(std::move(str));
      prog->advance();
    }

    // сюда видимо бедт приходить какая таблица, по которой мы будем создавать генератор
    void setup_map_generator(const systems::map_t* map_data) {
//     auto interface = global::get<systems::core_t>()->interface_container;
//     ASSERT(interface != nullptr);
//     auto ptr = new map::creator(interface);
    auto ptr = map_data->map_creator;
    ASSERT(ptr != nullptr);

    ptr->run_interface_script(global::root_directory() + "scripts/gen_part1.lua");
    ptr->run_interface_script(global::root_directory() + "scripts/gen_part2.lua");
    ptr->run_interface_script(global::root_directory() + "scripts/gen_part3.lua");
//     ptr->run_interface_script(global::root_directory() + "scripts/generator_progress.lua");
//     ptr->progress_interface("gen_progress");
//     interface->register_function("gen_part1_fun", "gen_part1_fun"); // тут регистрировать функции?
//     interface->register_function("gen_part2_fun", "gen_part2_fun");
//     interface->register_function("gen_part3_fun", "gen_part3_fun");

    {
      const std::vector<map::generator_pair> pairs = {
//         map::default_generator_pairs[0],
        map::default_generator_pairs[1],
        map::default_generator_pairs[2],
        map::default_generator_pairs[3]
      };
      ptr->create("Tectonic plates generator", "gen_part1_fun", pairs);
    }

    {
      const std::vector<map::generator_pair> pairs = {
        map::default_generator_pairs[4],
        map::default_generator_pairs[5],
        map::default_generator_pairs[6],
        map::default_generator_pairs[7],
        map::default_generator_pairs[8],
        map::default_generator_pairs[9],
        map::default_generator_pairs[10],
        map::default_generator_pairs[11],
        map::default_generator_pairs[12],
        map::default_generator_pairs[13]
      };

      ptr->create("Biomes generator", "gen_part2_fun", pairs);
    }

    {
      const std::vector<map::generator_pair> pairs = {
        map::default_generator_pairs[14],
        map::default_generator_pairs[15],
        map::default_generator_pairs[16],
        map::default_generator_pairs[17],
        map::default_generator_pairs[18],
        map::default_generator_pairs[19],
        map::default_generator_pairs[20],
        map::default_generator_pairs[21],
      };

      ptr->create("Countries generator", "gen_part3_fun", pairs);
    }
  }

    void find_border_points(core::map* map, const core::context* ctx) {
      // возможно нужно найти первый граничный тайл, а потом обходить его соседей
      // тут можно поступить несколькими способами
      // 1) попытаться обойти тайлы правильно
      // 2) составить облако точек (ну хотя вряд ли)
      // у меня не всегда провинция более менее верной формы
      // не говоря даже про страны, значит нужно попытаться обойти тайлы верно
      // находится первый случайный граничный тайл, от него, по часовой стрелке
      // обходим граничные тайлы (у всех граничных тайлов, должно быть как минимум два граничных соседа)
      // вот еще вариант, отрисовывать границу с помощью тайловой графики
      // (то есть несколько спрайтов шестигранников с границами в разном положении)
      // осталось понять как сделать эту тайловую графику
      // самый простой способ - задавать рендер для трех точек (центр + ребро)
      // у этих трех точек всегда одинаковые uv координаты (потому что по сути рисуем только одну одинаковую картинку)
      // осталось только понять как решить некоторые проблемы такого рендеринга
      // есть разрывы в местах перехода от одного тайла к другому
      // разрывы по идее также можно подкорректировать тайловой графикой
      // короче вот что я подумал (точнее вспомнил как это по идее сделано в цк2)
      // границ должно быть две: граница провинций и граница государств (граница вассалов?)
      // граница провинций - статичная, граница государств и вассалов обновляются почти каждый ход
      // вообще я могу сгенерировать границы между провинциями и менять их свойства по ходу игры
      // то есть мне нужно верно определить ребра провинции
      // у ребра две стороны: мы должны рисовать границы разных свойств на каждой стороне
      // все точки ребер мы добавляем в буфер, ребро - треугольник, каждому ребру нужно передать
      // верную текстурку и какие то дополнительные данные

      // тайлы нам сильно помогут сгенерить границы
      // границы делаем по ребрам провинции, нам их нужно верно соединить
      // ребра провинции должны быть по часовой стрелке относительно провинции
      // для каждой провниции я должен сгенерировать ребра
      // между провинциями ребра нужно соединить,
      // для этого нужно подвести ребро провинции с помощью точки на ребре тайла
      // как получить uv координаты?
      // их можно расчитать для правильного тайла (для неправильного тайла тоже по идее, но нужно проверить)
      // тайл состоит из равносторонних треугольников + у нас имеется размер границы
      // uv для внутренней расчитываются с использованием размера
      // остается только последовательность гарантировать

      // теперь где то эту информацию нужно сохранить

      // как то так мы составляем ребра, что теперь с ними делать?
      // по идее все, можно рендерить
      // обходим все ребра и выдаем им характеристики на базе текущей ситуации (тип границы, цвет) (1 раз в ход)
      // затем параллельно обходим каждый треугольник и проверяем его с фрустумом
      // (можно на гпу сделать по идее) (каждый кадр)
      // рисуем поверх карты

      // так вообще я тут подумал
      // тайловая графика чем хороша - она предсказуема
      // все что мне нужно сделать это определить 3 типа отрисовки границы:
      // только внутреннее ребро, внутренее и внешняя часть с одной стороны, полное ребро
      // то есть по сути нужно только где-то убрать и где-то добавить внешнюю часть ребра
      // значит нужно просто найти все граничные ребра, с информацией о двух тайлов
      // как определить типы? внешняя часть добавляется только к ребру, смежное ребро которого
      // приходится на тайл с тем же самым государством (вассалом, провинцей)
      // нужно проверить смежные ребра
      // по идее этого достаточно для того чтобы собрать буфер после фрустум проверки
      // но это означает что мы каждый кадр обходим этот массив и закидываем данные в буфер
      // с другой стороны как иначе? добавить суда сразу данные о границе (цвет, размер)
      // и считать все на гпу
      struct border_data2 {
        uint32_t tile_index;
        uint32_t opposite_tile_index;
        // по этому индексу мы во первых должны найти opposite_tile_index
        // а во вторых две точки + два смежных тайла
        uint32_t edge_index;
      };

      struct border_buffer {
        glm::vec4 points[2];
        glm::vec4 dirs[2];
      };

      struct border_type2 {
        render::color_t color1; // цвет достаточно хранить в uint32
        uint32_t image;
        render::color_t color2;
        float thickness; // эту штуку не нужно делать здесь - это должны быть константы от титула
      };

      // кому еще соседи могут пригодиться?

  //     auto ctx = global::get<core::context>();

      std::vector<border_data2> borders;
      const uint32_t provinces_count = ctx->get_entity_count<core::province>();
      for (size_t i = 0; i < provinces_count; ++i) {
        const uint32_t province_index = i;
        auto province = ctx->get_entity<core::province>(province_index);
        // нужен более строгий способ получать тайлы у провинций
        //const auto &childs = container->get_childs(map::debug::entities::province, province_index);
        const auto &childs = province->tiles;
        if (childs.empty()) throw std::runtime_error("Could not find province tiles");

        const uint32_t tiles_count = childs.size();
        for (size_t j = 0; j < tiles_count; ++j) {
          const uint32_t tile_index = childs[j];

          const uint32_t offset = borders.size();
          uint32_t border_count = 0;
          const auto &data = render::unpack_data(map->get_tile(tile_index));
          const uint32_t n_count = render::is_pentagon(data) ? 5 : 6;
          for (uint32_t k = 0; k < n_count; ++k) {
            const uint32_t n_index = data.neighbours[k];

            //const uint32_t n_province_index = container->get_data<uint32_t>(map::debug::entities::tile, n_index, map::debug::properties::tile::province_index);
            const uint32_t n_province_index = ctx->get_tile(n_index).province;
            if (province_index == n_province_index) continue;

  //           const auto &n_data = render::unpack_data(map->get_tile(n_index));
  //           const uint32_t k2 = k == 0 ? n_count-1 : k-1;
            const border_data2 d{
              tile_index,
              n_index,
              k
            };
            borders.push_back(d);
            ++border_count;
          }

          // теперь у нас есть offset и border_count
          if (border_count == 0) continue;

          map->set_tile_border_data(tile_index, offset, border_count);
        }
      }

      const float borders_size[] = {0.5f, 0.3f, 0.15f}; // было бы неплохо это убрать в defines.lua (то есть считывать с диска)

      std::unordered_map<const core::titulus*, uint32_t> type_index;
      std::vector<border_type2> types;
      types.push_back({render::make_color(0.0f, 0.0f, 0.0f, 1.0f), UINT32_MAX, render::make_color(0.0f, 0.0f, 0.0f, 1.0f), 0.0f});
      for (size_t i = 0; i < ctx->get_entity_count<core::titulus>(); ++i) {
        // нужно задать типы границ
        // по владельцам?
        auto title = ctx->get_entity<core::titulus>(i);
        if (title->type == core::titulus::type::city) continue;
  //       if (title->type == core::titulus::type::baron) continue;
        if (title->owner == nullptr) continue;
        if (title->owner->main_title != title) continue;
        ASSERT(type_index.find(title) == type_index.end());
        type_index.insert(std::make_pair(title, types.size()));
        types.push_back({title->border_color1, GPU_UINT_MAX, title->border_color2, 0.0f}); // толщину границы мы должны в поинт записать
      }

  //     struct advance_borders_count {
  //       const core::map* map;
  //       size_t index;
  //
  //       advance_borders_count(const core::map* map, size_t i) : map(map), index(i) {}
  //       ~advance_borders_count() {
  //         std::unique_lock<std::mutex> lock(map->mutex);
  //         global::get<render::tile_borders_optimizer>()->set_borders_count(index+1);
  //       }
  //     };

  //     std::unordered_set<uint32_t> unique_index;
      yavf::Buffer* buffer;
      {
        std::unique_lock<std::mutex> lock(map->mutex);
        buffer = global::get<systems::map_t>()->world_buffers->border_buffer;
        buffer->resize(borders.size() * sizeof(border_buffer));
        yavf::Buffer* types_buffer = global::get<systems::map_t>()->world_buffers->border_types;
        types_buffer->resize(types.size() * sizeof(border_type2));
        auto ptr = types_buffer->ptr();
        ASSERT(ptr != nullptr);
        memcpy(ptr, types.data(), types.size() * sizeof(border_type2));
        //global::get<render::tile_borders_optimizer>()->set_borders_count(0);
      }
      //global::get<render::tile_borders_optimizer>()->set_borders_count(borders.size());
      auto pool = global::get<dt::thread_pool>();
      auto* arr = reinterpret_cast<border_buffer*>(buffer->ptr());
      utils::submit_works_async(pool, borders.size(), [&] (const size_t &start, const size_t &count) {
      //for (size_t i = 0; i < borders.size(); ++i) {
      for (size_t i = start; i < start+count; ++i) {
        const auto &current_data = borders[i];
        const auto &tile_data = render::unpack_data(map->get_tile(current_data.tile_index));
        const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;

        ASSERT(tile_data.neighbours[current_data.edge_index] == current_data.opposite_tile_index);

  #ifndef _NDEBUG
        {
          const uint32_t k = current_data.edge_index;
          //const uint32_t k2 = k == 0 ? n_count-1 : k-1;
          const uint32_t k2 = (k+1)%n_count;
          const uint32_t point1 = tile_data.points[k];
          const uint32_t point2 = tile_data.points[k2];
          ASSERT(point1 != UINT32_MAX);
          ASSERT(point2 != UINT32_MAX);
          ASSERT(point1 != point2);

          const auto &n_tile_data = render::unpack_data(map->get_tile(current_data.opposite_tile_index));
          const uint32_t n_count2 = render::is_pentagon(n_tile_data) ? 5 : 6;
          bool found1 = false;
          bool found2 = false;
          for (uint32_t j = 0; j < n_count2; ++j) {
            const uint32_t n_point_index = n_tile_data.points[j];
            if (n_point_index == point1) found1 = true;
            if (n_point_index == point2) found2 = true;
          }

          ASSERT(found1 && found2);
        }
  #endif

        const uint32_t tmp_index = (current_data.edge_index)%n_count;
        const uint32_t adjacent1 = tile_data.neighbours[(tmp_index+1)%n_count];
        const uint32_t adjacent2 = tile_data.neighbours[tmp_index == 0 ? n_count-1 : tmp_index-1];

  #ifndef _NDEBUG
        {
          const uint32_t k = (tmp_index+1)%n_count;
          const uint32_t k2 = k == 0 ? n_count-1 : k-1;
          const uint32_t k3 = (tmp_index+2)%n_count;
          const uint32_t k4 = k == 0 ? n_count-2 : (k == 1 ? n_count-1 : k-2);
          const uint32_t point1 = tile_data.points[k];
          const uint32_t point2 = tile_data.points[k2];
          const uint32_t point3 = tile_data.points[k3];
          const uint32_t point4 = tile_data.points[k4];
          ASSERT(point1 != UINT32_MAX);
          ASSERT(point2 != UINT32_MAX);
          ASSERT(point1 != point2);

          {
            bool found1 = false;
            bool found2 = false;
            bool found3 = false;
            bool found4 = false;

            const auto &n_tile_data = render::unpack_data(map->get_tile(adjacent1));
            const uint32_t n_count2 = render::is_pentagon(n_tile_data) ? 5 : 6;
            for (uint32_t j = 0; j < n_count2; ++j) {
              const uint32_t n_point_index = n_tile_data.points[j];
              if (n_point_index == point1) found1 = true;
              if (n_point_index == point2) found2 = true;
              if (n_point_index == point3) found3 = true;
              if (n_point_index == point4) found4 = true;
            }

            ASSERT(found1);
            ASSERT(found3);
            ASSERT(!found2);
            ASSERT(!found4);
          }

          {
            bool found1 = false;
            bool found2 = false;
            bool found3 = false;
            bool found4 = false;

            const auto &n_tile_data = render::unpack_data(map->get_tile(adjacent2));
            const uint32_t n_count2 = render::is_pentagon(n_tile_data) ? 5 : 6;
            for (uint32_t j = 0; j < n_count2; ++j) {
              const uint32_t n_point_index = n_tile_data.points[j];
              if (n_point_index == point1) found1 = true;
              if (n_point_index == point2) found2 = true;
              if (n_point_index == point3) found3 = true;
              if (n_point_index == point4) found4 = true;
            }

            ASSERT(!found1);
            ASSERT(!found3);
            ASSERT(found2);
            ASSERT(found4);
          }
        }
  #endif

        // каждый ход нам необходимо произвести небольшие вычисления
        // чтобы обновить границы по ходу игры
        // каждый кадр обновляю то что нужно нарисовать фрустум тест

        //advance_borders_count adv(map, i);
  //       if (i % 1000 == 0) {
  //         std::unique_lock<std::mutex> lock(map->mutex);
  //         global::get<render::tile_borders_optimizer>()->set_borders_count(i);
  //       }

        const uint32_t k = (tmp_index+1)%n_count;
        const uint32_t k2 = k == 0 ? n_count-1 : k-1;
        //const uint32_t k2 = (k+1)%n_count;
        const uint32_t point1_index = tile_data.points[k];
        const uint32_t point2_index = tile_data.points[k2];
        const glm::vec4 point1 = map->get_point(point1_index);
        const glm::vec4 point2 = map->get_point(point2_index);
        arr[i].points[0] = point2;
        arr[i].points[1] = point1;

        // нужно заполнить направления
        // нужно проверить принадлежат ли смежные тайлы к тем же государствам
        // тут же мы определяем тип границы (государственная, вассальная, граница провинции)

        const uint32_t province_index = ctx->get_tile(current_data.tile_index).province;
        const uint32_t opposite_province_index = ctx->get_tile(current_data.opposite_tile_index).province;

        const uint32_t adjacent1_province_index = ctx->get_tile(adjacent1).province;
        const uint32_t adjacent2_province_index = ctx->get_tile(adjacent2).province;

        if (province_index != adjacent1_province_index) {
          const glm::vec4 center = map->get_point(tile_data.center);
          arr[i].dirs[1] = glm::normalize(center - arr[i].points[1]);
        } else {
          const uint32_t k3 = (tmp_index+2)%n_count;
          const uint32_t point3_index = tile_data.points[k3];
          const glm::vec4 point3 = map->get_point(point3_index);
          arr[i].dirs[1] = glm::normalize(point3 - arr[i].points[1]);
        }

        if (province_index != adjacent2_province_index) {
          const glm::vec4 center = map->get_point(tile_data.center);
          arr[i].dirs[0] = glm::normalize(center - arr[i].points[0]);
        } else {
          const uint32_t k4 = k == 0 ? n_count-2 : (k == 1 ? n_count-1 : k-2);
          const uint32_t point4_index = tile_data.points[k4];
          const glm::vec4 point4 = map->get_point(point4_index);
          arr[i].dirs[0] = glm::normalize(point4 - arr[i].points[0]);
        }

        const auto province = ctx->get_entity<core::province>(province_index);
        const auto opposite_province = opposite_province_index == UINT32_MAX ? nullptr : ctx->get_entity<core::province>(opposite_province_index);
        ASSERT(province->title->owner != nullptr);

        arr[i].dirs[0].w = glm::uintBitsToFloat(current_data.tile_index);
        if (opposite_province == nullptr) {
          auto title = province->title->owner->main_title;
          ASSERT(type_index.find(title) != type_index.end());
          const uint32_t type_idx = type_index[title];
          arr[i].points[0].w = borders_size[0];
          arr[i].dirs[1].w = glm::uintBitsToFloat(type_idx); // или здесь другую границу?
          continue;
        }

        ASSERT(opposite_province->title->owner != nullptr);
        if (province->title->owner == opposite_province->title->owner) {
          // один владелец, мы должны нарисовать базовую границу
          arr[i].points[0].w = borders_size[2];
          arr[i].dirs[1].w = glm::uintBitsToFloat(0);
          continue;
        }

        std::unordered_set<core::faction*> factions;
        auto liege1 = province->title->owner;
        while (liege1 != nullptr) {
          factions.insert(liege1);
          liege1 = liege1->liege;
        }

        bool found = false;
        auto liege2 = opposite_province->title->owner;
        while (liege2 != nullptr) {
          found = found || factions.find(liege2) != factions.end();
          liege2 = liege2->liege;
        }

        if (found) {
          // эти провинции находятся в одном государстве
          auto title = province->title->owner->main_title;
          ASSERT(type_index.find(title) != type_index.end());
          const uint32_t type_idx = type_index[title];
          arr[i].points[0].w = borders_size[1];
          arr[i].dirs[1].w = glm::uintBitsToFloat(type_idx);

          continue;
        }

        // это граница разных государств
        auto title = province->title->owner->main_title;
        ASSERT(type_index.find(title) != type_index.end());
        const uint32_t type_idx = type_index[title];
        arr[i].points[0].w = borders_size[0];
        arr[i].dirs[1].w = glm::uintBitsToFloat(type_idx);

  //       const std::vector<uint32_t> prop_arr = {
  //         map::debug::properties::tile::country_index,
  //         map::debug::properties::tile::province_index
  //       };
  //
  //       for (const auto &prop : prop_arr) {
  //         // или лучше брать эти данные из провинции
  //         const uint32_t country_index = container->get_data<uint32_t>(map::debug::entities::tile, current_data.tile_index, prop);
  //         const uint32_t opposing_country_index = container->get_data<uint32_t>(map::debug::entities::tile, current_data.opposite_tile_index, prop);
  //
  //         if (country_index == opposing_country_index) continue;
  //
  //         const uint32_t adjacent1_index = container->get_data<uint32_t>(map::debug::entities::tile, adjacent1, prop);
  //         const uint32_t adjacent2_index = container->get_data<uint32_t>(map::debug::entities::tile, adjacent2, prop);
  //
  //         if (country_index != adjacent1_index) {
  //           const glm::vec4 center = map->get_point(tile_data.center);
  //           arr[i].dirs[1] = glm::normalize(center - arr[i].points[1]);
  //         } else {
  //           const uint32_t k3 = (tmp_index+2)%n_count;
  //           const uint32_t point3_index = tile_data.points[k3];
  //           const glm::vec4 point3 = map->get_point(point3_index);
  //           arr[i].dirs[1] = glm::normalize(point3 - arr[i].points[1]);
  //         }
  //
  //         if (country_index != adjacent2_index) {
  //           const glm::vec4 center = map->get_point(tile_data.center);
  //           arr[i].dirs[0] = glm::normalize(center - arr[i].points[0]);
  //         } else {
  //           const uint32_t k4 = k == 0 ? n_count-2 : (k == 1 ? n_count-1 : k-2);
  //           const uint32_t point4_index = tile_data.points[k4];
  //           const glm::vec4 point4 = map->get_point(point4_index);
  //           arr[i].dirs[0] = glm::normalize(point4 - arr[i].points[0]);
  //         }
  //
  //         if (prop == map::debug::properties::tile::country_index) {
  //           types[country_index+1] = {render::make_color(0.0f, 0.0f, 0.0f, 1.0f), UINT32_MAX, render::make_color(0.0f, 0.0f, 0.0f, 1.0f), 0.3f}; // это мы заполняем для каждого титула
  //           arr[i].dirs[1].w = glm::uintBitsToFloat(country_index+1);
  //         }
  //
  //         if (prop == map::debug::properties::tile::province_index) {
  //           types[0] = {render::make_color(0.0f, 0.0f, 0.0f, 1.0f), UINT32_MAX, render::make_color(0.0f, 0.0f, 0.0f, 1.0f), 0.15f};
  //           arr[i].dirs[1].w = glm::uintBitsToFloat(0);
  //         }
  //
  //         break;
  //       }
  //
  //       arr[i].dirs[0].w = glm::uintBitsToFloat(current_data.tile_index);
        //arr[i].dirs[1].w = glm::uintBitsToFloat(border_type);

        // по идее все что мне нужно теперь делать каждый ход это:
        // обновить тип границы, обновить направления
        // и все
        // каждый кадр запихиваем три координаты в фрустум тест
      }
      });
      utils::async_wait(pool);

  //     yavf::Buffer* types_buffer = global::get<render::buffers>()->border_types;
  //     types_buffer->resize(types.size() * sizeof(border_type2));
  //     auto ptr = types_buffer->ptr();
  //     ASSERT(ptr != nullptr);
  //     memcpy(ptr, types.data(), types.size() * sizeof(border_type2));
      std::unique_lock<std::mutex> lock(map->mutex);
      //global::get<render::tile_borders_optimizer>()->set_borders_count(borders.size());
      global::get<render::tile_optimizer>()->set_borders_count(borders.size());
    }

//     const uint32_t layers_count = 10;
//     const float mountain_height = 0.5f;
//     const float render_tile_height = 10.0f;
//     const float layer_height = mountain_height / float(layers_count);
    void generate_tile_connections(core::map* map, dt::thread_pool* pool) {
      struct wall {
        uint32_t tile1;
        uint32_t tile2;
        uint32_t point1;
        uint32_t point2;
      };

      std::vector<wall> walls;
      std::mutex mutex;

      utils::submit_works_async(pool, map->tiles_count(), [map, &walls, &mutex] (const size_t &start, const size_t &count) {
        for (size_t i = start; i < start+count; ++i) {
          const uint32_t tile_index = i;
          const auto &tile_data = render::unpack_data(map->get_tile(tile_index));
          const uint32_t n_count = render::is_pentagon(tile_data) ? 5 : 6;
          const float tile_height = tile_data.height;
          const uint32_t height_layer = render::compute_height_layer(tile_height);
          if (height_layer == 0) continue;

          uint32_t size = 0;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            const auto &n_tile_data = render::unpack_data(map->get_tile(n_index));
            const float n_tile_height = n_tile_data.height;
            const uint32_t n_height_layer = render::compute_height_layer(n_tile_height);
            if (height_layer <= n_height_layer) continue;
            ++size;
          }

          if (size == 0) continue;

          wall tmp_arr[size];
          uint32_t counter = 0;
          for (uint32_t j = 0; j < n_count; ++j) {
            const uint32_t n_index = tile_data.neighbours[j];
            const auto &n_tile_data = render::unpack_data(map->get_tile(n_index));
            const float n_tile_height = n_tile_data.height;
            const uint32_t n_height_layer = render::compute_height_layer(n_tile_height);
            if (height_layer <= n_height_layer) continue;

            // добавляем стенку
            // нам нужны две точки и индексы тайлов

            const uint32_t point1 = tile_data.points[j];
            const uint32_t point2 = tile_data.points[(j+1)%n_count];

  #ifndef _NDEBUG
            {
              std::unordered_set<uint32_t> tmp;
              tmp.insert(point1);
              tmp.insert(point2);

              uint32_t found = 0;
              const uint32_t n_n_count = render::is_pentagon(n_tile_data) ? 5 : 6;
              for (uint32_t k = 0; k < n_n_count; ++k) {
                const uint32_t point_index = n_tile_data.points[k];
                found += uint32_t(tmp.find(point_index) != tmp.end());

  //               if (point1 == point_index) {
  //                 const bool a = (n_tile_data.points[(k+1)%n_n_count] == point2 || n_tile_data.points[k == 0 ? n_n_count-1 : k-1] == point2);
  //                 if (a) found = true;
  //               }
              }

              ASSERT(found == 2);
            }
  #endif

//             std::unique_lock<std::mutex> lock(mutex);
//             walls.push_back({tile_index, n_index, point1, point2});
            tmp_arr[counter] = {tile_index, n_index, point1, point2};
            ++counter;
          }

          uint32_t offset = UINT32_MAX;
          {
            std::unique_lock<std::mutex> lock(mutex);
            offset = walls.size();
            for (uint32_t i = 0; i < size; ++i) {
              walls.push_back(tmp_arr[i]);
            }
          }

          map->set_tile_connections_data(tile_index, offset, size);
        }
      });
      utils::async_wait(pool); // текущий тред не должен быть главным!!!

      std::unique_lock<std::mutex> lock(map->mutex);
      auto connections = global::get<systems::map_t>()->world_buffers->tiles_connections;
      connections->resize(walls.size() * sizeof(walls[0]));
      auto ptr = connections->ptr();
      memcpy(ptr, walls.data(), walls.size() * sizeof(walls[0]));
      //ASSERT(global::get<render::tile_walls_optimizer>() != nullptr);
      //global::get<render::tile_walls_optimizer>()->set_connections_count(walls.size());
      ASSERT(global::get<render::tile_optimizer>() != nullptr);
      global::get<render::tile_optimizer>()->set_connections_count(walls.size());
    }

    void connect_game_data(core::map* map, core::context* ctx) {
      {
        const size_t count = ctx->get_entity_count<core::city>();
        for (size_t i = 0; i < count; ++i) {
          auto city = ctx->get_entity<core::city>(i);
          ASSERT(city->title != nullptr);
          ASSERT(city->title->count == 0);
          city->title->create_children(1);
          city->title->set_city(city);
          ASSERT(city->province != nullptr);
          ASSERT(city->province->cities_count < core::province::cities_max_game_count);
          city->province->cities[city->province->cities_count] = city;
          ++city->province->cities_count;
        }
      }

      {
        const size_t count = ctx->get_entity_count<core::province>();
        for (size_t i = 0; i < count; ++i) {
          auto province = ctx->get_entity<core::province>(i);
          ASSERT(province->cities_count <= province->cities_max_count);
          ASSERT(province->title != nullptr);
          ASSERT(province->title->count == 0);
          province->title->create_children(1);
          province->title->set_province(province);
          ASSERT(!province->tiles.empty());
          for (const auto &tile_index : province->tiles) {
            core::tile t;
            t.height = map->get_tile_height(tile_index);
            t.province = i;
            ctx->set_tile(tile_index, t);
          }
        }
      }

      {
        const size_t count = ctx->get_entity_count<core::titulus>();
        std::unordered_map<core::titulus*, std::vector<core::titulus*>> childs;
        for (size_t i = 0; i < count; ++i) {
          auto title = ctx->get_entity<core::titulus>(i);
          if (title->parent != nullptr) childs[title->parent].push_back(title);
        }

        for (size_t i = 0; i < count; ++i) {
          auto title = ctx->get_entity<core::titulus>(i);
          auto itr = childs.find(title);
          if (itr == childs.end()) continue;
          const auto &childs_arr = itr->second;
          ASSERT(title->childs == nullptr);
          title->create_children(childs_arr.size());
          if (childs_arr.size() == 1) {
            title->set_child(0, childs_arr[0]);
            continue;
          }

          for (size_t j = 0; j < childs_arr.size(); ++j) {
            title->set_child(j, childs_arr[j]);
          }
        }
      }

      {
        const size_t count = ctx->characters_count();
        std::unordered_map<core::faction*, std::vector<core::faction*>> vassals;
        std::unordered_map<core::faction*, std::vector<core::character*>> prisoners;
        std::unordered_map<core::character*, std::vector<core::character*>> court;
        std::unordered_map<core::character*, std::vector<core::character*>> concubines;
        std::unordered_map<core::character*, std::vector<core::character*>> children;
        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          if (character->suzerain != nullptr) court[character->suzerain].push_back(character);
          if (character->imprisoner != nullptr) prisoners[character->imprisoner].push_back(character);
          if (character->factions[core::character::self] != nullptr && character->factions[core::character::self]->liege != nullptr) {
            vassals[character->factions[core::character::self]->liege].push_back(character->factions[core::character::self]);
          }
          if (character->family.owner != nullptr) concubines[character->family.owner].push_back(character);
          // братья сестры, нужно выбрать кого то из родителей и положить туда? как быть с полуродственниками?
          // скорее всего несколько супругов может быть только у правителей
          // предыдущих супругов я кажется пока не задаю, тогда нужно найти правителя
          ASSERT(character->family.previous_consorts == nullptr);
          const bool has_parent1 = character->family.parents[0] != nullptr;
          const bool has_parent2 = character->family.parents[1] != nullptr;
          core::character* parent = nullptr;
          if (has_parent1 || has_parent2) {
            if (parent == nullptr) parent = has_parent1 && has_parent2 && character->family.parents[0]->factions[core::character::self] != nullptr ? character->family.parents[0] : nullptr;
            if (parent == nullptr) parent = has_parent1 && has_parent2 && character->family.parents[1]->factions[core::character::self] != nullptr ? character->family.parents[1] : nullptr;
            if (parent == nullptr) parent = has_parent1 ? character->family.parents[0] : nullptr;
            if (parent == nullptr) parent = has_parent2 ? character->family.parents[1] : nullptr;
          }

          if (parent != nullptr) children[parent].push_back(character);

          for (size_t j = 0; j < core::character::relations::max_game_friends; ++j) {
            auto char_friend = character->relations.friends[j];
            if (char_friend == nullptr) continue;
            size_t counter = 0;
            for (size_t k = 0; k < core::character::relations::max_game_friends; ++k) {
              auto char_char_friend = char_friend->relations.friends[j];
              counter += size_t(char_char_friend != nullptr) + 500 * size_t(char_char_friend == character);
              if (char_char_friend == character) break;
            }

            if (counter > 499) continue;
            ASSERT(counter < core::character::relations::max_game_friends);
            char_friend->relations.friends[counter] = character;
          }

          for (size_t j = 0; j < core::character::relations::max_game_friends; ++j) {
            auto char_rival = character->relations.rivals[j];
            if (char_rival == nullptr) continue;
            size_t counter = 0;
            for (size_t k = 0; k < core::character::relations::max_game_rivals; ++k) {
              auto char_char_rival = char_rival->relations.rivals[j];
              counter += size_t(char_char_rival != nullptr) + 500 * size_t(char_char_rival == character);
              if (char_char_rival == character) break;
            }

            if (counter > 499) continue;
            ASSERT(counter < core::character::relations::max_game_rivals);
            char_rival->relations.rivals[counter] = character;
          }

          for (size_t j = 0; j < core::character::relations::max_game_lovers; ++j) {
            auto char_lover = character->relations.lovers[j];
            if (char_lover == nullptr) continue;
            size_t counter = 0;
            for (size_t k = 0; k < core::character::relations::max_game_lovers; ++k) {
              auto char_char_lovar = char_lover->relations.lovers[j];
              counter += size_t(char_char_lovar != nullptr) + 500 * size_t(char_char_lovar == character);
              if (char_char_lovar == character) break;
            }

            if (counter > 499) continue;
            ASSERT(counter < core::character::relations::max_game_lovers);
            char_lover->relations.friends[counter] = character;
          }

          if (character->family.consort != nullptr) {
            auto consort = character->family.consort;
            ASSERT(consort->family.consort == nullptr || consort->family.consort == character);
            consort->family.consort = character;
          }
        }

        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          // у персонажа может быть другая форма правления (но наверное в этом случае персонажи не будут считаться за вассалов)
          auto faction = character->factions[core::character::self];
          if (faction == nullptr) continue;
          auto itr = vassals.find(faction);
          if (itr == vassals.end()) continue;
          const auto &vs = itr->second;
          for (size_t j = 0; j < vs.size(); ++j) {
            faction->add_vassal_raw(vs[j]);
          }
        }

        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          auto faction = character->factions[core::character::self];
          if (faction == nullptr) continue;
          auto itr = prisoners.find(faction);
          if (itr == prisoners.end()) continue;
          const auto &ps = itr->second;
          for (size_t j = 0; j < ps.size(); ++j) {
            faction->add_prisoner_raw(character);
          }
        }

        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          auto itr = court.find(character);
          if (itr == court.end()) continue;
          const auto &cs = itr->second;
          for (size_t j = 0; j < cs.size(); ++j) {
            character->add_courtier_raw(cs[j]);
          }
        }

        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          auto itr = concubines.find(character);
          if (itr == concubines.end()) continue;
          const auto &cs = itr->second;
          for (size_t j = 0; j < cs.size(); ++j) {
            ASSERT(character->is_male() != cs[j]->is_male());
            character->add_concubine_raw(cs[j]);
          }
        }

        for (size_t i = 0; i < count; ++i) {
          auto character = ctx->get_character(i);
          auto itr = children.find(character);
          if (itr == children.end()) continue;
          const auto &cs = itr->second;
          for (size_t j = 0; j < cs.size(); ++j) {
            character->add_child_raw(cs[j]);
          }

          if (character->family.consort != nullptr) {
            ASSERT(character->family.consort->family.children == nullptr);
            character->family.consort->family.children = character->family.children;
          }
        }
      }
    }

    template <typename T>
    void create_entities_without_id(core::context* ctx, utils::table_container* tables) {
      const auto &data = tables->get_tables(T::s_type);
      ctx->create_container<T>(data.size());

      PRINT_VAR("cities count", data.size())
    }

    template <typename T>
    void create_entities(core::context* ctx, utils::table_container* tables) {
      // это заполнить в валидации не выйдет (потому что string_view)
      // в провинции нет id
      auto to_data = global::get<utils::data_string_container>();

      const auto &data = tables->get_tables(T::s_type);
      ctx->create_container<T>(data.size());
      for (size_t i = 0; i < data.size(); ++i) {
        auto ptr = ctx->get_entity<T>(i);
        ptr->id = data[i]["id"];
        const size_t index = to_data->get(ptr->id);
        if (index != SIZE_MAX) throw std::runtime_error("Found duplicate id " + ptr->id + " while parsing " + std::string(magic_enum::enum_name<core::structure>(T::s_type)) + " type data");
        to_data->insert(ptr->id, i);
      }
    }

    void create_characters(core::context* ctx, utils::table_container* tables) {

      const auto &data = tables->get_tables(core::structure::character);
      ASSERT(!data.empty());
      for (const auto &table : data) {
        bool male = true;
        bool dead = false;

        if (const auto &proxy = table["male"]; proxy.valid()) {
          male = proxy.get<bool>();
        }

        if (const auto &proxy = table["dead"]; proxy.valid()) {
          dead = proxy.get<bool>();
        }

        auto c = ctx->create_character(male, dead);

        // нам нужно заполнить стейт рандомайзера
        // как это лучше всего сделать?
        // советуют использовать splitmix64
        // но с чем его использовать не говорят
        // мне нужно придумать как получить сид
        // по идее нужно задать некий игровой сид и от него отталкиваться
        // с другой стороны у меня локальные сиды на всех персах
        // да но они не будут меняться если я начинаю на тойже карте по нескольку раз
        // поэтому нужно придумать стейт глобальный
        const size_t state1 = global::advance_state();
        const size_t state2 = global::advance_state();
        c->rng_state = {state1, state2};
      }
    }

    template <typename T>
    void parse_entities(core::context* ctx, utils::table_container* tables, const std::function<void(T*, const sol::table&)> &parsing_func) {
      const auto &data = tables->get_tables(T::s_type);
      for (size_t i = 0; i < data.size(); ++i) {
        auto ptr = ctx->get_entity<T>(i);
        parsing_func(ptr, data[i]);
      }
    }

    void parse_characters(core::context* ctx, utils::table_container* tables, const std::function<void(core::character*, const sol::table&)> &parsing_func) {
      const auto &data = tables->get_tables(core::character::s_type);
      for (size_t i = 0; i < data.size(); ++i) {
        auto ptr = ctx->get_character(i);
        parsing_func(ptr, data[i]);
      }
    }

    void validate_and_create_data(systems::map_t* map_systems, utils::progress_container* prog) { //systems::core_t* systems,
      auto ctx = map_systems->core_context;
      auto creator = map_systems->map_creator;
      auto tables = &creator->table_container();
      utils::data_string_container string_container;
      global::get(&string_container);

      advance_progress(prog, "validating data"); // 1

      //if ()

      utils::world_serializator cont;
  //     const std::string_view test_name = "Test world 1\0";
  //     const std::string_view test_tname = "test_world_1\0";
  //     const std::string_view test_settings = "{}\0";
      cont.set_name(creator->get_world_name());
      cont.set_technical_name(creator->get_folder_name());
      cont.set_settings(creator->get_settings());
      cont.set_rand_seed(creator->get_rand_seed());
      cont.set_noise_seed(creator->get_noise_seed());

      const std::function<bool(const size_t &, sol::this_state, const sol::table&, utils::world_serializator*)> validation_funcs[] = {
        nullptr,                   // tile    : нужна ли тайлу валидация? я не уверен что хорошей идеей будет использовать луа таблицы для заполнения тайла
        utils::validate_province_and_save,  // province
        utils::validate_building_and_save,  // building_type,
        utils::validate_city_type_and_save, // city_type,
        utils::validate_city_and_save,      // city,
        nullptr,                   // trait,
        nullptr,                   // modificator,
        nullptr,                   // troop_type,
        nullptr,                   // decision,
        nullptr,                   // religion_group,
        nullptr,                   // religion,
        nullptr,                   // culture,
        nullptr,                   // law,
        nullptr,                   // event,
        utils::validate_title_and_save,     // titulus,
        utils::validate_character_and_save, // character,
        nullptr,                   // dynasty,
        nullptr,                   // faction,    // это и далее делать не нужно по идее
        nullptr,                   // hero_troop,
        nullptr,                   // army,

      };

      global::get<utils::calendar>()->validate();

      auto &lua = creator->state();
      //ASSERT(lua != nullptr);

  //     auto &tables = creator->table_container();
      const size_t count = static_cast<size_t>(core::structure::count);
      bool ret = true;
      for (size_t i = 0; i < count; ++i) {
        if (!validation_funcs[i]) continue;
        const auto &data = tables->get_tables(static_cast<core::structure>(i));
        size_t counter = 0;
        for (const auto &table : data) {
          ret = ret && validation_funcs[i](counter, lua.lua_state(), table, &cont);
          ++counter;
        }
      }

      if (!ret) throw std::runtime_error("There is validation errors");

      advance_progress(prog, "allocating memory"); // 2

      // нужно собрать инфу о дубликатах
      create_entities_without_id<core::province>(ctx, tables);
      create_entities<core::building_type>(ctx, tables);
      create_entities<core::city_type>(ctx, tables);
      create_entities_without_id<core::city>(ctx, tables);
      create_entities<core::titulus>(ctx, tables);
      create_characters(ctx, tables);

      advance_progress(prog, "creating entities"); // 3

      global::get(ctx);

      parse_entities<core::titulus>(ctx, tables, utils::parse_title);
      parse_entities<core::province>(ctx, tables, utils::parse_province);
      parse_entities<core::building_type>(ctx, tables, utils::parse_building);
      parse_entities<core::city_type>(ctx, tables, utils::parse_city_type);
      parse_entities<core::city>(ctx, tables, utils::parse_city);
      parse_characters(ctx, tables, utils::parse_character);
      parse_characters(ctx, tables, utils::parse_character_goverment);
      // тут нужно еще соединить все полученные данные друг с другом
      connect_game_data(map_systems->map, ctx);

      // по идее в этой точке все игровые объекты созданы
      // и можно непосредственно переходить к геймплею
      // если валидация и парсинг успешны это повод сохранить мир на диск
      // это означает: сериализация данных карты + записать на диск все таблицы + сериализация персонажей и династий (первых)
      // могу ли я сериализовать конкретные типы? скорее да чем нет,
      // но при этом мне придется делать отдельный сериализатор для каждого типа
      // понятное дело делать отдельный сериализатор не сруки

      advance_progress(prog, "serializing world"); // 4

      cont.copy_seasons(map_systems->seasons);
      cont.set_world_matrix(map_systems->map->world_matrix);

      for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
        const auto &d = ctx->get_tile(i);
        cont.set_tile_data(i, {d.height, d.province});
      }

      cont.serialize(); //

      map_systems->world_name = cont.get_name();
      map_systems->folder_name = cont.get_technical_name();
      map_systems->generator_settings = cont.get_settings();
      memcpy(map_systems->hash, cont.get_hash(), systems::map_t::hash_size);

      advance_progress(prog, "checking world"); // 5

      utils::world_serializator test;
      test.deserialize(global::root_directory() + "saves/" + creator->get_folder_name() + "/world_data");

      ASSERT(test.get_name() == creator->get_world_name());
      ASSERT(test.get_technical_name() == creator->get_folder_name());
      ASSERT(test.get_settings() == creator->get_settings());
      ASSERT(test.get_rand_seed() == creator->get_rand_seed());
      ASSERT(test.get_noise_seed() == creator->get_noise_seed());

      ASSERT(tables->get_tables(core::structure::province).size() == cont.get_data_count(core::structure::province));
      ASSERT(tables->get_tables(core::structure::city_type).size() == cont.get_data_count(core::structure::city_type));
      ASSERT(tables->get_tables(core::structure::city).size() == cont.get_data_count(core::structure::city));
      ASSERT(tables->get_tables(core::structure::building_type).size() == cont.get_data_count(core::structure::building_type));
      ASSERT(tables->get_tables(core::structure::titulus).size() == cont.get_data_count(core::structure::titulus));
      ASSERT(tables->get_tables(core::structure::character).size() == cont.get_data_count(core::structure::character));

  //     PRINT_VAR("size", tables->get_tables(core::structure::province).size())
  //     PRINT_VAR("size", tables->get_tables(core::structure::city_type).size())
  //     PRINT_VAR("size", tables->get_tables(core::structure::city).size())
  //     PRINT_VAR("size", tables->get_tables(core::structure::building_type).size())
  //     PRINT_VAR("size", tables->get_tables(core::structure::titulus).size())
  //     PRINT_VAR("size", tables->get_tables(core::structure::character).size())
  //
  //     PRINT_VAR("size", cont.get_data_count(core::structure::province))
  //     PRINT_VAR("size", cont.get_data_count(core::structure::city_type))
  //     PRINT_VAR("size", cont.get_data_count(core::structure::city))
  //     PRINT_VAR("size", cont.get_data_count(core::structure::building_type))
  //     PRINT_VAR("size", cont.get_data_count(core::structure::titulus))
  //     PRINT_VAR("size", cont.get_data_count(core::structure::character))

      ASSERT(cont.get_data_count(core::structure::province) == test.get_data_count(core::structure::province));
      ASSERT(cont.get_data_count(core::structure::city_type) == test.get_data_count(core::structure::city_type));
      ASSERT(cont.get_data_count(core::structure::city) == test.get_data_count(core::structure::city));
      ASSERT(cont.get_data_count(core::structure::building_type) == test.get_data_count(core::structure::building_type));
      ASSERT(cont.get_data_count(core::structure::titulus) == test.get_data_count(core::structure::titulus));
      ASSERT(cont.get_data_count(core::structure::character) == test.get_data_count(core::structure::character));

      //PRINT(cont.get_data(var, i))
  #define CHECK_CONTENT(var) for (uint32_t i = 0; i < cont.get_data_count(var); ++i) { ASSERT(cont.get_data(var, i) == test.get_data(var, i)); }
      CHECK_CONTENT(core::structure::province)
      CHECK_CONTENT(core::structure::city_type)
      CHECK_CONTENT(core::structure::city)
      CHECK_CONTENT(core::structure::building_type)
      CHECK_CONTENT(core::structure::titulus)
      CHECK_CONTENT(core::structure::character)

      // кажется все правильно сериализуется и десериализуется
      // сохранения должны храниться в папках с файлом world_data
      // название папки - техническое название мира,
      // техническое название мира - нужно зафорсить пользователя задать валидное техническое название
      // (какнибудь бы упростить для пользователя это дело)
      // что самое главное я могу оставить пока так как есть и это покроет почти всю сериализацию
      // остается решить вопрос с климатом, хотя че тут решать: нужно выделить
      // 64*500к или 128*500к памяти чтобы сохранить сезоны, сезонов по идее не имеет смысла делать больше чем размер массива на каждый тайл
      // соответственно 64 сезона на каждый тайл, означает ли это что мы можем сохранить 4 тайла в одном чаре?
      // в сохранениях видимо придется делать протомессадж для каждой сущности которую необходимо сохранить
      // мне нужно еще придумать стендалоне сохранения: то есть сохранения в которых записаны данные мира дополнительно
      //

      global::get(reinterpret_cast<utils::data_string_container*>(SIZE_MAX));
      global::get(reinterpret_cast<core::context*>(SIZE_MAX));
      map_systems->render_modes->at(render::modes::biome)();
    }

  //   void create_interface(system_container_t &systems) {
  //     systems.interface = systems.container.create<utils::interface>();
  //     global::get(systems.interface);
  //     systems.interface->init_constants();
  //     systems.interface->init_input();
  //   }

    void post_generation_work(systems::map_t* map_systems, systems::core_t* systems, utils::progress_container* prog) {
      UNUSED_VARIABLE(systems);
      prog->set_hint1(std::string_view("Load map"));
      prog->set_max_value(8);
      prog->set_hint2(std::string_view("starting"));

      auto ctx = map_systems->core_context;

  //     create_game_state();
      validate_and_create_data(map_systems, prog); // создаем объекты
      advance_progress(prog, "connecting tiles");
      generate_tile_connections(map_systems->map, global::get<dt::thread_pool>());
      advance_progress(prog, "making borders");
      find_border_points(map_systems->map, ctx); // после генерации нужно сделать много вещей
      // по идее создать границы нужно сейчас, так как в титулах появились данные о цвете

      advance_progress(prog, "ending");

      //create_interface(systems);
  //     systems.interface->init_types();
  //     systems.interface->init_game_logic();
  //     auto &lua = systems.interface->get_state();
  //     load_interface_functions(systems.interface, lua);
  //     create_ai_systems(systems);
      // нужно выбрать себе какого нибудь персонажа
      // кажется у меня сейчас все персонажи живы, так что можно любого
      const size_t chars_count = ctx->characters_count();
      utils::rng::state s = {67586, 987699695};
      s = utils::rng::next(s);
      const double val = utils::rng::normalize(utils::rng::value(s));
      const size_t index = chars_count * val;
      auto c = ctx->get_character(index);
      c->make_player();
      game::update_player(c);
    }

    void load_map_data(core::map* map, utils::world_serializator* world) {
      glm::mat4 mat1 = world->get_world_matrix();
  //     PRINT_VEC4("mat 0", mat1[0])
  //     PRINT_VEC4("mat 1", mat1[1])
  //     PRINT_VEC4("mat 2", mat1[2])
  //     PRINT_VEC4("mat 3", mat1[3])
      map::container generated_core(core::map::world_radius, core::map::detail_level, glm::mat3(mat1)); // возможно нужно как то это ускорить
  //     ASSERT(false);

      ASSERT(generated_core.points.size() == map->points_count());
      ASSERT(generated_core.tiles.size() == map->tiles_count());
      ASSERT(generated_core.triangles.size() == map->triangles_count());

      map->world_matrix = mat1;
      auto pool = global::get<dt::thread_pool>();

      // придется переделать функции и добавить ожидание треду
      utils::submit_works_async(pool, generated_core.tiles.size(), [&generated_core] (const size_t &start, const size_t &count) {
  //       size_t start = 0;
  //       size_t count = generated_core.tiles.size();
        for (size_t i = start; i < start+count; ++i) {
          generated_core.fix_tile(i);
        }
      });
      utils::async_wait(pool);

      utils::submit_works_async(pool, generated_core.tiles.size(), [&generated_core, map] (const size_t &start, const size_t &count) {
        for (size_t i = start; i < start+count; ++i) {
          map->set_tile_data(&generated_core.tiles[i], i);
        }
      });
      utils::async_wait(pool);

      utils::submit_works_async(pool, generated_core.points.size(), [&generated_core, map] (const size_t &start, const size_t &count) {
        for (size_t i = start; i < start+count; ++i) {
          map->set_point_data(generated_core.points[i], i);
        }
      });
      utils::async_wait(pool);

      const size_t tri_count = core::map::tri_count_d(core::map::accel_struct_detail_level);
      ASSERT(tri_count == map->accel_triangles_count());
  //     const size_t hex_count = map::hex_count_d(detail_level);
      std::mutex mutex;
      std::unordered_set<uint32_t> unique_tiles;
      std::atomic<uint32_t> tiles_counter(0);

      utils::submit_works_async(pool, tri_count, [&mutex, &unique_tiles, &generated_core, &tiles_counter, map] (const size_t &start, const size_t &count) {
        std::vector<uint32_t> tiles_array;
        size_t offset = 0;
        for (size_t i = 0; i < core::map::accel_struct_detail_level; ++i) {
          offset += core::map::tri_count_d(i);
        }

        for (size_t i = start; i < start+count; ++i) {
          const size_t tri_index = i + offset;
          const auto &tri = generated_core.triangles[tri_index];
          ASSERT(tri.current_level == core::map::accel_struct_detail_level);

          map::map_triangle_add2(&generated_core, tri_index, mutex, unique_tiles, tiles_array);

          uint32_t counter = 0;
          for (int64_t i = tiles_array.size()-1; i > -1 ; --i) {
            const uint32_t tile_index = tiles_array[i];
            if (generated_core.tiles[tile_index].is_pentagon()) {
              ++counter;
              ASSERT(counter < 2);
              std::swap(tiles_array[i], tiles_array.back());
            }
          }

          const uint32_t offset = tiles_counter.fetch_add(tiles_array.size());
          ASSERT(offset + tiles_array.size() <= generated_core.tiles.size());
          map->set_tile_indices(i, tri.points, tiles_array, offset, tiles_array.size(), counter != 0);

          tiles_array.clear();
        }
      });
      utils::async_wait(pool); // похоже что работает

      ASSERT(pool->working_count() == 1 && pool->tasks_count() == 0);

      ASSERT(generated_core.triangles.size() == map->triangles.size());
      ASSERT(sizeof(core::map::triangle) == sizeof(map::triangle));
      {
        std::unique_lock<std::mutex> lock(map->mutex);
        memcpy(map->triangles.data(), generated_core.triangles.data(), map->triangles.size()*sizeof(core::map::triangle));
      }

      map->flush_data();

  //       ctx->container->set_entity_count(debug::entities::tile, map->tiles_count());
      map->set_status(core::map::status::valid);

      for (size_t i = 0; i < core::map::hex_count_d(core::map::detail_level); ++i) {
        map->set_tile_height(i, world->get_tile_data(i).height);
      }
    }

    template <typename T>
    void create_entity_without_id(core::context* ctx, utils::world_serializator* world) {
      ctx->create_container<T>(world->get_data_count(T::s_type));
    }

    template <typename T>
    void create_entity(core::context* ctx, utils::world_serializator* world, sol::state_view tmp_state, utils::data_string_container* to_data) {
      ctx->create_container<T>(world->get_data_count(T::s_type));
      for (size_t i = 0; i < world->get_data_count(T::s_type); ++i) {
        auto ret = tmp_state.script("return " + std::string(world->get_data(T::s_type, i)));
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load entity table");
        }
        sol::table t = ret;
        auto proxy = t["id"];
        ASSERT(proxy.valid());

        std::string_view str = proxy.get<std::string_view>();
        auto ptr = ctx->get_entity<T>(i);
        ptr->id = str;
        to_data->insert(ptr->id, i);
      }
    }

    void create_characters(core::context* ctx, utils::world_serializator* world, sol::state_view tmp_state) {
      const size_t count = world->get_data_count(core::character::s_type);
      for (size_t i = 0; i < count; ++i) {
        auto ret = tmp_state.script("return " + std::string(world->get_data(core::character::s_type, i)));
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load entity table");
        }
        sol::table t = ret;

        bool male = true;
        bool dead = false;

        if (const auto &proxy = t["male"]; proxy.valid()) {
          male = proxy.get<bool>();
        }

        if (const auto &proxy = t["dead"]; proxy.valid()) {
          dead = proxy.get<bool>();
        }

        auto c = ctx->create_character(male, dead);

        // нам нужно заполнить стейт рандомайзера
        // нужно проконтролировать какой глобальный стейт мы используем
        const size_t state1 = global::advance_state();
        const size_t state2 = global::advance_state();
        c->rng_state = {state1, state2};
      }
    }

    template <typename T>
    void parse_entities(core::context* ctx, utils::world_serializator* world, sol::state_view tmp_state, const std::function<void(T*, const sol::table&)> &parsing_func) {
      const size_t count = world->get_data_count(T::s_type);
      for (size_t i = 0; i < count; ++i) {
        auto ret = tmp_state.script("return " + std::string(world->get_data(T::s_type, i)));
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load entity table");
        }
        sol::table t = ret;
        auto ptr = ctx->get_entity<T>(i);

        parsing_func(ptr, t);
      }
    }

    void parse_character(core::context* ctx, utils::world_serializator* world, sol::state_view tmp_state, const std::function<void(core::character*, const sol::table&)> &parsing_func) {
      const size_t count = world->get_data_count(core::character::s_type);
      for (size_t i = 0; i < count; ++i) {
        auto ret = tmp_state.script("return " + std::string(world->get_data(core::character::s_type, i)));
        if (!ret.valid()) {
          sol::error err = ret;
          std::cout << err.what();
          throw std::runtime_error("Could not load entity table");
        }
        sol::table t = ret;
        auto ptr = ctx->get_character(i);

        parsing_func(ptr, t);
      }
    }

    void loading_world(systems::map_t* map_systems, utils::progress_container* prog, const std::string &world_path) {
      ASSERT(!world_path.empty());
      advance_progress(prog, "deserialization");
      utils::world_serializator world_data;
      world_data.deserialize(world_path);
      utils::data_string_container to_data;
      global::get(&to_data);
      global::get(map_systems->core_context);

      map_systems->world_name = world_data.get_name();
      map_systems->folder_name = world_data.get_technical_name();
      map_systems->generator_settings = world_data.get_settings();
      static_assert(systems::map_t::hash_size == utils::world_serializator::hash_size);
      memcpy(map_systems->hash, world_data.get_hash(), systems::map_t::hash_size);
      // и что собственно теперь? мы должны парсить таблицы и заполнять данные в типах
      // create, parse, connect, end

      map_systems->map->world_matrix = world_data.get_world_matrix();

      advance_progress(prog, "creating earth");
      // нужно создать непосредственно core map
      load_map_data(map_systems->map, &world_data);

      advance_progress(prog, "creating entities");
      sol::state lua;
      create_entity_without_id<core::province>(map_systems->core_context, &world_data);
      create_entity<core::building_type>(map_systems->core_context, &world_data, lua, &to_data);
      create_entity<core::city_type>(map_systems->core_context, &world_data, lua, &to_data);
      create_entity_without_id<core::city>(map_systems->core_context, &world_data);
      create_entity<core::titulus>(map_systems->core_context, &world_data, lua, &to_data);
  //     map_systems->core_context->create_container<core::province>(world_data.get_data_count(core::structure::province));
  //     map_systems->core_context->create_container<core::building_type>(world_data.get_data_count(core::structure::building_type));
  //     map_systems->core_context->create_container<core::city_type>(world_data.get_data_count(core::structure::city_type));
  //     map_systems->core_context->create_container<core::city>(world_data.get_data_count(core::structure::city));
  //     map_systems->core_context->create_container<core::titulus>(world_data.get_data_count(core::structure::titulus));
      //map_systems->core_context->create_container<core::character>(world_data.get_data_count(core::structure::character));
      create_characters(map_systems->core_context, &world_data, lua);

      advance_progress(prog, "parsing entities");
      parse_entities<core::titulus>(map_systems->core_context, &world_data, lua, utils::parse_title);
      parse_entities<core::province>(map_systems->core_context, &world_data, lua, utils::parse_province);
      parse_entities<core::building_type>(map_systems->core_context, &world_data, lua, utils::parse_building);
      parse_entities<core::city_type>(map_systems->core_context, &world_data, lua, utils::parse_city_type);
      parse_entities<core::city>(map_systems->core_context, &world_data, lua, utils::parse_city);
      parse_character(map_systems->core_context, &world_data, lua, utils::parse_character);
      parse_character(map_systems->core_context, &world_data, lua, utils::parse_character_goverment);
      // тут нужно еще соединить все полученные данные друг с другом
      advance_progress(prog, "connecting game data");
      connect_game_data(map_systems->map, map_systems->core_context);
      world_data.fill_seasons(map_systems->seasons);

      map_systems->render_modes->at(render::modes::biome)();
      global::get(reinterpret_cast<utils::data_string_container*>(SIZE_MAX));
      global::get(reinterpret_cast<core::context*>(SIZE_MAX));

      advance_progress(prog, "filling tile connections");
      generate_tile_connections(map_systems->map, global::get<dt::thread_pool>());
      advance_progress(prog, "creating borders");
      find_border_points(map_systems->map, map_systems->core_context); // после генерации нужно сделать много вещей
    }

    void from_menu_to_create_map(utils::progress_container* prog) {
      // тут по идее нам нужно создать мап системы
      // ну и все
      auto map_systems = global::get<systems::map_t>();
      ASSERT(map_systems != nullptr);
      prog->set_max_value(2);
      prog->set_hint1(std::string_view("Creating demiurge"));
      prog->set_hint2(std::string_view("create container"));
      prog->set_type(utils::progress_container::creating_map);
  //     map_systems->create_map_container(); // эти вещи нужно сделать во время загрузок
      advance_progress(prog, "setup rendering");
      map_systems->setup_rendering_modes();
      //advance_progress(prog, "feeding up demiurge");
      //map_systems->setup_map_generator();
      //advance_progress(prog, "creating tools for demiurge");
      //setup_map_generator(map_systems);
      advance_progress(prog, "end");
    }

    void from_menu_to_map(utils::progress_container* prog) {
      auto base_systems = global::get<systems::core_t>();
      auto map_systems = global::get<systems::map_t>();
      prog->set_max_value(9);
      prog->set_hint1(std::string_view("Load world"));
      prog->set_hint2(std::string_view("create container"));
      prog->set_type(utils::progress_container::loading_map);
      // так я могу не успеть создать ничего более прежде чем подойду к блокировке мьютекса в мейне
      // что делать в этом случае? использовать atomic_bool?
  //     map_systems->create_map_container();
      advance_progress(prog, "setup rendering");
      map_systems->setup_rendering_modes();
  //     advance_progress(prog, "feeding up demiurge");
  //     map_systems->setup_map_generator();
  //     advance_progress(prog, "creating tools for demiurge");
  //     setup_map_generator(*map_systems);

      loading_world(map_systems, prog, base_systems->menu->loading_path); // мы должны выбрать: либо загрузка мира, либо загрузка сохранения (там загрузка мира тоже будет)
      base_systems->menu->loading_path.clear();
      base_systems->menu->loading_path.shrink_to_fit();
      advance_progress(prog, "end");
    }

    void from_create_map_to_map(utils::progress_container* prog) {
      auto map_systems = global::get<systems::map_t>();
      auto base_systems = global::get<systems::core_t>();
      prog->set_type(utils::progress_container::loading_created_map);
      post_generation_work(map_systems, base_systems, prog);
      map_systems->destroy_map_generator();
    }
  }
}
