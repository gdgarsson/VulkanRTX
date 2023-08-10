#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;

struct PointLight {
	vec4 position; // ignore w
	vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
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
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);

    gl_Position = ubo.projection * (ubo.view * positionWorld);
	
	// normal matrix is truncated back into a mat3
	fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
	fragPosWorld = positionWorld.xyz;
	fragColor = color;
	
	
}