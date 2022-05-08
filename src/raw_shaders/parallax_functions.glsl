// scale for size of Parallax Mapping effect
//uniform float parallax_scale; // ~0.1

vec2 parallax_mapping(const vec3 V, const vec2 T, out float parallax_height) {
  // determine optimal number of layers
  const float min_layers = 10;
  const float max_layers = 15;
  const float num_layers = mix(max_layers, min_layers, abs(dot(vec3(0, 0, 1), V)));

  // height of each layer
  const float layer_height = 1.0 / num_layers;
  // shift of texture coordinates for each layer
  const vec2 dtex = parallax_scale * V.xy / V.z / num_layers;

  float cur_layer_height = 0; // current depth of the layer
  vec2 current_texture_coords = T; // current texture coordinates
  float height_from_texture = texture(u_heightTexture, current_texture_coords).r; // depth from heightmap

  // while point is above the surface
  while(height_from_texture > cur_layer_height) {
    // to the next layer
    cur_layer_height += layer_height;
    // shift of texture coordinates
    current_texture_coords -= dtex;
    // new depth from heightmap
    height_from_texture = texture(u_heightTexture, current_texture_coords).r;
  }

  ///////////////////////////////////////////////////////////

  // previous texture coordinates
  const vec2 prevT_coords = current_texture_coords + texStep;

  // heights for linear interpolation
  const float nextH = height_from_texture - cur_layer_height;
  const float prevH = texture(u_heightTexture, prevT_coords).r - cur_layer_height + layer_height;
  
  // proportions for linear interpolation
  const float weight = nextH / (nextH - prevH);

  // interpolation of texture coordinates
  const vec2 final_tex_coords = prevT_coords * weight + current_texture_coords * (1.0-weight);

  // interpolation of depth values
  parallax_height = cur_layer_height + prevH * weight + nextH * (1.0 - weight);

  // return result
  return final_tex_coords;
} 

float parallax_soft_shadow_multiplier(const vec3 L, const vec2 initial_tex_coord, const float initial_height) {
  // calculate lighting only for surface oriented to the light source
  if (dot(vec3(0, 0, 1), L) <= 0) return 1; 

  float shadow_multiplier = 1;

  const float min_layers = 15;
  const float max_layers = 30;

  // calculate initial parameters
  float num_samples_under_surface = 0;
  shadow_multiplier = 0;
  const float num_layers = mix(max_layers, min_layers, abs(dot(vec3(0.0f, 0.0f, 1.0f), L)));
  const float layer_height = initial_height / num_layers;
  const vec2 tex_step = parallax_scale * L.xy / L.z / num_layers;

  // current parameters
  float current_layer_height = initial_height - layer_height;
  vec2 current_texture_coords = initial_tex_coord + tex_step;
  float height_from_texture = texture(u_heightTexture, current_texture_coords).r;
  uint step_index = 1;

  // while point is below depth 0.0 )
  while (current_layer_height > 0) {
    // if point is under the surface
    if (height_from_texture < current_layer_height) {
      // calculate partial shadowing factor
      num_samples_under_surface += 1;
      const float new_shadow_multiplier = (current_layer_height - height_from_texture) * (1.0f - step_index / num_layers);
      shadow_multiplier = max(shadow_multiplier, new_shadow_multiplier);
    }

    // offset to the next layer
    step_index += 1;
    current_layer_height -= layer_height;
    current_texture_coords += tex_step;
    height_from_texture = texture(u_heightTexture, current_texture_coords).r;
  }

  // Shadowing factor should be 1 if there were no points under the surface
  shadow_multiplier = num_samples_under_surface < 1 ? 1.0f : 1.0f - shadow_multiplier;
  return shadow_multiplier;
} 

// тут неполные денные
vec4 normalMappingLighting(const vec2 T, const vec3 L, const vec3 V, const float shadow_multiplier) {
  // restore normal from normal map
  const vec3 N = normalize(texture(u_normalTexture, T).xyz * 2.0f - 1.0f);
  const vec3 D = texture(u_diffuseTexture, T).rgb;

  // ambient lighting
  const float iamb = 0.2f;
  // diffuse lighting
  const float idiff = clamp(dot(N, L), 0.0f, 1.0f);
  // specular lighting
  const float ispec = 0;
  if (dot(N, L) > 0.2) {
    const vec3 R = reflect(-L, N);
    ispec = pow(dot(R, V), 32.0f) / 1.5f;
  }

  vec4 resColor;
  resColor.rgb = D * (ambientLighting + (idiff + ispec) * pow(shadow_multiplier, 4));
  resColor.a = 1;

  return resColor;
}

void main() {
  // normalize vectors after vertex shader
  const vec3 V = normalize(o_toCameraInTangentSpace);
  const vec3 L = normalize(o_toLightInTangentSpace);

  // get new texture coordinates from Parallax Mapping
  float parallax_height;
  const vec2 T = parallax_mapping(V, o_texcoords, parallax_height);

  // get self-shadowing factor for elements of parallax
  const float shadow_multiplier = parallax_soft_shadow_multiplier(L, T, parallax_height - 0.05f);

  // calculate lighting
  resulting_color = normal_mapping_lighting(T, L, V, shadow_multiplier);
} 