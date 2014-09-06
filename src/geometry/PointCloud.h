/*
 * PointCloud.h
 *
 *  Created on: 6/09/2014
 *      Author: remnanjona
 */

#ifndef POINTCLOUD_H_
#define POINTCLOUD_H_

#include "Geometry.h"

namespace std {

class PointCloud: public Geometry {
public:
	PointCloud();
	virtual ~PointCloud();

	virtual void move(float, float, float);

	virtual optix::Transform get();

	virtual optix::GeometryInstance makeGeometry(optix::Context &m_context, const std::string &, optix::Material);

	static void initialise(optix::Context);

private:
	optix::float3 initial_pos;
	optix::Transform tr;

	static bool initialised;
	static optix::Program bounding_box;
	static optix::Program intersection;

};

} /* namespace std */

#endif /* POINTCLOUD_H_ */
