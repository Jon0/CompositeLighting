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
#include "Arcball.h"
#include "GLrenderer.h"

namespace std {

class GLFWDisplay {
public:
	GLFWDisplay();
	virtual ~GLFWDisplay();

	void setBuffer(optix::Buffer);
	void run(PathTracer &);

private:
	GLFWwindow *window;
	optix::Buffer shared_buffer;

	static uint width, height;
	static int mb1, mb2;
	static double mx, my;
	static Arcball *arcball;
	static Scene *sn;
	static Camera *cam;
	static PathTracer *ptr;
	static void keyFunc(GLFWwindow *, int, int, int, int);
	static void mouseFunc(GLFWwindow *, int, int, int);
	static void posFunc(GLFWwindow *, double, double);
	static void scrollFunc(GLFWwindow *, double, double);

};

} /* namespace std */
#endif /* GLFWDISPLAY_H_ */
