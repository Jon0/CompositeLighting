/*
 * GLFWDisplay.h
 *
 *  Created on: 23/08/2014
 *      Author: asdf
 */

#ifndef GLFWDISPLAY_H_
#define GLFWDISPLAY_H_

#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <optixu/optixpp_namespace.h>

#include "../PathTracer.h"
#include "GLrenderer.h"

namespace std {

class GLFWDisplay {
public:
	GLFWDisplay();
	virtual ~GLFWDisplay();

	void setBuffer(optix::Buffer);

	void run(PathTracer &);
	void draw(optix::Buffer);

private:
	GLFWwindow *window;
	optix::Buffer shared_buffer;

	static PathTracer *ptr;
	static void keyFunc(GLFWwindow *, int, int, int, int);

};

} /* namespace std */
#endif /* GLFWDISPLAY_H_ */
