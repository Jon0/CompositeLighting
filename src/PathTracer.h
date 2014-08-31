/*
 * PathTracer.h
 *
 *  Created on: 7/08/2014
 *      Author: asdf
 */

#ifndef PATHTRACER_H_
#define PATHTRACER_H_

#include <sutil.h>
#include <iostream>
#include <stdlib.h>
#include <string>

#include <GLUTDisplay.h>
#include <ImageLoader.h>
#include <PPMLoader.h>
#include <sampleConfig.h>

#include "random.h"
#include "path_tracer.h"
#include "helpers.h"

#include "texture/PPMTexture.h"
#include "scene/Scene.h"


// EXR
#include <ImfArray.h>
#include "exr/imageio.h"

namespace std {

//-----------------------------------------------------------------------------
//
// PathTracer
//
//-----------------------------------------------------------------------------
class PathTracer: public SampleScene {
public:
	// Set the actual render parameters below in main().
	PathTracer() :
			m_rr_begin_depth(1u), m_max_depth(100u), m_sqrt_num_samples(0u),
			m_width(512u), m_height(512u) {
		lightmap_y_rot = 0.28f;
	}

	void initScene( shared_ptr<Scene>, bool );
	virtual void initScene(InitialCameraData& camera_data);

	void trace();
	virtual void trace(const RayGenCameraData& camera_data);
	virtual optix::Buffer getOutputBuffer();

	void setNumSamples(unsigned int sns) {
		m_sqrt_num_samples = sns;
	}

	void setDimensions(const unsigned int w, const unsigned int h) {
		m_width = w;
		m_height = h;
	}

private:
	void resetScene();

	// Should return true if key was handled, false otherwise.
	virtual bool keyPressed(unsigned char key, int x, int y);

	PinholeCamera *m_camera;

	optix::Context optix_context;
	optix::Program bounding_box;
	optix::Program intersection;
	optix::Program diffuse_ch_out;
	optix::Program diffuse_ch;
	optix::Program diffuse_ah;

	unsigned int m_rr_begin_depth;
	unsigned int m_max_depth;
	unsigned int m_sqrt_num_samples;
	unsigned int m_width;
	unsigned int m_height;
	unsigned int m_frame;
	unsigned int m_sampling_strategy;
	float lightmap_y_rot;

	// scene to path trace
	shared_ptr<Scene> scene;

	// textures required for differential rendering
	PPMTexture final, local, all, virt_out;
};
} /* namespace std */

#endif /* PATHTRACER_H_ */
