#include "trait.h"

#include "utils/shared_mathematical_constant.h"

namespace devils_engine {
  namespace core {
    const structure trait::s_type;
    const size_t trait::max_stat_modifiers_count;
    const size_t trait::max_opinion_modifiers_count;
    trait_group::trait_group() :
      same_group_opinion(fNAN),
      different_group_opinion(fNAN),
      opposite_opinion(fNAN),
      nogroup_opinion(fNAN)
    {}
    
    trait::trait() : 
      opposite(nullptr),
      group(nullptr),
      numeric_attribs{0,0,0,0}, 
      icon{GPU_UINT_MAX},
      same_group_opinion(fNAN),
      different_group_opinion(fNAN),
      opposite_opinion(fNAN),
      nogroup_opinion(fNAN)
    {}
    
    float trait::get_same_group_opinion() const {
      if (std::isnan(same_group_opinion) && group != nullptr) return std::isnan(group->same_group_opinion) ? 0.0f : group->same_group_opinion;
      return std::isnan(same_group_opinion) ? 0.0f : same_group_opinion;
    }
    
    float trait::get_different_group_opinion() const {
      if (std::isnan(different_group_opinion) && group != nullptr) return std::isnan(group->different_group_opinion) ? 0.0f : group->different_group_opinion;
      return std::isnan(different_group_opinion) ? 0.0f : different_group_opinion;
    }
    
    float trait::get_opposite_opinion() const {
      if (std::isnan(opposite_opinion) && group != nullptr) return std::isnan(group->opposite_opinion) ? 0.0f : group->opposite_opinion;
      return std::isnan(opposite_opinion) ? 0.0f : opposite_opinion;
    }
    
    float trait::get_nogroup_opinion() const {
      if (std::isnan(nogroup_opinion) && group != nullptr) return std::isnan(group->nogroup_opinion) ? 0.0f : group->nogroup_opinion;
      return std::isnan(nogroup_opinion) ? 0.0f : nogroup_opinion;
    }
  }
}
