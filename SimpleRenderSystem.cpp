#include "SimpleRenderSystem.hpp"

// libs
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp";
#include "glm/gtc/constants.hpp"

// std
#include <stdexcept>
#include <array>
#include <iostream>

namespace prx {

	struct SimplePushConstantData {
		glm::mat4 transform{ 1.f };
		alignas(16) glm::vec3 color; // alignas(16) due to shader alignment requirement
	};

	SimpleRenderSystem::SimpleRenderSystem(PrxDevice& device, VkRenderPass renderPass) : prxDevice{ device } {
		
		createPipelineLayout();
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem() {
		vkDestroyPipelineLayout(prxDevice.device(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout() {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(prxDevice.device(), &pipelineLayoutInfo,
			nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {

		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

		PipelineConfigInfo pipelineConfig{};

		PrxPipeline::defaultPipelineConfigInfo(pipelineConfig);


		// NOT PERMANENT! Will change this to be based on individual render passes rather than
		//		swap chain later
		pipelineConfig.renderPass = renderPass;

		pipelineConfig.pipelineLayout = pipelineLayout;
		prxPipeline = std::make_unique<PrxPipeline>(prxDevice,
			"shaders/simple_vert.spv", "shaders/simple_frag.spv",
			pipelineConfig);
	}

	void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer,
			std::vector<PrxGameObject>& gameObjects,
			const PrxCamera& camera) {
		prxPipeline->bind(commandBuffer);

		auto projectionView = camera.getProjection() * camera.getView();

		for (auto& obj : gameObjects) {
			SimplePushConstantData push{};
			push.color = obj.color;
			push.transform = projectionView * obj.transform.mat4();

			vkCmdPushConstants(commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);

			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}


}