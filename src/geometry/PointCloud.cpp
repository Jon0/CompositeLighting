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
	string mesh_ptx_path = ptxpath("path_tracer", "point_cloud.cu");
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

} /* namespace std */
