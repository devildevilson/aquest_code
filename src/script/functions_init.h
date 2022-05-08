#ifndef DEVILS_ENGINE_SCRIPT_FUNCTIONS_INIT_H
#define DEVILS_ENGINE_SCRIPT_FUNCTIONS_INIT_H

namespace devils_engine {
  namespace script {
    class system;
    
    void register_functions(system* sys);
  }
}

#endif


// скрипты выглядят теперь как то так:
// "context:actor" = {
//   func1 = 123,
//   func2 = "title:abc.func3:1.23",
//   save_local = "abc123"
// }
