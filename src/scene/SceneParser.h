/*
 * SceneParser.h
 *
 *  Created on: 11/09/2014
 *      Author: asdf
 */

#ifndef SCENEPARSER_H_
#define SCENEPARSER_H_

#include <memory>
#include <string>

#include "Scene.h"

namespace std {

vector<string> split(const string &s, char delim);

/*
 * reads scene setup information from config file
 * config commands:
 * directory = <directory to look in>
 * photo = <photo filename>
 * lightmap = <lightmap filename>
 */
class SceneParser {
public:
	SceneParser(string file);
	virtual ~SceneParser();

	shared_ptr<Scene> readScene();

private:
	bool hasOption(string);
	void addOption(string);
	void loadConfig(string);

	string fname;
	map<string, string> options;
};

} /* namespace std */
#endif /* SCENEPARSER_H_ */
