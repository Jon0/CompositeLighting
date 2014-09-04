/*
 * PolygonMesh.cpp
 *
 *  Created on: 4/09/2014
 *      Author: remnanjona
 */

#include "../PathTracer.h"

#include "PolygonMesh.h"

namespace std {

PolygonMesh::PolygonMesh(optix::Context optix_context) {

	string mesh_ptx_path = ptxpath("path_tracer", "triangle_mesh_iterative.cu");
	bounding_box = optix_context->createProgramFromPTXFile(mesh_ptx_path, "mesh_bounds");
	intersection = optix_context->createProgramFromPTXFile(mesh_ptx_path, "mesh_intersect");

}

PolygonMesh::~PolygonMesh() {
	// TODO Auto-generated destructor stub
}

} /* namespace std */
