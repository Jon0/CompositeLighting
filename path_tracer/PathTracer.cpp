/*
 * PathTracer.cpp
 *
 *  Created on: 7/08/2014
 *      Author: asdf
 */

#include "PathTracer.h"

using namespace optix;

namespace std {

void PathTracerScene::initScene( InitialCameraData& camera_data )
{
  m_context->setRayTypeCount( 3 );
  m_context->setEntryPointCount( 1 );
  m_context->setStackSize( 1800 );

  m_context["scene_epsilon"]->setFloat( 1.e-3f );
  m_context["max_depth"]->setUint(m_max_depth);
  m_context["pathtrace_ray_type"]->setUint(0u);
  m_context["pathtrace_shadow_ray_type"]->setUint(1u);
  m_context["pathtrace_bsdf_shadow_ray_type"]->setUint(2u);
  m_context["rr_begin_depth"]->setUint(m_rr_begin_depth);


  // Setup output buffer
  Variable output_buffer = m_context["output_buffer"];
  Buffer buffer = createOutputBuffer( RT_FORMAT_FLOAT4, m_width, m_height ); // use float format buffer
  output_buffer->set(buffer);


  // Set up camera
  	  camera_data = InitialCameraData( make_float3( 1272.55f, 717.138f, -1107.04f ), // eye
                                     make_float3( 190.117f, 83.4595f, 418.087f ),    // lookat
                                     make_float3( -0.205594f, 0.946826f, 0.247492f ),       // up
                                     47.3788f );                                // vfov

  // Declare these so validation will pass
  m_context["eye"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
  m_context["U"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
  m_context["V"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );
  m_context["W"]->setFloat( make_float3( 0.0f, 0.0f, 0.0f ) );

  m_context["sqrt_num_samples"]->setUint( m_sqrt_num_samples );
  m_context["bad_color"]->setFloat( 0.0f, 1.0f, 0.0f );
  m_context["bg_color"]->setFloat( make_float3(0.0f) );

  m_context["lightmap_y_rot"]->setFloat( lightmap_y_rot );

  // Setup programs
  std::string ptx_path = ptxpath( "path_tracer", "path_tracer.cu" );
  Program ray_gen_program = m_context->createProgramFromPTXFile( ptx_path, "pathtrace_camera" );
  m_context->setRayGenerationProgram( 0, ray_gen_program );
  Program exception_program = m_context->createProgramFromPTXFile( ptx_path, "exception" );
  m_context->setExceptionProgram( 0, exception_program );


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

bool PathTracerScene::keyPressed( unsigned char key, int x, int y )
{
	std::cout << lightmap_y_rot << std::endl;
  if (key == 'j') {
	  m_frame = 1;
	  m_context["frame_number"]->setUint( m_frame++ );
	  lightmap_y_rot += 0.01;
	  m_context["lightmap_y_rot"]->setFloat( lightmap_y_rot );
	  return true;
  }
  else if (key == 'k') {
	  m_frame = 1;
	  m_context["frame_number"]->setUint( m_frame++ );
	  lightmap_y_rot -= 0.01;
	  m_context["lightmap_y_rot"]->setFloat( lightmap_y_rot );
	  return true;
  }
  else if (key == 'o') {
	  // try change the output mode
	  scene.setMaterialPrograms(diffuse_ch_out, diffuse_ah);
	  scene.virtualGeometry( m_context, "resource" );
  }
  return false;
}

void PathTracerScene::trace(const RayGenCameraData& camera_data) {
	m_context["eye"]->setFloat(camera_data.eye);
	m_context["U"]->setFloat(camera_data.U);
	m_context["V"]->setFloat(camera_data.V);
	m_context["W"]->setFloat(camera_data.W);

	Buffer buffer = m_context["output_buffer"]->getBuffer();
	RTsize buffer_width, buffer_height;
	buffer->getSize(buffer_width, buffer_height);

	if (m_camera_changed) {
		m_camera_changed = false;
		m_frame = 1;
	}

	m_context["frame_number"]->setUint(m_frame++);

	m_context->launch(0, static_cast<unsigned int>(buffer_width),
			static_cast<unsigned int>(buffer_height));

}

//-----------------------------------------------------------------------------

Buffer PathTracerScene::getOutputBuffer() {
	return m_context["output_buffer"]->getBuffer();
}

} /* namespace std */
