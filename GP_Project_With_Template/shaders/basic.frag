#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 cameraTarget;
uniform vec3 cameraPosition;
uniform float timeOfDay;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor * timeOfDay;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor * timeOfDay;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor * timeOfDay;
}

void computeDirFlashlight2()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    //vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction
    //vec3 viewDir = normalize(cameraTarget - fPosEye.xyz);
vec3 viewDir = cameraTarget - cameraPosition;
    //float angle = dot(viewDir, normalEye);
    //vec3 lightAngle = fPosition - fPosEye.xyz;
vec3 distance = fPosition - cameraPosition;

    float angle = dot(-viewDir, normalize(distance));
    if(angle>=0.9999999f && length(distance) < 10){
	vec3 yellowColor = vec3(1.0f, 1.0f, 0.0f);

        //compute ambient light
        ambient = ambientStrength * yellowColor;

        //compute diffuse light
        diffuse = max(dot(normalEye, distance), 0.0f) * yellowColor;

        //compute specular light
        vec3 reflectDir = reflect(viewDir, normalEye);
        float specCoeff = pow(max(dot(distance, reflectDir), 0.0f), 10.0f);
        //specular = specularStrength * specCoeff * yellowColor;
    } 
}
void computeDirFlashlight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 yellowColor = vec3(1.0f, 1.0f, 0.0f);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(-(cameraTarget-cameraPosition), 0.0f)));
    float distance = length(fPosEye.xyz-cameraPosition);
    float angle = dot(fPosEye.xyz-cameraPosition, cameraTarget-cameraPosition) / (length(cameraTarget-cameraPosition) * distance);

    //compute view direction 
    vec3 viewDir = normalize(- fPosEye.xyz);

    if(distance<=15.0f && angle >=0.99f) {
         //compute ambient light
    ambient = ambientStrength * yellowColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * yellowColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    specular = specularStrength * specCoeff * yellowColor;
    }		
    
}

float computeShadow()
{

	//perform perspective divide
	vec3 normalizedCoords= fragPosLightSpace.xyz / fragPosLightSpace.w;

	//tranform from [-1,1] range to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	//get closest depth value from lights perspective
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	//get depth of current fragment from lights perspective
	float currentDepth = normalizedCoords.z;

	//if the current fragments depth is greater than the value in the depth map, the current fragment is in shadow 
	//else it is illuminated
	//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	if (normalizedCoords.z > 1.0f)
		return 0.0f;
	return shadow;
}

void main() 
{
    computeDirLight();
if(timeOfDay<=0.3f)
	computeDirFlashlight();
    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;

    float shadow = computeShadow();
    //compute final vertex color
    vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
    fColor = vec4(color, 1.0f);
}
