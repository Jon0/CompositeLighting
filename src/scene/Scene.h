/*
 * Scene.h
 *
 *  Created on: 26/07/2014
 *      Author: asdf
 */

#ifndef SCENE_H_
#define SCENE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <opencv2/core/core.hpp>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "../geometry/Geometry.h"
#include "Camera.h"

namespace std {

typedef vector<shared_ptr<Geometry>> geom_list;

/**
 * Scene contains geometry, lighting and a camera
 */
class Scene {
public:
	Scene();
	virtual ~Scene();

	inline Camera *getCam() {
		return &camera;
	}

	inline cv::Mat &getPhoto() {
		return photo_color;
	}

	inline bool isModified() {
		return modified || camera.isModified();
	}

	inline void setModified(bool m) {
		modified = m;
	}

	void setPhoto(cv::Mat &);
	void setDepthPhoto(cv::Mat &);
	void setLightMap(cv::Mat &);

	void init(optix::Context &);
	void modify(float k);

	// to be removed
	void setMaterialPrograms( optix::Program, optix::Program );

private:
	optix::Context context;

	Camera camera;
	geom_list models, local_models;
	optix::Group maingroup, localgroup;

	cv::Mat photo_color, lightmap;
	bool initialised, modified;



	// move to material class
	optix::Program diffuse_ch;
	optix::Program diffuse_ah;

	void addModel(geom_list &, string fname, float scale, optix::float3 pos, optix::float3 c);

	void initGeometry();

	optix::Material createMaterials(string);

	void setCamera();

	void testSetup();
};

} /* namespace std */

#endif /* SCENE_H_ */
