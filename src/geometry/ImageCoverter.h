/*
 * ImageCoverter.h
 *
 *  Created on: 10/09/2014
 *      Author: asdf
 */

#ifndef IMAGECOVERTER_H_
#define IMAGECOVERTER_H_

#include <opencv2/core/core.hpp>

#include "../scene/Camera.h"
#include "PointCloud.h"

namespace std {

class ImageCoverter {
public:
	ImageCoverter();
	virtual ~ImageCoverter();

	PointCloud makePointCloud(cv::Mat &, Camera *);


};

} /* namespace std */
#endif /* IMAGECOVERTER_H_ */
