//#include "helper.h"
#include "application.h"

using namespace devils_engine;

// зачем мне это разделение? может быть потом что нибудь добавить походу можно будет

int main(int argc, char const *argv[]) {
  core::application app(argc, argv);
  while (app.continue_computation()) {
    app.update();
  }
  
  return 0;
}

// отвалился вулкан хпп (почему то в мемори аллокаторе хпп отсутсвет перегрузка класса со всеми флагами для энумов)
// исправил, но скорее всего не особ правильно
