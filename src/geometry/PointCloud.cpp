/*
 * PointCloud.cpp
 *
 *  Created on: 6/09/2014
 *      Author: remnanjona
 */

#include <iostream>

#include <pcl/features/integral_image_normal.h>

#include "../PathTracer.h"
#include "PointCloud.h"

namespace std {

optix::float3 tofloat3(pcl::PointXYZ &p) {
	return optix::make_float3(p.x, p.y, p.z);
}

optix::float3 tofloat3(pcl::Normal &n) {
	return optix::make_float3(n.normal_x, n.normal_y, n.normal_z);
}

bool PointCloud::initialised;
optix::Program PointCloud::bounding_box;
optix::Program PointCloud::intersection;

void PointCloud::initialise(optix::Context c) {
	string mesh_ptx_path = ptxpath("path_tracer", "PointCloud.cu");
	bounding_box = c->createProgramFromPTXFile(mesh_ptx_path, "cloud_bounds");
	intersection = c->createProgramFromPTXFile(mesh_ptx_path, "cloud_intersect");
	initialised = true;
}

PointCloud::PointCloud(): cloud(new pcl::PointCloud<pcl::PointXYZ>),
		normals(new pcl::PointCloud<pcl::Normal>) {
	initial_pos = optix::make_float3(0.0f, 0.0f, 0.0f);
}

PointCloud::PointCloud(vector<optix::float3> v, vector<optix::float3> n): PointCloud() {
	for (optix::float3 &p: v) {
		cloud->push_back(pcl::PointXYZ(p.x, p.y, p.z));
	}
	for (optix::float3 &p: n) {
		normals->push_back(pcl::Normal(p.x, p.y, p.z));
	}
}

PointCloud::~PointCloud() {}

void PointCloud::bufferSubset(optix::Context &c, optix::Geometry &, pcl::PointXYZ focus, float radius) {
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloudSubset(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::Normal>::Ptr normalSubset(new pcl::PointCloud<pcl::Normal>);

	unsigned int cSize = cloud->size();
	for (int i = 0; i < cSize; ++i) {

		// check within radius, and normal faces towards given point
		pcl::PointXYZ &tp = cloud->points[i];
		pcl::Normal &tn = normals->points[i];
		//float d = (tp - focus).length();

		cloudSubset->push_back(tp);
		normalSubset->push_back(tn);
	}

	// print size
	unsigned int num_vertices = cloudSubset->size();
	cout << "point cloud with " << num_vertices << " points" << endl;

	// Create vertex buffer
	optix::Buffer m_vbuffer = c->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, num_vertices);
	optix::float3* vbuffer_data = static_cast<optix::float3 *>(m_vbuffer->map());

	// Normal buffer
	optix::Buffer m_nbuffer = c->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, num_vertices);
	optix::float3* nbuffer_data = static_cast<optix::float3 *>(m_nbuffer->map());

	// copy buffer
	for (int i = 0; i < num_vertices; ++i) {
		vbuffer_data[i] = tofloat3(cloudSubset->points[i]);
		nbuffer_data[i] = tofloat3(normalSubset->points[i]);
	}
	m_vbuffer->unmap();
	m_nbuffer->unmap();

	// set geometry
	pc->setPrimitiveCount( num_vertices );
	pc[ "vertex_buffer" ]->setBuffer( m_vbuffer );
	pc[ "normal_buffer" ]->setBuffer( m_nbuffer );

}

void PointCloud::zoom(float) {

}

void PointCloud::move(glm::vec3 v) {

}

void PointCloud::rotate(glm::quat) {

}

optix::Transform PointCloud::get() {
	return tr;
}

optix::GeometryInstance PointCloud::makeGeometry(optix::Context &c, optix::Material material) {
	if (!initialised) {
		throw runtime_error("point cloud programs not initialised");
	}

	// make geometry group containing this instance
	pc = c->createGeometry();
	pc->setIntersectionProgram( intersection);
	pc->setBoundingBoxProgram( bounding_box );
	bufferSubset(c, pc, pcl::PointXYZ(0,0,0), 5.0f);


	optix::GeometryInstance instance = c->createGeometryInstance( pc, &material, &material+1 );
	instance->addMaterial(material);
	instance["diffuse_color"]->setFloat(optix::make_float3(0.95f, 0.95f, 0.95f));

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

void PointCloud::makeSphere(float radius) {
	// generate a sphere
	unsigned int num_vertices = 10000 * radius * radius;
	for (int i = 0; i < num_vertices; ++i) {
		float a = -1.0f + static_cast<float>(rand())
						/ (static_cast<float>(RAND_MAX / 2.0f));
		float b = -1.0f+ static_cast<float>(rand())
						/ (static_cast<float>(RAND_MAX / 2.0f));
		float c = -1.0f+ static_cast<float>(rand())
						/ (static_cast<float>(RAND_MAX / 2.0f));
		float l = sqrtf(a * a + b * b + c * c);
		a /= l;
		b /= l;
		c /= l;

		optix::float3 ppos = initial_pos + radius * optix::make_float3(a, b, c);

		cloud->push_back(pcl::PointXYZ(ppos.x, ppos.y, ppos.z));
		normals->push_back(pcl::Normal(a, b, c));
	}
}

void PointCloud::load(string fname) {
	if (pcl::io::loadPCDFile < pcl::PointXYZ > (fname, *cloud) == -1) {
		PCL_ERROR(("Couldn't read file "+fname+"\n").c_str());
		return;
	}
	cout << "Loaded " << cloud->width * cloud->height
			<< " data points from test_pcd.pcd with the following fields: "
			<< endl;

	pcl::IntegralImageNormalEstimation < pcl::PointXYZ, pcl::Normal > ne;
	ne.setNormalEstimationMethod(ne.AVERAGE_3D_GRADIENT);
	ne.setMaxDepthChangeFactor(0.02f);
	ne.setNormalSmoothingSize(10.0f);
	ne.setInputCloud(cloud);
	ne.compute(*normals);
}

} /* namespace std */
