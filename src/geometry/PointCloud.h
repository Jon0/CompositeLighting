/*
 * PointCloud.h
 *
 *  Created on: 6/09/2014
 *      Author: remnanjona
 */

#ifndef POINTCLOUD_H_
#define POINTCLOUD_H_

#include <optixu/optixpp_namespace.h>

namespace std {

class PointCloud {
public:
	PointCloud();
	virtual ~PointCloud();

	static void initialise(optix::Context);

private:
	static bool initialised;
	static optix::Program bounding_box;
	static optix::Program intersection;

};

} /* namespace std */

#endif /* POINTCLOUD_H_ */
