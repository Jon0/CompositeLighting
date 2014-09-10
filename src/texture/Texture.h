/*
 * Texture.h
 *
 *  Created on: 29/08/2014
 *      Author: asdf
 */

#ifndef TEXTURE_H_
#define TEXTURE_H_

namespace std {

class Texture {
public:
	virtual ~Texture() {}

	virtual int width() = 0;
	virtual int height() = 0;

	virtual float asF(int, int) = 0;
};

} /* namespace std */
#endif /* TEXTURE_H_ */
