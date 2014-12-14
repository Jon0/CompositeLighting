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

#include "random.h"
#include "path_tracer.h"
#include "helpers.h"

#include "scene/Scene.h"

namespace std {

/**
 * find a cuda compiled binary file path
 * @param target the name of the project
 * @param base the name of the source file 
 */
const char* const ptxpath( const std::string &target, const std::string &base );

//-----------------------------------------------------------------------------
//
// PathTracer
//
//-----------------------------------------------------------------------------
class PathTracer {
public:
	// Default parameters
	// Set the actual render parameters in main().
	PathTracer() :
			m_rr_begin_depth(1u), m_max_depth(1u), m_sqrt_num_samples(1u),
			m_width(512u), m_height(512u), m_use_vbo_buffer( true ),
			m_num_devices( 0 ), m_cpu_rendering_enabled( false ) {
		optix_context = optix::Context::create();
		lightmap_y_rot = 0.28f; // TODO should be in the scene.......
	}

	Scene &getScene();
	void setScene( shared_ptr<Scene>, bool );

	void keyPressed(unsigned char key);

	void trace();
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
	/**
	 * the main setup function
	 */
	void resetScene();

	/**
	 * pass updated params to the gpu
	 */
	void updateCamera();

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

	bool m_use_vbo_buffer;
	int m_num_devices;
	bool m_cpu_rendering_enabled;

	// scene to path trace
	shared_ptr<Scene> scene;
};

} /* namespace std */

#endif /* PATHTRACER_H_ */
