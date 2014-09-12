/*
 * PolygonMesh.h
 *
 *  Created on: 4/09/2014
 *      Author: remnanjona
 */

#ifndef POLYGONMESH_H_
#define POLYGONMESH_H_

#include "Geometry.h"

namespace std {

// load options
struct Model {
	string filepath;
	optix::Matrix4x4 transform; // initial value
	optix::Transform tr;		// updated value
	optix::float3 colour;
	optix::float3 position;
};

class PolygonMesh: public Geometry {
public:
	PolygonMesh(Model &);
	virtual ~PolygonMesh();

	virtual void move(float, float, float);

	virtual optix::Transform get();

	virtual optix::GeometryInstance makeGeometry(optix::Context &m_context, optix::Material);

	void setPosition(optix::Transform &, optix::float3);

	void setMaterial( optix::GeometryInstance& gi,
						optix::Material material,
	                    const std::string& color_name,
	                    const optix::float3& color);

	static void initialise(optix::Context);

private:
	Model model;
	static bool initialised;
	static optix::Program bounding_box;
	static optix::Program intersection;
};

} /* namespace std */

#endif /* POLYGONMESH_H_ */
