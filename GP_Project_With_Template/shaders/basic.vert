#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec4 fPosEye;
out vec3 fNormal;
out vec2 fTexCoords;
out vec4 fPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main() 
{
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
	fNormal = vNormal;
	fTexCoords = vTexCoords;

	
	fPosEye = view * model * vec4(vPosition, 1.0f);
	fPosLightSpace = lightSpaceMatrix* model* vec4(vPosition, 1.0f);
}
