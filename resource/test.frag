#version 430
#extension GL_ARB_texture_rectangle : enable

in vec2 ex_TexCoor;
out vec4 color;

uniform sampler2D texture;

void main(void) {
	color = texture2D(texture, ex_TexCoor);
}