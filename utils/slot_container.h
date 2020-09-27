#ifndef SLOT_CONTAINER_H
#define SLOT_CONTAINER_H

#include <cstdint>
#include <cstddef>
#include <initializer_list>
#include <cmath>
#include <stdexcept>
#include <string>
#include <cstring>

#ifdef _WIN32
  #define SIZE_WIDTH 64
#endif

namespace devils_engine {
  namespace utils {
    class slot_container {
    public:
      using init_bit_field_type = size_t;
      static const size_t field_width = SIZE_WIDTH;
      static const size_t current_supported_aligment = 8;

      inline slot_container(const std::initializer_list<size_t> &slots) : mem_size(0), memory(nullptr), initialized_ptr(nullptr), slots_size(slots.size()), slots(new size_t[slots.size()]) {
        for (const auto &val : slots) mem_size += align_to(val, current_supported_aligment);
        memcpy(this->slots, slots.begin(), sizeof(this->slots[0]) * slots_size);
        //init();
      }

      inline ~slot_container() {
        clear();
        delete [] slots;
      }

      template <typename T, size_t slot_num, typename... Args>
      T* create(Args&&... args) {
        static_assert(alignof(T) <= current_supported_aligment, "Do not use with 16 aligment data");
        if (slot_num >= slots_size) throw std::runtime_error("slot_num >= slot.size()");
        if (sizeof(T) != slots[slot_num]) throw std::runtime_error("Trying to initialize value to slot with different size");
        if (is_initialized(slot_num)) throw std::runtime_error("Slot " + std::to_string(slot_num) + " is already initialized");
        size_t current_place = 0;
        size_t final_slot = slot_num;
        for (size_t i = 0; i < final_slot; ++i) current_place += align_to(slots[i], current_supported_aligment);
        initialize(slot_num, true);
        auto ptr = &memory[current_place];
        auto valid_data = new (ptr) T(std::forward<Args>(args)...);
        return valid_data;
      }

      template <typename T>
      void destroy(T* ptr) {
        const auto diff = reinterpret_cast<char*>(ptr) - memory;
        if (diff < 0 || diff > int64_t(mem_size)) throw std::runtime_error("Bad data pointer");
        size_t counter = slots[0];
        size_t slot_index = 0;
        while (size_t(diff) >= counter) {
          ++slot_index;
          counter += align_to(slots[slot_index], current_supported_aligment);
        }

        if (!is_initialized(slot_index)) throw std::runtime_error("Trying to remove not initialized data");

        ptr->~T();
        initialize(slot_index, false);
      }

      inline size_t slots_count() const { return slots_size; }
      inline size_t memory_size() const { return mem_size; }

      inline bool is_initialized(const size_t &index) const {
        if (slots_size < field_width) {
          const size_t mask = size_t(1) << index;
          return (initialized & mask) == mask;
        }

        const size_t field_index = index / field_width;
        const size_t data_index  = index % field_width;
        const size_t mask = size_t(1) << data_index;
        return (initialized_ptr[field_index] & mask) == mask;
      }

      inline bool inited() const { return memory != nullptr; }
      inline void init() {
        memory = new char[mem_size];
        memset(memory, 0, sizeof(memory[0]) * mem_size);
        init_bit_field();
      }

      inline void clear() {
        if (slots_size >= field_width) delete [] initialized_ptr;
        initialized = 0;
        initialized_ptr = nullptr;
        delete [] memory;
        memory = nullptr;
      }
    private:
      size_t mem_size;
      char* memory;
      union {
        init_bit_field_type* initialized_ptr;
        init_bit_field_type initialized;
      };
      size_t slots_size;
      size_t* slots;

      inline void init_bit_field() {
        if (slots_size < field_width) return;
        const size_t bit_field_size = size_t(std::ceil(double(slots_size) / double(field_width)));
        initialized_ptr = new init_bit_field_type[bit_field_size];
        memset(initialized_ptr, 0, sizeof(initialized_ptr[0]) * bit_field_size);
      }

      inline void initialize(const size_t &index, const bool value) {
        if (slots_size < field_width) {
          const size_t mask = size_t(1) << index;
          initialized = value ? initialized | mask : initialized & ~(mask);
          return;
        }

        const size_t field_index = index / field_width;
        const size_t data_index  = index % field_width;
        const size_t mask = size_t(1) << data_index;
        initialized_ptr[field_index] = value ? initialized_ptr[field_index] | mask : initialized_ptr[field_index] & ~(mask);
      }

      inline size_t align_to(const size_t &value, const size_t aligment) const {
        return (value + aligment - 1) / aligment * aligment;
      }
    };
  }
}

#endif
