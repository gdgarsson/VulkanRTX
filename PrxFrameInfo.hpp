#pragma once

#include "PrxCamera.hpp"
#include "PrxGameObject.hpp"

// lib
#include <vulkan/vulkan.h>

namespace prx {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		PrxCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		PrxGameObject::Map& gameObjects;
	};
}