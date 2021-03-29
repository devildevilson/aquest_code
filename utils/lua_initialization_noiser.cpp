#include "lua_initialization.h"

#include "FastNoise.h"
#include "magic_enum.hpp"

#define SET_NOISE_FUNCTION(arg_type, func_name) [] (const FastNoise* self, sol::variadic_args va) -> FN_DECIMAL { \
  const size_t arg_size = va.size();                                                                              \
  if (arg_size < 2) return 1000.0;                                                                                \
  if (arg_size == 2) return self-> func_name (va.get<arg_type>(0), va.get<arg_type>(1));                          \
  return self-> func_name (va.get<arg_type>(0), va.get<arg_type>(1), va.get<arg_type>(2));                        \
}

#define SET_NOISE_FUNCTION2(arg_type, func_name) [] (const FastNoise* self, sol::variadic_args va) -> FN_DECIMAL { \
  const size_t arg_size = va.size();                                                                               \
  if (arg_size < 2) return 1000.0;                                                                                 \
  if (arg_size == 2) return self-> func_name (va.get<arg_type>(0), va.get<arg_type>(1));                           \
  if (arg_size == 3) return self-> func_name (va.get<arg_type>(0), va.get<arg_type>(1), va.get<arg_type>(2));      \
  return self-> func_name (va.get<arg_type>(0), va.get<arg_type>(1), va.get<arg_type>(2), va.get<arg_type>(3));    \
}

namespace devils_engine {
  namespace utils {
    void setup_lua_noiser(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name<reserved_lua::values>(reserved_lua::utils)].get_or_create<sol::table>();
      utils.new_usertype<FastNoise>("noiser", 
        sol::no_constructor,
        "set_seed", &FastNoise::SetSeed,
        "get_seed", &FastNoise::GetSeed,
        "set_frequency", &FastNoise::SetFrequency,
        "get_frequency", &FastNoise::GetFrequency,
        "set_interp", &FastNoise::SetInterp,
        "get_interp", &FastNoise::GetInterp,
        "set_noise_type", &FastNoise::SetNoiseType,
        "get_noise_type", &FastNoise::GetNoiseType,
        "set_fractal_octaves", &FastNoise::SetFractalOctaves,
        "get_fractal_octaves", &FastNoise::GetFractalOctaves,
        "set_fractal_lacunarity", &FastNoise::SetFractalLacunarity,
        "get_fractal_lacunarity", &FastNoise::GetFractalLacunarity,
        "set_fractal_gain", &FastNoise::SetFractalGain,
        "get_fractal_gain", &FastNoise::GetFractalGain,
        "set_fractal_type", &FastNoise::SetFractalType,
        "get_fractal_type", &FastNoise::GetFractalType,
        "set_cellular_distance_function", &FastNoise::SetCellularDistanceFunction,
        "get_cellular_distance_function", &FastNoise::GetCellularDistanceFunction,
        "set_cellular_return_type", &FastNoise::SetCellularReturnType,
        "get_cellular_return_type", &FastNoise::GetCellularReturnType,
        "set_cellular_noise_lookup", &FastNoise::SetCellularNoiseLookup,
        "get_cellular_noise_lookup", &FastNoise::GetCellularNoiseLookup,
        "set_cellular_distance_2_indices", &FastNoise::SetCellularDistance2Indices,
        "get_cellular_distance_2_indices", [] (const FastNoise* self) -> std::tuple<int, int> { int a, b; self->GetCellularDistance2Indices(a, b); return std::tie(a, b); },
        "set_cellular_jitter", &FastNoise::SetCellularJitter,
        "get_cellular_jitter", &FastNoise::GetCellularJitter,
        "set_gradient_perturb_amp", &FastNoise::SetGradientPerturbAmp,
        "get_gradient_perturb_amp", &FastNoise::GetGradientPerturbAmp,
        
        //"get_value",                [] (const FastNoise* self, FN_DECIMAL a, FN_DECIMAL b) { return self->GetValue(a, b); },
        "get_value",                SET_NOISE_FUNCTION(FN_DECIMAL, GetValue),
        //"get_value",                sol::overload(&FastNoise::GetValue, ),
        "get_value_fractal",        SET_NOISE_FUNCTION(FN_DECIMAL, GetValueFractal),
        "get_perlin",               SET_NOISE_FUNCTION(FN_DECIMAL, GetPerlin),
        "get_perlin_fractal",       SET_NOISE_FUNCTION(FN_DECIMAL, GetPerlinFractal),
        "get_simplex",              SET_NOISE_FUNCTION2(FN_DECIMAL, GetSimplex),
        "get_simplex_fractal",      SET_NOISE_FUNCTION(FN_DECIMAL, GetSimplexFractal),
        "get_cellular",             SET_NOISE_FUNCTION(FN_DECIMAL, GetCellular),
        "get_white_noise",          SET_NOISE_FUNCTION2(FN_DECIMAL, GetWhiteNoise),
        "get_white_noise_int",      SET_NOISE_FUNCTION2(int, GetWhiteNoiseInt),
        "get_cubic",                SET_NOISE_FUNCTION(FN_DECIMAL, GetCubic),
        "get_cubic_fractal",        SET_NOISE_FUNCTION(FN_DECIMAL, GetCubicFractal),
        "get_noise",                SET_NOISE_FUNCTION(FN_DECIMAL, GetNoise),
        "gradient_perturb",         [] (const FastNoise* self) -> std::tuple<FN_DECIMAL, FN_DECIMAL> { FN_DECIMAL a, b; self->GradientPerturb(a, b); return std::tie(a, b); },
        "gradient_perturb_fractal", [] (const FastNoise* self) -> std::tuple<FN_DECIMAL, FN_DECIMAL> { FN_DECIMAL a, b; self->GradientPerturbFractal(a, b); return std::tie(a, b); },
        "gradient_perturb_3d",         [] (const FastNoise* self) -> std::tuple<FN_DECIMAL, FN_DECIMAL, FN_DECIMAL> { FN_DECIMAL a, b, c; self->GradientPerturb(a, b, c); return std::tie(a, b, c); },
        "gradient_perturb_fractal_3d", [] (const FastNoise* self) -> std::tuple<FN_DECIMAL, FN_DECIMAL, FN_DECIMAL> { FN_DECIMAL a, b, c; self->GradientPerturbFractal(a, b, c); return std::tie(a, b, c); }
      );
    }
  }
}
