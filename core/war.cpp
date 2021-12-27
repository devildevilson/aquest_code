#include "war.h"

namespace devils_engine {
  namespace core {
    war::war() : object_token(SIZE_MAX), cb(nullptr), war_opener(nullptr), target_character(nullptr), claimant(nullptr) {} // opener_realm(nullptr), target_realm(nullptr),
    war::~war() { object_token = SIZE_MAX; }
    
    // нужно найти способ вернуть конст
    OUTPUT_CASUS_BELLI_TYPE war::get_casus_belli() const { return cb; }
    OUTPUT_CHARACTER_TYPE war::get_claimant() const { return claimant; }
    OUTPUT_CHARACTER_TYPE war::get_primary_attacker() const { return war_opener; }
//     OUTPUT_REALM_TYPE war::get_primary_attacker_realm() const { return opener_realm; }
    OUTPUT_CHARACTER_TYPE war::get_primary_defender() const { return target_character; }
//     OUTPUT_REALM_TYPE war::get_primary_defender_realm() const { return target_realm; }
  }
}
