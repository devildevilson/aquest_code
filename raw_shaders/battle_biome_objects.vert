#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"
#include "../render/shared_battle_structures.h"
#include "../utils/shared_mathematical_constant.h"

const vec2 hex_map_row_const_offset[] = {
  vec2(-0.5f, 0.0f),
  vec2( 0.5f, 0.0f),
  vec2( 0.0f,-0.5f),
  vec2( 0.0f, 0.5f)
};

const vec4 object_points[] = {
  vec4(-1.0f, 1.0f, 0.0f, 1.0f),
  vec4( 1.0f, 1.0f, 0.0f, 1.0f),
  vec4(-1.0f,-1.0f, 0.0f, 1.0f),
  vec4( 1.0f,-1.0f, 0.0f, 1.0f)
};

const vec2 object_uv[] = {
  vec2(0.0f, 0.0f),
  vec2(1.0f, 0.0f),
  vec2(0.0f, 1.0f),
  vec2(1.0f, 1.0f)
};

layout(set = 0, binding = 0) uniform camera_uniform {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
  uvec4 dim;
} camera;

layout(set = 0, binding = 1) uniform matrices_uniform {
  mat4 proj;
  mat4 view;
  mat4 invProj;
  mat4 invView;
  mat4 invViewProj;
} camera_matrices;

layout(set = 2, binding = 0) uniform tiles_uniform_buffer {
  uvec4 map_properties;
} tiles_uniform;

layout(std140, set = 2, binding = 1) readonly buffer tiles_buffer {
  packed_battle_map_tile_data_t tiles[];
};

layout(std140, set = 2, binding = 3) readonly buffer biomes_buffer {
  packed_battle_biome_data_t biomes[];
};

out gl_PerVertex {
  vec4 gl_Position;
};

layout(location = 0) out flat image_t out_biom_texture;
//layout(location = 1) out flat color_t out_biom_color;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out flat float out_tile_height;

mat4 translate(const mat4 mat, const vec4 vec);
mat4 rotate(const mat4 mat, const float angle, const vec4 normal);
mat4 scale(const mat4 mat, const vec4 vec);
vec4 project_vector_on_plane(const vec4 normal, const vec4 origin, const vec4 vector);

void main() {
  const uint tile_index  = gl_VertexIndex / PACKED_INDEX_COEF;
  const uint point_index = gl_VertexIndex % PACKED_INDEX_COEF;
  const uint instance_index = gl_InstanceIndex;

  const uint tmp_state1 = prng(tile_index); // prng2 не работает с нулем
  const uint tmp_state2 = prng(instance_index);
  const uint final_state = prng2(tmp_state1, tmp_state2);

  const uvec4 uniform_property_data = tiles_uniform.map_properties;
  const uint map_width   = uniform_property_data.y;
  const uint map_height  = uniform_property_data.z;
  const uint map_type    = uniform_property_data.w;

  const bool is_square_map       = bool(map_type & (1 << 0));
  const bool is_flat_orientation = bool(map_type & (1 << 1));
  const bool is_odd_offset       = bool(map_type & (1 << 2));

  const uint tile_map_width  = map_width;
  const uint tile_map_height = map_height;
  const uint row_index    = tile_index / tile_map_height;
  const uint column_index = tile_index % tile_map_height;
  const uvec2 tile_coord = uvec2(column_index, row_index);
  //const uvec2 tile_coord = uvec2(0, 0);

  const float hex_size = 1.0f;
  const float hex_width = mix(sqrt(3.0f) * hex_size, 2.0f * hex_size, float(is_flat_orientation));
  const float hex_height = mix(2.0f * hex_size, sqrt(3.0f) * hex_size, float(is_flat_orientation));
  const float hex_width_dist = mix(1.0f * hex_width, (3.0f/4.0f) * hex_width, float(is_flat_orientation));
  const float hex_height_dist = mix((3.0f/4.0f) * hex_height, 1.0f * hex_height, float(is_flat_orientation));

  const uint offset_type_index = uint(is_odd_offset) + 2 * uint(is_flat_orientation);

  const uint row_column = uint(is_flat_orientation) * column_index + uint(!is_flat_orientation) * row_index;
  const vec2 const_pos_k = float(row_column % 2 == 1) * hex_map_row_const_offset[offset_type_index] * vec2(hex_width_dist, hex_height_dist);

  const battle_map_tile_data_t current_tile = unpack_data(tiles[tile_index]);
  const float tile_height = current_tile.height;

  const vec2 tile_pos = vec2(tile_coord) * vec2(hex_width_dist, hex_height_dist) + const_pos_k;
  const vec4 final_tile_pos = vec4(tile_pos.x, tile_height, tile_pos.y, 1.0f);

  const uint biome_index = current_tile.biome_index;
  const battle_biome_data_t biome_data = unpack_data(biomes[biome_index]);

  const float probability_sum = biome_data.probabilities[0] + biome_data.probabilities[1] + biome_data.probabilities[2];
  const float probabilities_norm[] = {biome_data.probabilities[0] / probability_sum, biome_data.probabilities[1] / probability_sum, biome_data.probabilities[2] / probability_sum};

  const uint probability_state = prng(final_state);
  const float probability_state_norm = prng_normalize(probability_state);

  float current_chance = 0.0f;
  uint current_probability_index = 0;
  for (uint i = 0; i < battle_biome_objects_count; ++i) {
    const float max_prob = max(current_chance + probabilities_norm[i], float(i+1 == battle_biome_objects_count));
    current_probability_index = probability_state_norm >= current_chance && probability_state_norm <= max_prob ? i : current_probability_index;
    current_chance += probabilities_norm[i];
  }

  const vec2 scales = biome_data.scales[current_probability_index];
  const uint scale_state = prng(probability_state);
  const float scale_state_norm = prng_normalize(scale_state);

  const float final_scale = mix(scales.x, scales.y, scale_state_norm);

  const float max_zoom = biome_data.zooms[current_probability_index];
  const float current_zoom = uintBitsToFloat(camera.dim.z);
  const bool sprite_mode = current_zoom < max_zoom;

  const uint additional = prng(scale_state);
  const float val_norm1 = prng_normalize(final_state);
  const float val_norm2 = prng_normalize(additional);

  const float objects_radius = 1.0f;
  const float norm1 = val_norm1 * 2.0f * PI;
  const float norm2 = sqrt(val_norm2) * objects_radius;

  const float cos_num = norm2 * cos(norm1); // x
  const float sin_num = norm2 * sin(norm1); // y

  const vec4 center = final_tile_pos;
  const vec4 normal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
  const vec4 x = vec4(1.0f, 0.0f, 0.0f, 0.0f);
  const vec4 y = vec4(0.0f, 0.0f,-1.0f, 0.0f);
  // по идее было бы неплохо здесь тоже сделать по-слоевую высоту
  const vec4 point = center + x * cos_num + y * sin_num + normal * (final_scale); // *2.0f

  // как составить матрицу для спрайта? нужны нормали: одна вверх (по y оси),
  // одна против напраления камеры приведенной к 010 плоскости
  // и одна вправо относительно направления веперед и вверх
  const vec4 camera_dir = camera.dir;
  const vec4 up_dir = vec4(0.0f, 1.0f, 0.0f, 0.0f);
  const vec4 forward_dir = normalize(project_vector_on_plane(up_dir, vec4(0.0f, 0.0f, 0.0f, 1.0f), camera_dir));
  const vec4 right_dir = normalize(vec4(cross(vec3(forward_dir), vec3(up_dir)), 0.0f));

  const mat4 sprite_matrix = mat4(
    right_dir,
    -up_dir,
    forward_dir, // инвертированный?
    vec4(0.0f, 0.0f, 0.0f, 1.0f)
  );

  const battle_biome_object_images_t images_struct = biome_data.textures[current_probability_index];

  // теперь нужно посчитать матрицы, нужно сделать матрицу билборда и матрицу спрайта
  const mat4 rotation_matrix = sprite_mode ? sprite_matrix : camera_matrices.invView;
  const image_t img = sprite_mode ? images_struct.face : images_struct.top;

  const mat4 translaion = translate(mat4(1.0f), point);
  const mat3 rot = mat3(rotation_matrix);
  const mat4 rotation = mat4(rot);
  const mat4 scaling = scale(rotation, vec4(final_scale, -final_scale, final_scale, 0.0f));

  gl_Position = camera.viewproj * translaion * scaling * (object_points[point_index]);
  out_biom_texture = img;
  out_uv = object_uv[point_index];
  out_tile_height = tile_height;
}

mat4 translate(const mat4 mat, const vec4 vec) {
  mat4 ret = mat;
  ret[3] = mat[0] * vec[0] + mat[1] * vec[1] + mat[2] * vec[2] + mat[3];
  return ret;
}

mat4 rotate(const mat4 mat, const float angle, const vec4 normal) {
  const float c = cos(angle);
  const float s = sin(angle);

  const vec4 temp = normalize(normal) * (1.0f - c);

  mat3 rot;

  rot[0][0] = c + temp[0] * normal[0];
  rot[0][1] = temp[0] * normal[1] + s * normal[2];
  rot[0][2] = temp[0] * normal[2] - s * normal[1];

  rot[1][0] = temp[1] * normal[0] - s * normal[2];
  rot[1][1] = c + temp[1] * normal[1];
  rot[1][2] = temp[1] * normal[2] + s * normal[0];

  rot[2][0] = temp[2] * normal[0] + s * normal[1];
  rot[2][1] = temp[2] * normal[1] - s * normal[0];
  rot[2][2] = c + temp[2] * normal[2];

  mat4 result;
  result[0] = mat[0] * rot[0][0] + mat[1] * rot[0][1] + mat[2] * rot[0][2];
  result[1] = mat[0] * rot[1][0] + mat[1] * rot[1][1] + mat[2] * rot[1][2];
  result[2] = mat[0] * rot[2][0] + mat[1] * rot[2][1] + mat[2] * rot[2][2];
  result[3] = mat[3];
  return result;
}

mat4 scale(const mat4 mat, const vec4 vec) {
  mat4 ret = mat;
  ret[0] *= vec[0];
  ret[1] *= vec[1];
  ret[2] *= vec[2];

  return ret;
}

vec4 project_vector_on_plane(const vec4 normal, const vec4 origin, const vec4 vector) {
  const float dist = dot(vector, normal);
  const vec4 point2 = origin + vector - normal*dist;
  return point2 - origin;
}
