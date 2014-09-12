/*
 * SceneParser.cpp
 *
 *  Created on: 11/09/2014
 *      Author: asdf
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "SceneParser.h"

using namespace cv;

namespace std {

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
    	item.erase(remove(item.begin(), item.end(), ' '), item.end());
        elems.push_back(item);
    }
    return elems;
}

SceneParser::SceneParser(string f) {
	fname = f;
}

SceneParser::~SceneParser() {}

shared_ptr<Scene> SceneParser::readScene() {
	loadConfig(fname);
	string basepath = options["directory"];

	// load data from image file
	string colorpath = basepath + options["photo_color"];
	string depthpath = basepath + options["photo_depth"];
	Mat pcolor = imread(colorpath, CV_LOAD_IMAGE_COLOR);
	Mat pdepth = imread(depthpath, CV_LOAD_IMAGE_COLOR);

	string lmpath = basepath + options["lightmap"];
	Mat lmap = imread(lmpath, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	//loadExrTexture( lightMapPath().c_str(), context, false );


	 shared_ptr<Scene> s = make_shared<Scene>();
	 s->setPhoto(pcolor);
	 s->setDepthPhoto(pdepth);
	 s->setLightMap(lmap);
	 return s;
}

void SceneParser::addOption(string op) {
	vector<string> ss = split(op, '=');
    if (ss.size() == 2) {
    	options.insert(make_pair(ss[0], ss[1]));
    }
}

void SceneParser::loadConfig(string fname) {
	ifstream ifs(fname, ifstream::in);
	if (!ifs) {
		throw runtime_error(fname + " not found");
	}
	cout << "config file = " << fname << endl;
    for (string line; getline(ifs, line); ) {
    	addOption(line);
    }
}

} /* namespace std */
