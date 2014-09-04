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

#include "../texture/PPMTexture.h"

namespace std {

struct Model {
	string filepath;
	optix::Matrix4x4 transform; // initial value
	optix::Transform tr;		// updated value
	optix::float3 colour;
	optix::float3 position;
};


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

	void setMeshPrograms( optix::Program, optix::Program );
	void setMaterialPrograms( optix::Program, optix::Program );

	void modify(float k);

private:
	map<string, string> options;
	optix::Context context;
	vector<Model> models, local_models;

	optix::Group maingroup;
	optix::Group localgroup;
	optix::Group virtgroup;
	optix::Group emptygroup;

	optix::Program m_pgram_bounding_box;
	optix::Program m_pgram_intersection;

	optix::Program diffuse_ch;
	optix::Program diffuse_ah;

	PPMTexture photo;

	bool config_loaded;

	void addOption(string);

	void addModel(vector<Model> &, string fname, float scale, optix::float3 pos, optix::float3 c);
	void virtualGeometry( const std::string& path );

	optix::GeometryInstance makeGeometry(optix::Context &m_context, const std::string &, Model &, optix::Material);

	void setPosition(optix::Transform &, optix::float3);

	void setMaterial( optix::GeometryInstance& gi,
						optix::Material material,
	                    const std::string& color_name,
	                    const optix::float3& color);

	void makeMaterialPrograms( optix::Material material, const char *filename,
	                                                            const char *ch_program_name,
	                                                            const char *ah_program_name );

	optix::Material createMaterials(optix::Context &m_context, string);
};

} /* namespace std */

#endif /* SCENE_H_ */
