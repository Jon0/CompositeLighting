
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "helpers.h"
#include "path_tracer.h"
#include "random.h"

using namespace optix;

struct PerRayData_pathtrace {
	float3 result;
	float3 radiance;
	float3 attenuation;
	float3 origin;
	float3 direction;
	unsigned int seed;
	int depth;
	int countEmitted;
	int done;
	int inside;
	int outline;	// outline mode = 1
};

struct PerRayData_pathtrace_shadow {
	bool inShadow;
};

// Scene wide
rtDeclareVariable(float,         scene_epsilon, , );
rtDeclareVariable(rtObject,      top_object, , );
rtDeclareVariable(rtObject,      local_object, , );

// For camera
rtDeclareVariable(float3,        eye, , );
rtDeclareVariable(float3,        U, , );
rtDeclareVariable(float3,        V, , );
rtDeclareVariable(float3,        W, , );
rtDeclareVariable(float3,        bad_color, , );
rtDeclareVariable(unsigned int,  frame_number, , );
rtDeclareVariable(unsigned int,  sqrt_num_samples, , );
rtBuffer<float4, 2>              output_buffer;
rtBuffer<float4, 2>              output_buffer_empty;	// photo
rtBuffer<float4, 2>              output_buffer_local;
rtBuffer<float4, 2>              output_buffer_all;
rtBuffer<float4, 2>              output_buffer_virt_out;

// use differential rendering
rtDeclareVariable(unsigned int,  display_mode, , );

// Lighting
rtDeclareVariable(float,        lightmap_y_rot, , );

rtDeclareVariable(unsigned int,  pathtrace_ray_type, , );
rtDeclareVariable(unsigned int,  pathtrace_shadow_ray_type, , );
rtDeclareVariable(unsigned int,  rr_begin_depth, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal,   attribute shading_normal, );

rtDeclareVariable(PerRayData_pathtrace, current_prd, rtPayload, );

rtDeclareVariable(optix::Ray, ray,          rtCurrentRay, );
rtDeclareVariable(float,      t_hit,        rtIntersectionDistance, );
rtDeclareVariable(uint2,      launch_index, rtLaunchIndex, );

// For miss program
rtDeclareVariable(float3,       bg_color, , );

static __device__ inline float3 powf(float3 a, float exp) {
	return make_float3(powf(a.x, exp), powf(a.y, exp), powf(a.z, exp));
}

static __device__ inline float toLinear(float a) {
	if (a < 0.04045) {
		return a / 12.92;
	}
	else {
		return powf((a + 0.055) / 1.055, 2.4);
	}
}

static __device__ inline float toSRGB(float a) {
	if (a < 0.0031308) {
		return a * 12.92;
	}
	else {
		return 1.055*powf(a, 0.416) - 0.055;
	}
}

static __device__ float3 getRay(rtObject geometry, int outline) {
	  size_t2 screen = output_buffer.size();

	  float2 inv_screen = 1.0f/make_float2(screen) * 2.f;
	  float2 pixel = (make_float2(launch_index)) * inv_screen - 1.f;

	  float2 jitter_scale = inv_screen / sqrt_num_samples;
	  unsigned int samples_per_pixel = sqrt_num_samples*sqrt_num_samples;
	  float3 result = make_float3(0.0f);

	  unsigned int seed = tea<16>(screen.x*launch_index.y+launch_index.x, frame_number);
	  do {
	    unsigned int x = samples_per_pixel%sqrt_num_samples;
	    unsigned int y = samples_per_pixel/sqrt_num_samples;
	    float2 jitter = make_float2(x-rnd(seed), y-rnd(seed));
	    float2 d = pixel + jitter*jitter_scale;
	    float3 ray_origin = eye;
	    float3 ray_direction = normalize(d.x*U + d.y*V + W);

	    PerRayData_pathtrace prd;
	    prd.result = make_float3(0.f);
	    prd.attenuation = make_float3(1.f);
	    prd.countEmitted = true;
	    prd.done = false;
	    prd.inside = false;
	    prd.seed = seed;
	    prd.depth = 0;
	    prd.outline = outline;

	    for(;;) {
	    	// eye ray
	      Ray ray = make_Ray(ray_origin, ray_direction, pathtrace_ray_type, scene_epsilon, RT_DEFAULT_MAX);
	      rtTrace(geometry, ray, prd);
	      prd.result += prd.radiance * prd.attenuation;

	      if(prd.done) {
	        break;
	      }

	      // RR
	      prd.depth++;
	      if(prd.depth >= 0){
	        float pcont = fmaxf(prd.attenuation);
	        if(rnd(prd.seed) >= pcont)
	          break;
	        prd.attenuation *= pcont;
	      }

	      ray_origin = prd.origin;
	      ray_direction = prd.direction;
	    }

	    result += prd.result;
	    seed = prd.seed;
	  } while (--samples_per_pixel);

		float3 pixel_color = result/(sqrt_num_samples*sqrt_num_samples);
		pixel_color.x = toSRGB(pixel_color.x);
		pixel_color.y = toSRGB(pixel_color.y);
		pixel_color.z = toSRGB(pixel_color.z);

	return pixel_color;
}

static __device__ float4 getDifferential() {

	// red for virtual geometry
	float geomWeight = output_buffer_virt_out[launch_index].x;

	// green for local geometry
	float localWeight = output_buffer_virt_out[launch_index].y;

	float nongeomWeight = 1.0f - geomWeight;
	float nonlocalWeight = 1.0f - localWeight;

	float4 out = make_float4(0.0f);
	out += geomWeight * output_buffer_all[launch_index];
	out += nonlocalWeight * nongeomWeight * output_buffer_empty[launch_index];

	// local * m = geom
	// +0.01f to avoid divide by zero
	float4 m0 = (output_buffer_all[launch_index]+0.01f) / (output_buffer_local[launch_index]+0.01f);
	out += localWeight * nongeomWeight * output_buffer_empty[launch_index] * m0;

	// doesnt work?
	out = clamp(out, 0.0f, 1.0f);

	return out;
}

//-----------------------------------------------------------------------------
//
//  Camera program -- main ray tracing loop
//
//-----------------------------------------------------------------------------
RT_PROGRAM void pathtrace_camera() {
	float3 pixel_color_local = getRay(local_object, 0);
	float3 pixel_color_all = getRay(top_object, 0);


	if (frame_number > 1) {
		float a = 1.0f / (float) frame_number;
		float b = ((float) frame_number - 1.0f) * a;

		float3 old_color_all = make_float3(output_buffer_all[launch_index]);
		output_buffer_all[launch_index] = make_float4(a * pixel_color_all + b * old_color_all, 0.0f);

		float3 old_color_local = make_float3(output_buffer_local[launch_index]);
		output_buffer_local[launch_index] = make_float4(a * pixel_color_local + b * old_color_local, 0.0f);

		if (frame_number < 50) {
			float3 old_color_out = make_float3(output_buffer_virt_out[launch_index]);
			output_buffer_virt_out[launch_index] = make_float4(a * getRay(top_object, 1) + b * old_color_out, 0.0f);
		}
	} else {
		// reset buffers
		output_buffer_local[launch_index] = make_float4( pixel_color_local, 0.0f );
		output_buffer_all[launch_index] = make_float4( pixel_color_all, 0.0f );
		output_buffer_virt_out[launch_index] = make_float4( getRay(top_object, 1), 0.0f );
	}

	// final output
	if (display_mode == 1) {
		output_buffer[launch_index] = output_buffer_all[launch_index];
	}
	else if (display_mode == 2) {
		output_buffer[launch_index] = output_buffer_local[launch_index];
	}
	else if (display_mode == 3) {
		output_buffer[launch_index] = output_buffer_virt_out[launch_index];
	}
	else if (display_mode == 4) {
		output_buffer[launch_index] = output_buffer_empty[launch_index];
	}
	else {
		output_buffer[launch_index] = getDifferential();
	}
}

rtDeclareVariable(float3,        emission_color, , );

RT_PROGRAM void diffuseEmitter()
{
  current_prd.radiance = current_prd.countEmitted? emission_color : make_float3(0.f);
  current_prd.done = true;
}

rtDeclareVariable(float3,        diffuse_color, , );
rtDeclareVariable(float3,        outline_color, , );

RT_PROGRAM void diffuse_outline()
{

  current_prd.attenuation = make_float3(1.0f, 0.0f, 0.0f); // red
  current_prd.countEmitted = false;

  float3 result = make_float3(1.0f, 0.0f, 0.0f);

  current_prd.radiance = result;
  current_prd.done = true;
}

RT_PROGRAM void diffuse() {

	if (current_prd.outline == 1) {
		current_prd.countEmitted = false;
		current_prd.radiance = outline_color;
		current_prd.done = true;
		return;
	}
	float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
	float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );

	float3 ffnormal = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

	float3 hitpoint = ray.origin + t_hit * ray.direction;
	current_prd.origin = hitpoint;

	// normal distribution of outgoing rays with variance of 0.5
	//float r = sqrtf(0.5 * -2 * logf ( rnd(current_prd.seed) ) ) * cosf(2*M_PIf*rnd(current_prd.seed));

	//float ref = rnd(current_prd.seed);

	//if (ref < 0.0) {
		//float3 R = reflect( current_prd.direction, ffnormal );
		//current_prd.direction = R;
	//}
	//else {
		float z1 = rnd(current_prd.seed); // 0.5 + r/2;
		float z2 = rnd(current_prd.seed);
		float3 p;
		cosine_sample_hemisphere(z1, z2, p);
		float3 v1, v2;
		createONB(ffnormal, v1, v2);
		current_prd.direction = v1 * p.x + v2 * p.y + ffnormal * p.z;
	//}

	//float3 normal_color = (normalize(world_shading_normal) * 0.5f + 0.5f) * 0.9;
	current_prd.attenuation = current_prd.attenuation * diffuse_color; // use the diffuse_color as the diffuse response
	current_prd.countEmitted = false;

	// Compute light...
	current_prd.radiance = make_float3(0.0f);
}

rtDeclareVariable(float3,        glass_color, , );
rtDeclareVariable(float,         index_of_refraction, , );

RT_PROGRAM void glass_refract() {
	float3 world_shading_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 world_geometric_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal));
	float3 ffnormal = faceforward(world_shading_normal, -ray.direction, world_geometric_normal);

	float3 hitpoint = ray.origin + t_hit * ray.direction;
	current_prd.origin = hitpoint;
	current_prd.countEmitted = true;
	float iof;
	if (current_prd.inside) {
		// Shoot outgoing ray
		iof = 1.0f / index_of_refraction;
	} else {
		iof = index_of_refraction;
	}
	refract(current_prd.direction, ray.direction, ffnormal, iof);
	//prd.direction = reflect(ray.direction, ffnormal);

	if (current_prd.inside) {
		// Compute Beer's law
		current_prd.attenuation = current_prd.attenuation * powf(glass_color, t_hit);
	}
	current_prd.inside = !current_prd.inside;
	current_prd.radiance = make_float3(0.0f);
}

//-----------------------------------------------------------------------------
//
//  Exception program
//
//-----------------------------------------------------------------------------

RT_PROGRAM void exception()
{
  output_buffer[launch_index] = make_float4(bad_color, 0.0f);
}

//-----------------------------------------------------------------------------
//
//  Miss programs
//
//-----------------------------------------------------------------------------
rtTextureSampler<float4, 2> envmap;
RT_PROGRAM void miss() {

	if (current_prd.outline == 1) {
		current_prd.radiance = bg_color;
		current_prd.done = true;
		return;
	}

	// sample the light map
	float theta = atan2f(ray.direction.x, ray.direction.z);
	float phi = M_PIf * 0.5f - acosf(ray.direction.y);
	float u = (theta + M_PIf) * (0.5f * M_1_PIf);
	float v = 0.5f * (1.0f + sin(phi));
	float3 emap = make_float3(tex2D(envmap, u + lightmap_y_rot, v));
	//emap = emap + 2*powf(emap, 2.0f) + 4*powf(emap, 3.0f)+ 3*powf(emap, 4.0f) + 2*powf(emap, 5.0f);

	current_prd.radiance = emap;
	current_prd.done = true;
}

RT_PROGRAM void miss2()
{
  current_prd.radiance = bg_color;
  current_prd.done = true;
}

rtDeclareVariable(PerRayData_pathtrace_shadow, current_prd_shadow, rtPayload, );

RT_PROGRAM void shadow()
{
  current_prd_shadow.inShadow = true;
  rtTerminateRay();
}
