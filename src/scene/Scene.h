/*
 * Scene.h
 *
 *  Created on: 26/07/2014
 *      Author: asdf
 */

#ifndef SCENE_H_
#define SCENE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "../geometry/Geometry.h"
#include "../texture/PPMTexture.h"

namespace std {

typedef vector<shared_ptr<Geometry>> geom_list;

/**
 * Contains scene setup information read from config file
 * config commands:
 * directory = <directory to look in>
 * photo = <photo filename>
 * lightmap = <lightmap filename>
 */
class Scene {
public:
	Scene();
	virtual ~Scene();

	string photoPath();
	string lightMapPath();


	void loadConfig(string);
	Texture &getPhoto();
	void init(optix::Context &);

	void setMaterialPrograms( optix::Program, optix::Program );

	void modify(float k);

private:
	map<string, string> options;
	optix::Context context;
	geom_list models, local_models;

	optix::Group maingroup;
	optix::Group localgroup;
	optix::Group emptygroup;

	optix::Program diffuse_ch;
	optix::Program diffuse_ah;

	PPMTexture photo;

	bool config_loaded;

	void addOption(string);

	void addModel(geom_list &, string fname, float scale, optix::float3 pos, optix::float3 c);
	void virtualGeometry( const std::string& path );

	optix::Material createMaterials(string);
};

} /* namespace std */

#endif /* SCENE_H_ */
