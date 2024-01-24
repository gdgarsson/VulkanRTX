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
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	SimpleRenderSystem::SimpleRenderSystem(PrxDevice& device, VkRenderPass renderPass,
		VkDescriptorSetLayout globalSetLayout) : prxDevice{ device } {
		
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);

	}

	SimpleRenderSystem::~SimpleRenderSystem() {
		vkDestroyPipelineLayout(prxDevice.device(), pipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		renderSystemLayout = PrxDescriptorSetLayout::Builder(prxDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT) // send uniforms to both stages
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
				VK_SHADER_STAGE_FRAGMENT_BIT)	// ONLY send texture data to the fragment shader; useless in the vertex shader
			.build();

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout,
			renderSystemLayout->getDescriptorSetLayout()};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
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

	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
		prxPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;

			// The way things are currently set up is ineffecient,
			//	as right now textures are being stored one-per-game-object, with the primary
			//	storage location being ON the game objects themselves.
			// In the future, do it this way: https://www.reddit.com/r/vulkan/comments/10tfz49/creating_descriptor_sets_for_texture_and_uniform/
			// "Instead of binding a new texture to the descriptor every time you draw the object, 
			//	bind all the textures as an array to 1 binding. Then you can pass through an index 
			//  through push constants corresponding to the texture in the array."
			// The link also has another link to getting started with arrays of textures.

			// This is for setting up the game object's descriptors into the descriptor set
			//	It is bound at set = 1 (0 is the global descriptor set)
			// This method is HIGHLY inefficient, and it would be better to implement caching
			//	Once everything works, that is next on my list
			auto bufferInfo = obj.getBufferInfo(frameInfo.frameIndex);
			auto imageInfo = obj.diffuseMap->getImageInfo();
			VkDescriptorSet gameObjectDescriptorSet;
			PrxDescriptorWriter(*renderSystemLayout, frameInfo.frameDescriptorPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &imageInfo)
				.build(gameObjectDescriptorSet);

			vkCmdBindDescriptorSets(frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				1, // starting set (0 is set to be the global descriptor, 1 should be this one)
				1, // only binding 1 descriptor (this one)
				&gameObjectDescriptorSet, 0, nullptr);

			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix(); // Note: GLM auto-converts Mat3 to a Mat4, so there's no need to make a function for extra padding when one already exists


			vkCmdPushConstants(frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
			
		}
	}


}