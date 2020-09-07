#ifndef SERIALIZATOR_HELPER_H
#define SERIALIZATOR_HELPER_H

#include <vector>
#include <array>
#include <string>

namespace devils_engine {
  namespace utils {
    // наверное какие то типы данных нужно будет отдельно сериализовывать
    // вообще я так понимаю что все типы в которых нет описания условий или эффектов
    // могут быть сериализованы обычным способом
    class serializator_container {
    public:
      enum data_type {
        event,
        count
      };
      
      void add_data(const enum data_type &type, const std::string &data);
      void serialize(); // здесь нужно передать протобаф сериализатор
    private:
      struct container {
        //enum data_type data_type;
        std::vector<std::string> data;
      };
      
      std::array<container, count> data_container;
    };
  }
}

#endif
