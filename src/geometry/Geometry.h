/*
 * Geometry.h
 *
 *  Created on: 6/09/2014
 *      Author: asdf
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <optixu/optixpp_namespace.h>

namespace std {

class Geometry {
public:
	virtual ~Geometry() {}

	virtual void move(float, float, float) = 0;

	virtual optix::Transform get() = 0;

	virtual optix::GeometryInstance makeGeometry(optix::Context &m_context, optix::Material) = 0;
};

} /* namespace std */
#endif /* GEOMETRY_H_ */
