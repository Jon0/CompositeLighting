/*
 * PathTracer.cpp
 *
 *  Created on: 7/08/2014
 *      Author: asdf
 */

#include "PathTracer.h"

using namespace optix;

namespace std {

void PathTracerScene::initScene(InitialCameraData& camera_data) {
	m_context->setRayTypeCount(3);
	m_context->setEntryPointCount(1);
	m_context->setStackSize(1800);

	m_context["scene_epsilon"]->setFloat(1.e-3f);
	m_context["max_depth"]->setUint(m_max_depth);
	m_context["pathtrace_ray_type"]->setUint(0u);
	m_context["pathtrace_shadow_ray_type"]->setUint(1u);
	m_context["pathtrace_bsdf_shadow_ray_type"]->setUint(2u);
	m_context["rr_begin_depth"]->setUint(m_rr_begin_depth);
	m_context["display_mode"]->setUint(0);

	// buffers required for differential rendering
	final.init(m_context, "output_buffer", m_width, m_height, true);
	local.init(m_context, "output_buffer_local", m_width, m_height, false);
	all.init(m_context, "output_buffer_all", m_width, m_height, false);
	local_out.init(m_context, "output_buffer_local_out", m_width, m_height, false);
	virt_out.init(m_context, "output_buffer_virt_out", m_width, m_height, false);
	empty.init(m_context, "output_buffer_empty", "resource/ot.ppm");	// load photo from ppm file

	// Set up camera
	camera_data = InitialCameraData( make_float3( -42.067986f, 13.655909f, -7.266403f ), // eye
                                     make_float3( 0.938559f, -0.304670f, 0.162117f ),    // lookat
                                     make_float3( 0.300224f, 0.952457f, 0.051857f ),       // up
                                     32.0f ); // vfov

	// Declare these so validation will pass
	m_context["eye"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));
	m_context["U"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));
	m_context["V"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));
	m_context["W"]->setFloat(make_float3(0.0f, 0.0f, 0.0f));

	m_context["sqrt_num_samples"]->setUint(m_sqrt_num_samples);
	m_context["bad_color"]->setFloat(0.0f, 1.0f, 0.0f);
	m_context["bg_color"]->setFloat(make_float3(0.0f));

	m_context["lightmap_y_rot"]->setFloat(lightmap_y_rot);

	// Setup programs
	std::string ptx_path = ptxpath("path_tracer", "path_tracer.cu");
	Program ray_gen_program = m_context->createProgramFromPTXFile(ptx_path, "pathtrace_camera");
	m_context->setRayGenerationProgram(0, ray_gen_program);
	Program exception_program = m_context->createProgramFromPTXFile(ptx_path, "exception");
	m_context->setExceptionProgram(0, exception_program);


  if ( outline ) {
	  m_context->setMissProgram( 0, m_context->createProgramFromPTXFile( ptx_path, "miss2" ) );
  }
  else {
	  m_context->setMissProgram( 0, m_context->createProgramFromPTXFile( ptx_path, "miss" ) );
  }

  m_context["frame_number"]->setUint(1);

   // Index of sampling_stategy (BSDF, light, MIS)
  m_sampling_strategy = 0;
  m_context["sampling_stategy"]->setInt(m_sampling_strategy);

  // Create scene geometry
  // setup mesh programs
  std::string mesh_ptx_path = ptxpath( "path_tracer", "triangle_mesh_iterative.cu" );
  Program bounding_box = m_context->createProgramFromPTXFile( mesh_ptx_path, "mesh_bounds" );
  Program intersection = m_context->createProgramFromPTXFile( mesh_ptx_path, "mesh_intersect" );
  scene.setMeshPrograms(bounding_box, intersection);

  // material programs
  diffuse_ch_out = m_context->createProgramFromPTXFile( ptxpath( "path_tracer", "path_tracer.cu" ), "diffuse_outline" );
  diffuse_ch = m_context->createProgramFromPTXFile( ptxpath( "path_tracer", "path_tracer.cu" ), "diffuse" );
  diffuse_ah = m_context->createProgramFromPTXFile( ptxpath( "path_tracer", "path_tracer.cu" ), "shadow" );
  if ( outline ) {
	  scene.setMaterialPrograms(diffuse_ch_out, diffuse_ah);
  }
  else {
	  scene.setMaterialPrograms(diffuse_ch, diffuse_ah);
  }
  scene.virtualGeometry( m_context, "resource" );

  // Finalize
  m_context->validate();
  m_context->compile();
}

bool PathTracerScene::keyPressed(unsigned char key, int x, int y) {
	//std::cout << lightmap_y_rot << std::endl;
	if (key == 'j') {
		m_frame = 1;
		m_context["frame_number"]->setUint(m_frame++);
		lightmap_y_rot += 0.01;
		m_context["lightmap_y_rot"]->setFloat(lightmap_y_rot);
		return true;
	} else if (key == 'k') {
		m_frame = 1;
		m_context["frame_number"]->setUint(m_frame++);
		lightmap_y_rot -= 0.01;
		m_context["lightmap_y_rot"]->setFloat(lightmap_y_rot);
		return true;
	} else if (key == 'q') {
		m_context["display_mode"]->setUint(0);
		return true;
	} else if (isdigit(key)) {
		unsigned int newmode = key - '0';
		std::cout << "set display mode " << newmode << std::endl;
		m_context["display_mode"]->setUint(newmode);
		return true;
	}
	return false;
}

void PathTracerScene::trace(const RayGenCameraData& camera_data) {
	m_context["eye"]->setFloat(camera_data.eye);
	m_context["U"]->setFloat(camera_data.U);
	m_context["V"]->setFloat(camera_data.V);
	m_context["W"]->setFloat(camera_data.W);

	if (m_camera_changed) {
		m_camera_changed = false;
		m_frame = 1;
	}
	m_context["frame_number"]->setUint(m_frame++);


	Buffer buffer = m_context["output_buffer"]->getBuffer();
	RTsize buffer_width, buffer_height;
	buffer->getSize(buffer_width, buffer_height);
	m_context->launch(0, static_cast<unsigned int>(buffer_width),
			static_cast<unsigned int>(buffer_height));
}

//-----------------------------------------------------------------------------

Buffer PathTracerScene::getOutputBuffer() {
	return m_context["output_buffer"]->getBuffer();
}

} /* namespace std */
