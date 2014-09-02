/*
 * Display.cpp
 *
 *  Created on: 23/08/2014
 *      Author: asdf
 */

#include <iostream>

#include "GLFWDisplay.h"

namespace std {

PathTracer *GLFWDisplay::ptr;

GLFWDisplay::GLFWDisplay() {
	if (!glfwInit()) return;

	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	window = glfwCreateWindow(100, 100, "Window", NULL, NULL);
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return;
	}
}

GLFWDisplay::~GLFWDisplay() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void GLFWDisplay::setBuffer(optix::Buffer b) {
	shared_buffer = b;
}

void GLFWDisplay::run(PathTracer &pt) {
	ptr = &pt;
	if (!shared_buffer) {
		cout << "need to set buffer before running" << endl;
		return;
	}
    RTsize buffer_width_rts, buffer_height_rts;
    shared_buffer->getSize( buffer_width_rts, buffer_height_rts );
    int width  = static_cast<int>(buffer_width_rts);
    int height = static_cast<int>(buffer_height_rts);

    // complete window setup
    glfwSetWindowSize(window, width, height);
    glfwSetKeyCallback(window, keyFunc);

	GLrenderer renderer( shared_buffer->getGLBOId(), width, height );
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	pt.trace();
	renderer.draw();
	glFlush();
	glfwSwapBuffers(window);
	glfwShowWindow(window);

	// start render loop
	while (!glfwWindowShouldClose(window)) {
		pt.trace();
		renderer.draw();
		glFlush();
		glfwSwapBuffers(window);
		glfwPollEvents();
		if (int error = glGetError()) cout << "error " << error << endl;
	}
}

void GLFWDisplay::draw(optix::Buffer) {

}

void GLFWDisplay::keyFunc(GLFWwindow *w, int key, int scan, int act, int) {
	if (act == GLFW_PRESS && isdigit(key)) {
		unsigned int newmode = key - '0';
		ptr->setDisplayMode(newmode);
	}
}

} /* namespace std */
