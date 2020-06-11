#ifndef YAVF_CORE_H
#define YAVF_CORE_H

//#include <execinfo.h>
//#include <csignal>
//#include <unistd.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

#include <string.h>

#include "MemoryPool.h"

#include "Internal.h"
#include "RenderTarget.h"
#include "Tasks.h"
#include "Types.h"

#ifndef YAVF_IMAGE_POOL_SIZE
#define YAVF_IMAGE_POOL_SIZE 100
#endif

#ifndef YAVF_BUFFER_POOL_SIZE
#define YAVF_BUFFER_POOL_SIZE 100
#endif

#ifndef YAVF_VECTOR_UPDATE_KOEF
#define YAVF_VECTOR_UPDATE_KOEF 2
#endif

#ifndef YAVF_DEFAULT_VECTOR_CAPASITY
#define YAVF_DEFAULT_VECTOR_CAPASITY 20
#endif

#ifndef YAVF_NO_MULTITHREADING
#include <mutex>
#define YAVF_LOCK_MUTEX(mutex_variable) std::unique_lock<std::mutex> lock(mutex_variable);
#else
#define YAVF_LOCK_MUTEX(mutex_variable)
#endif

#define VK_VERSION_TO_STRING(ver) \
std::to_string((ver >> 22) & 0xffc) + "." + \
std::to_string((ver >> 12) & 0x3ff) + "." + \
std::to_string(ver & 0xfff)

#define YAVF_ANY_QUEUE VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT

// TODO: нужно запилить синхронизацию к yavf, нужно ли пользоваться mem alloc синхронизацией?
// сейчас она мне нужна чтобы пересоздавать массивы в разных GPUOptimizer'ах

namespace yavf {
  class Instance;
  class Device;

  std::vector<VkExtensionProperties> getRequiredDeviceExtensions(VkPhysicalDevice device, const std::vector<const char*> &layers, const std::vector<const char*> &extensions);
  bool checkDeviceExtensions(VkPhysicalDevice device, const std::vector<const char*> &layers, const std::vector<const char*> &extensions);
  VkFormat findSupportedFormat(VkPhysicalDevice phys, const std::vector<VkFormat> &candidates, const VkImageTiling &tiling, const VkFormatFeatureFlags &features);

  VkExtent2D chooseSwapchainExtent(const uint32_t &width, const uint32_t &height, const VkSurfaceCapabilitiesKHR& capabilities);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &presentModes);
  bool checkSwapchainPresentMode(const std::vector<VkPresentModeKHR> &presentModes, const VkPresentModeKHR &mode);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
  
  size_t alignMemorySize(const size_t &alignment, const size_t &size);

  class SimpleRenderTarget : public RenderTarget {
  public:
    struct CreateInfo {
      VkRect2D frameSize;
      Framebuffer buffer;
      RenderPass pass;
//       std::vector<VkClearValue> values;
    };
    SimpleRenderTarget();
    SimpleRenderTarget(const CreateInfo &info);
    ~SimpleRenderTarget();
    
    std::vector<VkClearValue> clearValues() const override;
    VkRect2D size() const override;
    RenderPass renderPass() const override;
    VkViewport viewport() const override;
    VkRect2D scissor() const override;
    Framebuffer framebuffer() const override;
    
    //void resize(const VkRect2D &newSize, const std::vector<VkImageView> &views);
    void setFramebuffer(const Framebuffer &f);
    void setSize(const VkRect2D &size);
  private:
    VkRect2D frameSize;
    Framebuffer buffer;
    VkRenderPass pass;
//     std::vector<VkClearValue> values;
  };
  
  struct VirtualFrame {
//     SimpleRenderTarget framebuffer;
    Semaphore imageAvailable;
    Semaphore finishRendering;
  };
  
  // : public RenderTarget
  class SimpleWindow {
  public:
    SimpleWindow(Instance* instance, Device* device, VkSurfaceKHR surface, const uint32_t &width, const uint32_t &height);
    ~SimpleWindow();

//     std::vector<VkClearValue> clearValues() const override;
//     VkRect2D size() const override;
//     VkRenderPass renderPass() const override;
//     VkViewport viewport() const override;
//     VkRect2D scissor() const override;
// 
//     Framebuffer framebuffer() const override;
    
    //SemaphoreProxy* createSemaphoreProxy(const VkPipelineStageFlags &flag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) override;
//     SemaphoreProxy* getSemaphoreProxy() const override;
//     void addSemaphoreProxy(SemaphoreProxy* proxy) override;
    //VkSemaphore imageAvailable() const override;
    //VkSemaphore finishRendering() const override;
    
    uint32_t framesCount() const;
    
    VirtualFrame & operator[](const uint32_t &index);
    const VirtualFrame & operator[](const uint32_t &index) const;

    VkResult nextFrame();
    // не обязательно пересоздавать фреймбуфер каждый кадр (зачем я вообще это делал?)
    //void updateFramebuffer();
    
    uint32_t currentFrame() const;
    uint32_t currentImage() const;
    RenderTarget* currentRenderTarget();
    
    VkResult present(const Internal::Queue &queue);
    VkResult present();
    
    void recreate(const uint32_t &width, const uint32_t &height);
    bool isValid() const;
    uint32_t getFamily() const;
  private:
    void createRenderPass();

    bool valid = false;
    uint32_t currentVirtualFrame;
    uint32_t imageIndex = 0;
    uint32_t family = UINT32_MAX;
    
    Internal::SurfaceData surfaceData;
    
    Device* device = nullptr;
    Instance* instance = nullptr;
    VkSurfaceKHR surface;
    RenderPass renderPassHandle;
    Image* depthImage;
    
//     Internal::SwapchainData swapchain;
    
    Swapchain swapchain;
    std::vector<Image*> swapchainImages;
    
//     std::vector<Internal::VirtualFrame> frames;
    std::vector<SimpleRenderTarget> framebuffers;
    std::vector<VirtualFrame> frames;
    
    //SemaphoreOwner* waitSemaphore = nullptr;
    //std::vector<SemaphoreProxy*> signalSemaphores;
  };
  
  class PhysicalDevice {
  public:
    PhysicalDevice();
    PhysicalDevice(const PhysicalDevice &device);
    PhysicalDevice(VkPhysicalDevice device);
    ~PhysicalDevice();
    
//     void enumerateDeviceGroups(uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);
    void getProperties(VkPhysicalDeviceProperties* pProperties);
    void getProperties2(VkPhysicalDeviceProperties2* pProperties);
    void getQueueFamilyProperties(uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties);
    void getQueueFamilyProperties2(uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties);
    void getMemoryProperties(VkPhysicalDeviceMemoryProperties* pMemoryProperties);
    void getMemoryProperties2(VkPhysicalDeviceMemoryProperties2* pMemoryProperties);
    void getFeatures(VkPhysicalDeviceFeatures* features);
    
    VkPhysicalDevice handle() const;
    
    PhysicalDevice & operator=(const PhysicalDevice &another);
    PhysicalDevice & operator=(const VkPhysicalDevice &another);
    
    operator VkPhysicalDevice() const;
    bool operator==(const PhysicalDevice &another) const;
    bool operator!=(const PhysicalDevice &another) const;
  private:
    VkPhysicalDevice physDevice;
  };
  
  class Instance;
  
  // нужно добавить поддержку мультитрединга и сюда
  class Device {
  public:
    struct CreateInfo {
      Instance* inst;
      VkDevice device;
      PhysicalDevice phys;
      uint32_t familiesCount;
      const yavf::Internal::QueueInfo* families;
      size_t bufferSizeBlock;
      size_t imageSizeBlock;
      // думаю что тут наверное еще надо отдельно прописать создание командных пулов
    };
    
    //Device(VkDevice device, VkPhysicalDevice phys, const std::vector<yavf::Internal::QueueInfo> &families);
    Device(const CreateInfo &info);
    ~Device();
    
    // кажется только эти 2 функции попадают пока что в мультипоток
    Buffer* create(const BufferCreateInfo &info, const VmaMemoryUsage &usage);
    Image* create(const ImageCreateInfo &info, const VmaMemoryUsage &usage);
    BufferView* create(const VkBufferViewCreateInfo &info, Buffer* relatedBuffer);
    ImageView* create(const VkImageViewCreateInfo &info, Image* relatedImage);

    DescriptorPool create(const VkDescriptorPoolCreateInfo &info, const std::string &name);
    Sampler create(const VkSamplerCreateInfo &info, const std::string &name);
    DescriptorSetLayout create(const VkDescriptorSetLayoutCreateInfo &info, const std::string &name);
    PipelineLayout create(const VkPipelineLayoutCreateInfo &info, const std::string &name);
    Pipeline create(const VkPipelineCache &cache, const VkGraphicsPipelineCreateInfo &info, const std::string &name);
    Pipeline create(const VkPipelineCache &cache, const VkComputePipelineCreateInfo &info, const std::string &name);
    RenderPass create(const VkRenderPassCreateInfo &info, const std::string &name);
    Framebuffer create(const FramebufferCreateInfo &info, const std::string &name);
    Swapchain create(const VkSwapchainCreateInfoKHR &info, const std::string &name);
    Swapchain recreate(const VkSwapchainCreateInfoKHR &info, const std::string &name);
    
    DescriptorSet* create(const DescriptorPool &pool, const VkDescriptorSet &set);
    
    std::vector<Image*> getSwapchainImages(const Swapchain &swapchain);
    std::vector<Image*> getSwapchainImages(const std::string &name);

    void destroy(Buffer* buffer);
    void destroy(Image* image);
    void destroy(BufferView* bufferView);
    void destroy(ImageView* imageView);

    GraphicTask*  allocateGraphicTask(const bool &primary = true);
    ComputeTask*  allocateComputeTask(const bool &primary = true);
    CombinedTask* allocateCombinedTask(const bool &primary = true);
    TransferTask* allocateTransferTask(const bool &primary = true);

    void deallocate(GraphicTask* task);
    void deallocate(ComputeTask* task);
    void deallocate(CombinedTask* task);
    void deallocate(TransferTask* task);
    
//     SemaphoreOwner* createSemaphoreProxy();
//     void destroySemaphoreProxy(SemaphoreOwner* proxy);
    
    Semaphore createSemaphore();
//     void destroySemaphore(const uint32_t &index);
    
    //, uint32_t* index = nullptr
    Fence createFence(const VkFenceCreateFlags &flags);
//     void destroyFence(const uint32_t &index);
    
    void destroy(const Fence &f);
    void destroy(const Semaphore &s);
    
    Internal::Queue submit(const uint32_t &family, const uint32_t &submitCount, const VkSubmitInfo* submitInfo);
    Internal::Queue submit(const std::vector<TaskInterface*> &tasks);
    uint32_t getQueueFamilyIndex(const VkQueueFlags &flags) const;
    Internal::Queue getQueue(const uint32_t &familyIndex) const;
    uint32_t getFamiliesCount() const;

    void wait() const;
    VkDevice handle() const;
    VkPhysicalDevice physicalHandle() const;
    
    VmaAllocator bufferAllocator() const;
    VmaAllocator imageAllocator()  const;
    
    CommandPool commandPool(const uint32_t &index) const;
    RenderPass renderpass(const std::string &name) const;
    
    Pipeline pipeline(const std::string &name) const;
    PipelineLayout layout(const std::string &name) const;
    
    DescriptorSetLayout setLayout(const std::string &name) const;
    Sampler sampler(const std::string &name) const;
    DescriptorPool descriptorPool(const std::string &name) const;
    Swapchain swapchain(const std::string &name) const;
    Framebuffer framebuffer(const std::string &name) const;
    
    void destroy(const Framebuffer &framebuffer);
    void destroy(const Swapchain &swapchain);
    
    void destroy(const RenderPass &pass);
    void destroy(const Pipeline &pipeline);
    void destroy(const PipelineLayout &pipelineLayout);
    void destroy(const DescriptorSetLayout &setLayout);
    void destroy(const Sampler &sampler);
    void destroy(const DescriptorPool &descriptorPool);
    
    void destroyRenderPass(const std::string &name);
    void destroyPipeline(const std::string &name);
    void destroyLayout(const std::string &name);
    void destroySetLayout(const std::string &name);
    void destroySampler(const std::string &name);
    void destroyDescriptorPool(const std::string &name);
    void destroySwapchain(const std::string &name);
    void destroyFramebuffer(const std::string &name);

    VmaStats getBufferStatistic() const;
    VmaStats getImageStatistic() const;
    
    void getProperties(VkPhysicalDeviceProperties* props) const;
    size_t getMinMemoryMapAlignment() const;
    size_t getNonCoherentAtomSize() const;
  private:
    //using yavf::Internal::QueueFamily;
    size_t minMemoryMapAlignment = 0;
    size_t nonCoherentAtomSize = 0;

    Instance* inst;
    PhysicalDevice phys = VK_NULL_HANDLE;
    VkDevice h = VK_NULL_HANDLE;
    VmaAllocator bufferAllocatorHandle = VK_NULL_HANDLE;
    VmaAllocator imageAllocatorHandle  = VK_NULL_HANDLE;
    
    std::vector<CommandPool> commandPools;
    
    std::vector<Internal::QueueFamily> families;

//     std::vector<VkSampler> samplerContainer;
    
#ifndef YAVF_NO_MULTITHREADING
    std::mutex buffer_mutex;
    std::mutex image_mutex;
    // пока что 2 мьютекса, нужны ли другие?
#endif

    std::vector<Buffer*> buffers;
    std::vector<Image*> images;
    MemoryPool<Buffer, sizeof(Buffer)*YAVF_BUFFER_POOL_SIZE> bufferPool;
    MemoryPool<Image, sizeof(Image)*YAVF_IMAGE_POOL_SIZE> imagePool;
    
    std::vector<BufferView*> bufferViews;
    std::vector<ImageView*> imageViews;
    MemoryPool<BufferView, sizeof(BufferView)*YAVF_BUFFER_POOL_SIZE/5> bufferViewPool;
    MemoryPool<ImageView, sizeof(ImageView)*YAVF_IMAGE_POOL_SIZE> imageViewPool;
    
//     MemoryPool<SemaphoreOwner, sizeof(SemaphoreOwner)*50> proxies;
    
    std::vector<Semaphore> semaphores;
    std::vector<Fence> fences;

    std::vector<GraphicTask*> graphicTasks;
    MemoryPool<GraphicTask, sizeof(GraphicTask)*6> graphicsPool;
    std::vector<ComputeTask*> computeTasks;
    MemoryPool<ComputeTask, sizeof(ComputeTask)*6> computePool;
    std::vector<CombinedTask*> combinedTasks;
    MemoryPool<CombinedTask, sizeof(CombinedTask)*6> combinedPool;
    std::vector<TransferTask*> transferTasks;
    MemoryPool<TransferTask, sizeof(TransferTask)*6> copyPool;

    std::unordered_map<std::string, RenderPass> passes;
    std::unordered_map<std::string, Pipeline> pipelines;
    std::unordered_map<std::string, PipelineLayout> layouts;
    std::unordered_map<std::string, DescriptorSetLayout> setLayouts;
    std::unordered_map<std::string, Sampler> samplers;
    
    struct DescPoolData {
      DescriptorPool pool;
      std::vector<DescriptorSet*> descriptorSetArray;
    };
    std::unordered_map<std::string, DescPoolData> pools;
    
    MemoryPool<DescriptorSet, sizeof(DescriptorSet)*20> descriptorSetPool;
    
    struct SwapData {
      Swapchain swap;
      std::vector<Image*> images;
    };
    
    std::unordered_map<std::string, SwapData> swapchains;
    std::unordered_map<std::string, Framebuffer> framebuffers;
  };

  class Instance {
  public:
//     static void addLayer(const char* layerName);
//     static void addExtension(const char* extensionName);
//     static void setLayers(const std::vector<const char*> &layers);
//     static void setExtensions(const std::vector<const char*> &extensions);
//     static std::vector<const char*> & getLayers();
//     static std::vector<const char*> & getExtensions();
    
    struct ApplicationInfo {
      std::string appName;
      uint32_t appVersion;
      std::string engineName;
      uint32_t engineVersion;
      uint32_t apiVersion;
    };
    
    struct CreateInfo {
      void* creationExtension;
      const ApplicationInfo* appInfoPtr;
      std::vector<const char*> layerNames;
      std::vector<const char*> extensionNames;
      bool createDebugLayer;
      bool printLayerInfo;
      bool printExtensionInfo;
    };

    Instance();
    Instance(const CreateInfo &info);
    ~Instance();
    
    void construct(const CreateInfo &info);

    //std::vector<VkPhysicalDevice> getDevices(const std::function<bool(const VkPhysicalDevice)> &func);
    std::vector<PhysicalDevice> getPhysicalDevices();
    Device* getDevice(const std::string &name) const;

//     void create(const uint32_t &apiVersion, const std::string &appName, const uint32_t &appVersion, const std::string &enName, const uint32_t &engineVersion);
    Device* create(const Device::CreateInfo &info, const std::string &name);
    
    bool debugSupport() const;
    
    VkInstance handle() const;
    
    operator VkInstance() const;
    bool operator==(const Instance &another) const;
    bool operator!=(const Instance &another) const;
  private:
    VkInstance inst;
//     VkDebugReportCallbackEXT callback;
    VkDebugUtilsMessengerEXT messenger;

    std::unordered_map<std::string, Device*> devices;
//     static std::vector<const char*> layers;
//     static std::vector<const char*> extensions;
  };

  template<typename T>
  class vector {
  public:
    typedef T* iterator;
    typedef const T* const_iterator;
    
    class reverse_iterator {
    public:
      reverse_iterator(T* ptr) {
        this->ptr = ptr;
      }
      
      T& operator-> () {
        return *ptr;
      }
      
      const T& operator-> () const {
        return *ptr;
      }
      
      bool operator== (const reverse_iterator &other) const {
        return this->ptr == other.ptr;
      }

      bool operator!= (const reverse_iterator &other) const {
        return this->ptr != other.ptr;
      }
      
      reverse_iterator& operator++() {
        --ptr;
        return *this;
      }
      
      reverse_iterator& operator--() {
        ++ptr;
        return *this;
      }
      
      reverse_iterator operator++(int) {
        reverse_iterator it(ptr);
        --ptr;
        return it;
      }
      
      reverse_iterator operator--(int) {
        reverse_iterator it(ptr);
        ++ptr;
        return it;
      }
    private:
      T* ptr;
    };

    typedef const reverse_iterator const_reverse_iterator;
    
    vector() : usage(0), m_capacity(0), m_size(0), ptr(nullptr), device(nullptr), buffer(nullptr) {}
    
    vector(Device* device, const VkBufferUsageFlags &usage) : usage(usage), m_capacity(0), m_size(0), ptr(nullptr), device(device), buffer(nullptr) {
      construct(device, usage);
    }
    
    vector(Device* device, const VkBufferUsageFlags &usage, const size_t &size) : usage(usage), m_capacity(size), m_size(size), ptr(nullptr), device(device), buffer(nullptr) {
      construct(device, usage, size);
    }
    
    vector(Device* device, const VkBufferUsageFlags &usage, const size_t &size, const T& initial) : usage(usage), m_capacity(size), m_size(size), ptr(nullptr), device(device), buffer(nullptr) {
      construct(device, usage, size, initial);
    }
    
    vector(Device* device, const VkBufferUsageFlags &usage, const vector<T> &v) : usage(usage), m_capacity(v.size()), m_size(v.size()), ptr(nullptr), device(device), buffer(nullptr) {
      construct(device, usage, v);
    }
    
    ~vector() {
      destroy();
    }
    
    void construct(Device* device, const VkBufferUsageFlags &usage) {
      if (isValid()) destroy();
      
      this->device = device;
      this->usage = usage;

      m_capacity = YAVF_DEFAULT_VECTOR_CAPASITY;
      buffer = device->create(BufferCreateInfo::buffer(m_capacity * sizeof(T), usage), VMA_MEMORY_USAGE_CPU_ONLY);
      
      ptr = reinterpret_cast<T*>(buffer->ptr());
      m_size = 0;
    }
    
    void construct(Device* device, const VkBufferUsageFlags &usage, const size_t &size) {
      if (isValid()) destroy();
      
      this->device = device;
      this->usage = usage;

      m_capacity = std::max(size_t(YAVF_DEFAULT_VECTOR_CAPASITY), size);
      buffer = device->create(BufferCreateInfo::buffer(m_capacity * sizeof(T), usage), VMA_MEMORY_USAGE_CPU_ONLY);
      
      ptr = reinterpret_cast<T*>(buffer->ptr());
      
      for (size_t i = 0; i < size; ++i) {
        ptr[i] = T();
      }
      
      m_size = size;
    }
    
    void construct(Device* device, const VkBufferUsageFlags &usage, const size_t &size, const T& initial) {
      if (isValid()) destroy();
      
      this->device = device;
      this->usage = usage;

      m_capacity = std::max(size_t(YAVF_DEFAULT_VECTOR_CAPASITY), size);
      buffer = device->create(BufferCreateInfo::buffer(m_capacity * sizeof(T), usage), VMA_MEMORY_USAGE_CPU_ONLY);
      
      ptr = reinterpret_cast<T*>(buffer->ptr());
      
      for (size_t i = 0; i < size; ++i) {
        ptr[i] = initial;
      }
      
      m_size = size;
    }
    
    void construct(Device* device, const VkBufferUsageFlags &usage, const vector<T> &v) {
      if (isValid()) destroy();
      
      this->device = device;
      this->usage = usage;

      m_capacity = std::max(size_t(YAVF_DEFAULT_VECTOR_CAPASITY), v.size());
      buffer = device->create(BufferCreateInfo::buffer(m_capacity * sizeof(T), usage), VMA_MEMORY_USAGE_CPU_ONLY);
      
      ptr = reinterpret_cast<T*>(buffer->ptr());
      
      for (size_t i = 0; i < v.size(); ++i) {
        ptr[i] = v[i];
      }
      
      m_size = v.size();
      
//       copyDescriptor(v.updateData);
    }

    void destroy() {
//       if (buffer != VK_NULL_HANDLE) {
//         vmaDestroyBuffer(device->bufferAllocator(), buffer, allocation);
//         buffer = VK_NULL_HANDLE;
//       }
      if (buffer != nullptr) {
        clear();
        device->destroy(buffer);
        buffer = nullptr;
      }

      m_capacity = 0;
      m_size = 0;
      ptr = nullptr;
      device = nullptr;
    }
    
    Buffer* handle() const {
      return buffer;
    }
    
    bool isValid() const {
//       return device != nullptr;
      return buffer != nullptr;
    }
    
    bool empty() const {
      return m_size == 0;
    }
    
    size_t size() const {
      return m_size;
    }
    
    size_t buffer_size() const {
      return m_capacity * sizeof(T);
    }
    
    size_t max_size() const {
      return UINT64_MAX;
    }
    
    size_t capacity() const {
      return m_capacity;
    }

    void reserve(const size_t &size) {
      if (m_capacity >= size) return;
      
      recreate(size);
    }
    
    void resize(const size_t &size) {
      if (m_size == size) return;
      
      if (size == 0) {
        clear();
        return;
      }
      
      if (size > m_capacity) {
        recreate(size);
        
        for (size_t i = m_size; i < size; ++i) {
          //ptr[i]();
          ptr[i] = T();
        }
        
        m_size = size;
        
        return;
      }
      
      if (size > m_size) {
        for (size_t i = m_size; i < size; ++i) {
          //ptr[i]();
          ptr[i] = T();
        }
        
        m_size = size;
        return;
      }
      
      std::_Destroy_aux<!std::is_trivially_destructible<T>::value>::
      __destroy(ptr + size, ptr + m_size);
//       std::_Destroy_aux<__has_trivial_destructor(T)>::
//       __destroy(ptr + size, ptr + m_size);
      
      m_size = size;
    }
    
    void clear() {
      std::_Destroy_aux<!std::is_trivially_destructible<T>::value>::
      __destroy(ptr, ptr + m_size);
//       std::_Destroy_aux<__has_trivial_destructor(T)>::
//       __destroy(ptr, ptr + m_size);
      
      m_size = 0;
    }
    
    void shrink_to_fit() {
      if (m_size == m_capacity) return;
      
      recreate(m_size);
    }
    
    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
      if (pos >= end()) {
        emplace_back(std::forward<Args>(args)...);
        return end();
      }
      
      if (m_size == m_capacity) {
        recreate(m_capacity * YAVF_VECTOR_UPDATE_KOEF);
      }
      
      const size_t s = pos - ptr;
      memmove(ptr + s + 1, ptr + s, (m_size - s) * sizeof(T));
      
      new (&ptr[s]) T(std::forward<Args>(args)...);
      ++m_size;
      
      return iterator(ptr + s);
    }
    
    template<typename... Args>
    void emplace_back(Args&&... args) {
      if (m_size == m_capacity) {
        recreate(m_capacity * YAVF_VECTOR_UPDATE_KOEF);
      }
      
      new (&ptr[m_size]) T(std::forward<Args>(args)...);
      ++m_size;
    }
    
    void assign(const size_t &count, const T& value) {
      if (count > m_capacity) recreate(count);
      
      for (size_t i = 0; i < count; ++i) {
        ptr[i] = value;
      }
      
      m_size = std::max(count, m_size);
    }
    
    void push_back(const T &obj) {
      if (m_size == m_capacity) {
        recreate(m_capacity * YAVF_VECTOR_UPDATE_KOEF);
      }
      
      ptr[m_size] = obj;
      ++m_size;
    }
    
    void pop_back() {
//       std::_Destroy_aux<__has_trivial_destructor(T)>::
//       __destroy(ptr + m_size-1, ptr + m_size);
      std::_Destroy_aux<!std::is_trivially_destructible<T>::value>::
        __destroy(ptr + m_size-1, ptr + m_size);
      --m_size;
    }
    
    iterator insert(const_iterator pos, const T& value) {
      if (pos >= end()) {
        push_back(value);
        return end();
      }
      
      if (m_size == m_capacity) {
        recreate(m_capacity * YAVF_VECTOR_UPDATE_KOEF);
      }
      
      const size_t s = pos - ptr;
      memmove(ptr + s + 1, ptr + s, (m_size - s) * sizeof(T));
      
      ptr[s] = value;
      ++m_size;
      
      return iterator(ptr + s);
    }
    
    iterator insert(const_iterator pos, const size_t &count, const T& value) {
      const size_t s = pos - ptr;
      
      if (m_size + count > m_capacity) {
        recreate(m_size + count);
      }
      
      memmove(ptr + s + count, ptr + s, (m_size - s) * sizeof(T));
      
      for (size_t i = s; i < count; ++i) {
        ptr[i] = value;
      }
      m_size += count;
      
      return iterator(ptr + s);
    }
    
    iterator erase(const_iterator pos) {
//       if (pos == end()-1) {
//         pop_back();
//         return end();
//       }
//       
       const size_t s = pos - ptr;
       std::_Destroy_aux<__has_trivial_destructor(T)>::
       __destroy(ptr + s, ptr + s+1);
//       
//       memmove(ptr + s, ptr + s+1, (m_size - s) * sizeof(T));

      if (pos != end()-1) _GLIBCXX_MOVE3(pos + 1, end(), pos);
      pop_back();
      
      return pos;
    }
    
    iterator erase(const_iterator first, const_iterator last) {
      if (first == last) return first;
      
      // не понимаю правильно или нет?
      if (last != end()) _GLIBCXX_MOVE3(last, end(), first);
      std::_Destroy_aux<!std::is_trivially_destructible<T>::value>::
        __destroy(first + (end() - last), ptr + m_size);
//       std::_Destroy_aux<__has_trivial_destructor(T)>::
//       __destroy(first + (end() - last), ptr + m_size);
      m_size -= last - first;
      
      return first;
    }
    
    iterator begin() {
      return iterator(ptr);
    }
    
    const_iterator begin() const {
      return const_iterator(ptr);
    }
    
    iterator end() {
      return iterator(ptr + m_size);
    }
    
    const_iterator end() const {
      return const_iterator(ptr + m_size);
    }
    
    reverse_iterator rbegin() {
      return reverse_iterator(ptr+m_size-1);
    }

    const_reverse_iterator rbegin() const {
      return const_reverse_iterator(ptr+m_size-1);
    }
    
    reverse_iterator rend() {
      return reverse_iterator(ptr-1);
    }

    const_reverse_iterator rend() const {
      return const_reverse_iterator(ptr-1);
    }
    
    T& front() {
      return ptr[0];
    }

    const T& front() const {
      return ptr[0];
    }
    
    T& back() {
      return ptr[m_size-1];
    }
    
    const T& back() const {
      return ptr[m_size-1];
    }
    
    T* data() {
      return ptr;
    }
    
    const T* data() const {
      return ptr;
    }
    
    T& at(const size_t &size) {
      return ptr[size];
    }
    
    const T& at(const size_t &size) const {
      return ptr[size];
    }
    
    T& operator[] (const size_t &size) {
      return ptr[size];
    }
    
    const T& operator[] (const size_t &size) const {
      return ptr[size];
    }
    
    vector<T>& operator=(const vector<T> &v) {
      if (m_capacity < v.size()) recreate(v.size());
      
      for (size_t i = 0; i < v.size(); ++i) {
        ptr[i] = v[i];
      }
      
      //copyDescriptor(v.updateData);
      if (descriptorSet().valid()) {
        descriptorSet()->update(descriptorSetIndex());
      }
    }

    DescriptorSet* descriptorSet() const {
      return buffer->descriptorSet();
    }
    
    size_t descriptorSetIndex() const {
      return buffer->descriptorSetIndex();
    }
    
    void setDescriptor(DescriptorSet* set, const size_t &index) {
      buffer->setDescriptor(set, index);
    }
  private:
    VkBufferUsageFlags usage;
    
    size_t m_capacity;
    size_t m_size;
    T* ptr;
    Device* device;
    Buffer* buffer;
    
    void recreate(const size_t &newCapacity) {
      T* oldPtr = ptr;
      size_t oldSize = m_size;

      Buffer* newBuffer = device->create(BufferCreateInfo::buffer(newCapacity * sizeof(T), usage), VMA_MEMORY_USAGE_CPU_ONLY);
      ptr = reinterpret_cast<T*>(newBuffer->ptr());

      // просто копируем или нужно вызывать конструкторы деструкторы?
      //memcpy(ptr, oldPtr, oldSize * sizeof(T));
      std::copy(oldPtr, oldPtr+oldSize, ptr);

      if (buffer->descriptorSet() != nullptr) {
        buffer->descriptorSet()->at(buffer->descriptorSetIndex()).bufferData.buffer = newBuffer;
        buffer->descriptorSet()->at(buffer->descriptorSetIndex()).bufferData.range = newCapacity * sizeof(T);

        newBuffer->setDescriptor(buffer->descriptorSet(), buffer->descriptorSetIndex());
      }

      std::_Destroy_aux<!std::is_trivially_destructible<T>::value>::__destroy(oldPtr, oldPtr + m_size);

      device->destroy(buffer);
      buffer = newBuffer;

//      buffer->resize(newCapacity * sizeof(T));
//      ptr = reinterpret_cast<T*>(buffer->ptr());

      m_capacity = newCapacity;

//      {
//        void* btarray[200];
//        size_t btSize;
//
//        // get void*'s for all entries on the stack
//        btSize = backtrace(btarray, 200);
//        // print out all the frames to stderr
////     fprintf(stderr, "Error: signal %d:\n", sig);
//        backtrace_symbols_fd(btarray, btSize, STDERR_FILENO);
//      }
    }
  };
  
  template <typename T, size_t N = 4096>
  class memory_pool {
  public:
    memory_pool(Device* device, const VkBufferUsageFlags &usage) 
    : usage(usage), ptr(nullptr), currentSlot_(nullptr), lastSlot_(nullptr), freeSlots_(SIZE_MAX), m_size(N), device(device), buffer(nullptr) {
      buffer = device->create(BufferCreateInfo::buffer(N, usage), VMA_MEMORY_USAGE_CPU_ONLY);
      
      currentSlot_ = reinterpret_cast<Slot_*>(buffer->ptr());
      lastSlot_ = reinterpret_cast<Slot_*>(reinterpret_cast<char*>(currentSlot_) + m_size);
    }
    
    memory_pool(memory_pool&& memoryPool) noexcept {
      device       = memoryPool.device;
      usage        = memoryPool.usage;
      ptr          = memoryPool.ptr;
      buffer       = memoryPool.buffer;
      currentSlot_ = memoryPool.currentSlot_;
      lastSlot_    = memoryPool.lastSlot_;
      freeSlots_   = memoryPool.freeSlots_;
      m_size       = memoryPool.m_size;
      
      memoryPool.ptr           = nullptr;
      memoryPool.buffer        = nullptr;
      memoryPool.currentSlot_  = nullptr;
      memoryPool.lastSlot_     = nullptr;
      memoryPool.freeSlots_    = SIZE_MAX;
      memoryPool.m_size        = 0;
    }
    
    ~memory_pool() noexcept {
      clear();
    }
    
    memory_pool& operator=(const memory_pool& memoryPool) = delete;
    
    memory_pool& operator=(memory_pool&& memoryPool) noexcept {
      if (this != &memoryPool) {
        device       = memoryPool.device;
        usage        = memoryPool.usage;
        ptr          = memoryPool.ptr;
        buffer       = memoryPool.buffer;
        currentSlot_ = memoryPool.currentSlot_;
        lastSlot_    = memoryPool.lastSlot_;
        freeSlots_   = memoryPool.freeSlots_;
        m_size       = memoryPool.m_size;
        
        memoryPool.ptr           = nullptr;
        memoryPool.buffer        = nullptr;
        memoryPool.currentSlot_  = nullptr;
        memoryPool.lastSlot_     = nullptr;
        memoryPool.freeSlots_    = SIZE_MAX;
        memoryPool.m_size        = 0;
      }

      return *this;
    }
    
    T* address(T& x) const {
        return &x;
    }

    const T* address(const T& x) const {
        return &x;
    }
    
    T* allocate(const size_t &n = 1, const T* hint = nullptr) {
      (void)n;
      (void)hint;
      
      if (freeSlots_ != SIZE_MAX) {
        const size_t index = reinterpret_cast<size_t>(freeSlots_);
        Slot_* result = reinterpret_cast<Slot_*>(ptr + index);
        
        freeSlots_ = result->next;

        return reinterpret_cast<T*>(result);
      } 
      
      if (currentSlot_ >= lastSlot_) allocateBlock();

      return reinterpret_cast<T*>(currentSlot_++);
    }

    void deallocate(T* p, const size_t &n = 1) {
      (void)n;
      if (p != nullptr) {
        reinterpret_cast<Slot_*>(p)->next = freeSlots_;
        freeSlots_ = p - ptr;// reinterpret_cast<Slot_*>(p);
      }
    }
    
    template <class U, class... Args> void construct(U* p, Args&&... args) {
      new (p) U (std::forward<Args>(args)...);
    }

    template <class U> void destroy(U* p) {
        p->~U();
    }

    template <class... Args> size_t newElement(Args&&... args)  {
      T* result = allocate();
      construct<T>(result, std::forward<Args>(args)...);
      //return result;
      return ptr - result;
    }
    
    void deleteElement(const size_t &i) {
      T* p = getPointer(i);
      if (p != nullptr) {
        p->~T();
        deallocate(p);
      }
    }

    void deleteElement(T* p) {
      if (p != nullptr) {
        p->~T();
        deallocate(p);
      }
    }
    
    T* getPointer(const size_t &index) {
      T* p = ptr + index;
      if (p >= lastSlot_) return nullptr;
      
      return p;
    }
    
    size_t getBufferSize() const {
      return m_size * sizeof(T);
    }
    
    void clear() {
      //vmaDestroyBuffer(device->bufferAllocator(), buffer, allocation);
      if (buffer != nullptr) {
        device->destroy(buffer);
        buffer = nullptr;
      }
      
      ptr = nullptr;
      currentSlot_ = nullptr;
      lastSlot_ = nullptr;
      freeSlots_ = SIZE_MAX;
      m_size = 0;
      device = nullptr;
    }
    
    DescriptorSet* descriptorSet() const {
      return buffer->descriptorSet();
    }
    
    size_t descriptorSetIndex() const {
      return buffer->descriptorSetIndex();
    }
    
    void setDescriptor(DescriptorSet* set, const size_t &index) {
      buffer->setDescriptor(set, index);
    }
  private:
    union Slot_ {
      T element;
      size_t next;
    };
    
    VkBufferUsageFlags usage;
    T* ptr;
    Slot_* currentSlot_;
    Slot_* lastSlot_;
    size_t freeSlots_;
    
    size_t m_size;
    Device* device;
    Buffer* buffer;
    
    size_t padPointer(void* p, const size_t &align) const noexcept {
      uintptr_t result = reinterpret_cast<uintptr_t>(p);
      return ((align - result) % align);
    }

    void allocateBlock() {
      // здесь нам не нужно выделять дополнительных 8 байт под указатель

      const size_t oldSize = m_size;
      const size_t size = m_size + N;
      recreate(oldSize, size);

      // немножко пододвинем указатель для того чтобы добиться выравнивания элементов
      size_t bodyPadding = 0; // здесь нам по идее не нужно выравнивание
      //size_t bodyPadding = padPointer(ptr, alignof(Slot_)); // получаем выравнивание
      currentSlot_ = reinterpret_cast<Slot_*>(reinterpret_cast<char*>(ptr) + oldSize + bodyPadding); // двигаем указатель
      lastSlot_ = reinterpret_cast<Slot_*>(reinterpret_cast<char*>(ptr) + m_size); // находим последний слот
    }
    
//     void create(const size_t &capacity, VkBuffer* buffer, VmaAllocation* allocation) {
//       VmaAllocationCreateInfo a{
//         VMA_ALLOCATION_CREATE_MAPPED_BIT,
//         VMA_MEMORY_USAGE_CPU_ONLY,
//         0,
//         0,
//         0,
//         VK_NULL_HANDLE,
//         nullptr
//       };
//       
//       VkBufferCreateInfo i{
//         VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
//         nullptr,
//         0,
//         capacity,
//         usage,
//         VK_SHARING_MODE_EXCLUSIVE,
//         0,
//         nullptr
//       };
//       
//       VmaAllocationInfo info;
//       vmaCreateBuffer(device->bufferAllocator(), &i, &a, buffer, allocation, &info);
//       
//       ptr = (T*)info.pMappedData;
//     }
    
    void recreate(const size_t &oldCapasity, const size_t &newCapasity) {
      Buffer* newBuffer = device->create(BufferCreateInfo::buffer(newCapasity, usage), VMA_MEMORY_USAGE_CPU_ONLY);
      ptr = reinterpret_cast<T*>(newBuffer->ptr());
      
      m_size = newCapasity;
      
      // просто копируем или нужно вызывать конструкторы деструкторы?
      memcpy(newBuffer->ptr(), buffer->ptr(), oldCapasity);
      
      if (buffer->descriptorSet() != nullptr) {
        buffer->descriptorSet()->at(buffer->descriptorSetIndex()).bufferData.buffer = newBuffer;
        buffer->descriptorSet()->at(buffer->descriptorSetIndex()).bufferData.range = newCapasity;
        
        newBuffer->setDescriptor(buffer->descriptorSet(), buffer->descriptorSetIndex());
      }
      
      device->destroy(buffer);
      
      buffer = newBuffer;
    }
    
//     void updateDescriptor() {
//       if (updateData.handle == VK_NULL_HANDLE) return;
//       if (updateData.bindingNum == UINT32_MAX) return;
//       if (updateData.arrayElement == UINT32_MAX) return;
//       if (updateData.type == VK_DESCRIPTOR_TYPE_MAX_ENUM) return;
//       
//       VkDescriptorBufferInfo i{
//         buffer,
//         0,
//         getBufferSize()
//       };
//       
//       VkWriteDescriptorSet w{
//         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//         nullptr,
//         updateData.handle,
//         updateData.bindingNum,
//         updateData.arrayElement,
//         1,
//         updateData.type,
//         nullptr,
//         &i,
//         nullptr
//       };
//       
//       vkUpdateDescriptorSets(device->handle(), 1, &w, 0, nullptr);
//     }
//     
//     void copyDescriptor(const DescriptorUpdate &other) {
//       if (updateData.handle == VK_NULL_HANDLE) return;
//       if (updateData.bindingNum == UINT32_MAX) return;
//       if (updateData.arrayElement == UINT32_MAX) return;
//       if (updateData.type == VK_DESCRIPTOR_TYPE_MAX_ENUM) return;
//       
//       if (other.handle == VK_NULL_HANDLE) return;
//       if (other.bindingNum == UINT32_MAX) return;
//       if (other.arrayElement == UINT32_MAX) return;
//       if (other.type == VK_DESCRIPTOR_TYPE_MAX_ENUM) return;
//       
//       VkCopyDescriptorSet copy{
//         VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
//         nullptr,
//         other.handle,
//         other.bindingNum,
//         other.arrayElement,
//         updateData.handle,
//         updateData.bindingNum,
//         updateData.arrayElement,
//         1
//       };
//       
//       vkUpdateDescriptorSets(device->handle(), 0, nullptr, 1, &copy);
//     }
  };
}

#endif
