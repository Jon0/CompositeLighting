/*
 * Pipeline.h
 *
 *  Created on: 28/10/2013
 *      Author: remnanjona
 */

#ifndef PIPELINE_H_
#define PIPELINE_H_

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Shader.h"

namespace std {

class Pipeline {
public:
	GLuint name;
	vector<GLuint> stage;

	Pipeline();
	virtual ~Pipeline();

	void addStage(Shader &, GLbitfield);
};

} /* namespace std */
#endif /* PIPELINE_H_ */
