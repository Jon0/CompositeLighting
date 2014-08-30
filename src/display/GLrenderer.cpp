/*
 * Renderer.cpp
 *
 *  Created on: 24/03/2014
 *      Author: remnanjona
 */

#include "Shader.h"
#include "GLrenderer.h"

namespace std {

GLrenderer::GLrenderer(): buff(GL_ARRAY_BUFFER, false) {

	// width = ?, height = ?;
    // t_addr = ????

	vector<texvec> data = {
			{glm::vec4(-1,-1,0,1), glm::vec2(0,0)}, {glm::vec4(1,-1,0,1), glm::vec2(width,0)},
			{glm::vec4(-1,1,0,1), glm::vec2(0,height)}, {glm::vec4(1,1,0,1), glm::vec2(width,height)}
	};
	buff.insert(data);

    // Init Pipeline
    Shader test_vert("glsl/test.vert", GL_VERTEX_SHADER);
    Shader test_frag("glsl/test.frag", GL_FRAGMENT_SHADER);
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

	glBindTexture(GL_TEXTURE_RECTANGLE_NV, t_addr);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

} /* namespace std */
