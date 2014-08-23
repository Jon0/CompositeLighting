/*
 * Args.cpp
 *
 *  Created on: 23/08/2014
 *      Author: asdf
 */

#include "Args.h"

namespace std {

Args::Args(int argc, char *argv[]) {
	for (int i = 0; i < argc; ++i) {
		args.push_back(string(argv[i]));
	}
}

Args::~Args() {}

bool Args::contains(string) {
	//args.
}


string Args::optionS(string, int) {}
int Args::optionI(string, int) {}
double Args::optionF(string, int) {}

} /* namespace std */
