#pragma once

// prx
#include "PrxPipeline.hpp"
#include "PrxGameObject.hpp"
#include "PrxDevice.hpp"
#include "PrxCamera.hpp"

// std
#include <memory>
#include <vector>

namespace prx {

	class SimpleRenderSystem
	{
	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		PrxDevice& prxDevice;

		std::unique_ptr<PrxPipeline> prxPipeline;
		VkPipelineLayout pipelineLayout;


	public:

		SimpleRenderSystem(PrxDevice &device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		// do not allow for copying
		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		void operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer,
			std::vector<PrxGameObject>& gameObjects,
			const PrxCamera& camera);
	};
}


