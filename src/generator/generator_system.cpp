// #include "generator_system.h"

// namespace devils_engine {
//   namespace systems {
//     generator::generator(const size_t &container_size) : container(container_size) {}
//     generator::~generator() {
//       for (auto ptr : generators) {
//         container.destroy(ptr);
//       }
//     }
//     
//     void generator::begin(const map::generator_data* data) {
//       for (auto ptr : generators) {
//         ptr->begin(data);
//       }
//     }
//     
//     void generator::generate(core::map* map, const map::generator_data* data) {
//       for (auto ptr : generators) {
//         ptr->process(map, data);
//       }
//     }
//     
//     void generator::clear() {
//       for (auto ptr : generators) {
//         ptr->clear();
//       }
//     }
//   }
// }
