#version 450

layout(set = 0, binding = 0) uniform camera_buffer {
  mat4 viewproj;
  mat4 view;
  vec4 pos;
  vec4 dir;
} camera;

layout(set = 0, binding = 2) uniform common_uniform {
  vec4 cursor_dir;
  uvec4 dim;
  uvec4 state;
} additional;

const vec4 box_points[] = {
  vec4( 1.0f, 1.0f,-1.0f, 1.0f), // 0
  vec4( 1.0f,-1.0f,-1.0f, 1.0f), // 1
  vec4( 1.0f, 1.0f, 1.0f, 1.0f), // 2
  vec4( 1.0f,-1.0f, 1.0f, 1.0f), // 3
  vec4(-1.0f, 1.0f,-1.0f, 1.0f), // 4
  vec4(-1.0f,-1.0f,-1.0f, 1.0f), // 5
  vec4(-1.0f, 1.0f, 1.0f, 1.0f), // 6
  vec4(-1.0f,-1.0f, 1.0f, 1.0f), // 7
};

const uint box_indices[] = {
  0, 4, 2, /**/ 4, 2, 6, // верх
  3, 2, 7, /**/ 2, 7, 6,
  7, 6, 5, /**/ 6, 5, 4,
  5, 1, 7, /**/ 1, 7, 3,
  1, 0, 3, /**/ 0, 3, 2,
  5, 4, 1, /**/ 4, 1, 0,
};

const vec2 box_uv[] = {
  vec2(0.625f, 0.5f),  // 0
  vec2(0.875f, 0.5f),  // 1
  vec2(0.875f, 0.75f), // 2
  vec2(0.625f, 0.75f), // 3
  vec2(0.375f, 0.75f), // 4
  vec2(0.625f, 1.0f),  // 5
  vec2(0.375f, 1.0f),  // 6
  vec2(0.375f, 0.0f),  // 7
  vec2(0.625f, 0.0f),  // 8
  vec2(0.625f, 0.25f), // 9
  vec2(0.375f, 0.25f), // 10
  vec2(0.125f, 0.5f),  // 11
  vec2(0.375f, 0.5f),  // 12
  vec2(0.125f, 0.75f), // 13
};

const uint box_uv_indices[] = {
   0,  1,  3, /**/  1, 3,  2,
   4,  3,  6, /**/  3, 6,  5,
   7,  8, 10, /**/  8, 10, 9,
  11, 12, 13, /**/ 12, 13, 4,
  12,  0,  4, /**/  0, 4,  3,
  10,  9, 12, /**/  9, 12, 0,
};

const vec4 viewport_tri_points[] = {
  vec4(-3.0f,-1.0f, 0.0f, 1.0f),
  vec4( 1.0f,-1.0f, 0.0f, 1.0f),
  vec4( 1.0f, 3.0f, 0.0f, 1.0f)
};

layout (location = 0) out vec4 out_point;
layout (location = 1) out vec2 out_uv;
layout (location = 2) out flat uint out_state;

out gl_PerVertex {
  vec4 gl_Position;
};

mat4 rotate(const mat4 mat, const float angle, const vec4 normal);

// тут не пишется ничего в буфер глубины, поэтому можно оставить дефолтный размер

// еще вариант сгенерировать кучу объектов вокруг камеры и рисовать билборды,
// неудачное решение в плане производительности

// и наконец использовать текстурку с шумом, брать оттуда пиксели, их смешивать
void main() {
  const uint box_point_index = box_indices[gl_VertexIndex];
  const uint box_uv_index = box_uv_indices[gl_VertexIndex];

  mat4 mat_pos = mat4(1.0f);
  mat_pos[3] = camera.pos; //  + camera.dir * 5.0f
  //const mat4 mat_scale = mat4(200.0f);
  //const mat4 mat_rot1 = rotate(mat4(1.0f), radians(45.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f));
  //const mat4 mat_rot2 = rotate(mat_rot1,   radians(45.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f));

  out_point = vec4(box_points[box_point_index].xyz * 50, 1.0f);
  out_uv = box_uv[box_uv_index];
  // out_point = viewport_tri_points[gl_VertexIndex];
  out_state = additional.state.y;
  gl_Position = camera.viewproj * mat_pos * out_point;
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
