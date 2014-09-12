/*
 * Camera.cpp
 *
 *  Created on: 25/08/2013
 *      Author: remnanjona
 */

#include <iostream>
#include <math.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

namespace std {

Camera::Camera():
		focus {0, 0, 0},
		eye {0, 0, 0},
		U {0, 0, 0},
		V {0, 0, 0},
		W {0, 0, 0},
		cam_angle {1, 0, 0, 0},
		cam_angle_d {1, 0, 0, 0},
		click_old {1, 0, 0, 0},
		click_new {1, 0, 0, 0}
{
	cam_aspect = 1.0;
	viewzoom = 12.0;

	// mouse action settings
	arcball_x = arcball_y = 0.0;
	arcball_radius = 1.0;
	windowwidth = windowheight = 1;
	hfov = vfov = 40.0f;
	modified = false;
}

Camera::~Camera() {}

void Camera::reset() {
	focus = glm::vec3(0, 0, 0);
	cam_angle = glm::quat(1, 0, 0, 0),
	viewzoom = 12.0;
	modified = true;
}

void Camera::update( float tick ) {
	cam_angle = cam_angle_d * cam_angle;
	cam_angle_d = glm::quat(1, 0, 0, 0);

	// make camera frustum
	eye = focus + cam_angle * glm::vec3(0.0f, 0.0f, -viewzoom);
	W = focus - eye; // do not normalize lookdir -- implies focal length
	float lookdir_len = glm::length(W);
	glm::vec3 up = glm::vec3(0, 1, 0);
	U = glm::normalize(glm::cross(W, up));
	V = glm::normalize(glm::cross(U, W));
	float ulen = lookdir_len * tanf(DtoR(hfov * 0.5f));
	U = U * ulen;
	float vlen = lookdir_len * tanf(DtoR(vfov * 0.5f));
	V = V * vlen;
	modified = false;
}

void Camera::resize(int x, int y) {
	windowwidth = x;
	windowheight = y;
	cam_aspect = (double) x / (double) y;
	arcball_x = (x / 2.0);
	arcball_y = (y / 2.0);
	arcball_radius = (x / 2.0);
	hfov = RtoD(2.0f*atanf(cam_aspect*tanf(DtoR(0.5f*(vfov)))));
	update(0.0f);
}

void Camera::printAngle() {
	cout << "camera quat = " << cam_angle.w << ", " << cam_angle.x << ", " <<  cam_angle.y << ", " <<  cam_angle.z << endl;
	cout << "camera zoom = " << viewzoom << endl;
}

void Camera::mouseDragRotation(int x1, int y1, int x2, int y2) {
	getArc(arcball_x, arcball_y, x1, y1, arcball_radius, click_old); // initial click down
	getArc(arcball_x, arcball_y, x2, y2, arcball_radius, click_new);
	glm::quat q = click_new * glm::inverse(click_old);
	cam_angle_d = q * cam_angle_d;
	click_old = click_new;
	modified = true;
}

void Camera::mouseDragPanning(int x, int y) {
	float xn = x;
	float yn = y;
	float len_sq = xn*xn + yn*yn;
	if (len_sq > 0.1) {
		float len = sqrt(len_sq);
		glm::vec3 add = glm::axis( glm::inverse(cam_angle) * glm::quat(0, xn, yn, 0) * cam_angle );
		focus = focus + add * (len / arcball_radius);
	}
	modified = true;
}

glm::quat Camera::cameraAngle() {
	return cam_angle;
}

void getArc(int arcx, int arcy, int ix, int iy, float rad, glm::quat &result) {
	float x = (ix - arcx) / rad;
	float y = (iy - arcy) / rad;

	// check click is inside the arcball radius
	if (x*x + y*y < 1.0) {
		float z = sqrt(1 - (x*x + y*y));
		result = glm::quat(0, x, y, z);
	}
	else {
		float len = sqrt(x*x + y*y);
		result = glm::quat(0, x / len, y / len, 0);
	}
}

void getUnitCircle(int arcx, int arcy, int ix, int iy, glm::quat &result) {
	float x = ix - arcx;
	float y = iy - arcy;
	float len = sqrt(x*x + y*y);
	result = glm::quat(0, x / len, y / len, 0);
}

} /* namespace std */
