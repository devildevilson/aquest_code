#ifndef MAP_GENERATORS_H
#define MAP_GENERATORS_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <atomic>
#include <functional>
#include "map_generator.h"
#include "utils/utility.h"
#include "generator_context.h"

// абсолютно точно будет существовать несколько генераторов
// 1. генератор ладшафта (необходимо раскидать биомы по тайлам)
// 2. генератор провинций и/или людей (провинции должны генерится по плотности населения по хорошему)
// 3. генератор стран (страны должны генерироваться по населению и по провинциям)
// в итоге я должен получить ландшафт, провинции на этом ландшафте, страны на провинциях

// мультитрединг может внести существенные изменения в ход работы алгоритма 
// (из-за чего один и тот же сид будет давать разные результаты)
// может как то сортировать? 
// нужно чтобы после каждой итерации реультат был одинаков при одинаковом сиде
// похоже что нужно будет сделать некоторые шаги однопоточными

namespace dt {
  class thread_pool;
}

namespace devils_engine {
  namespace map {
    class beginner : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      beginner(const create_info &info);
      // нам здесь потребуется еще структура с данными извне (та же ротация, сид, максимальный подъем и проч)
      // как ее оформить? темплейт?
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class plates_generator : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      plates_generator(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class plate_datas_generator : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      plate_datas_generator(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class compute_boundary_edges : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      compute_boundary_edges(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class compute_plate_boundary_stress : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      compute_plate_boundary_stress(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class blur_plate_boundary_stress : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      blur_plate_boundary_stress(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class calculate_plate_boundary_distances : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      calculate_plate_boundary_distances(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class calculate_plate_root_distances : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      calculate_plate_root_distances(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class modify_plate_datas : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      modify_plate_datas(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class calculate_vertex_elevation : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
        float noise_multiplier;
      };
      calculate_vertex_elevation(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
      float noise_multiplier;
    };
    
    class blur_tile_elevation : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
        uint32_t iterations_count;
        float old_new_ratio;
        float water_ground_ratio;
      };
      blur_tile_elevation(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
      uint32_t iterations_count;
      float old_new_ratio;
      float water_ground_ratio;
    };
    
    class normalize_tile_elevation : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      normalize_tile_elevation(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class calculate_tile_distance : public generator<generator_context> {
    public:
      struct create_info {
        std::function<bool(const generator_context* context, const uint32_t &tile_index)> predicate;
        std::vector<std::pair<uint32_t, uint32_t>>* data_container;
        std::string hint_str;
      };
      calculate_tile_distance(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      std::function<bool(const generator_context* context, const uint32_t &tile_index)> predicate;
      std::vector<std::pair<uint32_t, uint32_t>>* data_container;
      std::string hint_str;
      size_t state;
    };
    
    class modify_tile_elevation : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
        std::function<float(const generator_context* context, const uint32_t &tile_index)> elevation_func;
        std::string hint_str;
      };
      modify_tile_elevation(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::function<float(const generator_context* context, const uint32_t &tile_index)> elevation_func;
      std::string hint_str;
      std::atomic<size_t> state;
    };
    
    class tile_postprocessing1 : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      tile_postprocessing1(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class tile_postprocessing2 : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      tile_postprocessing2(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class connect_water_pools : public generator<generator_context> {
    public:
      connect_water_pools();
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      size_t state;
    };
    
    class generate_water_pools : public generator<generator_context> {
    public:
      generate_water_pools();
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      size_t state;
    };
    
    class compute_tile_heat : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      compute_tile_heat(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class normalize_fractional_values : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
        std::vector<float>* data_container;
        std::string hint_str;
      };
      normalize_fractional_values(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
      std::vector<float>* data_container;
      std::string hint_str;
    };
    
    class compute_moisture : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      compute_moisture(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class create_biomes : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      create_biomes(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class generate_provinces : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      generate_provinces(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class province_postprocessing : public generator<generator_context> {
    public:
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    };
    
    class calculating_province_neighbours : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      calculating_province_neighbours(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class generate_cultures : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      generate_cultures(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class generate_countries : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      generate_countries(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class update_tile_data : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      update_tile_data(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class generate_air_whorls : public generator<generator_context> {
    public:
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      size_t state;
    };
    
    class calculate_vertex_air_current : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      calculate_vertex_air_current(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class calculate_air_current_outflows : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      calculate_air_current_outflows(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class initialize_circulating_heat : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      initialize_circulating_heat(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class propagate_circulating_heat : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      propagate_circulating_heat(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class calculate_temperatures : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      calculate_temperatures(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class initialize_circulating_moisture : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      initialize_circulating_moisture(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class propagate_circulating_moisture : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      propagate_circulating_moisture(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class calculate_wetness : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      calculate_wetness(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
    
    class calculate_biomes : public generator<generator_context> {
    public:
      struct create_info {
        dt::thread_pool* pool;
      };
      calculate_biomes(const create_info &info);
      void process(generator_context* context) override;
      size_t progress() const override;
      size_t complete_state(const generator_context* context) const override;
      std::string hint() const override;
    private:
      dt::thread_pool* pool;
      std::atomic<size_t> state;
    };
  }
}

#endif
