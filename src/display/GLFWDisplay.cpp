/*
 * Display.cpp
 *
 *  Created on: 23/08/2014
 *      Author: asdf
 */

#include <iostream>
#include <string>
#include <time.h>

#include "GLFWDisplay.h"

namespace std {

uint GLFWDisplay::width, GLFWDisplay::height;
int GLFWDisplay::mb1, GLFWDisplay::mb2;
double GLFWDisplay::mx, GLFWDisplay::my;
Arcball *GLFWDisplay::arcball;
Scene *GLFWDisplay::sn;
Camera *GLFWDisplay::cam;
PathTracer *GLFWDisplay::ptr;

string getTime() {
    time_t timeObj;
    time(&timeObj);
    tm *pTime = gmtime(&timeObj);
    char buffer[100];
    sprintf(buffer, "%d_%d_%d", pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
    return buffer;
}

GLFWDisplay::GLFWDisplay() {
	if (!glfwInit()) return;

	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	window = glfwCreateWindow(100, 100, "", NULL, NULL);
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
	sn = &ptr->getScene();
	cam = sn->getCam();
	if (!shared_buffer) {
		cout << "need to set buffer before running" << endl;
		return;
	}
    RTsize buffer_width_rts, buffer_height_rts;
    shared_buffer->getSize( buffer_width_rts, buffer_height_rts );
    width  = static_cast<int>(buffer_width_rts);
    height = static_cast<int>(buffer_height_rts);
    arcball = new Arcball(width, height);

    // complete window setup
    glfwSetWindowTitle(window, "path_tracer");
    glfwSetWindowSize(window, width, height);
    glfwSetKeyCallback(window, keyFunc);
    glfwSetMouseButtonCallback(window, mouseFunc);
    glfwSetCursorPosCallback(window, posFunc);
    glfwSetScrollCallback(window, scrollFunc);

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

void GLFWDisplay::keyFunc(GLFWwindow *w, int key, int scan, int act, int) {
	float rspeed = 0.005f, pspeed = 0.03;

	if (act == GLFW_PRESS && isdigit(key)) {
		unsigned int newmode = key - '0';
		ptr->setDisplayMode(newmode);
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_S) {
		string fname = "outputs/out_"+getTime()+".ppm";
		sutilDisplayFilePPM( fname.c_str(), ptr->getOutputBuffer()->get() );
		cout << "saved as " << fname << endl;
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_I) {
		sn->select();
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_O) {
		cam->reset();
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_P) {
		cam->printAngle();
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_K) {
		cam->modifyExposure(1.1);
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_L) {
		cam->modifyExposure(0.9);
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_Q) {
		sn->move(glm::vec3(pspeed, 0.0f, 0.0f));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_W) {
		sn->move(glm::vec3(-pspeed, 0.0f, 0.0f));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_E) {
		sn->move(glm::vec3(0.0f, pspeed, 0.0f));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_R) {
		sn->move(glm::vec3(0.0f, -pspeed, 0.0f));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_T) {
		sn->move(glm::vec3(0.0f, 0.0f, pspeed));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_Y) {
		sn->move(glm::vec3(0.0f, 0.0f, -pspeed));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_Z) {
		sn->rotate(glm::quat(glm::vec3(rspeed, 0.0f, 0.0f)));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_X) {
		sn->rotate(glm::quat(glm::vec3(-rspeed, 0.0f, 0.0f)));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_C) {
		sn->rotate(glm::quat(glm::vec3(0.0f, rspeed, 0.0f)));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_V) {
		sn->rotate(glm::quat(glm::vec3(0.0f, -rspeed, 0.0f)));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_B) {
		sn->rotate(glm::quat(glm::vec3(0.0f, 0.0f, rspeed)));
	}
	else if (act == GLFW_PRESS && key == GLFW_KEY_N) {
		sn->rotate(glm::quat(glm::vec3(0.0f, 0.0f, -rspeed)));
	}
	else if (act == GLFW_PRESS) {
		ptr->keyPressed(key);
	}
}

void GLFWDisplay::mouseFunc(GLFWwindow *w, int button, int act, int) {
	if (act == GLFW_PRESS && button == 0) {
		mb1 = 1;
	}
	else {
		mb1 = 0;
	}
	if (act == GLFW_PRESS && button == 1) {
		mb2 = 1;
	}
	else {
		mb2 = 0;
	}
}

void GLFWDisplay::posFunc(GLFWwindow *w, double x, double y) {
	// drag actions
	if (mb1) {
		sn->rotate( arcball->mouseDragged(mx, my, x, y, 0.1) );
	}
	if (mb2) {
		cam->mouseDragPanning(x - mx, y - my);
	}
	mx = x;
	my = y;
}

void GLFWDisplay::scrollFunc(GLFWwindow *, double x, double y) {
	if (y > 0) {
		sn->zoom( 1.05f );
	}
	else {
		sn->zoom( 0.95f );
	}
}

} /* namespace std */
