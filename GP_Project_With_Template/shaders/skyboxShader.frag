#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform float timeOfDay;
uniform bool fogOn;

float computeFog()
{
    float fogDensity = 0.05f;
    float fragmentDistance = 30.0f;
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
    
    return clamp(fogFactor, 0.0f, 1.0f);
}

void main()
{
    color = texture(skybox, textureCoordinates) * timeOfDay;

    if(fogOn){
        float fogFactor = computeFog();
        vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
        color = mix(fogColor* timeOfDay, color, fogFactor);
    }
}
