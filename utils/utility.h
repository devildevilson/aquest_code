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

#endif
