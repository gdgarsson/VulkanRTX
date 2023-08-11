#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

struct PointLight {
	vec4 position; // ignore w
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10]; // change this to eventually use Vulkan's Specialization Constants instead of hard-coding
							   // If you change the MAX_LIGHTS in c++, must also change the number here when hard-coded
	int numLights;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix; // mat4 for alignment reasons, truncate into mat3 when used
} push;

void main() {
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0);
	vec3 surfaceNormal = normalize(fragNormalWorld);
	
	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);
	
	for(int i = 0; i < ubo.numLights; i++){
		PointLight light = ubo.pointLights[i];
		
		vec3 directionToLight = light.position.xyz - fragPosWorld;
		float attenuation = 1.0 / dot(directionToLight, directionToLight); //dot of itself = distance squared
		directionToLight = normalize(directionToLight); // normalize direction to light AFTER attenuation calculation
		
		float cosAngIncidence = max(dot(surfaceNormal, directionToLight),0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;
		
		diffuseLight += intensity * cosAngIncidence;
		
		// specular light (Blinn-Phong method)
		vec3 halfAngle = normalize(directionToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		
		// Higher values in this power function = sharper highlights
		// Note: in the future, pass in the 2nd value on a per-object basis
		blinnTerm = pow(blinnTerm, 128.0); 
		specularLight += intensity * blinnTerm;
		
	}

	outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}