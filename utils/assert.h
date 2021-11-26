#ifndef ASSERT_H
#define ASSERT_H

#include <cassert>

#ifndef _NDEBUG
  #define ASSERT(expr) assert(expr);
#else
  #define ASSERT(expr)
#endif

#endif
