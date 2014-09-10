/*
 * Renderer.cpp
 *
 *  Created on: 14/08/2014
 *      Author: asdf
 */

#include <iostream>

#include <optixu/optixu_math_stream_namespace.h>
#include <optixu/optixu.h>

#include <PPMLoader.h>

#include "PPMTexture.h"

namespace std {

PPMTexture::PPMTexture() {
	addr = 0;
	index = 0;
}

PPMTexture::~PPMTexture() {
	// TODO Auto-generated destructor stub
}

inline int PPMTexture::width() {
	return i_width;
}

inline int PPMTexture::height() {
	return i_height;
}

float PPMTexture::asF(int x, int y) {
	unsigned int ppm_index = ((i_height - y - 1) * i_width + i_width - x - 1) * 3;

	return pixeldata[ppm_index + 0];
}

void PPMTexture::init(optix::Context &m_context, string name, int w, int h, bool useGL) {
	cout << "making " << name << endl;
	buf_name = name;
	i_width = w;
	i_height = h;
	format = RT_FORMAT_FLOAT4;

	// Setup output buffer
	optix::Variable output_buffer = m_context[buf_name];
	optix::Buffer buffer = makeBuffer(m_context, useGL);
	output_buffer->set(buffer);
}

void PPMTexture::init(optix::Context &m_context, string name, string file) {
	buf_name = name;
	format = RT_FORMAT_FLOAT4;

	// loading ppm image into optix buffer
	PPMLoader ppm(file, false);
	i_width = ppm.width();
	i_height = ppm.height();
	int chan = 3;
	unsigned char *pixels = ppm.raster();
	pixeldata = new float [3*i_width*i_height];

	optix::Variable output_buffer = m_context[buf_name];
	optix::Buffer buffer = m_context->createBuffer( RT_BUFFER_OUTPUT, format, i_width, i_height);
	float* buffer_data = static_cast<float *>(buffer->map());
	for (unsigned int i = 0; i < i_width; ++i) {
		for (unsigned int j = 0; j < i_height; ++j) {

			unsigned int ppm_index = ((i_height - j - 1) * i_width + i) * 3;
			unsigned int buf_index = (j * i_width + i) * 4;

			buffer_data[buf_index + 0] = pixels[ppm_index + 0] / 256.0f;
			buffer_data[buf_index + 1] = pixels[ppm_index + 1] / 256.0f;
			buffer_data[buf_index + 2] = pixels[ppm_index + 2] / 256.0f;
			buffer_data[buf_index + 3] = 1.0f;

			pixeldata[ppm_index + 0] = pixels[ppm_index + 0] / 256.0f;
			pixeldata[ppm_index + 1] = pixels[ppm_index + 1] / 256.0f;
			pixeldata[ppm_index + 2] = pixels[ppm_index + 2] / 256.0f;
		}
	}

	buffer->unmap();
	output_buffer->set(buffer);
}

void PPMTexture::setImage(optix::Context &m_context, string path, string file) {

}

optix::Buffer PPMTexture::makeBuffer(optix::Context &m_context, bool useGL) {
	optix::Buffer buffer;
	if ( useGL ) {
		glGenBuffers(1, &addr);
		glBindBuffer(GL_ARRAY_BUFFER, addr);
		size_t element_size;
		m_context->checkError(rtuGetSizeForRTformat(format, &element_size));
		glBufferData(GL_ARRAY_BUFFER, element_size * i_width * i_height, 0,	GL_STREAM_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		buffer = m_context->createBufferFromGLBO(RT_BUFFER_OUTPUT, addr);
		buffer->setFormat(format);
		buffer->setSize(i_width, i_height);
	}
	else {
		buffer = m_context->createBuffer( RT_BUFFER_OUTPUT, format, i_width, i_height);
	}
	return buffer;
}

} /* namespace std */
