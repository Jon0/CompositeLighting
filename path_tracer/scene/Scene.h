/*
 * Scene.h
 *
 *  Created on: 26/07/2014
 *      Author: asdf
 */

#ifndef SCENE_H_
#define SCENE_H_

#include <string>
#include <vector>

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

namespace std {

struct Model {
	string filepath;
	float transform[4*4];
	optix::float3 colour;
};

class Scene {
public:
	Scene(int);
	virtual ~Scene();

	void setMeshPrograms( optix::Program, optix::Program );
	void setMaterialPrograms( optix::Program, optix::Program );

	void virtualGeometry( optix::Context &m_context, const std::string& path );


	void setMaterial( optix::GeometryInstance& gi,
						optix::Material material,
	                    const std::string& color_name,
	                    const optix::float3& color);

	void makeMaterialPrograms( optix::Material material, const char *filename,
	                                                            const char *ch_program_name,
	                                                            const char *ah_program_name );

	  optix::Material createMaterials(optix::Context &m_context, string);

private:
	  vector<Model> models;

	  optix::Program m_pgram_bounding_box;
	  optix::Program m_pgram_intersection;

	  optix::Program diffuse_ch;
	  optix::Program diffuse_ah;

	  string lightmap_path;
};

} /* namespace std */

#endif /* SCENE_H_ */
