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

class PolygonMesh {
public:
	PolygonMesh(optix::Context);
	virtual ~PolygonMesh();

private:
	optix::Program bounding_box;
	optix::Program intersection;
};

} /* namespace std */

#endif /* POLYGONMESH_H_ */
