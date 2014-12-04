/*
 * Arcball.cpp
 *
 *  Created on: 19/09/2014
 *      Author: asdf
 */

#include "Arcball.h"

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

Arcball::Arcball(float x, float y) {
	arcball_x = (x / 2.0);
	arcball_y = (y / 2.0);
	arcball_radius = (x / 2.0);
}

Arcball::~Arcball() {}

glm::quat Arcball::mouseDragged(int fromx, int fromy, int tox, int toy, float scale) {
	glm::quat click_old, click_new;
	getArc(arcball_x, arcball_y, fromx, fromy, arcball_radius, click_old); // initial click down
	getArc(arcball_x, arcball_y, tox, toy, arcball_radius, click_new);
	glm::quat q = click_new * glm::inverse(click_old);
	return glm::slerp( glm::quat(), q, scale );;
}
