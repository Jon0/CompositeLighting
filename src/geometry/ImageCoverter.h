/*
 * ImageCoverter.h
 *
 *  Created on: 10/09/2014
 *      Author: asdf
 */

#ifndef IMAGECOVERTER_H_
#define IMAGECOVERTER_H_

#include <Mouse.h>

#include "../texture/Texture.h"
#include "PointCloud.h"

namespace std {

class ImageCoverter {
public:
	ImageCoverter();
	virtual ~ImageCoverter();

	PointCloud makePointCloud(Texture &, PinholeCamera *);


};

} /* namespace std */
#endif /* IMAGECOVERTER_H_ */
