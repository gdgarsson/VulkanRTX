#pragma once

// prx
#include "../PrxPipeline.hpp"
#include "../PrxGameObject.hpp"
#include "../PrxDevice.hpp"
#include "../PrxCamera.hpp"
#include "../PrxFrameInfo.hpp"

// std
#include <memory>
#include <vector>

namespace prx {

	class SimpleRenderSystem
	{
	public:

		SimpleRenderSystem(PrxDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		// do not allow for copying
		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		void operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		PrxDevice& prxDevice;

		std::unique_ptr<PrxPipeline> prxPipeline;
		VkPipelineLayout pipelineLayout;


	};
}


