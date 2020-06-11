#version 450

#extension GL_GOOGLE_include_directive : enable
#include "../render/shared_structures.h"

layout(constant_id = 0) const float   red_component = 0.0f;
layout(constant_id = 1) const float green_component = 1.0f;
layout(constant_id = 2) const float  blue_component = 0.0f;

const vec4 biomes_colors[] = {
  vec4(0.2f, 0.2f, 0.8f, 1.0f), // biome_ocean
  vec4(0.8f, 0.8f, 1.0f, 1.0f), // biome_ocean_glacier
  vec4(1.0f, 1.0f, 0.0f, 1.0f), // biome_desert
  vec4(0.0f, 0.7f, 0.2f, 1.0f), // biome_rain_forest
  vec4(1.0f, 0.2f, 0.2f, 1.0f), // biome_rocky
  vec4(0.0f, 1.0f, 0.2f, 1.0f), // biome_plains
  vec4(0.0f, 1.0f, 0.0f, 1.0f), // biome_swamp
  vec4(0.2f, 1.0f, 0.2f, 1.0f), // biome_grassland
  vec4(0.0f, 0.8f, 0.0f, 1.0f), // biome_deciduous_forest
  vec4(0.6f, 0.6f, 0.6f, 1.0f), // biome_tundra
  vec4(0.9f, 0.9f, 0.9f, 1.0f), // biome_land_glacier
  vec4(0.0f, 0.6f, 0.0f, 1.0f), // biome_conifer_forest
  vec4(0.2f, 0.2f, 0.2f, 1.0f), // biome_mountain
  vec4(1.0f, 1.0f, 1.0f, 1.0f)  // biome_snowy_mountain
};

const vec4 water_ground_colors[] = {
  vec4(0.2f, 1.0f, 0.2f, 1.0f),
  vec4(0.2f, 0.2f, 1.0f, 1.0f),
};

//layout(location = 0) in flat image_t in_image;
layout(location = 0) in flat uint in_biom_index;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in flat float in_tile_height;
layout(location = 0) out vec4 out_color;

void main() {
  //out_color = vec4(red_component, green_component, blue_component, 1.0f);
  //out_color = biomes_colors[in_biom_index];

  if (in_tile_height < 0.0f) out_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  else {
    const uint num1 = lcg(in_biom_index);
    const uint num2 = lcg(num1);
    const uint num3 = lcg(num2);

    const float float1 = lcg_normalize(num1);
    const float float2 = lcg_normalize(num2);
    const float float3 = lcg_normalize(num3);

    out_color = vec4(float1, float2, float3, 1.0f);
  }

  //out_color = water_ground_colors[in_biom_index];

  //if (in_tile_height < 0.0f) out_color = vec4(0.0f, 0.0f, 1.0f, 1.0f); //abs(in_tile_height)
  //else out_color = vec4(in_tile_height, 0.0f, 0.0f, 1.0f);
  //if (in_tile_height < 0.0f) out_color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
  //else out_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}
