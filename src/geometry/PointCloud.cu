#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>

using namespace optix;

rtBuffer<float3> vertex_buffer;
rtBuffer<float3> normal_buffer;

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

static __device__ inline float3 perp(float3 p, float a, float b) {
	//return make_float3(a, b, 0);
	return make_float3(p.y*a - p.z*b, p.z*b - p.x*a, p.x*b - p.y*a);
}

RT_PROGRAM void cloud_intersect(int primIdx) {
	float3 p = vertex_buffer[primIdx];
	float3 nrm = normal_buffer[primIdx];


	float3 tan = normalize( cross(nrm, make_float3(0.0f, 0.0f, 1.0f) ) );
	float3 bitan = normalize( cross(nrm, tan ) );

//	float3 p0 = p + 0.05 * tan;
//	float3 p1 = p - 0.015 * tan + 0.045 * bitan;
//	float3 p2 = p - 0.015 * tan - 0.045 * bitan;

	float3 p0 = p + 0.2 * tan;
	float3 p1 = p - 0.06 * tan + 0.18 * bitan;
	float3 p2 = p - 0.06 * tan - 0.18 * bitan;

	// Intersect ray with triangle
	float3 n;
	float t, beta, gamma;
	if (intersect_triangle(ray, p0, p1, p2, n, t, beta, gamma)) {

		if (rtPotentialIntersection(t)) {
			float3 geo_n = normalize(n);
			geometric_normal = geo_n;
			shading_normal = geo_n;
			rtReportIntersection(0);
		}
	}
}

RT_PROGRAM void cloud_bounds (int primIdx, float result[6]) {
	float3 p = vertex_buffer[primIdx];

	optix::Aabb* aabb = (optix::Aabb*) result;

	aabb->m_min = p - make_float3(0.05f);
	aabb->m_max = p + make_float3(0.05f);
}
