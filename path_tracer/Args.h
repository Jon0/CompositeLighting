/*
 * Args.h
 *
 *  Created on: 23/08/2014
 *      Author: asdf
 */

#ifndef ARGS_H_
#define ARGS_H_

#include <string>
#include <vector>

namespace std {

class Args {
public:
	Args(int, char *[]);
	virtual ~Args();

	bool contains(string);
	string optionS(string, int);
	int optionI(string, int);
	double optionF(string, int);

private:
	vector<string> args;
};

} /* namespace std */
#endif /* ARGS_H_ */
