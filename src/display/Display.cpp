/*
 * Display.cpp
 *
 *  Created on: 23/08/2014
 *      Author: asdf
 */

#include <iostream>

#include "Display.h"

namespace std {

Display::Display(int width, int height) {
	if (!glfwInit()) return;
	window = glfwCreateWindow(width, height, "Window", NULL, NULL);
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return;
	}
}

Display::~Display() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Display::run() {
	while (!glfwWindowShouldClose(window)) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);


		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		glFlush();
		glfwSwapBuffers(window);
		glfwPollEvents();
		if (int error = glGetError()) cout << "error " << error << endl;
	}
}

void Display::draw(optix::Buffer) {

}

} /* namespace std */
