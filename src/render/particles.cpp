// #include "particles.h"
// 
// #include "Frustum.h"
// 
// namespace devils_engine {
//   namespace render {
//     //particle::color::color() : container(0) {}
//     //particle::color::color(uint32_t color) : container(color) {}
//     //particle::color::color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) : r(r), g(g), b(b), a(a) {}
//     particle::flags::flags() : container(0) {}
//     particle::flags::flags(const uint32_t &container) : container(container) {}
//     bool particle::flags::min_speed_stop() const { return (container & MIN_SPEED_STOP) == MIN_SPEED_STOP; }
//     bool particle::flags::min_speed_remove() const { return (container & MIN_SPEED_REMOVE) == MIN_SPEED_REMOVE; }
//     bool particle::flags::max_speed_stop() const { return (container & MAX_SPEED_STOP) == MAX_SPEED_STOP; }
//     bool particle::flags::max_speed_remove() const { return (container & MAX_SPEED_REMOVE) == MAX_SPEED_REMOVE; }
//     bool particle::flags::speed_dec_over_time() const { return (container & SPEED_DEC_OVER_TIME) == SPEED_DEC_OVER_TIME; }
//     bool particle::flags::speed_inc_over_time() const { return (container & SPEED_INC_OVER_TIME) == SPEED_INC_OVER_TIME; }
//     bool particle::flags::scale_dec_over_time() const { return (container & SCALE_DEC_OVER_TIME) == SCALE_DEC_OVER_TIME; }
//     bool particle::flags::scale_inc_over_time() const { return (container & SCALE_INC_OVER_TIME) == SCALE_INC_OVER_TIME; }
//     particle::particle() :
//       image{UINT32_MAX},
//       life_time(0),
//       current_time(0),
//       max_scale(0.1f),
//       min_scale(0.0f),
//       gravity(1.0f),
//       bounce(1.0f),
//       max_speed(10000.0f),
//       min_speed(0.0f),
//       colorUINT32_MAX),
//       friction(1.0f)
//     {
//       const uint32_t mask = 0x7fffffff;
//       this->life_time = this->life_time & mask;
//     }
//     
//     particle::particle(const simd::vec4 &pos, const simd::vec4 &vel, const struct image &image, const uint32_t &life_time) : 
//       pos(pos), 
//       vel(vel), 
//       image(image), 
//       life_time(life_time), 
//       current_time(0),
//       max_scale(0.1f),
//       min_scale(0.0f),
//       gravity(1.0f),
//       bounce(1.0f),
//       max_speed(10000.0f),
//       min_speed(0.0f),
//       color(UINT32_MAX),
//       friction(1.0f)
//     {
//       const uint32_t mask = 0x7fffffff;
//       this->life_time = this->life_time & mask;
//     }
//     
//     particle::particle(const simd::vec4 &pos, const simd::vec4 &vel, const struct color &color, const uint32_t &life_time) : 
//       pos(pos), 
//       vel(vel), 
//       image(render::image{UINT32_MAX}), 
//       life_time(life_time), 
//       current_time(0),
//       max_scale(0.1f),
//       min_scale(0.0f),
//       gravity(1.0f),
//       bounce(1.0f),
//       max_speed(10000.0f),
//       min_speed(0.0f),
//       color(color),
//       friction(1.0f)
//     {
//       const uint32_t mask = 0x7fffffff;
//       this->life_time = this->life_time & mask;
//     }
//     
//     particles::particles(yavf::Device* device) : 
//       next_index(0), 
//       array(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), 
//       new_particles(device, yavf::BufferCreateInfo::buffer(sizeof(particle)*max_new_particles, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY),
//       data_buffer(device, yavf::BufferCreateInfo::buffer(sizeof(particles_data), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY),
//       indices(device, yavf::BufferCreateInfo::buffer(sizeof(VkDrawIndirectCommand) + sizeof(uint32_t)*max_new_particles, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT), VMA_MEMORY_USAGE_GPU_ONLY)
//     {
//       ASSERT(sizeof(particle) == sizeof(gpu_particle));
//       ASSERT(alignof(particle) == alignof(gpu_particle));
//       auto data = reinterpret_cast<particles_data*>(data_buffer.ptr());
//       data->frame_time = 0;
//       data->new_count = 0;
//       data->old_count = 0;
//       data->particles_count = 0;
//       data->new_max_count = 0;
//       data->indices_count = 0;
//       
//       yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
//       {
//         yavf::DescriptorLayoutMaker dlm(device);
//         layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                     .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                     .binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
//                     .binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL).create("particles_layout");
//       }
//       
//       {
//         yavf::DescriptorMaker dm(device);
//         
//         auto d = dm.layout(layout).create(pool)[0];
//         size_t i = d->add({array.vector().handle(), 0, array.vector().buffer_size(), 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         array.vector().setDescriptor(d, i);
//         d->add({&new_particles, 0, new_particles.info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         d->add({&data_buffer, 0, data_buffer.info().size, 0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         i = d->add({&indices, 0, indices.info().size, 0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
//         indices.setDescriptor(d, i);
//         d->update();
//       }
//     }
//     
//     bool particles::add(const struct particle &particle) {
//       const uint32_t index = next_index.fetch_add(1);
//       if (index >= max_new_particles) return false;
//       auto particles_ptr = reinterpret_cast<struct particle*>(new_particles.ptr());
//       particles_ptr[index] = particle;
//       return true;
//     }
//     
//     void particles::reset() {
//       next_index = 0;
//       memset(new_particles.ptr(), 0, new_particles.info().size); // ?
//     }
//     
//     void particles::update(const simd::mat4 &view_proj, const simd::vec4 &gravity, const size_t &time) {
//       const Frustum f(view_proj);
//       auto data = reinterpret_cast<particles_data*>(data_buffer.ptr());
//       data->frame_time = time;
//       data->gravity = gravity;
//       for (uint32_t i = 0; i < 6; ++i) {
//         data->frustum[i] = f.planes[i];
//       }
//     }
//     
//     void particles::recreate(const uint32_t &width, const uint32_t &height) { (void)width; (void)height; }
//   }
// }

#include "particles.h"

#include "utils/frustum.h"
#include "yavf.h"
#include "shared_particles_structures.h"

namespace devils_engine {
  namespace render {
    //particle::color::color() : container(0) {}
    //particle::color::color(uint32_t color) : container(color) {}
    //particle::color::color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) : r(r), g(g), b(b), a(a) {}
    particle::flags::flags() : container(0) {}
    particle::flags::flags(const uint32_t &container) : container(container) {}
    bool particle::flags::min_speed_stop() const { return (container & MIN_SPEED_STOP) == MIN_SPEED_STOP; }
    bool particle::flags::min_speed_remove() const { return (container & MIN_SPEED_REMOVE) == MIN_SPEED_REMOVE; }
    bool particle::flags::max_speed_stop() const { return (container & MAX_SPEED_STOP) == MAX_SPEED_STOP; }
    bool particle::flags::max_speed_remove() const { return (container & MAX_SPEED_REMOVE) == MAX_SPEED_REMOVE; }
    bool particle::flags::speed_dec_over_time() const { return (container & SPEED_DEC_OVER_TIME) == SPEED_DEC_OVER_TIME; }
    bool particle::flags::speed_inc_over_time() const { return (container & SPEED_INC_OVER_TIME) == SPEED_INC_OVER_TIME; }
    bool particle::flags::scale_dec_over_time() const { return (container & SCALE_DEC_OVER_TIME) == SCALE_DEC_OVER_TIME; }
    bool particle::flags::scale_inc_over_time() const { return (container & SCALE_INC_OVER_TIME) == SCALE_INC_OVER_TIME; }
    particle::particle() :
      image{UINT32_MAX},
      life_time(0),
      current_time(0),
      max_scale(0.1f),
      min_scale(0.0f),
      gravity(1.0f),
      bounce(1.0f),
      max_speed(10000.0f),
      min_speed(0.0f),
      color{UINT32_MAX},
      friction(1.0f)
    {
      const uint32_t mask = 0x7fffffff;
      this->life_time = this->life_time & mask;
    }
    
    particle::particle(const render::vec4 &pos, const render::vec4 &vel, const render::image_t &image, const uint32_t &life_time) : 
      pos(pos), 
      vel(vel), 
      image(image), 
      life_time(life_time), 
      current_time(0),
      max_scale(0.1f),
      min_scale(0.0f),
      gravity(1.0f),
      bounce(1.0f),
      max_speed(10000.0f),
      min_speed(0.0f),
      color{UINT32_MAX},
      friction(1.0f)
    {
      const uint32_t mask = 0x7fffffff;
      this->life_time = this->life_time & mask;
    }
    
    particle::particle(const render::vec4 &pos, const render::vec4 &vel, const render::color_t &color, const uint32_t &life_time) : 
      pos(pos), 
      vel(vel), 
      image(render::image_t{UINT32_MAX}), 
      life_time(life_time), 
      current_time(0),
      max_scale(0.1f),
      min_scale(0.0f),
      gravity(1.0f),
      bounce(1.0f),
      max_speed(10000.0f),
      min_speed(0.0f),
      color(color),
      friction(1.0f)
    {
      const uint32_t mask = 0x7fffffff;
      this->life_time = this->life_time & mask;
    }
    
    particles::particles(yavf::Device* device) : 
      next_index(0), 
      particles_array(nullptr), 
      new_particles(nullptr),
      data_buffer(nullptr),
      indices(nullptr),
      set(nullptr)
    {
      particles_array = device->create(yavf::BufferCreateInfo::buffer(sizeof(particle)*max_new_particles, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      new_particles   = device->create(yavf::BufferCreateInfo::buffer(sizeof(particle)*max_new_particles, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY);
      data_buffer     = device->create(yavf::BufferCreateInfo::buffer(sizeof(particles_data), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT), VMA_MEMORY_USAGE_CPU_ONLY); // ???
      indices         = device->create(
        yavf::BufferCreateInfo::buffer(
          sizeof(VkDrawIndirectCommand) + sizeof(uint32_t)*max_new_particles,
          VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT), 
        VMA_MEMORY_USAGE_GPU_ONLY
      );
      
      ASSERT(sizeof(particle) == sizeof(gpu_particle_t));
      ASSERT(alignof(particle) == alignof(gpu_particle_t));
      auto data = reinterpret_cast<particles_data*>(data_buffer->ptr());
      data->frame_time = 0;
      data->new_count = 0;
      data->old_count = 0;
      data->particles_count = 0;
      data->new_max_count = 0;
      data->indices_count = 0;
      
      yavf::DescriptorSetLayout layout = VK_NULL_HANDLE;
      yavf::DescriptorPool pool = device->descriptorPool(DEFAULT_DESCRIPTOR_POOL_NAME);
      {
        yavf::DescriptorLayoutMaker dlm(device);
        layout = dlm.binding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                    .binding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                    .binding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL)
                    .binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL).create("particles_layout");
      }
      
      {
        yavf::DescriptorMaker dm(device);
        
        set = dm.layout(layout).create(pool)[0];
        set->add({particles_array, 0, particles_array->info().size, 0, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->add({new_particles, 0, new_particles->info().size, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->add({data_buffer, 0, data_buffer->info().size, 0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->add({indices, 0, indices->info().size, 0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER});
        set->update();
      }
    }
    
    bool particles::add(const struct particle &particle) {
      const uint32_t index = next_index.fetch_add(1);
      if (index >= max_new_particles) return false;
      auto particles_ptr = reinterpret_cast<struct particle*>(new_particles->ptr());
      particles_ptr[index] = particle;
      return true;
    }
    
    void particles::reset() {
      next_index = 0;
      memset(new_particles->ptr(), 0, new_particles->info().size); // ?
    }
    
    void particles::update(const glm::mat4 &view_proj, const render::vec4 &gravity, const size_t &time) {
      const utils::frustum f = utils::compute_frustum(view_proj);
      auto data = reinterpret_cast<particles_data*>(data_buffer->ptr());
      data->frame_time = time;
      data->gravity = gravity;
      for (uint32_t i = 0; i < 6; ++i) {
        data->frustum[i] = f.planes[i];
      }
    }
    
    void particles::recreate(const uint32_t &width, const uint32_t &height) { (void)width; (void)height; }
  }
}
