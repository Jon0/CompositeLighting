/*
 * Renderer.h
 *
 *  Created on: 24/03/2014
 *      Author: remnanjona
 */

#ifndef GLRENDERER_H_
#define GLRENDERER_H_

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "GLbuffer.h"
#include "Pipeline.h"

namespace std {

struct texvec {
	glm::vec4 pos;
	glm::vec2 texCoor;
};

class GLrenderer {
public:
	GLrenderer();
	virtual ~GLrenderer();

	void draw();

private:
	int width, height;
	Pipeline tex_pipeline;

	GLbuffer<texvec> buff;
	GLuint t_addr;
};

} /* namespace std */

#endif /* GLRENDERER_H_ */
