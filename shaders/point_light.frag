#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

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
	vec4 position;
	vec4 color;
	float radius;
} push;

void main() {
	
	// discard any fragments that occur outside of the intended circle
	float dis = sqrt(dot(fragOffset, fragOffset));
	if (dis >= 1.0) {
		discard;
	}
	
	outColor = vec4(push.color.xyz, 1.0);
}