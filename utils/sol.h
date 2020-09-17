#ifndef SOL_H
#define SOL_H

#ifndef _NDEBUG
  #define SOL_SAFE_USERTYPE 1
  #define SOL_SAFE_REFERENCES 1
  #define SOL_SAFE_FUNCTION_CALLS 1
  //#define SOL_SAFE_NUMERICS 1
  #define SOL_SAFE_GETTER 1
  #define SOL_SAFE_FUNCTION_CALLS 1
  //#define SOL_ALL_SAFETIES_ON 1
  #include <sol/sol.hpp>
#else // release
  #include <sol/sol.hpp>
#endif

#endif
