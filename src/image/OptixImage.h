/*
 * OptixImage.h
 *
 *  Created on: 11/09/2014
 *      Author: asdf
 */

#ifndef OPTIXIMAGE_H_
#define OPTIXIMAGE_H_

#include <opencv2/core/core.hpp>
#include <optixu/optixpp_namespace.h>

namespace std {

/*
 * make empty buffer
 */
optix::Buffer makeBuffer(optix::Context &, RTbuffertype, int, int, bool);

/*
 * make buffer initialised with image
 */
optix::Buffer makeBuffer(optix::Context &, RTbuffertype, cv::Mat &, bool);

/*
 * make a texture sampler containing buffer
 */
optix::TextureSampler makeSampler(optix::Context &, optix::Buffer &);

} /* namespace std */
#endif /* OPTIXIMAGE_H_ */
