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

const vec4 box_points[] = { // 14
  vec4(-1.0f,  1.0f,  1.0f, 1.0f), // Front-top-left
  vec4( 1.0f,  1.0f,  1.0f, 1.0f), // Front-top-right
  vec4(-1.0f, -1.0f,  1.0f, 1.0f), // Front-bottom-left
  vec4( 1.0f, -1.0f,  1.0f, 1.0f), // Front-bottom-right
  vec4( 1.0f, -1.0f, -1.0f, 1.0f), // Back-bottom-right
  vec4( 1.0f,  1.0f,  1.0f, 1.0f), // Front-top-right
  vec4( 1.0f,  1.0f, -1.0f, 1.0f), // Back-top-right
  vec4(-1.0f,  1.0f,  1.0f, 1.0f), // Front-top-left
  vec4(-1.0f,  1.0f, -1.0f, 1.0f), // Back-top-left
  vec4(-1.0f, -1.0f,  1.0f, 1.0f), // Front-bottom-left
  vec4(-1.0f, -1.0f, -1.0f, 1.0f), // Back-bottom-left
  vec4( 1.0f, -1.0f, -1.0f, 1.0f), // Back-bottom-right
  vec4(-1.0f,  1.0f, -1.0f, 1.0f), // Back-top-left
  vec4( 1.0f,  1.0f, -1.0f, 1.0f)  // Back-top-right
};

layout (location = 0) out vec4 out_point;
layout (location = 1) out flat uint out_state;

out gl_PerVertex {
  vec4 gl_Position;
};

//mat4 rotate(const mat4 mat, const float angle, const vec4 normal);

// тут не пишется ничего в буфер глубины, поэтому можно оставить дефолтный размер
void main() {
  mat4 mat_pos = mat4(1.0f);
  mat_pos[3] = camera.pos; //  + camera.dir * 5.0f
  // const mat4 mat_scale = mat4(200.0f);
  // const mat4 mat_rot1 = rotate(mat4(1.0f), radians(45.0f), vec4(0.0f, 1.0f, 0.0f, 0.0f));
  // const mat4 mat_rot2 = rotate(mat_rot1,   radians(45.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f));
  out_point = camera.viewproj * mat_pos * box_points[gl_VertexIndex];
  out_state = additional.state.y;
  gl_Position = out_point;
}

// mat4 rotate(const mat4 mat, const float angle, const vec4 normal) {
//   const float c = cos(angle);
//   const float s = sin(angle);
//
//   const vec4 temp = normalize(normal) * (1.0f - c);
//
//   mat3 rot;
//
//   rot[0][0] = c + temp[0] * normal[0];
//   rot[0][1] = temp[0] * normal[1] + s * normal[2];
//   rot[0][2] = temp[0] * normal[2] - s * normal[1];
//
//   rot[1][0] = temp[1] * normal[0] - s * normal[2];
//   rot[1][1] = c + temp[1] * normal[1];
//   rot[1][2] = temp[1] * normal[2] + s * normal[0];
//
//   rot[2][0] = temp[2] * normal[0] + s * normal[1];
//   rot[2][1] = temp[2] * normal[1] - s * normal[0];
//   rot[2][2] = c + temp[2] * normal[2];
//
//   mat4 result;
//   result[0] = mat[0] * rot[0][0] + mat[1] * rot[0][1] + mat[2] * rot[0][2];
//   result[1] = mat[0] * rot[1][0] + mat[1] * rot[1][1] + mat[2] * rot[1][2];
//   result[2] = mat[0] * rot[2][0] + mat[1] * rot[2][1] + mat[2] * rot[2][2];
//   result[3] = mat[3];
//   return result;
// }
