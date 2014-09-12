/*
 * ImageCoverter.cpp
 *
 *  Created on: 10/09/2014
 *      Author: asdf
 */

#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>
#include <pcl/io/io.h>
#include <pcl/io/pcd_io.h>
#include <pcl/features/integral_image_normal.h>

#include "ImageCoverter.h"

namespace std {

bool randomB(float prob) {
	float a = static_cast<float>(rand())
					/ (static_cast<float>(RAND_MAX));
	return a < prob;
}

vector<optix::float3> makeNormals(vector<optix::float3> points) {
	cout << "creating normals for point cloud" << endl;
	// load point cloud
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	for (optix::float3 &p: points) {
		cloud->push_back(pcl::PointXYZ(p.x, p.y, p.z));
	}
	cloud->width = points.size();
	cloud->height = 1;
	cloud->is_dense = true;

	// estimate normals
	pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);

	pcl::IntegralImageNormalEstimation < pcl::PointXYZ, pcl::Normal > ne;
	ne.setNormalEstimationMethod(ne.AVERAGE_3D_GRADIENT);
	ne.setMaxDepthChangeFactor(0.02f);
	ne.setNormalSmoothingSize(10.0f);
	ne.setInputCloud(cloud);
	ne.compute(*normals);

	vector<optix::float3> optixnm;
	for (pcl::Normal &n: *normals) {
		optixnm.push_back(optix::make_float3(n.data_c[0], n.data_c[1], n.data_c[2]));
	}
	return optixnm;
}

ImageCoverter::ImageCoverter() {}

ImageCoverter::~ImageCoverter() {}

PointCloud ImageCoverter::makePointCloud(cv::Mat &t, Camera *cam) {
	float scene_min_depth = 15.0f;
	float scene_max_depth = 45.0f;
	float scene_depth_range = scene_max_depth - scene_min_depth;
	int w = t.cols;
	int h = t.rows;
	int hfw = (float) w / 2.0f;
	int hfh = (float) h / 2.0f;
	vector<optix::float3> v, n;

    optix::float3 eye, U, V, W;
    cam->getEyeUVW( eye, U, V, W );
	optix::float2 inv_screen = 1.0f/optix::make_float2(w, h) * 2.0f;

	// change this
	optix::float3 geom_center = optix::make_float3( 1.5f, 0.0f, 12.0f );
	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			cv::Vec3b pixcolor = t.at<cv::Vec3b>(y, x);
			float depth = pixcolor.val[0] / 256.0f;
			optix::float2 pixel = (optix::make_float2(x, h - y - 1))* inv_screen - 1.f;
			optix::float3 ray_origin = eye;
			optix::float3 ray_direction = normalize(pixel.x * U + pixel.y * V + W);

			if (depth < 0.7f && randomB(depth * 3.0f)) {
				optix::float3 point_pos = ray_origin + (scene_min_depth + depth * scene_depth_range) * ray_direction;
				float l = optix::length(geom_center - point_pos);

				if (l < 6.0f) {
					v.push_back(point_pos);
					n.push_back(optix::normalize(geom_center - point_pos));
				}
			}
		}
	}

	//cout << "verts = " << v.size() << ", normals = " << n.size() << endl;
	return PointCloud(v, n);
}

} /* namespace std */
