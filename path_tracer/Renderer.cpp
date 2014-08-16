/*
 * Renderer.cpp
 *
 *  Created on: 14/08/2014
 *      Author: asdf
 */

#include <GL/glew.h>
#include <GL/gl.h>
#include <PPMLoader.h>

#include "Renderer.h"

namespace std {

int Renderer::numRenderers = 0;

Renderer::Renderer() {
	index = 0; //numRenderers++;
}

Renderer::~Renderer() {
	// TODO Auto-generated destructor stub
}

void Renderer::init(optix::Context &m_context, string name, int w, int h, bool useGL) {
	buf_name = name;
	width = w;
	height = h;
	format = RT_FORMAT_FLOAT4;

	// Setup output buffer
	optix::Variable output_buffer = m_context[buf_name];
	optix::Buffer buffer = makeBuffer(m_context, useGL);
	output_buffer->set(buffer);
}

void Renderer::init(optix::Context &m_context, string name, string file) {
	buf_name = name;
	format = RT_FORMAT_FLOAT4;

	PPMLoader ppm(file, false);
	width = ppm.width();
	height = ppm.height();
	int chan = 3;
	unsigned char *pixels = ppm.raster();

	optix::Variable output_buffer = m_context[buf_name];
	optix::Buffer buffer = m_context->createBuffer( RT_BUFFER_OUTPUT, format, width, height);
	float* buffer_data = static_cast<float *>(buffer->map());
	for (unsigned int i = 0; i < width; ++i) {
		for (unsigned int j = 0; j < height; ++j) {

			unsigned int ppm_index = ((height - j - 1) * width + i) * 3;
			unsigned int buf_index = (j * width + i) * 4;

			buffer_data[buf_index + 0] = pixels[ppm_index + 0] / 256.0f;
			buffer_data[buf_index + 1] = pixels[ppm_index + 1] / 256.0f;
			buffer_data[buf_index + 2] = pixels[ppm_index + 2] / 256.0f;
			buffer_data[buf_index + 3] = 1.0f;
		}
	}

	buffer->unmap();
	output_buffer->set(buffer);
}

void Renderer::setImage(optix::Context &m_context, string path, string file) {

}

optix::Buffer Renderer::makeBuffer(optix::Context &m_context, bool useGL) {
	optix::Buffer buffer;
	if ( useGL ) {
		GLuint vbo = 0;
		glGenBuffers(1, &vbo);
		//glBindBuffer(GL_ARRAY_BUFFER, vbo);
		//size_t element_size;
		//m_context->checkError(rtuGetSizeForRTformat(format, &element_size));
		//glBufferData(GL_ARRAY_BUFFER, element_size * width * height, 0,
		//		GL_STREAM_DRAW);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);

		buffer = m_context->createBufferFromGLBO(RT_BUFFER_OUTPUT, vbo);
		buffer->setFormat(format);
		buffer->setSize(width, height);
	}
	else {
		buffer = m_context->createBuffer( RT_BUFFER_OUTPUT, format, width, height);
	}
	return buffer;
}

} /* namespace std */
