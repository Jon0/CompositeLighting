/*
 * PointCloud.h
 *
 *  Created on: 6/09/2014
 *      Author: remnanjona
 */

#ifndef POINTCLOUD_H_
#define POINTCLOUD_H_

#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>

#include "Geometry.h"

namespace std {

optix::float3 tofloat3(pcl::PointXYZ &);
optix::float3 tofloat3(pcl::Normal &);

// TODO wrapper for pcl point cloud class
class PointCloud: public Geometry {
public:
	PointCloud();
	PointCloud(vector<optix::float3>, vector<optix::float3>);
	virtual ~PointCloud();

	virtual void zoom(float);
	virtual void move(glm::vec3);
	virtual void rotate(glm::quat);

	virtual optix::Transform get();

	virtual optix::GeometryInstance makeGeometry(optix::Context &m_context, optix::Material);

	void makeSphere(float);

	void load(string);

	static void initialise(optix::Context);

private:
	optix::float3 initial_pos;
	optix::Transform tr;
	optix::Geometry pc;

	// optix types maybe better for doing math with
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud;
	pcl::PointCloud<pcl::Normal>::Ptr normals;

	// make optix buffer of nearby points
	void bufferSubset(optix::Context &, optix::Geometry &, pcl::PointXYZ, float);

	static bool initialised;
	static optix::Program bounding_box;
	static optix::Program intersection;

};

} /* namespace std */

#endif /* POINTCLOUD_H_ */
