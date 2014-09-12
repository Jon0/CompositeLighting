/*
 * OptixImage.cpp
 *
 *  Created on: 11/09/2014
 *      Author: asdf
 */

#include <iostream>

#include <GL/glew.h>

#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu.h>

#include "OptixImage.h"

namespace std {

optix::Buffer makeBuffer(optix::Context &context, RTbuffertype type, int w, int h, bool useGL) {
	RTformat format = RT_FORMAT_FLOAT4;
	optix::Buffer buffer;
	if ( useGL ) {
		GLuint addr;
		glGenBuffers(1, &addr);
		glBindBuffer(GL_ARRAY_BUFFER, addr);
		size_t element_size;
		context->checkError(rtuGetSizeForRTformat(format, &element_size));
		glBufferData(GL_ARRAY_BUFFER, element_size * w * h, 0,	GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		buffer = context->createBufferFromGLBO(type, addr);
		buffer->setFormat(format);
		buffer->setSize(w, h);
	}
	else {
		buffer = context->createBuffer(type, format, w, h);
	}
	return buffer;
}


optix::Buffer makeBuffer(optix::Context &context, RTbuffertype type, cv::Mat &image, bool useGL) {
	optix::Buffer buffer = makeBuffer(context, type, image.cols, image.rows, useGL);

	// fill data into buffer
	int datatype = image.type();
	float* buffer_data = static_cast<float *>(buffer->map());
	for (unsigned int i = 0; i < image.cols; ++i) {
		for (unsigned int j = 0; j < image.rows; ++j) {
			if (datatype == 21)  {
				cv::Vec3f pixel = image.at<cv::Vec3f>(j, i); // 3 channels?
				unsigned int buf_index = ((image.rows - j - 1) * image.cols + i) * 4;
				buffer_data[buf_index + 0] = pixel.val[2];
				buffer_data[buf_index + 1] = pixel.val[1];
				buffer_data[buf_index + 2] = pixel.val[0];
				buffer_data[buf_index + 3] = 1.0f;
			}
			else {
				cv::Vec3b pixel = image.at<cv::Vec3b>(j, i); // 3 channels?
				unsigned int buf_index = ((image.rows - j - 1) * image.cols + i)* 4;
				buffer_data[buf_index + 0] = pixel.val[2] / 256.0f;
				buffer_data[buf_index + 1] = pixel.val[1] / 256.0f;
				buffer_data[buf_index + 2] = pixel.val[0] / 256.0f;
				buffer_data[buf_index + 3] = 1.0f;
			}
		}
	}
	buffer->unmap();
	return buffer;
}

optix::TextureSampler makeSampler(optix::Context &context, optix::Buffer &buffer) {
	// Create tex sampler and populate with default values
	optix::TextureSampler sampler = context->createTextureSampler();
	sampler->setWrapMode(0, RT_WRAP_REPEAT);
	sampler->setWrapMode(1, RT_WRAP_REPEAT);
	sampler->setWrapMode(2, RT_WRAP_REPEAT);
	sampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
	sampler->setReadMode(RT_TEXTURE_READ_ELEMENT_TYPE);
	sampler->setMaxAnisotropy(1.0f);
	sampler->setMipLevelCount(1u);
	sampler->setArraySize(1u);
	sampler->setBuffer(0u, 0u, buffer);
	sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
	return sampler;
}


} /* namespace std */
