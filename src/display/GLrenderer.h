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
	GLrenderer(GLuint, GLuint, GLuint);
	virtual ~GLrenderer();

	void draw();
	void copyPbo(GLuint);

private:
	Pipeline tex_pipeline;
	GLbuffer<texvec> buff;
	GLuint tex_addr, pbo_addr, width, height;
};

} /* namespace std */

#endif /* GLRENDERER_H_ */
