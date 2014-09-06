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
PinholeCamera *GLUTDisplay2::m_camera;
unsigned int GLUTDisplay2::m_texId = 0;

void GLUTDisplay2::displayFrame() {
	// Draw the resulting image
	optix::Buffer buffer = ptr->getOutputBuffer();
	unsigned int vboId = buffer->getGLBOId();
	int buffer_width = 960;
	int buffer_height = 540;

	if (vboId) {
		glBindTexture(GL_TEXTURE_2D, m_texId);

		// send pbo to texture
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vboId);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 8);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, buffer_width, buffer_height, 0, GL_RGBA, GL_FLOAT, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glEnable(GL_TEXTURE_2D);

		// Initialize offsets to pixel center sampling.
		float u = 0.5f / buffer_width;
		float v = 0.5f / buffer_height;

		glBegin(GL_QUADS);
		glTexCoord2f(u, v);
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(1.0f, v);
		glVertex2f(1.0f, 0.0f);
		glTexCoord2f(1.0f - u, 1.0f - v);
		glVertex2f(1.0f, 1.0f);
		glTexCoord2f(u, 1.0f - v);
		glVertex2f(0.0f, 1.0f);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}
}

void GLUTDisplay2::display() {
	cout << "frame" << endl;
	try {
		// render the scene
		optix::float3 eye, U, V, W;
		m_camera->getEyeUVW(eye, U, V, W);
		SampleScene::RayGenCameraData camera_data(eye, U, V, W);
		ptr->trace(camera_data);
		displayFrame();
	} catch (optix::Exception& e) {
		sutilReportError(e.getErrorString().c_str());
		exit(2);
	}

	// Swap buffers
	glutSwapBuffers();
}

void GLUTDisplay2::idle() {
	glutPostRedisplay();
}

GLUTDisplay2::GLUTDisplay2(int argc, char *argv[]) {
	cout << "init glut display" << endl;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
}

GLUTDisplay2::~GLUTDisplay2() {}

void GLUTDisplay2::run(PathTracer &pt) {
	ptr = &pt;

	unsigned int buffer_width = 960;
	unsigned int buffer_height = 540;
	glutInitWindowSize(buffer_width, buffer_height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("test");

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return;
	}

	try {
		// Set up scene
		cout << "setup camera" << endl;
		SampleScene::InitialCameraData camera_data;

		// todo: get camera
		//ptr->initScene(camera_data);

		// Initialize camera according to scene params
		m_camera = new PinholeCamera(camera_data.eye, camera_data.lookat,
				camera_data.up, -1.0f, // hfov is ignored when using keep vertical
				camera_data.vfov, PinholeCamera::KeepVertical);
	} catch (optix::Exception& e) {
		sutilReportError(e.getErrorString().c_str());
		exit(2);
	}

	//cout << "make renderer" << endl;
	//renderer = new GLrenderer( pt.getOutputBuffer()->getGLBOId() );

	cout << "init texture" << endl;
	glGenTextures(1, &m_texId);
	glBindTexture(GL_TEXTURE_2D, m_texId);

	// Change these to GL_LINEAR for super- or sub-sampling
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// GL_CLAMP_TO_EDGE for linear filtering, not relevant for nearest.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Initialize state
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, buffer_width, buffer_height);

	cout << "begin glut loop" << endl;
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutShowWindow();
	glutReshapeWindow( buffer_width, buffer_height );
	glutMainLoop();
}

} /* namespace std */
