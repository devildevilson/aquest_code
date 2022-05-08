#include "lua_initialization_hidden.h"

#include "Cpp/FastNoiseLite.h"
#include "utils/magic_enum_header.h"

#define SET_NOISE_FUNCTION(arg_type, func_name) [] (const FastNoiseLite* self, sol::variadic_args va) -> float { \
  const size_t arg_size = va.size();                                                                              \
  if (arg_size < 2) return 1000.0;                                                                                \
  if (arg_size == 2) return self-> func_name (va.get<arg_type>(0), va.get<arg_type>(1));                          \
  return self-> func_name (va.get<arg_type>(0), va.get<arg_type>(1), va.get<arg_type>(2));                        \
}

#define SET_NOISE_FUNCTION2(arg_type, func_name) [] (const FastNoiseLite* self, sol::variadic_args va) -> float { \
  const size_t arg_size = va.size();                                                                               \
  if (arg_size < 2) return 1000.0;                                                                                 \
  if (arg_size == 2) return self-> func_name (va.get<arg_type>(0), va.get<arg_type>(1));                           \
  if (arg_size == 3) return self-> func_name (va.get<arg_type>(0), va.get<arg_type>(1), va.get<arg_type>(2));      \
  return self-> func_name (va.get<arg_type>(0), va.get<arg_type>(1), va.get<arg_type>(2), va.get<arg_type>(3));    \
}

namespace devils_engine {
  namespace utils {
    static std::tuple<double, double, double> domain_warp(FastNoiseLite* self, const sol::variadic_args &args) {
      if (args.size() < 2) return std::make_tuple(10000.0, 10000.0, 10000.0);
      if (args.size() == 2) {
        double x = args.get<double>(0);
        double y = args.get<double>(1);
        self->DomainWarp(x, y);
        return std::make_tuple(x, y, 0.0);
      }
      double x = args.get<double>(0);
      double y = args.get<double>(1);
      double z = args.get<double>(2);
      self->DomainWarp(x, y, z);
      return std::make_tuple(x, y, z);
    }
    
    static float get_noise(FastNoiseLite* self, const sol::variadic_args &args) { 
      if (args.size() < 2) return 10000.0f;
      if (args.size() == 2) return self->GetNoise(args.get<double>(0), args.get<double>(1));
      return self->GetNoise(args.get<double>(0), args.get<double>(1), args.get<double>(2));
    }
    
    void setup_lua_noiser(sol::state_view lua) {
      auto utils = lua[magic_enum::enum_name(reserved_lua::utils)].get_or_create<sol::table>();
      const auto noise_type = utils.new_enum(
        "noise_type",
        {
          std::make_pair(magic_enum::enum_name(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2), FastNoiseLite::NoiseType::NoiseType_OpenSimplex2),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2S), FastNoiseLite::NoiseType::NoiseType_OpenSimplex2S),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::NoiseType::NoiseType_Cellular), FastNoiseLite::NoiseType::NoiseType_Cellular),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::NoiseType::NoiseType_Perlin), FastNoiseLite::NoiseType::NoiseType_Perlin),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::NoiseType::NoiseType_ValueCubic), FastNoiseLite::NoiseType::NoiseType_ValueCubic),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::NoiseType::NoiseType_Value), FastNoiseLite::NoiseType::NoiseType_Value)
        }
      );
      
      const auto rotation_type3D = utils.new_enum(
        "rotation_type3D",
        {
          std::make_pair(magic_enum::enum_name(FastNoiseLite::RotationType3D::RotationType3D_None), FastNoiseLite::RotationType3D::RotationType3D_None),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::RotationType3D::RotationType3D_ImproveXYPlanes), FastNoiseLite::RotationType3D::RotationType3D_ImproveXYPlanes),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::RotationType3D::RotationType3D_ImproveXZPlanes), FastNoiseLite::RotationType3D::RotationType3D_ImproveXZPlanes)
        }
      );
      
      const auto fractal_type = utils.new_enum(
        "fractal_type",
        {
          std::make_pair(magic_enum::enum_name(FastNoiseLite::FractalType::FractalType_None), FastNoiseLite::FractalType::FractalType_None),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::FractalType::FractalType_FBm), FastNoiseLite::FractalType::FractalType_FBm),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::FractalType::FractalType_Ridged), FastNoiseLite::FractalType::FractalType_Ridged),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::FractalType::FractalType_PingPong), FastNoiseLite::FractalType::FractalType_PingPong),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::FractalType::FractalType_DomainWarpProgressive), FastNoiseLite::FractalType::FractalType_DomainWarpProgressive),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::FractalType::FractalType_DomainWarpIndependent), FastNoiseLite::FractalType::FractalType_DomainWarpIndependent)
        }
      );
      
      const auto cellular_distance_function = utils.new_enum(
        "cellular_distance_function",
        {
          std::make_pair(magic_enum::enum_name(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Euclidean), FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Euclidean),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq), FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_EuclideanSq),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Manhattan), FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Manhattan),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Hybrid), FastNoiseLite::CellularDistanceFunction::CellularDistanceFunction_Hybrid)
        }
      );
      
      const auto cellular_return_type = utils.new_enum(
        "cellular_return_type",
        {
          std::make_pair(
            magic_enum::enum_name(FastNoiseLite::CellularReturnType::CellularReturnType_CellValue), 
                                  FastNoiseLite::CellularReturnType::CellularReturnType_CellValue
          ),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::CellularReturnType::CellularReturnType_Distance), FastNoiseLite::CellularReturnType::CellularReturnType_Distance),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::CellularReturnType::CellularReturnType_Distance2), FastNoiseLite::CellularReturnType::CellularReturnType_Distance2),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::CellularReturnType::CellularReturnType_Distance2Add), FastNoiseLite::CellularReturnType::CellularReturnType_Distance2Add),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::CellularReturnType::CellularReturnType_Distance2Sub), FastNoiseLite::CellularReturnType::CellularReturnType_Distance2Sub),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::CellularReturnType::CellularReturnType_Distance2Mul), FastNoiseLite::CellularReturnType::CellularReturnType_Distance2Mul),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::CellularReturnType::CellularReturnType_Distance2Div), FastNoiseLite::CellularReturnType::CellularReturnType_Distance2Div),
        }
      );
      
      const auto domain_warp_type = utils.new_enum(
        "domain_warp_type",
        {
          std::make_pair(
            magic_enum::enum_name(FastNoiseLite::DomainWarpType::DomainWarpType_OpenSimplex2), 
                                  FastNoiseLite::DomainWarpType::DomainWarpType_OpenSimplex2
          ),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::DomainWarpType::DomainWarpType_OpenSimplex2Reduced), FastNoiseLite::DomainWarpType::DomainWarpType_OpenSimplex2Reduced),
          std::make_pair(magic_enum::enum_name(FastNoiseLite::DomainWarpType::DomainWarpType_BasicGrid), FastNoiseLite::DomainWarpType::DomainWarpType_BasicGrid),
        }
      );
      
      const auto noiser = utils.new_usertype<FastNoiseLite>(
        "noiser", sol::no_constructor,
        "set_seed", &FastNoiseLite::SetSeed,
//         "get_seed", &FastNoiseLite::GetSeed,
        "set_frequency", &FastNoiseLite::SetFrequency,
//         "get_frequency", &FastNoiseLite::GetFrequency,
//         "set_interp", &FastNoiseLite::SetInterp,
//         "get_interp", &FastNoiseLite::GetInterp,
        "set_noise_type", &FastNoiseLite::SetNoiseType,
        "set_rotation_type3D", &FastNoiseLite::SetRotationType3D,
//         "get_noise_type", &FastNoiseLite::GetNoiseType,
        "set_fractal_octaves", &FastNoiseLite::SetFractalOctaves,
//         "get_fractal_octaves", &FastNoiseLite::GetFractalOctaves,
        "set_fractal_lacunarity", &FastNoiseLite::SetFractalLacunarity,
//         "get_fractal_lacunarity", &FastNoiseLite::GetFractalLacunarity,
        "set_fractal_gain", &FastNoiseLite::SetFractalGain,
//         "get_fractal_gain", &FastNoiseLite::GetFractalGain,
        "set_fractal_type", &FastNoiseLite::SetFractalType,
//         "get_fractal_type", &FastNoiseLite::GetFractalType,
        "set_fractal_weighted_strength", &FastNoiseLite::SetFractalWeightedStrength,
        "set_fractal_ping_pong_strength", &FastNoiseLite::SetFractalPingPongStrength,
        "set_cellular_distance_function", &FastNoiseLite::SetCellularDistanceFunction,
//         "get_cellular_distance_function", &FastNoiseLite::GetCellularDistanceFunction,
        "set_cellular_return_type", &FastNoiseLite::SetCellularReturnType,
//         "get_cellular_return_type", &FastNoiseLite::GetCellularReturnType,
//         "set_cellular_noise_lookup", &FastNoiseLite::SetCellularNoiseLookup,
//         "get_cellular_noise_lookup", &FastNoiseLite::GetCellularNoiseLookup,
//         "set_cellular_distance_2_indices", &FastNoiseLite::SetCellularDistance2Indices,
//         "get_cellular_distance_2_indices", [] (const FastNoiseLite* self) -> std::tuple<int, int> { int a, b; self->GetCellularDistance2Indices(a, b); return std::tie(a, b); },
        "set_cellular_jitter", &FastNoiseLite::SetCellularJitter,
//         "get_cellular_jitter", &FastNoiseLite::GetCellularJitter,
//         "set_gradient_perturb_amp", &FastNoiseLite::SetGradientPerturbAmp,
//         "get_gradient_perturb_amp", &FastNoiseLite::GetGradientPerturbAmp,
        "set_domain_warp_type", &FastNoiseLite::SetDomainWarpType,
        "set_domain_warp_amp", &FastNoiseLite::SetDomainWarpAmp,
        "get_noise", &get_noise,
        "domain_warp", &domain_warp,
        "noise_type", noise_type,
        "rotation_type3D", rotation_type3D,
        "fractal_type", fractal_type,
        "cellular_distance_function", cellular_distance_function,
        "cellular_return_type", cellular_return_type,
        "domain_warp_type", domain_warp_type
        
//         //"get_value",                [] (const FastNoiseLite* self, FN_DECIMAL a, FN_DECIMAL b) { return self->GetValue(a, b); },
//         "get_value",                SET_NOISE_FUNCTION(double, GetValue),
//         //"get_value",                sol::overload(&FastNoiseLite::GetValue, ),
//         "get_value_fractal",        SET_NOISE_FUNCTION(FN_DECIMAL, GetValueFractal),
//         "get_perlin",               SET_NOISE_FUNCTION(FN_DECIMAL, GetPerlin),
//         "get_perlin_fractal",       SET_NOISE_FUNCTION(FN_DECIMAL, GetPerlinFractal),
//         "get_simplex",              SET_NOISE_FUNCTION2(FN_DECIMAL, GetSimplex),
//         "get_simplex_fractal",      SET_NOISE_FUNCTION(FN_DECIMAL, GetSimplexFractal),
//         "get_cellular",             SET_NOISE_FUNCTION(FN_DECIMAL, GetCellular),
//         "get_white_noise",          SET_NOISE_FUNCTION2(FN_DECIMAL, GetWhiteNoise),
//         "get_white_noise_int",      SET_NOISE_FUNCTION2(int, GetWhiteNoiseInt),
//         "get_cubic",                SET_NOISE_FUNCTION(FN_DECIMAL, GetCubic),
//         "get_cubic_fractal",        SET_NOISE_FUNCTION(FN_DECIMAL, GetCubicFractal),
//         "get_noise",                SET_NOISE_FUNCTION(FN_DECIMAL, GetNoise),
//         "gradient_perturb",         [] (const FastNoiseLite* self) -> std::tuple<FN_DECIMAL, FN_DECIMAL> { FN_DECIMAL a, b; self->GradientPerturb(a, b); return std::tie(a, b); },
//         "gradient_perturb_fractal", [] (const FastNoiseLite* self) -> std::tuple<FN_DECIMAL, FN_DECIMAL> { FN_DECIMAL a, b; self->GradientPerturbFractal(a, b); return std::tie(a, b); },
//         "gradient_perturb_3d",         [] (const FastNoiseLite* self) -> std::tuple<FN_DECIMAL, FN_DECIMAL, FN_DECIMAL> { FN_DECIMAL a, b, c; self->GradientPerturb(a, b, c); return std::tie(a, b, c); },
//         "gradient_perturb_fractal_3d", [] (const FastNoiseLite* self) -> std::tuple<FN_DECIMAL, FN_DECIMAL, FN_DECIMAL> { FN_DECIMAL a, b, c; self->GradientPerturbFractal(a, b, c); return std::tie(a, b, c); }
      );
    }
  }
}
