/*
 * Arcball.h
 *
 *  Created on: 19/09/2014
 *      Author: asdf
 */

#ifndef ARCBALL_H_
#define ARCBALL_H_

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

void getArc(int, int, int, int, float, glm::quat &);
void getUnitCircle(int, int, int, int, glm::quat &);

class Arcball {
public:
	Arcball(float, float);
	virtual ~Arcball();

	glm::quat mouseDragged(int, int, int, int, float);

private:
	float arcball_radius, arcball_x, arcball_y;
};

#endif /* ARCBALL_H_ */
