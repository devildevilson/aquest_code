#ifndef SHARED_TIME_CONSTANTS_H
#define SHARED_TIME_CONSTANTS_H

#define TARGET_CONSTANT_UPDATE_RATE 30

#define NANO_PRECISION    1000000000
#define MCS_PRECISION     1000000
#define MILI_PRECISION    1000
#define SECONDS_PRECISION 1

#ifdef __cplusplus
  #define NANO_CHRONO_TYPE    std::chrono::nanoseconds
  #define MCS_CHRONO_TYPE     std::chrono::microseconds
  #define MILI_CHRONO_TYPE    std::chrono::miliseconds
  #define SECONDS_CHRONO_TYPE std::chrono::seconds
  
  #define NANO_TIME_STRING "ns"
  #define MCS_TIME_STRING "mcs"
  #define MILI_TIME_STRING "ms"
  #define SECONDS_TIME_STRING "s"
  
  #define CHRONO_TIME_TYPE MCS_CHRONO_TYPE
  #define TIME_STRING MCS_TIME_STRING
#endif

#define TIME_PRECISION MCS_PRECISION

#define ONE_SECOND TIME_PRECISION
#define HALF_SECOND (ONE_SECOND/2)
#define THIRD_SECOND (ONE_SECOND/3)
#define QUARTER_SECOND (ONE_SECOND/4)
#define FIFTH_SECOND (ONE_SECOND/5)
#define TENTH_SECOND (ONE_SECOND/10)

// если у нас будет NANO_PRECISION то наверное лучше бы использовать double
#define MCS_TO_SEC(dt) (float(dt) / float(TIME_PRECISION))

#define ACCUMULATOR_MAX_CONSTANT FIFTH_SECOND
#define DELTA_TIME_CONSTANT (ONE_SECOND/TARGET_CONSTANT_UPDATE_RATE)

#endif
