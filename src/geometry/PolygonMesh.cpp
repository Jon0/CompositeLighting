/*
 * PolygonMesh.cpp
 *
 *  Created on: 4/09/2014
 *      Author: remnanjona
 */

#include <stdexcept>

#include <ObjLoader.h>

#include "../PathTracer.h"

#include "PolygonMesh.h"

namespace std {

bool PolygonMesh::initialised;
optix::Program PolygonMesh::bounding_box;
optix::Program PolygonMesh::intersection;

void PolygonMesh::initialise(optix::Context c) {
	string mesh_ptx_path = ptxpath("path_tracer", "PolygonMesh.cu");
	bounding_box = c->createProgramFromPTXFile(mesh_ptx_path, "mesh_bounds");
	intersection = c->createProgramFromPTXFile(mesh_ptx_path, "mesh_intersect");
	initialised = true;
}

PolygonMesh::PolygonMesh(Model &m) {
	model = m;
}

PolygonMesh::~PolygonMesh() {
	// TODO Auto-generated destructor stub
}

optix::Transform PolygonMesh::get() {
	return model.tr;
}

void PolygonMesh::move(float x, float y, float z) {
	model.position.x += x;
	model.position.y += y;
	model.position.z += z;
	setPosition(model.tr, model.position);
}

void PolygonMesh::setPosition(optix::Transform &t, optix::float3 p) {
	float mod[4*4] = {
			1,  0,  0,	p.x,
            0,  1,  0,  p.y,
            0,  0,  1,  p.z,
            0,  0,  0,  1.0
	};
	const optix::Matrix4x4 id(mod);
	t->setMatrix( false, id.getData(), 0 );
}

optix::GeometryInstance PolygonMesh::makeGeometry( optix::Context &m_context, optix::Material material ) {
	if (!initialised) {
		throw runtime_error("mesh programs not initialised");
	}

	optix::GeometryGroup model_group = m_context->createGeometryGroup();
	ObjLoader objloader0(model.filepath.c_str(), m_context, model_group, material);
	objloader0.setBboxProgram(bounding_box);
	objloader0.setIntersectProgram(intersection);
	objloader0.load(model.transform);


	model.tr = m_context->createTransform();
	model.tr->setChild(model_group);
	setPosition(model.tr, model.position);


	optix::GeometryInstance gi = model_group->getChild(0);
	setMaterial(gi, material, "diffuse_color", model.colour);
	return gi;
}

void PolygonMesh::setMaterial( optix::GeometryInstance& gi,
									optix::Material material,
                                   const std::string& color_name,
                                   const optix::float3& color) {
	gi->addMaterial(material);
	gi[color_name]->setFloat(color);
}


} /* namespace std */
