#ifndef PATH_FINDING_DATA_H
#define PATH_FINDING_DATA_H

#include "path_container.h"

namespace devils_engine {
  namespace ai {
    // по идее что у армии что у героев (что у всех остальных объектов движения)
    // данные поиска пути вообще не особенно будут отличаться,
    // поэтому лучше все это дело унифицировать, причем эта же структура
    // по идее должна работать и битве и в столкновении
    
    // путь? размер пути, пройденный путь, причем эти данные должны добавиться атомарно
    // если мы тут добавим атомарный флаг с состоянием - мы легко решим проблему остановки задачи 
    // нет не легко, нужно тогда еще свою очередь с заданиями тащить, короче тут сложно и непонятно
    // атомарный указатель на атомарную переменную? если есть указатель значит есть задача
    // да но при этом не известно когда она запустится, по идее у нас фифо очередь по этому можно подождать
    // я так подозреваю что в других играх весь поиск делает один поток (что скорее тупо чем практично)
    struct path_finding_data {
      std::atomic<ai::path_container*> path;
      size_t path_size;
      size_t current_path;
      std::atomic<uint32_t> start_tile;
      std::atomic<uint32_t> end_tile;
      std::atomic<size_t> path_task;
      
      path_finding_data();
      
      // тут мы должны определить 3 состояния у пути: пути нет (мы еще ничего не сделали), путь ищется, поиск окончен (путь найден или нет)
      bool finding_path() const;
      bool has_path() const;
      bool path_not_found() const;
      // + должен быть способ провести армию, но это наверное будет уже в конкретной армии
    };
    
    size_t unit_advance(const path_finding_data* unit, const size_t &start, const size_t &speed);
    
    template <typename T>
    size_t maximum_unit_advance(const T* unit, const size_t &start);
  }
}

#endif
