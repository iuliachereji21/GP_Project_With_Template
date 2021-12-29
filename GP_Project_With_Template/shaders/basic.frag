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
uniform vec3 cameraFrontDirection;
uniform vec3 lampPos;
uniform bool flashlightOn;
uniform bool fogOn;

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

float constant = 1.0f;
float linear = 0.22f;
float quadratic = 0.20f;

void computeDirLamp()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec4 lampPosEye = view * vec4(lampPos, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 yellowColor = vec3(1.0f, 1.0f, 0.0f);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(lampPosEye.xyz - fPosEye.xyz));

    //compute distance to light
    float dist = length(lampPosEye.xyz - fPosEye.xyz);
    //compute attenuation
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));



    //compute view direction 
    vec3 viewDir = normalize(- fPosEye.xyz);

    //if(dist < 11.0f){
        //compute ambient light
        vec3 ambient2 = att * ambientStrength * yellowColor;
        ambient+=ambient2;
        //compute diffuse light
        vec3 diffuse2 = att * max(dot(normalEye, lightDirN), 0.0f) * yellowColor;
        diffuse = max(diffuse, diffuse2);

        vec3 reflectDir = reflect(-lightDirN, normalEye);
        float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
        vec3 specular2 = att * specularStrength * specCoeff * yellowColor;
        specular = max(specular, specular2);
    //}

		
    
}

void computeDirFlashlightNewNew()
{
    float cutOff = cos(radians( 12.5f ));
    float outerCutOff = cos( radians( 17.5f ));
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 yellowColor = vec3(1.0f, 1.0f, 0.0f);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(- fPosEye.xyz));

    //compute view direction 
    vec3 viewDir = normalize(- fPosEye.xyz);

    vec4 camFrontDirectionEye =  view *  vec4(cameraFrontDirection, 0.0f);
    float dist = length(- fPosEye.xyz);
    float strength = 3/dist;

    float theta = dot(lightDirN, normalize(-camFrontDirectionEye.xyz));
    float epsilon = (cutOff - outerCutOff);
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);

    ambient += ambientStrength * yellowColor* intensity * strength;

    //compute diffuse light
    diffuse += max(dot(normalEye, lightDirN), 0.0f) * yellowColor * intensity * strength;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    specular += specularStrength * specCoeff * yellowColor * intensity * strength;






    //diffuse  += intensity* yellowColor;
    //specular += intensity* yellowColor;

    
}

float computeShadowSun()
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

float computeFog()
{
    float fogDensity = 0.05f;
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    float fragmentDistance = length(fPosEye.xyz);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
    
    return clamp(fogFactor, 0.0f, 1.0f);
}


void main() 
{
    computeDirLight();
    if(flashlightOn)
        computeDirFlashlightNewNew();
    if(timeOfDay<=0.05f)
        computeDirLamp();
    

    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;

    float shadowSun = computeShadowSun();
    shadowSun*=timeOfDay;
    //compute final vertex color
    vec3 color = min((ambient + (1.0f - shadowSun) * diffuse) + (1.0f - shadowSun) * specular, 1.0f);

    if(fogOn){
        float fogFactor = computeFog();
        vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
        fColor = mix(fogColor*timeOfDay, vec4(color, 1.0f), fogFactor);
    }
    else 
        fColor = vec4(color, 1.0f);
}
