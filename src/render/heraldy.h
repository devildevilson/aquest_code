#ifndef HERALDY_H
#define HERALDY_H

#include "shared_structures.h"

#ifdef __cplusplus

#include "utils/utility.h"

#define INLINE inline
#define INOUT(type) type&

// для геральдики нужно указать трафарет, описание слоев + слои
// геральдика рисуется по столичным городам (обязательно по городам?) (кажется в цк всегда рисовалась по городам)
// в цк3 можно было ее выделить и получить какую то информацию, мне и так и сяк нужно сделать какое то выделение знамен
// как это лучше всего сделать? вообще наверное это тоже рейкастинг, с учетом того что нам нужно взять ближайший, а не последний нарисованный
// AABB дерево? или нахер? пока наверное нахер, где хранить информацию о геральдике? хороший вопрос, 
// мне нужно куда то засунуть индекс первого слоя, у меня еще осталась одна переменная в тайле или в информацию о строениях?
// потребуется еще генерировать новую геральдику, тут все очень сложно, чтобы быть быстрой геральдика расположена вся в одном буфере
// генерация потребует копирование в буфер новой информации, большая часть проблем связана с указыванием кастомного
// следующего слоя, в идеале мы должны читать индекс слоя из другого буфера, и тогда можно будет предсоздать огромное количество слоев
// на любой вкус, а дальше просто раскидать массивы индексов по династиям и титулам, так можно будет написать очень простые генераторы геральдик
namespace devils_engine {
  namespace render {
    using glm::floatBitsToUint;
    using glm::uintBitsToFloat;

    using uint = uint32_t;
    // using mat4 = basic_mat4;
    // using vec4 = basic_vec4;
    using mat4 = glm::mat4;
    using vec4 = glm::vec4;
    using vec2 = glm::vec2;
    using uvec2 = glm::uvec2;
    using vec3 = glm::vec3;
    using uvec4 = glm::uvec4;
    
#else
    
#define INLINE
#define INOUT(type) inout type
    
#endif
    
#define DISCARD_LAYER_BIT (1 << 0)
#define CONTINUE_LAYER_BIT (1 << 1)
#define REVERSE_STENCIL_ALPHA_BIT (1 << 2)
    
struct heraldy_layer_t {
  image_t stencil; // трафарет, раскидываем 4 цвета по ргб и (1-ргб)
  image_t image;
  uint stencil_type; // как расположены картинки, мы можем оставить, здесь будет тип, например отсечение последующих слоев
  uint next_layer; // может быть нужно ограничить количество слоев? больше не используется
  color_t colors[4]; // нужно придумать 4 базовых цвета, белый, черный, серый + (?), или ргб + черный (лучше последнее)
  vec4 coords;
  vec4 tex_coords;
};

struct packed_heraldy_layer_t {
  uvec4 packed_indices;
  uvec4 packed_colors;
  vec4 coords;
  vec4 tex_coords;
};

INLINE heraldy_layer_t unpack_heraldy_data(const packed_heraldy_layer_t packed) {
  heraldy_layer_t l;
  l.stencil.container = packed.packed_indices[0];
  l.image.container = packed.packed_indices[1];
  l.stencil_type = packed.packed_indices[2];
  l.next_layer = packed.packed_indices[3];
  l.colors[0].container = packed.packed_colors[0];
  l.colors[1].container = packed.packed_colors[1];
  l.colors[2].container = packed.packed_colors[2];
  l.colors[3].container = packed.packed_colors[3];
  l.coords = packed.coords;
  l.tex_coords = packed.tex_coords;
  return l;
}

INLINE packed_heraldy_layer_t pack_heraldy_data(const heraldy_layer_t l) {
  packed_heraldy_layer_t packed;
  packed.packed_indices[0] = l.stencil.container;
  packed.packed_indices[1] = l.image.container;
  packed.packed_indices[2] = l.stencil_type;
  packed.packed_indices[3] = l.next_layer;
  packed.packed_colors[0] = l.colors[0].container;
  packed.packed_colors[1] = l.colors[1].container;
  packed.packed_colors[2] = l.colors[2].container;
  packed.packed_colors[3] = l.colors[3].container;
  packed.coords = l.coords;
  packed.tex_coords = l.tex_coords;
  return packed;
}

#ifdef __cplusplus
    
  }
}

#endif

#endif
