#ifndef RELIGION_H
#define RELIGION_H

#include <string>
#include "render/shared_structures.h"
#include "utils/bit_field.h"
#include "utils/constexpr_funcs.h"
#include "utils/structures_utils.h"
#include "utils/list.h"
#include "utils/sol.h"
#include "realm_mechanics.h"
#include "declare_structures.h"

namespace devils_engine {
  namespace core {  
    struct religion_opinion_data {
      enum hostility_settings {
        intermarriage,
        title_usurpation,
        holy_wars,
        settings_count
      };
      
      static const size_t bit_field_size = ceil(double(settings_count) / double(UINT32_WIDTH));
      
      float character_opinion;
      float popular_opinion;
      utils::bit_field_32<bit_field_size> hostility;
    };
    
    struct religion_group {
      static const structure s_type = structure::religion_group;
      std::string id;
      std::string name_id;
      std::string description_id;
      
      // по идее настройки отношений + настройки для ии 
      // теперь осталось понять нужно ли делать более тонкую настройку или оставить примерно так как это было в цк3
      religion_opinion_data different_groups;
      religion_opinion_data different_religions;
      religion_opinion_data different_faiths;
      
      render::image_t image;
      render::color_t color;
    };
    
    struct religion : public utils::flags_container, public utils::events_container, public utils::ring::list<religion, utils::list_type::faiths> {
      static const structure s_type = structure::religion;
      static const size_t max_religion_types = 128;
      static const size_t types_bit_field_size = ceil(double(max_religion_types) / double(SIZE_WIDTH));
      static const size_t max_bonuses = 16;
      static_assert(types_bit_field_size == 2);
      
      std::string id;
      std::string name_id;
      // описание (?) (возможно частично должно быть сгенерировано например на основе прав)
      std::string description_id;
      const religion_group* group;
      const religion* parent;   // родительская религия? эта конкретная религия может быть определенным верованием
      const religion* children; // верования религии
      // как сделать организованность религии?
      // по идее это вопрос института, а значит можно добавить сюда указатель на реалм по идее
      const religion* reformed;
      
      // сколько жен или наложниц? (нужно ли?) (это определяется светскими законами)
      // мультипликатор к короткому правлению
      // агрессивность ии
      double aggression;
      // элемент одежды главы (головной убор и сама одежда)
      // сколько ресурсов дает за отсутствие войны
      // название священного похода
      std::string crusade_name_id;
      std::string holy_order_names_table_id;
//       std::string holy_order_maa_id; // возможно специальные религиозные юниты
      // название священного текста
      std::string scripture_name_id;
      // имена богов и злых богов
      std::string good_gods_table_id;
      std::string evil_gods_table_id;
      // имя главного бога
      std::string high_god_name_id;
      // название благочестия
      std::string piety_name_id;
      // титул священников
      std::string priest_title_name_id;
      // дополнительные религиозные имена
      std::string reserved_male_names_table_id;
      std::string reserved_female_names_table_id;
      
//       uint32_t type_index; // 
      uint32_t opinion_stat_index; // все персонажи этой религии, нужно ли делать отношения отдельных групп персонажей
      render::image_t image;
      // цвет?
      render::color_t color;
      // права (ограничения механик последователей данной религии)
      // это скорее должны быть некие традиции, то есть некоторый догматический свод правил
      // к которому стремятся религиозные политические деятели (помимо стремлений к власти)
      // то есть тут должны быть всякие отношения мужчин и женщин, герои, отношение к неверным
      // и прочее и прочее, религиозный ии должен положительно относится к инициативам ведущим к 
      // традициям, я тут подумал: касты должны быть отнесены скорее к культурам
      // так же некоторые вещи отсюда должны быть скорее отнесены к ии и стоять где то отдельно
      utils::bit_field_32<religion_mechanics::bit_container_size> mechanics;
      
      // переопределяем здесь данные из религиозной группы или из родительской религии
      religion_opinion_data different_groups;
      religion_opinion_data different_religions;
      religion_opinion_data different_faiths;
      
      // модификаторы к персонажу и юнитам
      std::array<stat_modifier, max_bonuses> bonuses;
      // модификаторы если на своей земле
      std::array<stat_modifier, max_bonuses> home_bonuses;
      
      // священные земли?
      
      religion();
      inline bool get_mechanic(const size_t &index) const { return mechanics.get(index); }
      inline bool set_mechanic(const size_t &index, const bool value) { return mechanics.set(index, value); }
    };
    
    size_t add_religion_group(const sol::table &table);
    bool validate_religion_group(const size_t &index, const sol::table &table);
    void parse_religion_group(core::religion_group* religion_group, const sol::table &table);
    
    size_t add_religion(const sol::table &table);
    bool validate_religion(const size_t &index, const sol::table &table);
    void parse_religion(core::religion* religion, const sol::table &table);
  }
}

#endif
