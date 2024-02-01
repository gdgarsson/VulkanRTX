#pragma once
#include "PrxWindow.hpp"
#include "PrxDevice.hpp"


namespace prx {

	class PrxRenderGraph
	{

	};

	class PrxRenderPass {
	public:
		PrxRenderPass(PrxDevice& deviceRef);

		VkRenderPass getRenderPass() { return renderPass; };
	private:
		VkRenderPass renderPass;

		PrxDevice& device;

		VkFormat imageFormat;
		VkFormat depthFormat;

		void createRenderPass();
	};

}

