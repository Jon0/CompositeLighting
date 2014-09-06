#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>

rtBuffer<float3> vertex_buffer;

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

RT_PROGRAM void cloud_intersect( int primIdx ) {
  float3 p = vertex_buffer[ primIdx ];

  // Intersect ray with triangle
  float3 n;
  float  t, beta, gamma;
  if( intersect_triangle( ray, p, p, p, n, t, beta, gamma ) ) {

    if(  rtPotentialIntersection( t ) ) {
      rtReportIntersection( 0 );
    }
  }
}
