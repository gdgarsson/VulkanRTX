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

	

	class PointLightSystem
	{
	public:

		PointLightSystem(PrxDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		// do not allow for copying
		PointLightSystem(const PointLightSystem&) = delete;
		void operator=(const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		PrxDevice& prxDevice;

		std::unique_ptr<PrxPipeline> prxPipeline;
		VkPipelineLayout pipelineLayout;


	};
}


