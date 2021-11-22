#version 330 core

in vec3 fragPos;
in vec2 tc;
in vec3 n;

uniform vec3 light;
uniform sampler2D sampler;

out vec4 color;

void main() {
	vec4 d = texture(sampler, tc);
	vec3 lightDir = normalize(light - fragPos);
    vec3 normal = normalize(n);
    float diff = max(dot(lightDir, normal), 0.0);
	color = diff * d;
}
