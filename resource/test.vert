#version 430

layout(location=0) in vec4 currentPos;
layout(location=1) in vec2 texPos;

out gl_PerVertex {
	vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

out vec2 ex_TexCoor;

void main(void) {
	gl_Position = currentPos;
	ex_TexCoor = texPos;
}
