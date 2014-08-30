/*
 * Pipeline.cpp
 *
 *  Created on: 28/10/2013
 *      Author: remnanjona
 */

#include <iostream>

#include "Pipeline.h"

namespace std {

Pipeline::Pipeline() {
	glGenProgramPipelines(1, &name);
}

Pipeline::~Pipeline() {
	glDeleteProgramPipelines(1, &name);
}

void Pipeline::addStage(Shader &s, GLbitfield stages) {

	/*
	 * create a program stage
	 */
	GLuint program = glCreateProgram();
	glAttachShader( program, s.ShaderHandle );
	glProgramParameteri( program, GL_PROGRAM_SEPARABLE, GL_TRUE );
	glLinkProgram( program );

	/*
	 * list available shader storage blocks
	 */
	int num_blocks;
	glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &num_blocks);

	int len;
	char attname [32];
	for (int i = 0; i < num_blocks; ++i) {
		glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, 32, &len, attname );
		GLuint block_index = glGetProgramResourceIndex( program, GL_SHADER_STORAGE_BLOCK, attname );
		glShaderStorageBlockBinding( program, block_index, i+1 ); // bind to i.

		cout << "attribute: " << attname << " bound to " << (i+1) << endl;
	}

	/*
	 * check errors
	 */
	int rvalue;
	glGetProgramiv( program, GL_LINK_STATUS, &rvalue );
	if (!rvalue) {
		fprintf(stderr, "Error in linking shader program\n");
		GLchar log[10240];
		GLsizei length;
		glGetProgramInfoLog( program, 10239, &length, log );
		fprintf(stderr, "Linker log:\n%s\n", log);
	}


	/*
	 * add stage to pipeline
	 */
	glBindProgramPipeline(name);
	glUseProgramStages( name, stages, program);

	/*
	 * if everything worked
	 */
	stage.push_back( program );
}

} /* namespace std */
