#include "character.h"
#include <stdexcept>
#include "utils/assert.h"
#include "utils/globals.h"
#include "generator_container.h"

namespace devils_engine {
  namespace core {
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
    
    void titulus::set_province(const uint32_t &province_index) {
      if (is_formal()) throw std::runtime_error("Could not set province to formal titulus");
      if (count > 1) throw std::runtime_error("Could not set province to titulus with childs > 1");
      provinces[0] = province_index;
    }
    
    uint32_t titulus::get_province() const {
      if (is_formal()) throw std::runtime_error("Could not get province from formal titulus");
      if (count > 1) throw std::runtime_error("Could not get province from titulus with childs > 1");
      return provinces[0];
    }
    
    bool character::is_cousin(const character* a, const character* b) {
      bool ret = false;
      for (uint8_t i = 0; i < 4; ++i) {
        for (uint8_t j = 0; j < 4; ++j) {
          ret = ret || a->family.grandparents[i] == b->family.grandparents[j];
        }
      }
      
      return !is_sibling(a, b) && ret;
    }
    
    bool character::is_sibling(const character* a, const character* b) {
      return a->family.parents[0] == b->family.parents[0] || a->family.parents[1] == b->family.parents[1] || a->family.parents[0] == b->family.parents[1] || a->family.parents[1] == b->family.parents[0];
    }
    
    bool character::is_full_sibling(const character* a, const character* b) {
      return (a->family.parents[0] == b->family.parents[0] && a->family.parents[1] == b->family.parents[1]) || (a->family.parents[0] == b->family.parents[1] && a->family.parents[1] == b->family.parents[0]);
    }
    
    bool character::is_half_sibling(const character* a, const character* b) {
      return is_sibling(a, b) && !is_full_sibling(a, b);
    }
    
    bool character::is_relative(const character* a, const character* b) {
      // нужно проверить династию
      return false;
    }
    
    bool character::is_bastard(const character* a) {
      // незаконное рождение - нужно проверить законных жен
      auto parent1 = a->family.parents[0];
      auto parent2 = a->family.parents[1];
      bool ret = parent1->family.consort == parent2;
      auto prev_consort = parent1->family.previous_consorts;
      while (prev_consort != nullptr) {
        ret = ret || prev_consort == parent2;
        prev_consort = prev_consort->family.previous_consorts;
      }
      
      return !ret;
    }
    
    bool character::is_concubine_child(const character* a) {
      auto parent1 = a->family.parents[0];
      auto parent2 = a->family.parents[1];
      return (parent1->family.owner == parent2) || (parent1 == parent2->family.owner);
    }
    
    bool character::is_concubine(const character* a, const character* b) {
      return a->family.owner == b;
    }
    
    character::family::family() : 
      parents{nullptr, nullptr}, 
      grandparents{nullptr, nullptr, nullptr, nullptr}, 
      children(nullptr), 
      next_sibling(nullptr), 
      prev_sibling(nullptr), 
      consort(nullptr), 
      previous_consorts(nullptr), 
      owner(nullptr), 
      concubines(nullptr),
      blood_dynasty(nullptr),
      dynasty(nullptr)
    {}
    
    character::character() : born_date(UINT32_MAX), name_str(UINT32_MAX), suzerain(nullptr), titles(nullptr) {}
    bool character::is_independent() const {
      return suzerain == nullptr;
    }
    
    void character::add_title(titulus* title) {
      //auto container = global::get<map::generator::container>();
      if (title->owner == this) return;
      if (title->owner != nullptr) title->owner->remove_title(title);
      
      ASSERT(title->next == nullptr);
      ASSERT(title->prev == nullptr);
      ASSERT(title->owner == nullptr);
      
      title->owner = this;
      title->next = titles;
      if (titles != nullptr) titles->prev = title;
      titles = title;
    
      if (!title->is_formal()) {
        // добавляем в мапу
        global::get<map::generator::container>()->add_playable_character(this);
      }
    }
    
    void character::remove_title(titulus* title) {
      if (title->owner != this) return;
      
      title->owner = nullptr;
      if (title->prev != nullptr) title->prev->next = title->next;
      if (title->next != nullptr) title->next->prev = title->prev;
      title->next = nullptr;
      title->prev = nullptr;
      
      if (!title->is_formal()) {
        global::get<map::generator::container>()->remove_playable_character(this);
      }
    }
  }
}
