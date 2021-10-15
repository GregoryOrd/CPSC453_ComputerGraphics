#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;

uniform vec3 translationVector;

out vec2 tc;

void main() {
	tc = texCoord;
	gl_Position = vec4(pos + translationVector, 1.0);
}
