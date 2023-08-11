#version 450

// billboard offset vertices
//  Note: these vertices are in camera space
const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

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
	vec4 position;
	vec4 color;
	float radius;
} push;

// Note: This shader is designed to be a "Billboard", 
//	meaning it will always face the camera no matter the rotation.
void main() {
	fragOffset = OFFSETS[gl_VertexIndex];
	
	vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
	vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};
	
	vec3 positionWorld = push.position.xyz
		+ push.radius * fragOffset.x * cameraRightWorld
		+ push.radius * fragOffset.y * cameraUpWorld;
	
	gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}