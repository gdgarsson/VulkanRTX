#include "PrxTest.h"

namespace prx {
	/*void PrxTest::bindTexture() {
		globalPool = PrxDescriptorPool::Builder(prxDevice)
			.setMaxSets(PrxSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, PrxSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, PrxSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = getSampler();
		imageInfo.imageView = getImageView();
		imageInfo.imageLayout = getImageLayout();

		std::vector<VkDescriptorSet> globalDescriptorSets(PrxSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			PrxDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &imageInfo)
				.build(globalDescriptorSets[i]);
		}
	}*/
}