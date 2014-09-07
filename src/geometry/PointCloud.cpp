/*
 * PointCloud.cpp
 *
 *  Created on: 6/09/2014
 *      Author: remnanjona
 */

#include <iostream>

#include "../PathTracer.h"

#include "PointCloud.h"

namespace std {

bool PointCloud::initialised;
optix::Program PointCloud::bounding_box;
optix::Program PointCloud::intersection;

void PointCloud::initialise(optix::Context c) {
	string mesh_ptx_path = ptxpath("path_tracer", "PointCloud.cu");
	bounding_box = c->createProgramFromPTXFile(mesh_ptx_path, "cloud_bounds");
	intersection = c->createProgramFromPTXFile(mesh_ptx_path, "cloud_intersect");
	initialised = true;
}

PointCloud::PointCloud() {
	initial_pos = optix::make_float3(-10.0f, 2.0f, 0.0f);
}

PointCloud::PointCloud(PPMTexture &t) {
	initial_pos = optix::make_float3(0.0f, 0.0f, 0.0f);

	int w = t.width();
	int h = t.height();

	for (int x = 0; x < w; ++x) {

	}



}

PointCloud::~PointCloud() {}

void PointCloud::move(float, float, float) {

}

optix::Transform PointCloud::get() {
	return tr;
}

optix::GeometryInstance PointCloud::makeGeometry(optix::Context &c, const std::string &, optix::Material material) {
	if (!initialised) {
		throw runtime_error("point cloud programs not initialised");
	}
	unsigned int num_vertices = 100000;

	// Create vertex buffer
	optix::Buffer m_vbuffer = c->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, num_vertices);
	optix::float3* vbuffer_data = static_cast<optix::float3 *>(m_vbuffer->map());

	// Normal buffer
	optix::Buffer m_nbuffer = c->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, num_vertices);
	optix::float3* nbuffer_data = static_cast<optix::float3 *>(m_nbuffer->map());

	// generate a sphere
	for (int i = 0; i < num_vertices; ++i) {
		float a = -1.0f + static_cast<float>(rand())/(static_cast<float>(RAND_MAX/2.0f));
		float b = -1.0f + static_cast<float>(rand())/(static_cast<float>(RAND_MAX/2.0f));
		float c = -1.0f + static_cast<float>(rand())/(static_cast<float>(RAND_MAX/2.0f));
		float l = sqrtf(a*a+b*b+c*c);
		a /= l;
		b /= l;
		c /= l;
		vbuffer_data[i].x = initial_pos.x + a * 5.0f;
		vbuffer_data[i].y = initial_pos.y + b * 5.0f;
		vbuffer_data[i].z = initial_pos.z + c * 5.0f;
		nbuffer_data[i].x = a;
		nbuffer_data[i].y = b;
		nbuffer_data[i].z = c;
	}
	m_vbuffer->unmap();
	m_nbuffer->unmap();

	// make geometry group containing this instance
	optix::Geometry pc = c->createGeometry();
	pc->setPrimitiveCount( num_vertices );
	pc->setIntersectionProgram( intersection);
	pc->setBoundingBoxProgram( bounding_box );
	pc[ "vertex_buffer" ]->setBuffer( m_vbuffer );
	pc[ "normal_buffer" ]->setBuffer( m_nbuffer );

	optix::GeometryInstance instance = c->createGeometryInstance( pc, &material, &material+1 );
	instance->addMaterial(material);
	instance["diffuse_color"]->setFloat(optix::make_float3(0.85f, 0.85f, 0.85f));



	optix::GeometryGroup model_group = c->createGeometryGroup();
	model_group->setChildCount( 1 );
	model_group->setChild(0, instance);


	optix::Acceleration acceleration = c->createAcceleration("Bvh", "Bvh");
	model_group->setAcceleration(acceleration);
	acceleration->markDirty();

	tr = c->createTransform();
	tr->setChild(model_group);
	return instance;
}

} /* namespace std */
