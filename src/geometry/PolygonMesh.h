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

	virtual optix::Transform get();

	virtual optix::GeometryInstance makeGeometry(optix::Context &m_context, optix::Material);

	virtual void zoom(float);
	virtual void move(glm::vec3);
	virtual void rotate(glm::quat);

	void setPosition(optix::Transform &, optix::float3, glm::quat);

	void setMaterial( optix::GeometryInstance& gi,
						optix::Material material,
	                    const std::string& color_name,
	                    const optix::float3& color);

	static void initialise(optix::Context);

private:
	glm::quat model_rot;
	Model model;
	static bool initialised;
	static optix::Program bounding_box;
	static optix::Program intersection;
};

} /* namespace std */

#endif /* POLYGONMESH_H_ */
