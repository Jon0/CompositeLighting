
/*
 * Copyright (c) 2008 - 2010 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

//------------------------------------------------------------------------------
//
// path_tracer.cpp: render cornell box using path tracing.
//
//------------------------------------------------------------------------------

#include <sutil.h>
#include <iostream>
#include <stdlib.h>
#include <string>

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include <GLUTDisplay.h>
#include <ImageLoader.h>
#include <ObjLoader.h>
#include <PPMLoader.h>
#include <sampleConfig.h>

#include "random.h"
#include "path_tracer.h"
#include "helpers.h"

// EXR
#include <ImfArray.h>
#include "exr/imageio.h"

using namespace std;
using namespace optix;

int sceneType = 2;
bool outline = false;

//-----------------------------------------------------------------------------
//
// PathTracerScene
//
//-----------------------------------------------------------------------------

optix::TextureSampler loadExrTexture( const char fileName[],
											optix::Context context,
                                            const float3& default_color)
{
	std::cout << "Reading " << fileName << std::endl;
	Imf::Array2D<Imf::Rgba> pixels;
	int width;
	int height;
	Imf::imageio::readRgba1(fileName, pixels, width, height);
	std::cout << "image dimensions " << width << "x" << height << std::endl;

  // Create tex sampler and populate with default values
  optix::TextureSampler sampler = context->createTextureSampler();
  sampler->setWrapMode( 0, RT_WRAP_REPEAT );
  sampler->setWrapMode( 1, RT_WRAP_REPEAT );
  sampler->setWrapMode( 2, RT_WRAP_REPEAT );
  sampler->setIndexingMode( RT_TEXTURE_INDEX_NORMALIZED_COORDINATES );
  sampler->setReadMode( RT_TEXTURE_READ_NORMALIZED_FLOAT );
  sampler->setMaxAnisotropy( 1.0f );
  sampler->setMipLevelCount( 1u );
  sampler->setArraySize( 1u );

//  if ( failed() ) {
//
//    // Create buffer with single texel set to default_color
//    optix::Buffer buffer = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_BYTE4, 1u, 1u );
//    unsigned char* buffer_data = static_cast<unsigned char*>( buffer->map() );
//    buffer_data[0] = (unsigned char)clamp((int)(default_color.x * 255.0f), 0, 255);
//    buffer_data[1] = (unsigned char)clamp((int)(default_color.y * 255.0f), 0, 255);
//    buffer_data[2] = (unsigned char)clamp((int)(default_color.z * 255.0f), 0, 255);
//    buffer_data[3] = 255;
//    buffer->unmap();
//
//    sampler->setBuffer( 0u, 0u, buffer );
//    // Although it would be possible to use nearest filtering here, we chose linear
//    // to be consistent with the textures that have been loaded from a file. This
//    // allows OptiX to perform some optimizations.
//    sampler->setFilteringModes( RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE );
//
//    return sampler;
//  }

  const unsigned int nx = width;
  const unsigned int ny = height;

  // Create buffer and populate with PPM data
  optix::Buffer buffer = context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT4, nx, ny );
  float* buffer_data = static_cast<float *>( buffer->map() );

  for ( unsigned int i = 0; i < nx; ++i ) {
    for ( unsigned int j = 0; j < ny; ++j ) {

      unsigned int ppm_index = ( (ny-j-1)*nx + (nx-i-1) );
      unsigned int buf_index = ( j*nx + i )*4;

      Imf::Rgba pix = pixels[0][ppm_index];

      //std::cout << pix << std::endl;

      buffer_data[ buf_index + 0 ] = pix.r;
      buffer_data[ buf_index + 1 ] = pix.g;
      buffer_data[ buf_index + 2 ] = pix.b;
      buffer_data[ buf_index + 3 ] = 1.0f;
    }
  }

  buffer->unmap();

  sampler->setBuffer( 0u, 0u, buffer );
  sampler->setFilteringModes( RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE );

  return sampler;
}


class PathTracerScene: public SampleScene
{
public:
  // Set the actual render parameters below in main().
  PathTracerScene()
  : m_rr_begin_depth(1u)
  , m_max_depth(100u)
  , m_sqrt_num_samples( 0u )
  , m_width(512u)
  , m_height(512u)
  {
	  lightmap_path = "/data/outside.ppm";
	  lightmap_y_rot = 1.7f;
  }

  virtual void   initScene( InitialCameraData& camera_data );
  virtual void   trace( const RayGenCameraData& camera_data );
  virtual Buffer getOutputBuffer();

  void   setNumSamples( unsigned int sns )                           { m_sqrt_num_samples= sns; }
  void   setDimensions( const unsigned int w, const unsigned int h ) { m_width = w; m_height = h; }

private:
  // Should return true if key was handled, false otherwise.
  virtual bool keyPressed(unsigned char key, int x, int y);


  void makeMaterialPrograms( Material material, const char *filename,
                                                            const char *ch_program_name,
                                                            const char *ah_program_name );

  optix::Material createMaterials(string);


  void createGlassGeometry( const std::string& path );
  void createGeometry();

  GeometryInstance createParallelogram( const float3& anchor,
                                        const float3& offset1,
                                        const float3& offset2);

  GeometryInstance createLightParallelogram( const float3& anchor,
                                             const float3& offset1,
                                             const float3& offset2,
                                             int lgt_instance = -1);
  void setMaterial( GeometryInstance& gi,
                    Material material,
                    const std::string& color_name,
                    const float3& color);

  Program        m_pgram_bounding_box;
  Program        m_pgram_intersection;

  unsigned int   m_rr_begin_depth;
  unsigned int   m_max_depth;
  unsigned int   m_sqrt_num_samples;
  unsigned int   m_width;
  unsigned int   m_height;
  unsigned int   m_frame;
  unsigned int   m_sampling_strategy;

  float lightmap_y_rot;

  string lightmap_path;
};


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
  Buffer buffer = createOutputBuffer( RT_FORMAT_FLOAT4, m_width, m_height ); // RT_FORMAT_UNSIGNED_BYTE4
  output_buffer->set(buffer);


  // Set up camera
//  camera_data = InitialCameraData( make_float3( 278.0f, 273.0f, -800.0f ), // eye
//                                   make_float3( 278.0f, 273.0f, 0.0f ),    // lookat
//                                   make_float3( 0.0f, 1.0f,  0.0f ),       // up
//                                   35.0f );                                // vfov
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

  std::string full_path = std::string( sutilSamplesDir() ) + lightmap_path; // ; //"/data/CedarCity.hdr"
  const float3 default_color = make_float3( 0.8f, 0.88f, 0.97f );

  //loadTexture( m_context, full_path, default_color)
  m_context["envmap"]->setTextureSampler( loadExrTexture( "vuw_sunny_hdr_mod1_5024.exr", m_context, default_color) );

  // Create scene geometry
  //createGeometry();
  createGlassGeometry("resource");

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
  return false;
}

void PathTracerScene::trace( const RayGenCameraData& camera_data )
{
  m_context["eye"]->setFloat( camera_data.eye );
  m_context["U"]->setFloat( camera_data.U );
  m_context["V"]->setFloat( camera_data.V );
  m_context["W"]->setFloat( camera_data.W );

  Buffer buffer = m_context["output_buffer"]->getBuffer();
  RTsize buffer_width, buffer_height;
  buffer->getSize( buffer_width, buffer_height );

  if( m_camera_changed ) {
    m_camera_changed = false;
    m_frame = 1;
  }

  m_context["frame_number"]->setUint( m_frame++ );

  m_context->launch( 0,
                    static_cast<unsigned int>(buffer_width),
                    static_cast<unsigned int>(buffer_height)
                    );
}

//-----------------------------------------------------------------------------

Buffer PathTracerScene::getOutputBuffer()
{
  return m_context["output_buffer"]->getBuffer();
}

GeometryInstance PathTracerScene::createParallelogram( const float3& anchor,
                                                       const float3& offset1,
                                                       const float3& offset2)
{
  Geometry parallelogram = m_context->createGeometry();
  parallelogram->setPrimitiveCount( 1u );
  parallelogram->setIntersectionProgram( m_pgram_intersection );
  parallelogram->setBoundingBoxProgram( m_pgram_bounding_box );

  float3 normal = normalize( cross( offset1, offset2 ) );
  float d = dot( normal, anchor );
  float4 plane = make_float4( normal, d );

  float3 v1 = offset1 / dot( offset1, offset1 );
  float3 v2 = offset2 / dot( offset2, offset2 );

  parallelogram["plane"]->setFloat( plane );
  parallelogram["anchor"]->setFloat( anchor );
  parallelogram["v1"]->setFloat( v1 );
  parallelogram["v2"]->setFloat( v2 );

  GeometryInstance gi = m_context->createGeometryInstance();
  gi->setGeometry(parallelogram);
  return gi;
}

GeometryInstance PathTracerScene::createLightParallelogram( const float3& anchor,
                                                            const float3& offset1,
                                                            const float3& offset2,
                                                            int lgt_instance)
{
  Geometry parallelogram = m_context->createGeometry();
  parallelogram->setPrimitiveCount( 1u );
  parallelogram->setIntersectionProgram( m_pgram_intersection );
  parallelogram->setBoundingBoxProgram( m_pgram_bounding_box );

  float3 normal = normalize( cross( offset1, offset2 ) );
  float d = dot( normal, anchor );
  float4 plane = make_float4( normal, d );

  float3 v1 = offset1 / dot( offset1, offset1 );
  float3 v2 = offset2 / dot( offset2, offset2 );

  parallelogram["plane"]->setFloat( plane );
  parallelogram["anchor"]->setFloat( anchor );
  parallelogram["v1"]->setFloat( v1 );
  parallelogram["v2"]->setFloat( v2 );
  parallelogram["lgt_instance"]->setInt( lgt_instance );

  GeometryInstance gi = m_context->createGeometryInstance();
  gi->setGeometry(parallelogram);
  return gi;
}

void PathTracerScene::setMaterial( GeometryInstance& gi,
                                   Material material,
                                   const std::string& color_name,
                                   const float3& color)
{
  gi->addMaterial(material);
  gi[color_name]->setFloat(color);
}

void PathTracerScene::makeMaterialPrograms( Material material, const char *filename,
                                                          const char *ch_program_name,
                                                          const char *ah_program_name )
{
  Program ch_program = m_context->createProgramFromPTXFile( ptxpath("path_tracer", filename), ch_program_name );
  Program ah_program = m_context->createProgramFromPTXFile( ptxpath("path_tracer", filename), ah_program_name );

  material->setClosestHitProgram( 0, ch_program );
  material->setAnyHitProgram( 1, ah_program );
}

optix::Material PathTracerScene::createMaterials(string name)
{
	optix::Material material[1];
  material[0] = m_context->createMaterial();

  Program diffuse_ch = m_context->createProgramFromPTXFile( ptxpath( "path_tracer", "path_tracer.cu" ), name );
  Program diffuse_ah = m_context->createProgramFromPTXFile( ptxpath( "path_tracer", "path_tracer.cu" ), "shadow" );
  material[0]->setClosestHitProgram( 0, diffuse_ch );
  material[0]->setAnyHitProgram( 1, diffuse_ah );

//  makeMaterialPrograms( material[0], "glass.cu", "closest_hit_radiance", "any_hit_shadow");
//
//  material[0]["importance_cutoff"  ]->setFloat( 0.01f );
//  material[0]["cutoff_color"       ]->setFloat( 0.2f, 0.2f, 0.2f );
//  material[0]["fresnel_exponent"   ]->setFloat( 4.0f );
//  material[0]["fresnel_minimum"    ]->setFloat( 0.1f );
//  material[0]["fresnel_maximum"    ]->setFloat( 1.0f );
//  material[0]["refraction_index"   ]->setFloat( 1.4f );
//  material[0]["refraction_color"   ]->setFloat( 0.99f, 0.99f, 0.99f );
//  material[0]["reflection_color"   ]->setFloat( 0.99f, 0.99f, 0.99f );
//  material[0]["refraction_maxdepth"]->setInt( 10 );
//  material[0]["reflection_maxdepth"]->setInt( 5 );
//  float3 extinction = make_float3(.80f, .89f, .75f);
//  material[0]["extinction_constant"]->setFloat( log(extinction.x), log(extinction.y), log(extinction.z) );
//  material[0]["shadow_attenuation"]->setFloat( 1.0f, 1.0f, 1.0f );

  return material[0];
}

void PathTracerScene::createGlassGeometry( const std::string& path ) {
	  // Set up parallelogram programs
	  std::string ptx_path = ptxpath( "path_tracer", "triangle_mesh_iterative.cu" );
	  m_pgram_bounding_box = m_context->createProgramFromPTXFile( ptx_path, "mesh_bounds" );
	  m_pgram_intersection = m_context->createProgramFromPTXFile( ptx_path, "mesh_intersect" );

	optix::Material material = createMaterials("diffuse");


  // Load OBJ files and set as geometry groups
  GeometryGroup geomgroup[3] = { m_context->createGeometryGroup(),
                                 m_context->createGeometryGroup(),
                                 m_context->createGeometryGroup() };

  // Set transformations
  float scale = 32.0f;
  const float matrix_0[4*4] = { scale,  0,  0,  0,
                                0,  scale,  0,  0,
                                0,  0,  scale, -5,
                                0,  0,  0,  1 };

  const float matrix_1[4*4] = { scale,  0,  0,  0,
                                0,  scale,  0,  0,
                                0,  0,  scale,  0,
                                0,  0,  0,  1 };

  const float matrix_2[4*4] = { scale,  0,  0, -5,
                                0,  scale,  0,  0,
                                0,  0,  scale,  0,
                                0,  0,  0,  1 };

  const optix::Matrix4x4 m0( matrix_0 );
  const optix::Matrix4x4 m1( matrix_1 );
  const optix::Matrix4x4 m2( matrix_2 );

  ObjLoader objloader0( (path + "/cognacglass.obj").c_str(), m_context, geomgroup[0], material );
  objloader0.setIntersectProgram( m_pgram_intersection );
  objloader0.load( m0 );
//  ObjLoader objloader1( (path + "/wineglass.obj").c_str(),   m_context, geomgroup[1], material );
//  objloader1.setIntersectProgram( m_pgram_intersection );
//  objloader1.load( m1 );
//  ObjLoader objloader2( (path + "/waterglass.obj").c_str(),  m_context, geomgroup[2], material );
//  objloader2.setIntersectProgram( m_pgram_intersection );
//  objloader2.load( m2 );


  GeometryInstance gi = geomgroup[0]->getChild(0);
  const float3 green = make_float3( 0.05f, 0.8f, 0.05f );
  setMaterial(gi, material, "diffuse_color", green);

  GeometryGroup maingroup = m_context->createGeometryGroup();
  maingroup->setChildCount( 1 );
  maingroup->setChild( 0, geomgroup[0]->getChild(0) );
  //maingroup->setChild( 1, geomgroup[1]->getChild(0) );
  //maingroup->setChild( 2, geomgroup[2]->getChild(0) );
  maingroup->setAcceleration( m_context->createAcceleration("Bvh","BvhSingle") );
  m_context["top_object"]->set( maingroup );
}

void PathTracerScene::createGeometry()
{
  // Set up material
  Material diffuse = m_context->createMaterial();

  if ( outline ) {
	  Program diffuse_ch = m_context->createProgramFromPTXFile( ptxpath( "path_tracer", "path_tracer.cu" ), "diffuse2" );
	  Program diffuse_ah = m_context->createProgramFromPTXFile( ptxpath( "path_tracer", "path_tracer.cu" ), "shadow" );
	  diffuse->setClosestHitProgram( 0, diffuse_ch );
	  diffuse->setAnyHitProgram( 1, diffuse_ah );

  } else {
	  Program diffuse_ch = m_context->createProgramFromPTXFile( ptxpath( "path_tracer", "path_tracer.cu" ), "diffuse" );
	  Program diffuse_ah = m_context->createProgramFromPTXFile( ptxpath( "path_tracer", "path_tracer.cu" ), "shadow" );
	  diffuse->setClosestHitProgram( 0, diffuse_ch );
	  diffuse->setAnyHitProgram( 1, diffuse_ah );
  }

  // Set up parallelogram programs
  std::string ptx_path = ptxpath( "path_tracer", "parallelogram.cu" );
  m_pgram_bounding_box = m_context->createProgramFromPTXFile( ptx_path, "bounds" );
  m_pgram_intersection = m_context->createProgramFromPTXFile( ptx_path, "intersect" );

  // create geometry instances
  std::vector<GeometryInstance> gis;

//  const float3 white = make_float3( 0.8f, 0.8f, 0.8f );
//  const float3 green = make_float3( 0.05f, 0.8f, 0.05f );
//  const float3 orange = make_float3( 0.8f, 0.4f, 0.05f );
//  const float3 grey   = make_float3( 0.2f, 0.2f, 0.2f );
//  const float3 light_em = make_float3( 15.0f, 15.0f, 5.0f );
//
//  // Floor
//  if (sceneType == 1 || sceneType == 2) {
//		gis.push_back(
//				createParallelogram(make_float3(-600.0f, 0.0f, -600.0f),
//						make_float3(0.0f, 0.0f, 1559.2f),
//						make_float3(1556.0f, 0.0f, 0.0f)));
//		setMaterial(gis.back(), diffuse, "diffuse_color", grey);
//  }
//
//  // Short block
//  if (sceneType > 1) {
//  gis.push_back( createParallelogram( make_float3( 130.0f, 165.0f, 65.0f),
//                                      make_float3( -48.0f, 0.0f, 160.0f),
//                                      make_float3( 160.0f, 0.0f, 49.0f) ) );
//  setMaterial(gis.back(), diffuse, "diffuse_color", green);
//  gis.push_back( createParallelogram( make_float3( 290.0f, 0.0f, 114.0f),
//                                      make_float3( 0.0f, 165.0f, 0.0f),
//                                      make_float3( -50.0f, 0.0f, 158.0f) ) );
//  setMaterial(gis.back(), diffuse, "diffuse_color", green);
//  gis.push_back( createParallelogram( make_float3( 130.0f, 0.0f, 65.0f),
//                                      make_float3( 0.0f, 165.0f, 0.0f),
//                                      make_float3( 160.0f, 0.0f, 49.0f) ) );
//  setMaterial(gis.back(), diffuse, "diffuse_color", green);
//  gis.push_back( createParallelogram( make_float3( 82.0f, 0.0f, 225.0f),
//                                      make_float3( 0.0f, 165.0f, 0.0f),
//                                      make_float3( 48.0f, 0.0f, -160.0f) ) );
//  setMaterial(gis.back(), diffuse, "diffuse_color", green);
//  gis.push_back( createParallelogram( make_float3( 240.0f, 0.0f, 272.0f),
//                                      make_float3( 0.0f, 165.0f, 0.0f),
//                                      make_float3( -158.0f, 0.0f, -47.0f) ) );
//  setMaterial(gis.back(), diffuse, "diffuse_color", green);
//
//  // Tall block
//  gis.push_back( createParallelogram( make_float3( 423.0f, 330.0f, 247.0f),
//                                      make_float3( -158.0f, 0.0f, 49.0f),
//                                      make_float3( 49.0f, 0.0f, 159.0f) ) );
//  setMaterial(gis.back(), diffuse, "diffuse_color", orange);
//  gis.push_back( createParallelogram( make_float3( 423.0f, 0.0f, 247.0f),
//                                      make_float3( 0.0f, 330.0f, 0.0f),
//                                      make_float3( 49.0f, 0.0f, 159.0f) ) );
//  setMaterial(gis.back(), diffuse, "diffuse_color", orange);
//  gis.push_back( createParallelogram( make_float3( 472.0f, 0.0f, 406.0f),
//                                      make_float3( 0.0f, 330.0f, 0.0f),
//                                      make_float3( -158.0f, 0.0f, 50.0f) ) );
//  setMaterial(gis.back(), diffuse, "diffuse_color", orange);
//  gis.push_back( createParallelogram( make_float3( 314.0f, 0.0f, 456.0f),
//                                      make_float3( 0.0f, 330.0f, 0.0f),
//                                      make_float3( -49.0f, 0.0f, -160.0f) ) );
//  setMaterial(gis.back(), diffuse, "diffuse_color", orange);
//  gis.push_back( createParallelogram( make_float3( 265.0f, 0.0f, 296.0f),
//                                      make_float3( 0.0f, 330.0f, 0.0f),
//                                      make_float3( 158.0f, 0.0f, -49.0f) ) );
//  setMaterial(gis.back(), diffuse, "diffuse_color", orange);
//  }

  // Create geometry group
  GeometryGroup geometry_group = m_context->createGeometryGroup(gis.begin(), gis.end());
  geometry_group->setAcceleration( m_context->createAcceleration("Bvh","Bvh") );
  m_context["top_object"]->set( geometry_group );
}

//-----------------------------------------------------------------------------
//
// main
//
//-----------------------------------------------------------------------------

void printUsageAndExit( const std::string& argv0, bool doExit = true )
{
  std::cerr
    << "Usage  : " << argv0 << " [options]\n"
    << "App options:\n"
    << "  -h  | --help                               Print this usage message\n"
    << " -rrd | --rr-begin-depth <d>                 Start Russian Roulette killing of rays at depth <d>\n"
    << "  -md | --max-depth <d>                      Maximum ray tree depth\n"
    << "  -n  | --sqrt_num_samples <ns>              Number of samples to perform for each frame\n"
    << "  -t  | --timeout <sec>                      Seconds before stopping rendering. Set to 0 for no stopping.\n"
    << std::endl;
  GLUTDisplay::printUsage();

  if ( doExit ) exit(1);
}


unsigned int getUnsignedArg(int& arg_index, int argc, char** argv)
{
  int result = -1;
  if (arg_index+1 < argc) {
    result = atoi(argv[arg_index+1]);
  } else {
    std::cerr << "Missing argument to "<<argv[arg_index]<<"\n";
    printUsageAndExit(argv[0]);
  }
  if (result < 0) {
    std::cerr << "Argument to "<<argv[arg_index]<<" must be positive.\n";
    printUsageAndExit(argv[0]);
  }
  ++arg_index;
  return static_cast<unsigned int>(result);
}

int main( int argc, char** argv )
{
  GLUTDisplay::init( argc, argv );

  // Process command line options
  unsigned int sqrt_num_samples = 1u;

  unsigned int width = 1600u, height = 900u;
  float timeout = 0.0f;

  for ( int i = 1; i < argc; ++i ) {
    std::string arg( argv[i] );
    if ( arg == "--sqrt_num_samples" || arg == "-n" ) {
      if ( i == argc-1 ) printUsageAndExit( argv[0] );
      sqrt_num_samples = atoi( argv[++i] );
    } else if ( arg == "-g" ) {
      if ( i == argc-1 ) printUsageAndExit( argv[0] );
      sceneType = atoi( argv[++i] );
      std::cout << "scene type = " << sceneType << std::endl;
    }
    else if ( arg == "-o" ) {
    	outline = true;
    }
    else if ( arg == "--timeout" || arg == "-t" ) {
      if(++i < argc) {
        timeout = static_cast<float>(atof(argv[i]));
      } else {
        std::cerr << "Missing argument to "<<arg<<"\n";
        printUsageAndExit(argv[0]);
      }
    } else if ( arg == "--help" || arg == "-h" ) {
      printUsageAndExit( argv[0] );
    } else {
      std::cerr << "Unknown option: '" << arg << "'\n";
      printUsageAndExit( argv[0] );
    }
  }

  string desc = "";
  if (sceneType == 0) {
	  desc = "scene";
  }
  else if (sceneType == 1) {
	  desc = "local";
  }
  else if (sceneType >= 2) {
	  desc = "geom";
  }

  if (outline) {
	  desc += "Out";
  }


  //if( !GLUTDisplay::isBenchmark() ) printUsageAndExit( argv[0], false );
  try {
    PathTracerScene scene;
    scene.setNumSamples( sqrt_num_samples );
    scene.setDimensions( width, height );
    GLUTDisplay::setProgressiveDrawingTimeout(timeout);
    GLUTDisplay::setUseSRGB(true);
    GLUTDisplay::run( desc, &scene, GLUTDisplay::CDProgressive );
  } catch( Exception& e ){
    sutilReportError( e.getErrorString().c_str() );
    exit(1);
  }


  return 0;
}
