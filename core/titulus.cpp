#include "titulus.h"

namespace devils_engine {
  namespace core {
    const structure titulus::s_type;
    const size_t titulus::events_container_size;
    const size_t titulus::flags_container_size;
    titulus::titulus() : type(type::count), count(0), childs(nullptr), parent(nullptr), owner(nullptr), name_str(UINT32_MAX), description_str(UINT32_MAX), next(nullptr), prev(nullptr) {}
    titulus::titulus(const enum type &t) : type(t), count(0), childs(nullptr), parent(nullptr), owner(nullptr), name_str(UINT32_MAX), description_str(UINT32_MAX), next(nullptr), prev(nullptr) {}
    titulus::titulus(const enum type &t, const uint32_t &count) : 
      type(t), 
      count(count), 
      childs(count < 2 ? nullptr : new titulus*[count]), 
      parent(nullptr), 
      owner(nullptr), 
      name_str(UINT32_MAX), 
      description_str(UINT32_MAX), 
      next(nullptr) 
    {}
    
    titulus::~titulus() {
      if (count >= 2) {
        delete [] childs;
      }
    }
    
    bool titulus::is_formal() const {
      return count == 0;
    }
    
    void titulus::set_child(const uint32_t &index, titulus* child) {
      if (index >= count) throw std::runtime_error("titulus wrong child index");
      if (count == 1) this->child = child;
      else this->childs[index] = child;
    }
    
    titulus* titulus::get_child(const uint32_t &index) const {
      if (index >= count) throw std::runtime_error("titulus wrong child index");
      if (count == 1) return child;
      return childs[index];
    }
    
    void titulus::set_province(struct province* province) {
      if (is_formal()) throw std::runtime_error("Could not set province to formal titulus");
      if (count > 1) throw std::runtime_error("Could not set province to titulus with childs > 1");
      if (type != type::baron) throw std::runtime_error("Bad title type");
      this->province = province;
    }
    
    struct province* titulus::get_province() const {
      if (is_formal()) throw std::runtime_error("Could not get province from formal titulus");
      if (count > 1) throw std::runtime_error("Could not get province from titulus with childs > 1");
      if (type != type::baron) throw std::runtime_error("Bad title type");
      return province;
    }

    void titulus::set_city(struct city* city) {
      if (is_formal()) throw std::runtime_error("Could not set city to formal titulus");
      if (count > 1) throw std::runtime_error("Could not set city to titulus with childs > 1");
      if (type != type::city) throw std::runtime_error("Bad title type");
      this->city = city;
    }
    
    struct city* titulus::get_city() const {
      if (is_formal()) throw std::runtime_error("Could not get city from formal titulus");
      if (count > 1) throw std::runtime_error("Could not get province from titulus with childs > 1");
      if (type != type::city) throw std::runtime_error("Bad title type");
      return city;
    }
    
    void titulus::create_children(const uint32_t &count) {
      this->count = 1;
      if (type == type::baron) return;
      if (type == type::city) return;
      if (count < 2) return;
      
      ASSERT(childs == nullptr);
      
      this->count = count;
      childs = new titulus*[count];
      memset(childs, 0, sizeof(titulus*) * count);
    }
  }
}
