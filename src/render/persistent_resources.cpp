#include "persistent_resources.h"

#include "shared_structures.h"
#include "yavf.h"

namespace devils_engine {
  namespace render {
    void create_persistent_resources(yavf::Device* device) {
      yavf::DescriptorSetLayout tiles_data_layout = VK_NULL_HANDLE;
      {
        yavf::DescriptorLayoutMaker dlm(device);
        tiles_data_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                               .create(TILES_DATA_LAYOUT_NAME);
      }
    }
  }
}
