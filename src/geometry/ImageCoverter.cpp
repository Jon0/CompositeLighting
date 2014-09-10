/*
 * ImageCoverter.cpp
 *
 *  Created on: 10/09/2014
 *      Author: asdf
 */

#include "/home/asdf/Optix/optixtest/src/geometry/ImageCoverter.h"

namespace std {

ImageCoverter::ImageCoverter() {
	// TODO Auto-generated constructor stub

}

ImageCoverter::~ImageCoverter() {
	// TODO Auto-generated destructor stub
}

PointCloud ImageCoverter::makePointCloud(Texture &t, PinholeCamera *cam) {
	float scene_min_depth = 0.2f;
	float scene_max_depth = 50.0f;
	int w = t.width();
	int h = t.height();
	int hfw = (float) w / 2.0f;
	int hfh = (float) h / 2.0f;
	vector<optix::float3> v, n;

    optix::float3 eye, U, V, W;
    cam->getEyeUVW( eye, U, V, W );
	optix::float2 inv_screen = 1.0f/optix::make_float2(w, h) * 2.0f;

	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			float depth = t.asF(x, y);
			optix::float2 pixel = (optix::make_float2(w - x - 1, y))* inv_screen - 1.f;
			optix::float3 ray_origin = eye;
			optix::float3 ray_direction = normalize(pixel.x * U + pixel.y * V + W);

			if (depth < 0.7f && rand() < 5000000000.0 * (depth)) {
				optix::float3 point_pos = ray_origin + (scene_min_depth + depth) * ray_direction * scene_max_depth;
				v.push_back(point_pos);
				n.push_back(optix::make_float3(0, 0.1, -1.0f));
			}
		}
	}

	return PointCloud(v, n);
}

} /* namespace std */
