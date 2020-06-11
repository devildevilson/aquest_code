#ifndef PERLIN_H
#define PERLIN_H

#include <cstddef>
#include <cstdint>

// https://github.com/stegu/perlin-noise

namespace devils_engine {
  namespace random {
   /*
    * This implementation is "Improved Noise" as presented by
    * Ken Perlin at Siggraph 2002. The 3D function is a direct port
    * of his Java reference code which was once publicly available
    * on www.noisemachine.com (although I cleaned it up, made it
    * faster and made the code more readable), but the 1D, 2D and
    * 4D functions were implemented from scratch by me.
    *
    * This is a backport to C of my improved noise class in C++
    * which was included in the Aqsis renderer project.
    * It is highly reusable without source code modifications.
    *
    */
    
    float perlin(const float x);
    float perlin(const float x, const float y);
    float perlin(const float x, const float y, const float z);
    float perlin(const float x, const float y, const float z, const float w);

    // 1D, 2D, 3D and 4D float Perlin periodic noise
    float periodic_perlin(const float x, const int32_t px);
    float periodic_perlin(const float x, const float y, const int32_t px, const int32_t py);
    float periodic_perlin(const float x, const float y, const float z, const int32_t px, const int32_t py, const int32_t pz);
    float periodic_perlin(const float x, const float y, const float z, const float w, const int32_t px, const int32_t py, const int32_t pz, const int32_t pw);
    
    double perlind(const double x);
    double perlind(const double x, const double y);
    double perlind(const double x, const double y, const double z);
    double perlind(const double x, const double y, const double z, const double w);

    // 1D, 2D, 3D and 4D double Perlin periodic noise
    double periodic_perlind(const double x, const int64_t px);
    double periodic_perlind(const double x, const double y, const int64_t px, const int64_t py);
    double periodic_perlind(const double x, const double y, const double z, const int64_t px, const int64_t py, const int64_t pz);
    double periodic_perlind(const double x, const double y, const double z, const double w, const int64_t px, const int64_t py, const int64_t pz, const int64_t pw);
    
   /*
    * This is a clean, fast, modern and free Perlin Simplex noise function.
    * It is a stand-alone compilation unit with no external dependencies,
    * highly reusable without source code modifications.
    *
    * Note:
    * Replacing the "float" type with "double" can actually make this run faster
    * on some platforms. Having both versions could be useful.
    */
    
    // 1D, 2D, 3D and 4D float Perlin simplex noise
    float simplex_perlin(const float x);
    float simplex_perlin(const float x, const float y);
    float simplex_perlin(const float x, const float y, const float z);
    float simplex_perlin(const float x, const float y, const float z, const float w);
    
    double simplex_perlind(const double x);
    double simplex_perlind(const double x, const double y);
    double simplex_perlind(const double x, const double y, const double z);
    double simplex_perlind(const double x, const double y, const double z, const double w);
    
    /*
     * This is an implementation of Perlin "simplex noise" over one
     * dimension (x), two dimensions (x,y), three dimensions (x,y,z)
     * and four dimensions (x,y,z,w). The analytic derivative is
     * returned, to make it possible to do lots of fun stuff like
     * flow animations, curl noise, analytic antialiasing and such.
     *
     * Visually, this noise is exactly the same as the plain version of
     * simplex noise provided in the file "snoise1234.c". It just returns
     * all partial derivatives in addition to the scalar noise value.
     *
     */
    /** 1D simplex noise with derivative.
     * If the last argument is not null, the analytic derivative
     * is also calculated.
     */
    float simplex_perlin(const float x, float *dnoise_dx);

    /** 2D simplex noise with derivatives.
     * If the last two arguments are not null, the analytic derivative
     * (the 2D gradient of the scalar noise field) is also calculated.
     */
    float simplex_perlin(const float x, const float y, float *dnoise_dx, float *dnoise_dy);

    /** 3D simplex noise with derivatives.
     * If the last tthree arguments are not null, the analytic derivative
     * (the 3D gradient of the scalar noise field) is also calculated.
     */
    float simplex_perlin(const float x, const float y, const float z,
                         float *dnoise_dx, float *dnoise_dy, float *dnoise_dz);

    /** 4D simplex noise with derivatives.
     * If the last four arguments are not null, the analytic derivative
     * (the 4D gradient of the scalar noise field) is also calculated.
     */
    float simplex_perlin(const float x, const float y, const float z, const float w,
                         float *dnoise_dx, float *dnoise_dy,
                         float *dnoise_dz, float *dnoise_dw);
    
    /*
     * This is an implementation of Perlin "simplex noise" over two dimensions
     * (x,y) and three dimensions (x,y,z). One extra parameter 't' rotates the
     * underlying gradients of the grid, which gives a swirling, flow-like
     * motion. The derivative is returned, to make it possible to do pseudo-
     * advection and implement "flow noise", as presented by Ken Perlin and
     * Fabrice Neyret at Siggraph 2001.
     *
     * When not animated and presented in one octave only, this noise
     * looks exactly the same as the plain version of simplex noise.
     * It's nothing magical by itself, although the extra animation
     * parameter 't' is useful. Fun stuff starts to happen when you
     * do fractal sums of several octaves, with different rotation speeds
     * and an advection of smaller scales by larger scales (or even the
     * other way around it you feel adventurous).
     *
     * The gradient rotations that can be performed by this noise function
     * and the true analytic derivatives are required to do flow noise.
     * You can't do it properly with regular Perlin noise.
     * The 3D version is my own creation. It's a hack, because unlike the 2D
     * version the gradients rotate around different axes, and therefore
     * they don't remain uncorrelated through the rotation, but it looks OK.
     *
     */

    /**
     * Simplex, rotating, derivative noise over 2 dimensions
     */
    float sr_perlin(const float x, const float y, const float t, float *dnoise_dx, float *dnoise_dy);

    /**
     * Simplex, rotating, derivative noise over 3 dimensions
     */
    float sr_perlin(const float x, const float y, const float z, const float t, float *dnoise_dx, float *dnoise_dy, float *dnoise_dz);
  }
}

#endif
