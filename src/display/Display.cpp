/*
 * Display.cpp
 *
 *  Created on: 23/08/2014
 *      Author: asdf
 */

#include <iostream>

#include "Display.h"

namespace std {

Display::Display() {
	if (!glfwInit()) return;
	window = glfwCreateWindow(640, 480, "Window", NULL, NULL);
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

} /* namespace std */
