#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform float timeOfDay;

void main()
{
    color = texture(skybox, textureCoordinates) * timeOfDay;
}
