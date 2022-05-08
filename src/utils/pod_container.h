#ifndef DEVILS_ENGINE_UTILS_POD_CONTAINER_H
#define DEVILS_ENGINE_UTILS_POD_CONTAINER_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include "parallel_hashmap/phmap.h"
#include "memory_pool.h"
#include "type_info.h"
#include "constexpr_funcs.h"

// можно попробовать еще сделать уникальность типа
// то есть обеспечить классу pod_container уникальность каждый раз когда создается объект типа
// тогда в нем мы можем заново сгенерировать уникальный id для типа и использовать его как индекс
// это по идее должно сократить время поиска по хешу в контейнере
// пока оставлю хеш мапу

namespace devils_engine {
  namespace utils {
    class pod_container {
    public:
      
      template <typename T, typename... Args>
      T* create(Args&& ...args) {
        auto cont_ptr = get_or_create_container<T>();
        auto cont = reinterpret_cast<pool_container<T>>(cont_ptr);
        return cont->create(std::forward<Args>(args)...);
      }
      
      template <typename T>
      void destroy(T* ptr) {
        auto cont_ptr = get_or_create_container<T>();
        auto cont = reinterpret_cast<pool_container<T>>(cont_ptr);
        cont->destroy(ptr);
      }
      
    private:
      class container_interface {
      public:
        container_interface(const uint64_t &id) : id(id) {}
        virtual ~container_interface() = default;
        uint64_t id;
      };
      
      template <typename T>
      class pool_container : public container_interface {
      public:
        inline pool_container(const uint64_t &id) : container_interface(id) {}
        ~pool_container() = default;
        pool_container(const pool_container &copy) = delete;
        pool_container & operator=(const pool_container &copy) = delete;
        pool_container(pool_container &&move) = default;
        pool_container & operator=(pool_container &&move) = default;
        
        template <typename... Args>
        T* create(Args&& ...args) {
          static_assert(std::is_pod_v<T>, "Pod container can handle only pod types");
          return pool.create(std::forward<Args>(args)...);
        }
        
        void destroy(T* ptr) {
          static_assert(std::is_pod_v<T>, "Pod container can handle only pod types");
          pool.destroy(ptr);
        }
      private:
        memory_pool<T, sizeof(T)*1000> pool;
      };
      
      // пока что единственный вариант это искать тем или иным образом контейнер в массиве
      // для того чтобы получить индекс на этапе компиляции пришлось бы задавать 
      // заранее все типы которые тут хранятся
      template <typename T>
      container_interface* get_or_create_container() {
        //const uint64_t hash = utils::type_id<T>(); // non constexpr
        const uint64_t hash = utils::type_hash<T>(); // constexpr
        auto itr = containers.find(hash);
        if (itr == containers.end()) {
          auto p = std::make_pair(hash, std::unique_ptr<container_interface>(new pool_container<T>(hash)));
          itr = containers.emplace(std::move(p)).first;
        }
        
        if (itr->second->id != hash) throw std::runtime_error("Container value type " + std::string(utils::type_name<T>()) + " collision");
        return itr->second.get();
      }
      
      // по идее писать сложный деструктор тут ни к чему
      phmap::flat_hash_map<uint64_t, std::unique_ptr<container_interface>> containers;
    };
  }
}

#endif
