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
#include "../geometry/PointCloud.h"
#include "../geometry/PolygonMesh.h"
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
	config_loaded = false;
	float scale = 1.0f;

	// TODO input camera location

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

	models.push_back(make_shared<PointCloud>());
}

Scene::~Scene() {}

string Scene::photoPath() {
	return options["photo"];
}

string Scene::lightMapPath() {
	// input light map
	//lightmap_path = "resource/outside.ppm";
	//lightmap_path = "resource/ennis.exr";
	return options["lightmap"];
}

void Scene::addOption(string op) {
	vector<string> ss = split(op, '=');
    if (ss.size() == 2) {
    	options.insert(make_pair(ss[0], ss[1]));
    }
}

void Scene::loadConfig(string fname) {
	ifstream ifs(fname, ifstream::in);
	if (!ifs) {
		throw runtime_error(fname + " not found");
	}
	cout << "config file = " << fname << endl;
    for (string line; getline(ifs, line); ) {
    	addOption(line);
    }
    config_loaded = true;
}

Texture &Scene::getPhoto() {
	return photo;
}

void Scene::init(optix::Context &m_context) {
	context = optix::Context(m_context);
	maingroup = context->createGroup();
	localgroup = context->createGroup();
	emptygroup = context->createGroup();
	PointCloud::initialise(context);
	PolygonMesh::initialise(context);
	virtualGeometry(options["directory"]);
}

void Scene::addModel(geom_list &set, string fname, float scale, optix::float3 pos, optix::float3 c) {
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
	set.push_back(make_shared<PolygonMesh>(m));
}

void Scene::setMaterialPrograms(optix::Program ch, optix::Program ah) {
	diffuse_ch = ch;
	diffuse_ah = ah;
}

void Scene::modify(float k) {
	for (int i = 0; i < models.size(); ++i) {
		models[i]->move(0.0f, k, 0.0f);
	}
	maingroup->getAcceleration()->markDirty();
}

void Scene::virtualGeometry( const std::string& path ) {
	cout << "make geometry and materials" << endl;
	optix::Material material = createMaterials("diffuse");

	// load photo ppm
	photo.init(context, "output_buffer_empty", path + photoPath());	// load photo from ppm file

	string full_path = path + lightMapPath();
	context["envmap"]->setTextureSampler( loadExrTexture( full_path.c_str(), context, false ) );

	// Load OBJ files and set as geometry groups
	cout << "reading " << (local_models.size() + models.size()) << " models" << endl;
	maingroup->setChildCount( local_models.size() + models.size() );
	localgroup->setChildCount( local_models.size() );
	emptygroup->setChildCount( 0 );

	// setup each model
	for (int i = 0; i < models.size(); ++i) {
		optix::GeometryInstance gi = models[i]->makeGeometry(context, path, material);
		optix::Variable v = gi->declareVariable("outline_color");
		v->set3fv(new float[3]{1.0, 0.0, 0.0});
		maingroup->setChild(i, models[i]->get());
	}

	// local models go in both groups
	for (int i = 0; i < local_models.size(); ++i) {
		optix::GeometryInstance gi = local_models[i]->makeGeometry(context, path, material);
		optix::Variable v = gi->declareVariable("outline_color");
		v->set3fv(new float[3]{0.0, 1.0, 0.0});
		maingroup->setChild(models.size() + i, local_models[i]->get());
		localgroup->setChild(i, local_models[i]->get());
	}

	cout << "finished loading" << endl;
	maingroup->setAcceleration(context->createAcceleration("Bvh", "Bvh")); // MedianBvh, BvhSingle
	localgroup->setAcceleration(context->createAcceleration("Bvh", "Bvh"));
	emptygroup->setAcceleration(context->createAcceleration("Bvh", "Bvh"));
	context["top_object"]->set(maingroup);
	context["local_object"]->set(localgroup);
	context["empty_object"]->set(emptygroup);
}

optix::Material Scene::createMaterials(string name) {
	optix::Material material;
	material = context->createMaterial();
	material->setClosestHitProgram(0, diffuse_ch);
	material->setAnyHitProgram(1, diffuse_ah);

  return material;
}

} /* namespace std */
