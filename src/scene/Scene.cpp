/*
 * Scene.cpp
 *
 *  Created on: 26/07/2014
 *      Author: asdf
 */

#include <iostream>

#include <SampleScene.h>
#include <ObjLoader.h>

#include "../exr/imageio.h"
#include "Scene.h"


namespace std {

Scene::Scene(int sceneType) {
	float scale = 1.0f;

	// input camera location

	// input light map
	//lightmap_path = "resource/outside.ppm";
	//lightmap_path = "resource/vuw_sunny_hdr_mod1_5024.exr";
	lightmap_path = "resource/vuw_quad_hdr_5024.exr";

	// input local models

	local_models.push_back(Model{"/base.obj", { scale, 0, 0, 0,
			0, scale, 0, 0,
			0, 0, scale, 0,
			0, 0, 0, scale },
			optix::make_float3( 0.9f, 0.9f, 0.9f )});
	local_models.push_back(Model{"/ycube.obj", { scale, 0, 0, 0,
			0, scale, 0, 0,
			0, 0, scale, 0,
			0, 0, 0, scale },
			optix::make_float3( 1.0f, 0.913f, 0.137f )});
	local_models.push_back(Model{"/bcyl.obj", { scale, 0, 0, 0,
			0, scale, 0, 0,
			0, 0, scale, 0,
			0, 0, 0, scale },
			optix::make_float3( 0.131f, 0.331f, 0.745f )});
	local_models.push_back(Model{"/gpipe.obj", { scale, 0, 0, 0,
			0, scale, 0, 0,
			0, 0, scale, 0,
			0, 0, 0, scale },
			optix::make_float3( 0.031f, 0.804f, 0.101f )});
	local_models.push_back(Model{"/bpln.obj", { scale, 0, 0, 0,
			0, scale, 0, 0,
			0, 0, scale, 0,
			0, 0, 0, scale },
			optix::make_float3( 0.08f, 0.08f, 0.08f )});

	// input virtual models
	models.push_back(Model{"/cognacglass.obj", { scale, 0, 0, 0,
			0, scale, 0, 0,
			0, 0, scale,-8 * scale,
			0, 0, 0, 1 * scale },
			optix::make_float3( 0.9f, 0.9f, 0.9f )});
	models.push_back(Model{"/wineglass.obj", { scale,  0,  0,  0,
            0,  scale,  0,  0,
            0,  0,  scale,  0,
            0,  0,  0,  1*scale },
			optix::make_float3( 0.3f, 0.8f, 0.3f )});
	models.push_back(Model{"/waterglass.obj", { scale,  0,  0, -5*scale,
            0,  scale,  0,  0,
            0,  0,  scale,  1*scale,
            0,  0,  0,  1*scale },
			optix::make_float3( 0.8f, 0.4f, 0.05f )});
	models.push_back(Model{"/dragon.obj", { 20*scale,  0,  0, -6*scale,
	            0,  20*scale,  0,  5.7*scale,
	            0,  0,  20*scale,  20*scale,
	            0,  0,  0,  1*scale },
				optix::make_float3( 0.3f, 0.4f, 0.85f )});

}

Scene::~Scene() {
	// TODO Auto-generated destructor stub
}

void Scene::setMeshPrograms( optix::Program bb, optix::Program inter) {
	  m_pgram_bounding_box = bb;
	  m_pgram_intersection = inter;
}

void Scene::setMaterialPrograms( optix::Program ch, optix::Program ah ) {
	diffuse_ch = ch;
	diffuse_ah = ah;
}

void Scene::virtualGeometry( optix::Context &m_context, const std::string& path ) {
	optix::Material material = createMaterials(m_context, "diffuse");

	std::string full_path = std::string( sutilSamplesDir() ) + lightmap_path;
	const optix::float3 default_color = optix::make_float3( 0.8f, 0.88f, 0.97f );
	m_context["envmap"]->setTextureSampler( loadExrTexture( lightmap_path.c_str(), m_context, default_color) );

	// Load OBJ files and set as geometry groups
	cout << "reading " << (local_models.size() + models.size()) << " models" << endl;


	optix::GeometryGroup maingroup = m_context->createGeometryGroup();
	maingroup->setChildCount( local_models.size() + models.size() );
	optix::GeometryGroup localgroup = m_context->createGeometryGroup();
	localgroup->setChildCount( local_models.size() );
	optix::GeometryGroup virtgroup = m_context->createGeometryGroup();
	virtgroup->setChildCount( models.size() );
	optix::GeometryGroup emptygroup = m_context->createGeometryGroup();
	emptygroup->setChildCount( 0 );

	// setup each model
	for (int i = 0; i < models.size(); ++i) {
		optix::GeometryInstance gi = makeGeometry(m_context, path, models[i], material);
		optix::Variable v = gi->declareVariable("outline_color");
		v->set3fv(new float[3]{1.0, 0.0, 0.0});
		maingroup->setChild(i, gi);
		virtgroup->setChild(i, gi);
	}

	// local models go in both groups
	for (int i = 0; i < local_models.size(); ++i) {
		optix::GeometryInstance gi = makeGeometry(m_context, path, local_models[i], material);
		optix::Variable v = gi->declareVariable("outline_color");
		v->set3fv(new float[3]{0.0, 1.0, 0.0});
		maingroup->setChild(models.size() + i, gi);
		localgroup->setChild(i, gi);
	}

	maingroup->setAcceleration(m_context->createAcceleration("Bvh", "Bvh")); // BvhSingle
	localgroup->setAcceleration(m_context->createAcceleration("Bvh", "Bvh"));
	virtgroup->setAcceleration(m_context->createAcceleration("Bvh", "Bvh"));
	emptygroup->setAcceleration(m_context->createAcceleration("Bvh", "Bvh"));
	m_context["top_object"]->set(maingroup);
	m_context["local_object"]->set(localgroup);
	m_context["virt_object"]->set(virtgroup);
	m_context["empty_object"]->set(emptygroup);
}

optix::GeometryInstance Scene::makeGeometry( optix::Context &m_context, const std::string& path, Model model, optix::Material material ) {
	optix::GeometryGroup model_group = m_context->createGeometryGroup();

	const optix::Matrix4x4 m(model.transform);
	ObjLoader objloader0((path + model.filepath).c_str(), m_context,
			model_group, material);
	objloader0.setIntersectProgram(m_pgram_intersection);
	objloader0.load(m);

	optix::GeometryInstance gi = model_group->getChild(0);
	setMaterial(gi, material, "diffuse_color", model.colour);
	return gi;
}

void Scene::setMaterial( optix::GeometryInstance& gi,
									optix::Material material,
                                   const std::string& color_name,
                                   const optix::float3& color)
{
  gi->addMaterial(material);
  gi[color_name]->setFloat(color);
}

void Scene::makeMaterialPrograms( optix::Material material, const char *filename,
                                                          const char *ch_program_name,
                                                          const char *ah_program_name ) {
	//optix::Program ch_program = m_context->createProgramFromPTXFile( ptxpath("path_tracer", filename), ch_program_name );
	//optix::Program ah_program = m_context->createProgramFromPTXFile( ptxpath("path_tracer", filename), ah_program_name );
	//material->setClosestHitProgram( 0, ch_program );
	//material->setAnyHitProgram( 1, ah_program );
}

optix::Material Scene::createMaterials( optix::Context &m_context, string name)
{
	optix::Material material[1];
	material[0] = m_context->createMaterial();
	material[0]->setClosestHitProgram(0, diffuse_ch);
	material[0]->setAnyHitProgram(1, diffuse_ah);

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

} /* namespace std */
