#ifndef DEVILS_ENGINE_CORE_SECRET_TYPES_H
#define DEVILS_ENGINE_CORE_SECRET_TYPES_H

#define SECRET_TYPES_LIST \
  SECRET_TYPE_FUNC(attempted_murder) \
  SECRET_TYPE_FUNC(murderer) \
  SECRET_TYPE_FUNC(cannibal) \
  SECRET_TYPE_FUNC(deviant) \
  SECRET_TYPE_FUNC(sodomite) \
  SECRET_TYPE_FUNC(incestuous) \
  SECRET_TYPE_FUNC(non_believer) \
  SECRET_TYPE_FUNC(witch) \
  SECRET_TYPE_FUNC(illegitimate_child) \
  SECRET_TYPE_FUNC(adulterer) \
  SECRET_TYPE_FUNC(mage) /* быт магом может осуждаться */ \
  SECRET_TYPE_FUNC(power_service_user) /* использовать услуги предоставляемые глобальной силой (например апати) может быть запрещено */
  
namespace devils_engine {
  namespace core {
    namespace secret_types {
      enum values {
#define SECRET_TYPE_FUNC(val) val,
        SECRET_TYPES_LIST
#undef SECRET_TYPE_FUNC
        
        count
      };
    }
  }
}

#endif
