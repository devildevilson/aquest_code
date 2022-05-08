#ifndef DEVILS_ENGINE_CORE_GAME_RESOURCES_T
#define DEVILS_ENGINE_CORE_GAME_RESOURCES_T

#include "utils/thread_pool.h"
#include "utils/systems.h"
#include "utils/deferred_tasks.h"
#include "utils/settings.h"
#include "utils/frame_time.h"
#include "utils/frame_allocator.h"

namespace devils_engine {
  namespace systems {
    struct loading_context;
  }
  
  namespace core {
    struct glfw_t {
      glfw_t();
      ~glfw_t();
    };
    
    struct block_input_t {
      block_input_t();
      ~block_input_t();
    };
    
    class loading_interface {
    public:
      inline loading_interface(systems::loading_context* ctx) noexcept : ctx(ctx), counter(0) {}
      virtual ~loading_interface() noexcept = default;
      virtual void update() = 0;
      virtual bool finished() const = 0;
      virtual size_t current() const = 0;
      virtual size_t count() const = 0;
      virtual std::string_view hint1() const = 0;
      virtual std::string_view hint2() const = 0;
      virtual std::string_view hint3() const = 0;
      
    protected:
      systems::loading_context* ctx;
      std::future<void> notify;
      size_t counter;
    };
    
    struct game_resources_t { // application::
      glfw_t glfw;
      dt::thread_pool pool;

      systems::core_t base_systems;           // это существует всегда
      std::unique_ptr<systems::map_t> map_systems;             // это существует только тогда когда есть загруженная игра
      std::unique_ptr<systems::generator_t> generator_systems; // это существует только на этапе генерации
      std::unique_ptr<systems::battle_t> battle_systems;       // это существует только в битве
      std::unique_ptr<systems::encounter_t> encounter_systems; // это существует только в столкновении
      std::unique_ptr<loading_interface> loading;
      
      utils::deferred_tasks tasks;
      
      utils::settings settings;
      
      utils::frame_time frame_time;
      utils::frame_allocator frame_allocator;
      
      game_resources_t(const int argc, const char* argv[]);
      ~game_resources_t();
      
      void poll_events();
    };
    
    struct load_main_menu_t final : public loading_interface {
      load_main_menu_t(game_resources_t* res);
      ~load_main_menu_t();
      
      void update() override;
      bool finished() const override;
      size_t current() const override;
      size_t count() const override;
      std::string_view hint1() const override;
      std::string_view hint2() const override;
      std::string_view hint3() const override;
    };
    
    struct load_save_game_t final : public loading_interface {
      load_save_game_t(game_resources_t* res);
      ~load_save_game_t();
      
      void update() override;
      bool finished() const override;
      size_t current() const override;
      size_t count() const override;
      std::string_view hint1() const override;
      std::string_view hint2() const override;
      std::string_view hint3() const override;
    };
    
    struct load_save_world_t final : public loading_interface {
      load_save_world_t(game_resources_t* res);
      ~load_save_world_t();
      
      void update() override;
      bool finished() const override;
      size_t current() const override;
      size_t count() const override;
      std::string_view hint1() const override;
      std::string_view hint2() const override;
      std::string_view hint3() const override;
    };
    
    struct load_gen_world_t final : public loading_interface {
      load_gen_world_t(game_resources_t* res);
      ~load_gen_world_t();
      
      void update() override;
      bool finished() const override;
      size_t current() const override;
      size_t count() const override;
      std::string_view hint1() const override;
      std::string_view hint2() const override;
      std::string_view hint3() const override;
    };
    
    struct load_gen_t final : public loading_interface {
      load_gen_t(game_resources_t* res);
      ~load_gen_t();
      
      void update() override;
      bool finished() const override;
      size_t current() const override;
      size_t count() const override;
      std::string_view hint1() const override;
      std::string_view hint2() const override;
      std::string_view hint3() const override;
    };
    
    struct gen_step_t final : public loading_interface {
      game_resources_t* res;
      
      gen_step_t(game_resources_t* res);
      ~gen_step_t();
      
      void update() override;
      bool finished() const override;
      size_t current() const override;
      size_t count() const override;
      std::string_view hint1() const override;
      std::string_view hint2() const override;
      std::string_view hint3() const override;
    };
    
    struct load_battle_t final : public loading_interface {
      load_battle_t(game_resources_t* res);
      ~load_battle_t();
      
      void update() override;
      bool finished() const override;
      size_t current() const override;
      size_t count() const override;
      std::string_view hint1() const override;
      std::string_view hint2() const override;
      std::string_view hint3() const override;
    };
    
    struct load_encounter_t final : public loading_interface {
      load_encounter_t(game_resources_t* res);
      ~load_encounter_t();
      
      void update() override;
      bool finished() const override;
      size_t current() const override;
      size_t count() const override;
      std::string_view hint1() const override;
      std::string_view hint2() const override;
      std::string_view hint3() const override;
    };
    
    struct post_gen_world_t final : public loading_interface {
      post_gen_world_t(game_resources_t* res);
      ~post_gen_world_t();
      
      void update() override;
      bool finished() const override;
      size_t current() const override;
      size_t count() const override;
      std::string_view hint1() const override;
      std::string_view hint2() const override;
      std::string_view hint3() const override;
    };
  }
}

#endif
