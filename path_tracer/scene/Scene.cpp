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
	float scale = 48.0f;

	// input camera location

	// input light map
	//lightmap_path = "/data/outside.ppm";
	//lightmap_path = "vuw_sunny_hdr_mod1_5024.exr";
	lightmap_path = "vuw_quad_hdr_5024.exr";

	// input local models

	if (sceneType == 1 || sceneType == 2) {
			models.push_back(Model{"/plane.obj", { scale, 0, 0, 0,
					0, scale, 0, -1.3 * scale,
					0, 0, scale, 0,
					0, 0, 0, scale },
					optix::make_float3( 0.9f, 0.9f, 0.9f )});
	}
	if (sceneType >= 2) {

	// input virtual models
	models.push_back(Model{"/cognacglass.obj", { scale, 0, 0, 0,
			0, scale, 0, 0,
			0, 0, scale,-5 * scale,
			0, 0, 0, 1 * scale },
			optix::make_float3( 0.05f, 0.8f, 0.05f )});
	models.push_back(Model{"/wineglass.obj", { scale,  0,  0,  0,
            0,  scale,  0,  0,
            0,  0,  scale,  0,
            0,  0,  0,  1*scale },
			optix::make_float3( 0.8f, 0.8f, 0.8f )});
	models.push_back(Model{"/waterglass.obj", { scale,  0,  0, -5*scale,
            0,  scale,  0,  0,
            0,  0,  scale,  0,
            0,  0,  0,  1*scale },
			optix::make_float3( 0.8f, 0.4f, 0.05f )});
	}


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
	cout << "read " << models.size() << endl;
	optix::GeometryGroup geomgroup[models.size()];
	optix::GeometryGroup maingroup = m_context->createGeometryGroup();
	maingroup->setChildCount( models.size() );

	// setup each model
	for (int i = 0; i < models.size(); ++i) {
		geomgroup[i] = m_context->createGeometryGroup();
		const optix::Matrix4x4 m(models[i].transform);
		ObjLoader objloader0((path + models[i].filepath).c_str(), m_context,
				geomgroup[i], material);
		objloader0.setIntersectProgram(m_pgram_intersection);
		objloader0.load(m);

		optix::GeometryInstance gi = geomgroup[i]->getChild(0);
		setMaterial(gi, material, "diffuse_color", models[i].colour);
		maingroup->setChild(i, geomgroup[i]->getChild(0));
	}

	maingroup->setAcceleration(m_context->createAcceleration("Bvh", "BvhSingle"));
	m_context["top_object"]->set(maingroup);
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
