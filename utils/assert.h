#ifndef ASSERT_H
#define ASSERT_H

#ifndef _NDEBUG
  #include <cassert>
  #define ASSERT(expr) assert(expr);
#else
  #define ASSERT(expr)
#endif

#endif
