#ifndef PATH_FINDING_DATA_H
#define PATH_FINDING_DATA_H

#include "path_container.h"
#include <functional>

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
    
    // у объекта поиска есть два свойства которые нужно определить: есть ли путь, ищется ли новый путь
    // это довольно независимые друг от друга состояния, причем у второго свойства 3 состояния: путь не ищется, путь ищется, путь найден
    // хотя при нахождении пути имеет смысл сразу его пихнуть в основной путь
    // нужно еще ответить на вопрос найден ли путь, нужно ли тут хранить старый путь во время поиска? я в этом как то не уверен
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
    
    using unit_check_next_tile_f = std::function<bool(const path_finding_data*, const uint32_t&, const uint32_t&)>;
    size_t unit_advance(const path_finding_data* unit, const size_t &start, const size_t &speed, const unit_check_next_tile_f &check_tile);
    
    template <typename T>
    size_t maximum_unit_advance(const T* unit, const size_t &start, const unit_check_next_tile_f &check_tile);
    
    bool default_army_tile_checker(const path_finding_data* unit, const uint32_t &cur_tile_index, const uint32_t &tile_index);
  }
}

#endif
