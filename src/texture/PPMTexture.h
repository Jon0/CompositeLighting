/*
 * Renderer.h
 *
 *  Created on: 14/08/2014
 *      Author: asdf
 */

#ifndef PPMTEXTURE_H_
#define PPMTEXTURE_H_

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "Texture.h"

namespace std {

class PPMTexture: public Texture {
public:
	PPMTexture();
	virtual ~PPMTexture();

	virtual inline int width();
	virtual inline int height();


	void init(optix::Context &, string, int, int, bool);
	void init(optix::Context &, string, string);

	void setImage(optix::Context &, string, string);

private:
	string buf_name;
	int index, i_width, i_height;
	RTformat format;
	optix::Buffer makeBuffer(optix::Context &m_context, bool useGL);
};

} /* namespace std */

#endif /* PPMTEXTURE_H_ */
