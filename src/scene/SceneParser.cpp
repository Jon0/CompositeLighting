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

	/*
	 * the scene being constructed
	 */
	shared_ptr<Scene> s = make_shared<Scene>();

	/*
	 * directory containing assets
	 */
	string basepath = options["directory"];


	// load data from image file
	if ( hasOption("photo_color") ) {
		string colorpath = basepath + options["photo_color"];
		Mat pcolor = imread(colorpath, CV_LOAD_IMAGE_COLOR);
		s->setPhoto(pcolor);
	}

	if ( hasOption("photo_depth") ) {
		string depthpath = basepath + options["photo_depth"];
		Mat pdepth = imread(depthpath, CV_LOAD_IMAGE_COLOR);
		s->setDepthPhoto(pdepth);
	}

	if ( hasOption("lightmap") ) {
		string lmpath = basepath + options["lightmap"];
		Mat lmap = imread(lmpath, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
		s->setLightMap(lmap);
	}

	if ( hasOption("local_model") ) {
		s->addGeometry(basepath + options["local_model"], true);
	}

	if ( hasOption("virtual_model") ) {
		s->addGeometry(basepath + options["virtual_model"], false);
	}

	if ( hasOption("camera") ) {
		 //s->setCamAngle(glm::quat(-0.0754814, 6.97229e-05, 0.989809, -0.121214), glm::vec3(-0.24233, 0.804301, -0.098939), 10.1601, 15.0);
	}
	if ( hasOption("camera_angle") ) {
		vector<string> a = split(options["camera_angle"], ',');
		s->setCamAngle( glm::quat(stof(a[0]), stof(a[1]), stof(a[2]), stof(a[3])) );
	}
	if ( hasOption("camera_pos") ) {
		vector<string> a = split(options["camera_pos"], ',');
		s->setCamPos( glm::vec3(stof(a[0]), stof(a[1]), stof(a[2])) );
	}
	if ( hasOption("camera_zoom") ) {
		s->setCamZoom( stof(options["camera_zoom"]) );
	}
	if ( hasOption("camera_vfov") ) {
		s->setCamFov( stof(options["camera_vfov"]) );
	}
	return s;
}

bool SceneParser::hasOption(string op) {
	return (options.count(op) > 0);
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
