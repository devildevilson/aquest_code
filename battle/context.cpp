#include "context.h"
#include "structures_enum.h"

namespace devils_engine {
  namespace battle {
    context::context() {
      for (size_t i = 0; i < containers.size(); ++i) {
        containers[i].count = 0;
        containers[i].memory = nullptr;
      }
    }
    
    context::~context() {
      for (size_t i = 0; i < containers.size(); ++i) {
        switch (i) {
          case static_cast<size_t>(structure_type::unit): {
            auto ptr = reinterpret_cast<unit*>(containers[i].memory);
            delete [] ptr;
            containers[i].memory = nullptr;
            break;
          }
          
          case static_cast<size_t>(structure_type::unit_state): {
            auto ptr = reinterpret_cast<core::state*>(containers[i].memory);
            delete [] ptr;
            containers[i].memory = nullptr;
            break;
          }
          
          case static_cast<size_t>(structure_type::troop): {
            auto ptr = reinterpret_cast<troop*>(containers[i].memory);
            delete [] ptr;
            containers[i].memory = nullptr;
            break;
          }
          
          case static_cast<size_t>(structure_type::troop_type): {
            auto ptr = reinterpret_cast<troop_type*>(containers[i].memory);
            delete [] ptr;
            containers[i].memory = nullptr;
            break;
          }
          
          case static_cast<size_t>(structure_type::biome): {
            auto ptr = reinterpret_cast<biome*>(containers[i].memory);
            delete [] ptr;
            containers[i].memory = nullptr;
            break;
          }
          
          default: throw std::runtime_error("Not implemented yet");
        }
      }
    }
    
    
  }
}
