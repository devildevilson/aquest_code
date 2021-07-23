#ifndef CONTEXT_H
#define CONTEXT_H

// namespace yavf {
//   class TransferTask;
//   class CombinedTask;
//   class ComputeTask;
//   class GraphicTask;
//   class TaskInterface;
// }

namespace vk {
  class CommandBuffer;
}

namespace devils_engine {
  namespace render {
//     class abstract_context {
//       virtual ~abstract_context() {}
//       virtual yavf::TaskInterface* interface() const = 0;
//     };
//     
//     class compute_context {
//       virtual ~compute_context() {}
//       virtual yavf::TaskInterface* interface() const = 0;
//     };
//     
//     class graphics_context {
//       virtual ~graphics_context() {}
//       virtual yavf::TaskInterface* interface() const = 0;
//     };
//     
//     class transfer_context {
//       virtual ~transfer_context() {}
//       virtual yavf::TaskInterface* interface() const = 0;
//     };
    
    class context {
    public:
      virtual ~context() {}
//       virtual yavf::TaskInterface* interface() const = 0;
//       virtual yavf::CombinedTask* combined() const = 0;
//       virtual yavf::ComputeTask* compute() const = 0;
//       virtual yavf::GraphicTask* graphics() const = 0;
//       virtual yavf::TransferTask* transfer() const = 0;
      virtual vk::CommandBuffer* command_buffer() const = 0;
    };
  }
}

#endif
