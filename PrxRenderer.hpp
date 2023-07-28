#pragma once

// prx
#include "PrxWindow.hpp"
#include "PrxDevice.hpp"
#include "PrxSwapChain.hpp"

// std
#include <memory>
#include <vector>
#include <cassert>

namespace prx {

	class PrxRenderer
	{
	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		
		PrxWindow& prxWindow;
		PrxDevice& prxDevice;
		std::unique_ptr<PrxSwapChain> prxSwapChain;

		std::vector<VkCommandBuffer> commandBuffers;

		bool isFrameStarted{ false };
		int currentFrameIndex{0};
		uint32_t currentImageIndex;

	public:

		PrxRenderer(PrxWindow& window, PrxDevice& device);
		~PrxRenderer();

		// do not allow for copying
		PrxRenderer(const PrxRenderer&) = delete;
		void operator=(const PrxRenderer&) = delete;

		VkCommandBuffer beginFrame();
		void endFrame();

		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		bool isFrameInProgress() { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer() const { 
			assert(isFrameStarted && "Cannot get command buffer while frame not in progress");
			return commandBuffers[currentFrameIndex]; 
		}

		VkRenderPass getSwapChainRenderPass() const { return prxSwapChain->getRenderPass(); }
		float getAspectRatio() const { return prxSwapChain->extentAspectRatio(); };

		int getFrameIndex() {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

	};
}


