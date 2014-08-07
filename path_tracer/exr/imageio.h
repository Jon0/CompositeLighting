/*
Author: Andrew Chalmers
Date:   23/04/2014

Feel free to do whatever you want.
*/

#pragma once

// OpenEXR
#include <ImfArray.h>
#include <ImfInputFile.h>
#include <ImfRgbaFile.h>


#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

optix::TextureSampler loadExrTexture( const char fileName[],
											optix::Context context,
                                            const optix::float3& default_color);


namespace Imf {

class imageio{
public:
  imageio(void);
  ~imageio(void);

  static void writeRgba1(const char fileName[], const Rgba *pixels, int width, int height);

  static void readRgba1(const char fileName[], Array2D<Rgba> &pixels, int &width, int &height);
  static void readRgba2(const char fileName[]);
  static bool ReadEXR(const char *name, float *&rgba, int &xRes, int &yRes, bool &hasAlpha);
};

}




