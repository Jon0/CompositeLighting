/*
 * PolygonMesh.h
 *
 *  Created on: 4/09/2014
 *      Author: remnanjona
 */

#ifndef POLYGONMESH_H_
#define POLYGONMESH_H_

#include <optixu/optixpp_namespace.h>

namespace std {

// load options
struct Model {
	string filepath;
	optix::Matrix4x4 transform; // initial value
	optix::Transform tr;		// updated value
	optix::float3 colour;
	optix::float3 position;
};

class PolygonMesh {
public:
	PolygonMesh(Model &);
	virtual ~PolygonMesh();

	optix::Transform get();

	void move(float, float, float);

	void setPosition(optix::Transform &, optix::float3);

	optix::GeometryInstance makeGeometry(optix::Context &m_context, const std::string &, optix::Material);

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
