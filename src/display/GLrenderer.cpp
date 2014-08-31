/*
 * Renderer.cpp
 *
 *  Created on: 24/03/2014
 *      Author: remnanjona
 */

#include "Shader.h"
#include "GLrenderer.h"

namespace std {

GLrenderer::GLrenderer(GLuint addr): buff(GL_ARRAY_BUFFER, false) {
	width = 960, height = 540;
    pbo_addr = addr;

    glGenTextures(1, &tex_addr);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex_addr);

    // Change these to GL_LINEAR for super- or sub-sampling
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // GL_CLAMP_TO_EDGE for linear filtering, not relevant for nearest.
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, 0);

	vector<texvec> data = {
			{glm::vec4(-1,-1,0,1), glm::vec2(0,0)}, {glm::vec4(1,-1,0,1), glm::vec2(width,0)},
			{glm::vec4(-1,1,0,1), glm::vec2(0,height)}, {glm::vec4(1,1,0,1), glm::vec2(width,height)}
	};
	buff.insert(data);

    // Init Pipeline
    Shader test_vert("resource/test.vert", GL_VERTEX_SHADER);
    Shader test_frag("resource/test.frag", GL_FRAGMENT_SHADER);
    tex_pipeline.addStage(test_vert, GL_VERTEX_SHADER_BIT);
    tex_pipeline.addStage(test_frag, GL_FRAGMENT_SHADER_BIT);
}

GLrenderer::~GLrenderer() {}

void GLrenderer::draw() {
	glBindProgramPipeline(tex_pipeline.name);

	glBindBuffer( GL_ARRAY_BUFFER, buff.location );
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(texvec), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(texvec), (GLvoid*)sizeof(glm::vec4));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex_addr);
	copyPbo(pbo_addr);



	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void GLrenderer::copyPbo(GLuint addr) {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_addr);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 8);

//    RTsize elementSize = buffer->getElementSize();
//    if      ((elementSize % 8) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
//    else if ((elementSize % 4) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
//    else if ((elementSize % 2) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
//    else                             glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // assume float4
    glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
    glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);
}

} /* namespace std */
