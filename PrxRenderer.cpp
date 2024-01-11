#include "PrxRenderer.hpp"
#include "PrxFrameInfo.hpp"

// std
#include <stdexcept>
#include <array>
#include <iostream>

namespace prx {

	std::unique_ptr<PrxDescriptorPool> PrxRenderer::globalPool;
	std::vector<std::unique_ptr<PrxBuffer>> PrxRenderer::uboBuffers;
	std::unique_ptr<PrxDescriptorSetLayout> PrxRenderer::globalSetLayout;

	PrxRenderer::PrxRenderer(PrxWindow& window, PrxDevice& device) : prxWindow{ window }, prxDevice{ device } {
		createGlobalDescriptors();
		recreateSwapChain();
		createCommandBuffers();
	}

	PrxRenderer::~PrxRenderer() {

		freeCommandBuffers();
	}

	void PrxRenderer::recreateSwapChain() {
		auto extent = prxWindow.getExtent();

		// wait if one or more extents are sizeless
		while (extent.width == 0 || extent.height == 0) {
			extent = prxWindow.getExtent();
			glfwWaitEvents();
		}

		// wait until old swap chain is done being used before creating new one
		vkDeviceWaitIdle(prxDevice.device());


		// destroy the old swap chain BEFORE creating a new one

		if (prxSwapChain == nullptr) {
			prxSwapChain = std::make_unique<PrxSwapChain>(prxDevice, extent);
		}
		else {
			std::shared_ptr<PrxSwapChain> oldSwapChain = std::move(prxSwapChain);

			prxSwapChain = std::make_unique<PrxSwapChain>(prxDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*prxSwapChain.get())) {
				throw std::runtime_error("Swap chain image (or depth) format has changed!");
				// TODO: Add a callback function that notifies the app that an incompatible render pass was created
			}
			
		}

		// COME BACK TO THIS
	}

	void PrxRenderer::createGlobalDescriptors() {
		//globalDescriptorSets = std::vector<VkDescriptorSet>(PrxSwapChain::MAX_FRAMES_IN_FLIGHT * 2);
		globalPool = PrxDescriptorPool::Builder(prxDevice)
			.setMaxSets(PrxSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, PrxSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, PrxSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		globalSetLayout = PrxDescriptorSetLayout::Builder(prxDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // textures should only go to the fragment shader
			.build();

		// setup ubo buffers
		uboBuffers = std::vector<std::unique_ptr<PrxBuffer>>(PrxSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<PrxBuffer>(
				prxDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				prxDevice.properties.limits.minUniformBufferOffsetAlignment);
			uboBuffers[i]->map();
		}
	}

	void PrxRenderer::createCommandBuffers() {

		commandBuffers.resize(PrxSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = prxDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(prxDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void PrxRenderer::freeGlobalDescriptors() {
		
	}

	void PrxRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
			prxDevice.device(),
			prxDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data()
		);

		commandBuffers.clear();
	}

	VkCommandBuffer PrxRenderer::beginFrame() {
		assert(!isFrameStarted && "Cannot call beginFrame() while frame already in progress");

		// fetches the index of the frame to render to next
		//	Also auto handles double and triple buffering CPU synchronization
		auto result = prxSwapChain->acquireNextImage(&currentImageIndex);

		// if the result is an out of date image, recreate the swap chain and return
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr; // frame has not successfully started
		}

		// Come back to this later, as the VK suboptimal khr thing can happen when you resize the window
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire the swap chain image");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("command buffer failed to begin recording!");
		}

		return commandBuffer;
	}
	void PrxRenderer::endFrame() {

		assert(isFrameStarted && "Cannot call endFrame() while frame not in progress");

		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}


		auto result = prxSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		// if the window was resized, reset the resize flag, recreate swap chain and return
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR 
			|| prxWindow.wasWindowResized()) {
			prxWindow.resetWindowResizedFlag();
			recreateSwapChain();
		} else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to acquire the swap chain image");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % PrxSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void PrxRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		
		assert(isFrameStarted && "Cannot call beginSwapChainRenderPassFrame() while frame not in progress");
		assert(commandBuffer == getCurrentCommandBuffer()
			&& "Cannot begin render pass on command buffer from a different frame");


		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = prxSwapChain->getRenderPass();
		renderPassInfo.framebuffer = prxSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = prxSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(prxSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(prxSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ { 0,0 }, prxSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	}

	void PrxRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Cannot call endSwapChainRenderPass if frame not in progress");
		assert(commandBuffer == getCurrentCommandBuffer()
			&& "Cannot end render pass on command buffer from a different frame");


		vkCmdEndRenderPass(commandBuffer);
	}

}