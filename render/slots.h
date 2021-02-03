#ifndef RENDER_SLOTS_H
#define RENDER_SLOTS_H

#include <cstdint>
#include <cstddef>
#include <array>

namespace devils_engine {
  namespace render {
    class stage;
    class context;
    
    class slots {
    public:
      static const size_t slots_count = 3 + 3 + 10; // вообще наверное не предполагается что у нас будет какое то вариативное количество слотов
      
      slots();
      void set_stage(const uint32_t &slot, stage* ptr);
      void clear_slot(const uint32_t &slot);
      void update(context* ctx);
      void clear();
      const stage* get_stage(const uint32_t &slot) const;
    private:
      std::array<stage*, slots_count> stages;
    };
  }
}

#endif
