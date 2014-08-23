/*
 * Renderer.h
 *
 *  Created on: 14/08/2014
 *      Author: asdf
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

namespace std {

class Renderer {
public:
	Renderer();
	virtual ~Renderer();

	void init(optix::Context &, string, int, int, bool);
	void init(optix::Context &, string, string);

	void setImage(optix::Context &, string, string);

private:
	string buf_name;
	int index, width, height;
	RTformat format;
	optix::Buffer makeBuffer(optix::Context &m_context, bool useGL);


	static int numRenderers;
};

} /* namespace std */

#endif /* RENDERER_H_ */
