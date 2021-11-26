#include "war.h"

namespace devils_engine {
  namespace core {
    war::war() : object_token(SIZE_MAX), cb(nullptr), war_opener(nullptr), target_character(nullptr), opener_realm(nullptr), target_realm(nullptr), claimant(nullptr) {}
    war::~war() { object_token = SIZE_MAX; }
  }
}
