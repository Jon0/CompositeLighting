/*
 * Display.h
 *
 *  Created on: 23/08/2014
 *      Author: asdf
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <optixu/optixpp_namespace.h>

#include "../PathTracer.h"
#include "GLrenderer.h"

namespace std {

class Display {
public:
	Display(int, int);
	virtual ~Display();

	void run(PathTracer &);
	void draw(optix::Buffer);

private:
	GLFWwindow *window;

};

} /* namespace std */
#endif /* DISPLAY_H_ */
