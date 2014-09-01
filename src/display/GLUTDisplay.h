/*
 * GLUTDisplay.h
 *
 *  Created on: 1/09/2014
 *      Author: asdf
 */

#ifndef GLUTDISPLAY_H_
#define GLUTDISPLAY_H_

#include "../PathTracer.h"
#include "GLrenderer.h"

namespace std {

class GLUTDisplay2 {
public:
	GLUTDisplay2(int argc, char *argv[]);
	virtual ~GLUTDisplay2();

	void run(PathTracer &);


	static PathTracer *ptr;
	static GLrenderer *renderer;

	static void display();
	static void idle();
};

} /* namespace std */
#endif /* GLUTDISPLAY_H_ */
