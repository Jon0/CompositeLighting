/*
 * Modifiable.h
 *
 *  Created on: 26/09/2014
 *      Author: asdf
 */

#ifndef MODIFIABLE_H_
#define MODIFIABLE_H_

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace std {

class Modifiable {
public:
	virtual ~Modifiable() {}

	virtual void zoom(float) = 0;
	virtual void move(glm::vec3) = 0;
	virtual void rotate(glm::quat) = 0;
};

} /* namespace std */
#endif /* MODIFIABLE_H_ */
