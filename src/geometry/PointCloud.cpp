/*
 * PointCloud.cpp
 *
 *  Created on: 6/09/2014
 *      Author: remnanjona
 */

#include "../PathTracer.h"

#include "PointCloud.h"

namespace std {

bool PointCloud::initialised;
optix::Program PointCloud::bounding_box;
optix::Program PointCloud::intersection;

void PointCloud::initialise(optix::Context c) {
	string mesh_ptx_path = ptxpath("path_tracer", "PointCloud.cu");
	bounding_box = c->createProgramFromPTXFile(mesh_ptx_path, "mesh_bounds");
	intersection = c->createProgramFromPTXFile(mesh_ptx_path, "mesh_intersect");
	initialised = true;
}

PointCloud::PointCloud() {
	// TODO Auto-generated constructor stub

}

PointCloud::~PointCloud() {
	// TODO Auto-generated destructor stub
}

void PointCloud::load(optix::Context c) {
	unsigned int num_vertices = 10;

	// Create vertex buffer
	optix::Buffer m_vbuffer = c->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, num_vertices);
	optix::float3* vbuffer_data = static_cast<optix::float3 *>(m_vbuffer->map());


	for (int i = 0; i < num_vertices; ++i) {
		float a = rand();
		float b = rand();
		float c = rand();
		float l = sqrtf(a*a+b*b+c*c);
		a /= l;
		b /= l;
		c /= l;
		vbuffer_data[i].x = a;
		vbuffer_data[i].y = b;
		vbuffer_data[i].z = c;
	}

	m_vbuffer->unmap();
}

} /* namespace std */
