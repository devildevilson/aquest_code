#ifndef UTILITY_H
#define UTILITY_H

#include <cstdint>
#include <cstddef>

#include "shared_time_constant.h"
#include "shared_application_constant.h"
#include "shared_mathematical_constant.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>

#include "assert.h"

#define PRINT_VEC4(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w << "\n";
#define PRINT_VEC3(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << " z: " << vec.z << "\n";
#define PRINT_VEC2(name, vec) std::cout << name << " x: " << vec.x << " y: " << vec.y << "\n";
#define PRINT_VAR(name, var) std::cout << name << ": " << var << "\n";
#define PRINT(var) std::cout << var << "\n";
#define DELETE_PTR(ptr) delete ptr; ptr = nullptr;
#define DELETE_ARR(arr) delete [] arr; arr = nullptr;
#define STRINGIFY(a) #a
#define CONCAT(a, b) a##b
#define UNUSED_VARIABLE(var) ((void)var)

namespace devils_engine {
  inline double cast_to_double(const int64_t &val) {
    union conv {double d; int64_t i; };
    conv c{.i = val};
    return c.d;
  }

  inline int64_t cast_to_int64(const double &val) {
    union conv {double d; int64_t i; };
    conv c{.d = val};
    return c.i;
  }

  inline float cast_to_float(const int32_t &val) {
    union conv {float d; int32_t i; };
    conv c{.i = val};
    return c.d;
  }

  inline int32_t cast_to_int32(const float &val) {
    union conv {float d; int32_t i; };
    conv c{.d = val};
    return c.i;
  }
  
  inline int32_t uns_to_signed32(const uint32_t &val) {
    union conv { uint32_t u; int32_t i; };
    conv c{.u = val};
    return c.i;
  }
  
  inline uint32_t s_to_unsigned32(const int32_t &val) {
    union conv { uint32_t u; int32_t i; };
    conv c{.i = val};
    return c.u;
  }
  
  inline int64_t uns_to_signed64(const uint64_t &val) {
    union conv { uint64_t u; int64_t i; };
    conv c{.u = val};
    return c.i;
  }
  
  inline uint64_t s_to_unsigned64(const int64_t &val) {
    union conv { uint64_t u; int64_t i; };
    conv c{.i = val};
    return c.u;
  }
}

#endif
