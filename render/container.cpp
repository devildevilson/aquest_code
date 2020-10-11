#include "container.h"

#include "utils/globals.h"
#include "shared_structures.h"
#include "render.h"
#include "yavf.h"
#include "window.h"

static const std::vector<const char*> instanceLayers = {
  //"VK_LAYER_LUNARG_standard_validation",
  "VK_LAYER_KHRONOS_validation",
//   "VK_LAYER_LUNARG_api_dump",
//   "VK_LAYER_LUNARG_object_tracker",
//   "VK_LAYER_GOOGLE_unique_objects",
//   "VK_LAYER_LUNARG_parameter_validation",
//   "VK_LAYER_LUNARG_assistant_layer"
};

namespace devils_engine {
  namespace render {
    container::container() : mem(sizeof(yavf::Instance) + sizeof(struct window) + sizeof(render::stage_container)), instance(nullptr), device(nullptr), window(nullptr), render(nullptr) {}
    container::~container() {
      device->wait();
      for (size_t i = 0 ; i < tasks.size(); ++i) {
        device->deallocate(tasks[i]);
      }

      mem.destroy(render);
      mem.destroy(window);
      mem.destroy(instance);
    }

    yavf::Instance* container::create_instance(const std::vector<const char*> &extensions, const application_info* app_info) {
      const yavf::Instance::ApplicationInfo info{
        app_info->app_name,
        app_info->app_version,
        app_info->engine_name,
        app_info->engine_version,
        app_info->api_version
      };
      
      instance = mem.create<yavf::Instance>(yavf::Instance::CreateInfo{
        nullptr,
        &info,
    #ifndef _NDEBUG
        instanceLayers,
        //{},
    #else
        {},
    #endif
        extensions,
    #ifndef _NDEBUG
        true,
        //false,
    #else
        false,
    #endif
        false,
        false
      });

      return instance;
    }

    struct window* container::create_window(const window_info &info) {
      window = mem.create<struct window>(window::create_info{instance, info.fullscreen, info.width, info.height, 0.0f, info.video_mode});
      return window;
    }

    yavf::Device* container::create_device() {
      const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
//         "VK_KHR_8bit_storage",
//         "VK_KHR_storage_buffer_storage_class",
      };

      VkSurfaceKHR s = window->surface.handle;

      auto physDevices = instance->getPhysicalDevices();

      // как выбирать устройство?
      size_t maxMem = 0;

      yavf::PhysicalDevice choosen = VK_NULL_HANDLE;
      for (size_t i = 0; i < physDevices.size(); ++i) {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        VkPhysicalDeviceMemoryProperties memProp;
        physDevices[i].getProperties(&deviceProperties);
        physDevices[i].getFeatures(&deviceFeatures);
        physDevices[i].getMemoryProperties(&memProp);

        size_t a = 0;
        for (uint32_t j = 0; j < memProp.memoryHeapCount; ++j) {
          if ((memProp.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) == VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            a = std::max(memProp.memoryHeaps[i].size, a);
          }
        }

    //     std::cout << "Device name: " << deviceProperties.deviceName << "\n";

        bool extSupp = yavf::checkDeviceExtensions(physDevices[i], instanceLayers, deviceExtensions);

        uint32_t count = 0;
        physDevices[i].getQueueFamilyProperties(&count, nullptr);

        bool presentOk = false;
        for (uint32_t i = 0; i < count; ++i) {
          VkBool32 present;
          vkGetPhysicalDeviceSurfaceSupportKHR(physDevices[i], i, s, &present);

          if (present) {
            presentOk = true;
            break;
          }
        }

        if (extSupp && presentOk && maxMem < a) {
          maxMem = a;
          choosen = physDevices[i];
          //break;
        }
      }

      VkPhysicalDeviceProperties deviceProperties;
      choosen.getProperties(&deviceProperties);
      if (choosen.handle() == VK_NULL_HANDLE) throw std::runtime_error("choosen.handle() == VK_NULL_HANDLE");
      std::cout << "Using device: " << deviceProperties.deviceName << '\n';
      VkPhysicalDeviceFeatures f_test;
      choosen.getFeatures(&f_test);
      if (f_test.samplerAnisotropy == VK_TRUE) device_properties.set(physical_device_sampler_anisotropy, true);

      yavf::DeviceMaker dm(instance);
      VkPhysicalDeviceFeatures f = {};
      f.samplerAnisotropy = is_properties_presented(physical_device_sampler_anisotropy);
      //f.geometryShader = VK_TRUE;
    //   f.multiDrawIndirect = VK_TRUE;
    //   f.drawIndirectFirstInstance = VK_TRUE;
    //   f.fragmentStoresAndAtomics = VK_TRUE;
//       f.wideLines = VK_TRUE;
      device = dm.beginDevice(choosen).setExtensions(deviceExtensions).createQueues().features(f).create(instanceLayers, "Graphics device");

      yavf::DescriptorPool defaultPool = VK_NULL_HANDLE;
      {
        yavf::DescriptorPoolMaker dpm(device);

        defaultPool = dpm.poolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 20)
                        .poolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10)
                        .poolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10)
                        .poolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10)
                        .create(DEFAULT_DESCRIPTOR_POOL_NAME);
      }

      yavf::DescriptorSetLayout uniform_layout = VK_NULL_HANDLE;
      yavf::DescriptorSetLayout storage_layout = VK_NULL_HANDLE;
      {
        yavf::DescriptorLayoutMaker dlm(device);

        uniform_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
                            .binding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL)
                            .create(UNIFORM_BUFFER_LAYOUT_NAME);
        storage_layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL).create(STORAGE_BUFFER_LAYOUT_NAME);
      }

      (void)defaultPool;
      (void)uniform_layout;
      (void)storage_layout;

      return device;
    }

    render::stage_container* container::create_system(const size_t &system_container_size) {
      render = mem.create<render::stage_container>(system_container_size);
      return render;
    }

    void container::create_tasks() {
      tasks.resize(window->frames.size(), nullptr);
      tasks.shrink_to_fit();

      for (size_t i = 0 ; i < tasks.size(); ++i) {
        tasks[i] = device->allocateCombinedTask();
        tasks[i]->pushWaitSemaphore(window->frames[i].image_available, window->frames[i].flags);
        tasks[i]->pushSignalSemaphore(window->frames[i].finish_rendering);
      }
    }
    
    bool container::is_properties_presented(const uint32_t &index) const {
      return device_properties.get(index);
    }

    yavf::TaskInterface* container::interface() const {
//       return tasks[window->swapchain.image_index];
      return tasks[window->current_frame];
    }

    yavf::CombinedTask* container::combined() const {
//       return tasks[window->swapchain.image_index];
      return tasks[window->current_frame];
    }

    yavf::ComputeTask* container::compute() const {
//       return tasks[window->swapchain.image_index];
      return tasks[window->current_frame];
    }

    yavf::GraphicTask* container::graphics() const {
//       return tasks[window->swapchain.image_index];  
      return tasks[window->current_frame];
    }

    yavf::TransferTask* container::transfer() const {
      //return tasks[window->current_frame];
      return nullptr;
    }
  }
}
