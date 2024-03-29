#pragma once

#include "PrxCamera.hpp"
#include "PrxGameObject.hpp"

// lib
#include <vulkan/vulkan.h>

namespace prx {

#define MAX_LIGHTS 10

	// Note: there IS a way to get virtually unlimited point lights!
	//	Stretch goal: Come back eventually to try and figure that one out!
	struct PointLight {
		glm::vec4 position{}; // ignore w
		glm::vec4 color{}; // w is intensity
	};

	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::mat4 inverseView{1.f}; // can access the camera's position from the last column
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // w is intensity
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		PrxCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		PrxDescriptorPool& frameDescriptorPool; // pool of descriptors that is cleared each frame
		PrxGameObject::Map& gameObjects;
	};
}