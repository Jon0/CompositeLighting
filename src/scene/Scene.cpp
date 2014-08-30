/*
 * Scene.cpp
 *
 *  Created on: 26/07/2014
 *      Author: asdf
 */

#include <algorithm>
#include <fstream>
#include <iostream>

#include <SampleScene.h>
#include <ObjLoader.h>

#include "../exr/imageio.h"
#include "Scene.h"


namespace std {

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
    	item.erase(remove(item.begin(), item.end(), ' '), item.end());
        elems.push_back(item);
    }
    return elems;
}

Scene::Scene() {
	float scale = 1.0f;

	// input camera location

	// input light map
	//lightmap_path = "resource/outside.ppm";
	//lightmap_path = "resource/ennis.exr";
	lightmap_path = "resource/vuw_quad_hdr_5024.exr";

	// input local models
	addModel(local_models, "/base.obj", scale, optix::make_float3( 0.0f, 0.0f, 0.0f ),
			optix::make_float3( 0.9f, 0.9f, 0.9f ));
	addModel(local_models, "/ycube.obj", scale, optix::make_float3( 0.0f, 0.0f, 0.0f ),
			optix::make_float3( 1.0f, 0.913f, 0.137f ));
	addModel(local_models, "/bcyl.obj", scale, optix::make_float3( 0.0f, 0.0f, 0.0f ),
			optix::make_float3( 0.131f, 0.331f, 0.745f ));
	addModel(local_models, "/gpipe.obj", scale, optix::make_float3( 0.0f, 0.0f, 0.0f ),
			optix::make_float3( 0.031f, 0.804f, 0.101f ));
	addModel(local_models, "/bpln.obj", scale, optix::make_float3( 0.0f, 0.0f, 0.0f ),
			optix::make_float3( 0.08f, 0.08f, 0.08f ));

	// input virtual models
	addModel(models, "/cognacglass.obj", scale, scale * optix::make_float3( 0.0f, 0.0f, -8.0f ),
			optix::make_float3( 0.9f, 0.9f, 0.9f ));
	addModel(models, "/wineglass.obj", scale, scale * optix::make_float3( 0.0f, 0.0f, 1.0f ),
			optix::make_float3( 0.3f, 0.8f, 0.3f ));
	addModel(models, "/waterglass.obj", scale, scale * optix::make_float3( -5.0f, 0.0f, 1.0f ),
			optix::make_float3( 0.8f, 0.4f, 0.05f ));
	addModel(models, "/dragon.obj", 20*scale, scale * optix::make_float3( -6.0f, 5.7f, 20.0f ),
			optix::make_float3( 0.3f, 0.4f, 0.85f ));
}

Scene::~Scene() {}

void Scene::addOption(string op) {
	vector<string> ss = split(op, '=');
    if (ss.size() == 2) {
    	options.insert(make_pair(ss[0], ss[1]));
    }
}

void Scene::loadConfig(string fname) {
	cout << "config file = " << fname << endl;
	ifstream ifs(fname, ifstream::in);
    for (string line; getline(ifs, line); ) {
    	addOption(line);
    }
}

Texture &Scene::getPhoto() {
	return photo;
}

void Scene::init(optix::Context &m_context) {
	context = optix::Context(m_context);
	maingroup = context->createGroup();
	localgroup = context->createGroup();
	virtgroup = context->createGroup();
	emptygroup = context->createGroup();
	virtualGeometry(options["directory"]);
	photo.init(context, "output_buffer_empty", "resource/ot.ppm");	// load photo from ppm file
}

void Scene::addModel(vector<Model> &set, string fname, float scale, optix::float3 pos, optix::float3 c) {
	Model m;
	m.filepath = fname;
	float f[4*4] = {
			scale,  0,  0, 	0.0f,
            0,  scale,  0,  0.0f,
            0,  0,  scale,  0.0f,
            0,  0,  0,  scale
	};
	m.transform = optix::Matrix4x4(f);
	m.colour = c;
	m.position = pos;
	set.push_back(m);
}

void Scene::setMeshPrograms(optix::Program bb, optix::Program inter) {
	m_pgram_bounding_box = bb;
	m_pgram_intersection = inter;
}

void Scene::setMaterialPrograms(optix::Program ch, optix::Program ah) {
	diffuse_ch = ch;
	diffuse_ah = ah;
}

void Scene::modify(float k) {
	for (int i = 0; i < models.size(); ++i) {
		models[i].position.y += k;
		setPosition(models[i].tr, models[i].position);
	}

	maingroup->getAcceleration()->markDirty();
	virtgroup->getAcceleration()->markDirty();
}

void Scene::virtualGeometry( const std::string& path ) {
	cout << "make geometry and materials" << endl;
	optix::Material material = createMaterials(context, "diffuse");

	string full_path = string( sutilSamplesDir() ) + lightmap_path;
	const optix::float3 default_color = optix::make_float3( 0.8f, 0.88f, 0.97f );
	context["envmap"]->setTextureSampler( loadExrTexture( lightmap_path.c_str(), context, default_color) );

	// Load OBJ files and set as geometry groups
	cout << "reading " << (local_models.size() + models.size()) << " models" << endl;
	maingroup->setChildCount( local_models.size() + models.size() );
	localgroup->setChildCount( local_models.size() );
	virtgroup->setChildCount( models.size() );
	emptygroup->setChildCount( 0 );

	// setup each model
	for (int i = 0; i < models.size(); ++i) {
		optix::GeometryInstance gi = makeGeometry(context, path, models[i], material);
		optix::Variable v = gi->declareVariable("outline_color");
		v->set3fv(new float[3]{1.0, 0.0, 0.0});
		maingroup->setChild(i, models[i].tr);
		virtgroup->setChild(i, models[i].tr);
	}


	// local models go in both groups
	for (int i = 0; i < local_models.size(); ++i) {
		optix::GeometryInstance gi = makeGeometry(context, path, local_models[i], material);
		optix::Variable v = gi->declareVariable("outline_color");
		v->set3fv(new float[3]{0.0, 1.0, 0.0});
		maingroup->setChild(models.size() + i, local_models[i].tr);
		localgroup->setChild(i, local_models[i].tr);
	}

	maingroup->setAcceleration(context->createAcceleration("Bvh", "Bvh")); // MedianBvh, BvhSingle
	localgroup->setAcceleration(context->createAcceleration("Bvh", "Bvh"));
	virtgroup->setAcceleration(context->createAcceleration("Bvh", "Bvh"));
	emptygroup->setAcceleration(context->createAcceleration("Bvh", "Bvh"));
	context["top_object"]->set(maingroup);
	context["local_object"]->set(localgroup);
	context["virt_object"]->set(virtgroup);
	context["empty_object"]->set(emptygroup);
}

optix::GeometryInstance Scene::makeGeometry( optix::Context &m_context, const std::string& path, Model &model, optix::Material material ) {
	optix::GeometryGroup model_group = m_context->createGeometryGroup();
	ObjLoader objloader0((path + model.filepath).c_str(), m_context,
			model_group, material);
	objloader0.setIntersectProgram(m_pgram_intersection);
	objloader0.load(model.transform);


	model.tr = context->createTransform();
	model.tr->setChild(model_group);
	setPosition(model.tr, model.position);


	optix::GeometryInstance gi = model_group->getChild(0);
	setMaterial(gi, material, "diffuse_color", model.colour);
	return gi;
}

void Scene::setPosition(optix::Transform &t, optix::float3 p) {
	float mod[4*4] = {
			1,  0,  0,	p.x,
            0,  1,  0,  p.y,
            0,  0,  1,  p.z,
            0,  0,  0,  1.0
	};
	const optix::Matrix4x4 id(mod);
	t->setMatrix( false, id.getData(), 0 );
}

void Scene::setMaterial( optix::GeometryInstance& gi,
									optix::Material material,
                                   const std::string& color_name,
                                   const optix::float3& color) {
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

optix::Material Scene::createMaterials( optix::Context &m_context, string name) {
	optix::Material material[1];
	material[0] = m_context->createMaterial();
	material[0]->setClosestHitProgram(0, diffuse_ch);
	material[0]->setAnyHitProgram(1, diffuse_ah);

  return material[0];
}

} /* namespace std */
