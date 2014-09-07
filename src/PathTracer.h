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

const char* const ptxpath( const std::string&, const std::string& );

//-----------------------------------------------------------------------------
//
// PathTracer
//
//-----------------------------------------------------------------------------
class PathTracer {
public:
	// Set the actual render parameters below in main().
	PathTracer() :
			m_rr_begin_depth(1u), m_max_depth(100u), m_sqrt_num_samples(1u),
			m_width(512u), m_height(512u), m_camera_changed( true ), m_use_vbo_buffer( true ),
			m_num_devices( 0 ), m_cpu_rendering_enabled( false ) {
		optix_context = optix::Context::create();
		lightmap_y_rot = 0.28f; // TODO should be in the scene.......
	}

	Scene &getScene();
	void setScene( shared_ptr<Scene>, bool );

	void keyPressed(unsigned char key);

	void trace();
	void trace(const SampleScene::RayGenCameraData& camera_data);
	optix::Buffer getOutputBuffer();

	void setNumSamples(unsigned int sns) {
		m_sqrt_num_samples = sns;
	}

	void setDimensions(const unsigned int w, const unsigned int h) {
		m_width = w;
		m_height = h;
	}

	void setDisplayMode(unsigned int);

private:
	void resetScene();

	optix::Context optix_context;
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

	bool m_camera_changed;
	bool m_use_vbo_buffer;
	int m_num_devices;
	bool m_cpu_rendering_enabled;

	// scene to path trace
	shared_ptr<Scene> scene;

	// textures required for differential rendering
	PPMTexture final, local, all, virt_out;
};
} /* namespace std */

#endif /* PATHTRACER_H_ */
