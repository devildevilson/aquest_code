#include "stages.h"

#include <array>
#include <iostream>

#include "window.h"
#include "container.h"
#include "targets.h"
#include "image_controller.h"
#include "image_container.h"
#include "makers.h"
#include "map_data.h"
#include "defines.h"
#include "pass.h"
#include "push_constant_data.h"

#include "utils/globals.h"
#include "utils/utility.h"
#include "utils/input.h"
#include "utils/systems.h"
#include "utils/frustum.h"
#include "utils/shared_time_constant.h"

#include "bin/interface_context.h"
#include "bin/interface2.h"

namespace devils_engine {
  namespace render {
    void set_event_cmd(vk::CommandBuffer task, vk::Event event) {
      task.setEvent(event, vk::PipelineStageFlagBits::eTransfer); // не понимаю когда это вызывается совсем =(
//       std::cout << "set_event_cmd" << "\n";
    }

    // виндовс не дает использовать базовый offsetof
#ifndef _WIN32
#define offsetof123(s,m) offsetof(s,m)
#else
#define offsetof123(s,m) ((::size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))    
#endif

    struct gui_vertex {
      glm::vec2 pos;
      glm::vec2 uv;
      uint32_t color;
    };

#define MAX_LAYERS_COUNT 128
#define MAX_VERTEX_BUFFER_SIZE (2 * 1024 * 1024)
#define MAX_INDEX_BUFFER_SIZE (512 * 1024)
#define MAX_LAYERS_BUFFER_SIZE (MAX_LAYERS_COUNT * sizeof(render::heraldy_layer_t))

    interface_stage::interface_stage(const create_info &info) :
      device(info.cont->vulkan->device),
      allocator(info.cont->vulkan->buffer_allocator),
      maximum_layers(MAX_LAYERS_COUNT),
      layers_counter(0)
    {
      vertex_gui.create(allocator, buffer(MAX_VERTEX_BUFFER_SIZE, vk::BufferUsageFlagBits::eVertexBuffer), vma::MemoryUsage::eCpuOnly, "interface_stage::vertex_gui");
      index_gui.create(allocator, buffer(MAX_INDEX_BUFFER_SIZE, vk::BufferUsageFlagBits::eIndexBuffer), vma::MemoryUsage::eCpuOnly, "interface_stage::index_gui");
      matrix.create(allocator, buffer(sizeof(glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer), vma::MemoryUsage::eCpuOnly, "interface_stage::matrix");
      image_layers_buffer.create(allocator, buffer(MAX_LAYERS_BUFFER_SIZE, vk::BufferUsageFlagBits::eStorageBuffer), vma::MemoryUsage::eCpuOnly, "interface_stage::image_layers_buffer");

      auto uniform_layout = info.cont->vulkan->uniform_layout;
      auto images_layout = info.images_layout;
      auto storage_layout = info.cont->vulkan->storage_layout;
      ASSERT(global::get<render::buffers>() != nullptr);
      auto heraldy_layout = global::get<render::buffers>()->heraldy_layout;
      images_set = *global::get<systems::core_t>()->image_controller->get_descriptor_set();

      {
        // вообще плохо тут использовать общий пул, но с другой стороны эта штука создается и удаляется только один раз за программу
        auto pool = info.cont->vulkan->descriptor_pool;

        descriptor_set_maker dm(&device);
        matrix_set = dm.layout(uniform_layout).create(pool, "interface_stage::matrix_set")[0];
        layers_set = dm.layout(storage_layout).create(pool, "interface_stage::layers_set")[0];

        descriptor_set_updater dsu(&device);
        dsu.currentSet(matrix_set).begin(0, 0, vk::DescriptorType::eUniformBuffer).buffer(matrix.handle).update();
        dsu.currentSet(layers_set).begin(0, 0, vk::DescriptorType::eStorageBuffer).buffer(image_layers_buffer.handle).update();
      }

      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout)
                      .addDescriptorLayout(images_layout)
                      .addDescriptorLayout(heraldy_layout)
                      .addDescriptorLayout(storage_layout)
                      .addPushConstRange(0, sizeof(push_constant_data))
                      .create("gui_layout");
      }

      {
        pipeline_maker pm(&device);
        pm.clearBlending();

        const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/gui.vert.spv");
        const auto fragment = create_shader_module(device, global::root_directory() + "shaders/gui.frag.spv");

        pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                 .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                 .vertexBinding(0, sizeof(gui_vertex))
                   .vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(gui_vertex, pos))
                   .vertexAttribute(1, 0, vk::Format::eR32G32Sfloat, offsetof(gui_vertex, uv))
                   .vertexAttribute(2, 0, vk::Format::eR8G8B8A8Uint, offsetof(gui_vertex, color))
                 .assembly(vk::PrimitiveTopology::eTriangleList)
                 .depthTest(VK_FALSE)
                 .depthWrite(VK_FALSE)
                 .clearBlending()
                 .colorBlendBegin()
                   .srcColor(vk::BlendFactor::eSrcAlpha)
                   .dstColor(vk::BlendFactor::eOneMinusSrcAlpha)
                   .srcAlpha(vk::BlendFactor::eOneMinusSrcAlpha)
                   .dstAlpha(vk::BlendFactor::eSrcAlpha)
                 .viewport()
                 .scissor()
                 .dynamicState(vk::DynamicState::eViewport).dynamicState(vk::DynamicState::eScissor)
                 .create("gui_pipeline", p_layout, info.pass->get_handle(), info.subpass_index);
      }
    }

    interface_stage::~interface_stage() {
      vertex_gui.destroy(allocator);
      index_gui.destroy(allocator);
      matrix.destroy(allocator);
      image_layers_buffer.destroy(allocator);
      device.destroy(p_layout);
      device.destroy(pipe);
    }

    void interface_stage::begin(resource_provider*) {
      auto data = global::get<devils_engine::interface::context>();
      auto w = global::get<render::window>();

      {
        void* vertices = vertex_gui.ptr;
        void* elements = index_gui.ptr;

        /* fill convert configuration */
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
          {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, offsetof(gui_vertex, pos)},
          {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, offsetof(gui_vertex, uv)},
          {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, offsetof(gui_vertex, color)},
          {NK_VERTEX_LAYOUT_END}
        };

        // нужен ли мне изменяющийся антиаляисинг?
        const nk_convert_config config{
          1.0f,
          NK_ANTI_ALIASING_OFF,
          NK_ANTI_ALIASING_OFF,
          22,
          22,
          22,
          data->null, // нулл текстура
          vertex_layout,
          sizeof(gui_vertex), // размер вершины
          alignof(gui_vertex) // алигн вершины
        };

        nk_buffer vbuf, ebuf;
        // мы создаем местные аналоги vkBuffer, то есть мета объект для памяти
        // а затем конвертим вершины в удобный нам формат
        nk_buffer_init_fixed(&vbuf, vertices, MAX_VERTEX_BUFFER_SIZE);
        nk_buffer_init_fixed(&ebuf, elements, MAX_INDEX_BUFFER_SIZE);
        nk_convert(&data->ctx, &data->cmds, &vbuf, &ebuf, &config);
      }

      const auto [width, height] = w->size();
      glm::mat4* mat = reinterpret_cast<glm::mat4*>(matrix.ptr);
      *mat = glm::mat4(
        2.0f / float(width),  0.0f,  0.0f,  0.0f,
        0.0f,  2.0f / float(height),  0.0f,  0.0f,
        0.0f,  0.0f, -1.0f,  0.0f,
        -1.0f, -1.0f,  0.0f,  1.0f
      );

      const nk_draw_command* cmd = nullptr;
      nk_draw_foreach(cmd, &data->ctx, &data->cmds) {
        const interface_draw_command command{
          cmd->elem_count,
          { cmd->clip_rect.x, cmd->clip_rect.y, cmd->clip_rect.w, cmd->clip_rect.h },
          cmd->texture.ptr,
          cmd->userdata.ptr
        };

        commands.push_back(command);
      }

      nk_buffer_clear(&data->cmds);
      nk_clear(&data->ctx);
    }

    bool interface_stage::process(resource_provider* ctx, vk::CommandBuffer task) {
      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      task.bindPipeline(bind_point, pipe);
      task.bindVertexBuffers(0, vertex_gui.handle, {0});
      task.bindIndexBuffer(index_gui.handle, 0, vk::IndexType::eUint16);

      auto heraldy_set = ctx->get_descriptor_set(string_hash(HERALDY_BUFFERS_DESCRIPTOR_SET_NAME));
      //auto heraldy = global::get<render::buffers>()->heraldy_set;
      task.bindDescriptorSets(bind_point, p_layout, 0, {matrix_set, images_set, heraldy_set, layers_set}, nullptr);

      uint32_t index_offset = 0;
      for (const auto &cmd : commands) {
        if (cmd.elem_count == 0) continue;
        
        const auto push_data = cmd.texture == nullptr ? 
          render::push_constant_data{ GPU_UINT_MAX, GPU_UINT_MAX, GPU_UINT_MAX, GPU_UINT_MAX } : 
          *reinterpret_cast<const render::push_constant_data*>(cmd.texture);
        //const image_handle_data data = nk_handle_to_image_data(nk_handle{cmd.texture});
        task.pushConstants(p_layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(push_data), &push_data);
        const glm::vec2 fb_scale = input::get_input_data()->fb_scale;
        const vk::Rect2D scissor{
          {
            int32_t(std::max(cmd.clip_rect.x * fb_scale.x, 0.0f)),
            int32_t(std::max(cmd.clip_rect.y * fb_scale.y, 0.0f))
          },
          {
            uint32_t(cmd.clip_rect.w * fb_scale.x),
            uint32_t(cmd.clip_rect.h * fb_scale.y)
          }
        };

        task.setScissor(0, scissor);
        task.drawIndexed(cmd.elem_count, 1, index_offset, 0, 0);
        index_offset += cmd.elem_count;
      }
      
      return true;
    }

    void interface_stage::clear() {
      commands.clear();
      layers_counter = 0;
    }
    
    uint32_t interface_stage::add_layers(const size_t &count, const render::heraldy_layer_t* layers) {
      const uint32_t start = layers_counter.fetch_add(count);
      if (start > maximum_layers || maximum_layers - start < count) throw std::runtime_error("Could not place image layers in buffer");
      auto buffer_ptr = reinterpret_cast<render::heraldy_layer_t*>(image_layers_buffer.ptr);
      memcpy(&buffer_ptr[start], layers, sizeof(layers[0])*count);
      return start;
    }
    
    skybox::skybox(container* cont, class pass* pass, const uint32_t &subpass_index) : device(cont->vulkan->device) {
      auto uniform_layout = cont->vulkan->uniform_layout;
      
      {
        pipeline_layout_maker plm(&device);
        p_layout = plm.addDescriptorLayout(uniform_layout).create("skybox_pipeline_layout");
      }
      
      // теперь везде передаем специальный объект чтобы получить хендл рендерпасса
      {
        pipeline_maker pm(&device);
        pm.clearBlending();

        const auto vertex   = create_shader_module(device, global::root_directory() + "shaders/skybox.vert.spv");
        const auto fragment = create_shader_module(device, global::root_directory() + "shaders/skybox.frag.spv");

        pipe = pm.addShader(vk::ShaderStageFlagBits::eVertex, vertex.get())
                 .addShader(vk::ShaderStageFlagBits::eFragment, fragment.get())
                 .assembly(vk::PrimitiveTopology::eTriangleList)
                 .depthTest(VK_FALSE)
                 .depthWrite(VK_FALSE)
                 .colorBlendBegin(VK_FALSE)
                   .colorWriteMask(DEFAULT_COLOR_WRITE_MASK)
                 .frontFace(vk::FrontFace::eClockwise)
                 //.frontFace(vk::FrontFace::eCounterClockwise)
                 .cullMode(vk::CullModeFlagBits::eNone)
                 .viewport()
                 .scissor()
                 .dynamicState(vk::DynamicState::eViewport).dynamicState(vk::DynamicState::eScissor)
                 .create("skybox_pipeline", p_layout, pass->get_handle(), subpass_index);
      }
    }
    
    skybox::~skybox() {
      device.destroy(pipe);
      device.destroy(p_layout);
    }
    
    void skybox::begin(resource_provider*) {}
    bool skybox::process(resource_provider* ctx, vk::CommandBuffer task) {
      const auto bind_point = vk::PipelineBindPoint::eGraphics;
      auto uniform_set = ctx->get_descriptor_set(string_hash(UNIFORM_BUFFERS_DESCRIPTOR_SET_NAME));
      task.bindPipeline(bind_point, pipe);
      task.bindDescriptorSets(bind_point, p_layout, 0, {uniform_set}, nullptr);
      task.draw(12 * 3, 1, 0, 0);
      return true;
    }
    void skybox::clear() {}
    
    void secondary_buffer_subpass::begin(resource_provider*) {}
    bool secondary_buffer_subpass::process(resource_provider* ctx, vk::CommandBuffer task) {
      auto buf = ctx->get_command_buffer(string_hash(ENVIRONMENT_COMMAND_BUFFER_NAME));
      task.nextSubpass(vk::SubpassContents::eSecondaryCommandBuffers);
      task.executeCommands(buf ? 1 : 0, &buf);
      task.nextSubpass(vk::SubpassContents::eInline);
      return true;
    }
    
    void secondary_buffer_subpass::clear() {}
  }
}
