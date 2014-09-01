/*
 * GLUTDisplay.cpp
 *
 *  Created on: 1/09/2014
 *      Author: asdf
 */

#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>

#include "GLUTDisplay.h"

namespace std {

PathTracer *GLUTDisplay2::ptr;
GLrenderer *GLUTDisplay2::renderer;

void GLUTDisplay2::display() {
	cout << "frame" << endl;
	ptr->trace();

	// Initialize state
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glViewport(0, 0, 960, 540);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, 960, 540);
	glClear(GL_COLOR_BUFFER_BIT);


	renderer->draw();

	glutSwapBuffers();
}

void GLUTDisplay2::idle() {
	glutPostRedisplay();
}

GLUTDisplay2::GLUTDisplay2(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutInitWindowSize(960, 540);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("test");

	glutDisplayFunc(display);
	glutIdleFunc(idle);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

GLUTDisplay2::~GLUTDisplay2() {
	// TODO Auto-generated destructor stub
}

void GLUTDisplay2::run(PathTracer &pt) {
	ptr = &pt;
	renderer = new GLrenderer( pt.getOutputBuffer()->getGLBOId() );

	cout << "begin glut loop" << endl;
	glutShowWindow();
	glutReshapeWindow( 960, 540 );



	glutMainLoop();
}

} /* namespace std */
