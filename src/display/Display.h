/*
 * Display.h
 *
 *  Created on: 23/08/2014
 *      Author: asdf
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace std {

class Display {
public:
	Display();
	virtual ~Display();

private:
	GLFWwindow* window;
};

} /* namespace std */
#endif /* DISPLAY_H_ */
