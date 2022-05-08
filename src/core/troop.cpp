#include "troop.h"

namespace devils_engine {
  namespace core {
    troop::troop() : 
      object_token(SIZE_MAX),
      type(nullptr), 
      origin(nullptr),
      provider(nullptr),
      formation(nullptr),
      character(nullptr) 
    {}
    
    troop::~troop() { object_token = SIZE_MAX; }
  }
}
