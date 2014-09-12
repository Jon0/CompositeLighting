/*
 * Scene.cpp
 *
 *  Created on: 26/07/2014
 *      Author: asdf
 */

#include <SampleScene.h>
#include <ObjLoader.h>

#include "../exr/imageio.h"
#include "../geometry/ImageCoverter.h"
#include "../geometry/PointCloud.h"
#include "../geometry/PolygonMesh.h"
#include "../image/OptixImage.h"
#include "Scene.h"


namespace std {

Scene::Scene() {
	initialised = false;
	modified = false;
	addModel(models, "resource/dragon.obj", 5, optix::make_float3( 1.5f, -2.0f, 12.0f ),
			optix::make_float3( 0.3f, 0.95f, 0.45f ));
}

Scene::~Scene() {}

void Scene::setPhoto(cv::Mat &c) {
	photo_color = c;

	// camera setup must occur after image is loaded - to determine width and height
	setCamera();
}

void Scene::setDepthPhoto(cv::Mat &d) {
	ImageCoverter cc;
	PointCloud cl = cc.makePointCloud(d, &camera);
	//cl.load(path+"scene1_0.pcd");
	local_models.push_back(make_shared<PointCloud>(cl));
}

void Scene::setLightMap(cv::Mat &lm) {
	lightmap = lm;
}

void Scene::init(optix::Context &m_context) {
	context = optix::Context(m_context);

	// set image buffer vars
	context["output_buffer_empty"]->set( makeBuffer(context, RT_BUFFER_OUTPUT, photo_color, false) );
	//context["output_buffer_depth"]->set( makeBuffer(context, RT_BUFFER_OUTPUT, photo_depth, false) );

	// set light map smapler
	optix::Buffer lightmapBuffer = makeBuffer(context, RT_BUFFER_INPUT, lightmap, false);
	context["envmap"]->setTextureSampler( makeSampler(context, lightmapBuffer) );

	initGeometry();
	initialised = true;
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
	modified = true;
}

void Scene::initGeometry() {
	cout << "make geometry and materials" << endl;

	PointCloud::initialise(context);
	PolygonMesh::initialise(context);

	maingroup = context->createGroup();
	localgroup = context->createGroup();
	optix::Material material = createMaterials("diffuse");

	// Load OBJ files and set as geometry groups
	cout << "reading " << (local_models.size() + models.size()) << " models" << endl;
	maingroup->setChildCount( local_models.size() + models.size() );
	localgroup->setChildCount( local_models.size() );

	// setup each model
	for (int i = 0; i < models.size(); ++i) {
		optix::GeometryInstance gi = models[i]->makeGeometry(context, material);
		optix::Variable v = gi->declareVariable("outline_color");
		v->set3fv(new float[3]{1.0, 0.0, 0.0});
		maingroup->setChild(i, models[i]->get());
	}

	// local models go in both groups
	for (int i = 0; i < local_models.size(); ++i) {
		optix::GeometryInstance gi = local_models[i]->makeGeometry(context, material);
		optix::Variable v = gi->declareVariable("outline_color");
		v->set3fv(new float[3]{0.0, 1.0, 0.0});
		maingroup->setChild(models.size() + i, local_models[i]->get());
		localgroup->setChild(i, local_models[i]->get());
	}
	maingroup->setAcceleration(context->createAcceleration("Bvh", "Bvh")); // MedianBvh, BvhSingle
	localgroup->setAcceleration(context->createAcceleration("Bvh", "Bvh"));
	context["top_object"]->set(maingroup);
	context["local_object"]->set(localgroup);
}

optix::Material Scene::createMaterials(string name) {
	optix::Material material;
	material = context->createMaterial();
	material->setClosestHitProgram(0, diffuse_ch);
	material->setAnyHitProgram(1, diffuse_ah);

  return material;
}

void Scene::setCamera() {
	// input camera location
	optix::float3 eye = optix::make_float3(0.0f, 0.0f, -12.0f); // eye
	optix::float3 lookat = optix::make_float3(0.0f, 0.0f, 0.0f);    // lookat
	optix::float3 up = optix::make_float3(0.0f, 1.0f, 0.0f);       // up
	float fov = 40.0f;

	// Initialize camera according to scene params
	camera.resize(photo_color.cols, photo_color.rows);
}

void Scene::testSetup() {
	float scale = 1.0f;

	// input camera location
	optix::float3 eye = optix::make_float3(-42.067986f, 13.655909f, -7.266403f); // eye
	optix::float3 lookat = optix::make_float3(0.938559f, -0.304670f, 0.162117f);    // lookat
	optix::float3 up = optix::make_float3(0.300224f, 0.952457f, 0.051857f);       // up
	float fov = 32.22f;

	// Initialize camera according to scene params
	camera.resize(960, 540);

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

} /* namespace std */
