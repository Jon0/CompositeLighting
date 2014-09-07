/*
 * PathTracer.cpp
 *
 *  Created on: 7/08/2014
 *      Author: asdf
 */

#include <iostream>

#include <optixu/optixu_math_stream_namespace.h>

#include <Mouse.h>

#include "PathTracer.h"

using namespace optix;

namespace std {

const char* const ptxpath( const std::string& target, const std::string& base ) {
  static std::string path;
  path = std::string(sutilSamplesPtxDir()) + "/" + target + "_generated_" + base + ".ptx";
  return path.c_str();
}

Scene &PathTracer::getScene() {
	return *scene;
}

void PathTracer::setScene(shared_ptr<Scene> s, bool prepare) {
	scene = s;

	if (prepare) {
		resetScene();
	}
}

void PathTracer::keyPressed(unsigned char key) {
	//std::cout << lightmap_y_rot << std::endl;
	if (key == 'J') {
		m_camera_changed = true;
		lightmap_y_rot += 0.01;
		optix_context["lightmap_y_rot"]->setFloat(lightmap_y_rot);
	}
	else if (key == 'K') {
		m_camera_changed = true;
		lightmap_y_rot -= 0.01;
		optix_context["lightmap_y_rot"]->setFloat(lightmap_y_rot);
	}
	else if (key == 'Q') {
		m_camera_changed = true;
		scene->modify(1.0f);
	}
	else if (key == 'W') {
		m_camera_changed = true;
		scene->modify(-1.0f);
	}
	else if (key == 'U') {
		m_camera_changed = true;
	}
	else if (isdigit(key)) {
		unsigned int newmode = key - '0';
		setDisplayMode(newmode);
	}
}

void PathTracer::resetScene() {
	optix_context->setRayTypeCount(3);
	optix_context->setEntryPointCount(1);
	optix_context->setStackSize(2000);

	optix_context["scene_epsilon"]->setFloat(1.e-3f);
	optix_context["max_depth"]->setUint(m_max_depth);
	optix_context["pathtrace_ray_type"]->setUint(0u);
	optix_context["pathtrace_shadow_ray_type"]->setUint(1u);
	optix_context["pathtrace_bsdf_shadow_ray_type"]->setUint(2u);
	optix_context["rr_begin_depth"]->setUint(m_rr_begin_depth);
	optix_context["display_mode"]->setUint(1);

	// Declare these so validation will pass
	optix_context["eye"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));
	optix_context["U"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));
	optix_context["V"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));
	optix_context["W"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));

	optix_context["sqrt_num_samples"]->setUint(m_sqrt_num_samples);
	optix_context["bad_color"]->setFloat(0.0f, 1.0f, 0.0f);
	optix_context["bg_color"]->setFloat(make_float3(0.0f));

	optix_context["lightmap_y_rot"]->setFloat(lightmap_y_rot);
	optix_context["frame_number"]->setUint(1);

	// Setup programs
	string ptx_path = ptxpath("path_tracer", "path_tracer.cu");
	Program ray_gen_program = optix_context->createProgramFromPTXFile(ptx_path, "pathtrace_camera");
	Program exception_program = optix_context->createProgramFromPTXFile(ptx_path, "exception");
	optix_context->setRayGenerationProgram(0, ray_gen_program);
	optix_context->setExceptionProgram(0, exception_program);
	optix_context->setMissProgram( 0, optix_context->createProgramFromPTXFile( ptx_path, "miss" ) );

	// Index of sampling_stategy (BSDF, light, MIS)
	m_sampling_strategy = 0;
	optix_context["sampling_stategy"]->setInt(m_sampling_strategy);

	// material programs
	diffuse_ch_out = optix_context->createProgramFromPTXFile(ptxpath("path_tracer", "path_tracer.cu"), "diffuse_outline");
	diffuse_ch = optix_context->createProgramFromPTXFile(ptxpath("path_tracer", "path_tracer.cu"), "diffuse");
	diffuse_ah = optix_context->createProgramFromPTXFile(ptxpath("path_tracer", "path_tracer.cu"), "shadow");

	scene->setMaterialPrograms(diffuse_ch, diffuse_ah);
	scene->init(optix_context);

	// buffers match size loaded by scene
	Texture &t = scene->getPhoto();
	setDimensions(t.width(), t.height());
	cout << "buffer size = " << m_width << "x" << m_height << endl;

	// buffers required for differential rendering
	final.init(optix_context, "output_buffer", m_width, m_height, true);
	local.init(optix_context, "output_buffer_local", m_width, m_height, false);
	all.init(optix_context, "output_buffer_all", m_width, m_height, false);
	virt_out.init(optix_context, "output_buffer_virt_out", m_width, m_height, false);

	//enableCPURendering(false);
	//setNumDevices( optix_context->getDeviceCount() );

	// Finalize
	optix_context->validate();
	optix_context->compile();
}

void PathTracer::setDisplayMode(unsigned int newmode) {
	std::cout << "set display mode " << newmode << std::endl;
	optix_context["display_mode"]->setUint(newmode);
}

void PathTracer::trace(const SampleScene::RayGenCameraData& camera_data) {
	if (m_camera_changed) {
		//cout << camera_data.eye << camera_data.U << camera_data.V << camera_data.W << endl;
		optix_context["eye"]->setFloat(camera_data.eye);
		optix_context["U"]->setFloat(camera_data.U);
		optix_context["V"]->setFloat(camera_data.V);
		optix_context["W"]->setFloat(camera_data.W);
		m_camera_changed = false;
		m_frame = 1;
	}
	optix_context["frame_number"]->setUint(m_frame++);

	// launch cuda
	Buffer buffer = optix_context["output_buffer"]->getBuffer();
	RTsize buffer_width, buffer_height;
	buffer->getSize(buffer_width, buffer_height);
	optix_context->launch(0, static_cast<unsigned int>(buffer_width),
			static_cast<unsigned int>(buffer_height));


}

void PathTracer::trace() {
    float3 eye, U, V, W;
    scene->getCam()->getEyeUVW( eye, U, V, W );
    SampleScene::RayGenCameraData camera_data( eye, U, V, W );
    trace( camera_data );
}

//-----------------------------------------------------------------------------

Buffer PathTracer::getOutputBuffer() {
	return optix_context["output_buffer"]->getBuffer();
}

} /* namespace std */
