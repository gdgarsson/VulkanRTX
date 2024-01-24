#include "PrxTexture.hpp"

// engine
#include "PrxBuffer.hpp"
#include "PrxSwapChain.hpp"
#include "PrxRenderer.hpp"
#include "PrxGlobalVars.hpp"
#include "PrxDescriptors.hpp"

// lib
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// std
#include <stdexcept>
#include <cmath>

namespace prx {
	unsigned int PrxTexture::next_id = 0;

	PrxTexture::PrxTexture(PrxDevice& device, const std::string& filepath) : prxDevice(device) {
		createTextureImage(filepath);
		createTextureImageView(VK_IMAGE_VIEW_TYPE_2D); // only supports 2D images for now
													   //	in the future, update to support more image types!
		createTextureSampler();
		assignID();
		updateDescriptor();
	}

	PrxTexture::PrxTexture(PrxDevice& device, VkFormat format, VkExtent3D extent,
		VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount) : prxDevice(device) {

		VkImageAspectFlags aspectMask = 0;
		VkImageLayout imageLayout;

		texFormat = format;
		texExtent = extent;

		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent = extent;
		imageCreateInfo.mipLevels = 1; // not really doing many mip layers rn
		imageCreateInfo.arrayLayers = 1; // consider changing to TEXTURE_ARRAY_SIZE when you're able to do texture arrays
		imageCreateInfo.samples = sampleCount;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = usage;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		device.createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			texImage, texImageMemory);

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.subresourceRange = {};
		viewCreateInfo.subresourceRange.aspectMask = aspectMask;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;
		viewCreateInfo.image = texImage;

		if (vkCreateImageView(device.device(), &viewCreateInfo, nullptr, &texImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		// seperate out the sampler in the future
		if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
			// create sampler to sample from the attachment in the  fragment shader
			VkSamplerCreateInfo samplerCreateInfo{};
			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
			samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
			samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
			samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
			samplerCreateInfo.mipLodBias = 0.0f;
			samplerCreateInfo.maxAnisotropy = 1.0f;
			samplerCreateInfo.minLod = 0;
			samplerCreateInfo.maxLod = 1;
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

			if (vkCreateSampler(device.device(), &samplerCreateInfo, nullptr, &texSampler) != VK_SUCCESS) {
				throw std::runtime_error("failed to create sampler!");
			}
		}

		VkImageLayout samplerImageLayout = imageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		texImageDescriptor.sampler = texSampler;
		texImageDescriptor.imageView = texImageView;
		texImageDescriptor.imageLayout = samplerImageLayout;

		// this stuff is for texture arrays, come back to it when you are going to implement them
		/*
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.descriptorCount = 8;
		layoutBinding.binding = 1; // binding textures to bind 1
		layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Again, textures only go in the fragment shader
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		layoutBinding.pImmutableSamplers = 0;

		VkDescriptorImageInfo texImageDescriptors[TEXTURE_ARRAY_SIZE];
		for (int i = 0; i < TEXTURE_ARRAY_SIZE; ++i) {
			imageDescriptors[i].sampler = nullptr;
			imageDescriptors[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescriptors[i].imageView = imageView;
		}*/

	}

	// go to the model class and start implementing this there!
	std::unique_ptr<PrxTexture> PrxTexture::makeTextureFromFile(PrxDevice& device, const std::string& filepath) {
		return std::make_unique<PrxTexture>(device, filepath);
	}

	void PrxTexture::updateDescriptor() {
		texImageDescriptor.sampler = texSampler;
		texImageDescriptor.imageView = texImageView;
		texImageDescriptor.imageLayout = texImageLayout;
	}

	void PrxTexture::createTextureImage(const std::string& filepath) {
		int texWidth;
		int texHeight; 
		int texChannels;
		// note: somewhere in here is why texture coordinates are flipped
		stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		// find a way to make more mip levels later on when support is added later
		// example would be to use this code from how you were doing it before:
		// mipLevels = std::floor(std::log2(std::max(width, height))) + 1;
		mipLevels = 1;

		PrxBuffer stagingBuffer(
			prxDevice, 1, static_cast<uint32_t>(imageSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.map();
		stagingBuffer.writeToBuffer(pixels);

		// the above buffer class functionally does the following,
		//	but does not unmap until the wrapper's EOL (unless otherwise specified)
		/*VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		prxDevice.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(prxDevice.device(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<uint32_t>(imageSize));
		vkUnmapMemory(prxDevice.device(), stagingBufferMemory);*/

		stbi_image_free(pixels);

		texFormat = VK_FORMAT_R8G8B8A8_SRGB;
		texExtent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = texFormat;
		imageCreateInfo.extent = texExtent;
		imageCreateInfo.mipLevels = mipLevels;
		imageCreateInfo.arrayLayers = layerCount;
		imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		prxDevice.createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			texImage, texImageMemory);
		prxDevice.transitionImageLayout(texImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, layerCount);
		prxDevice.copyBufferToImage(stagingBuffer.getBuffer(), texImage,
			static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),
			layerCount);

		// copy out once mips are implemented
		prxDevice.transitionImageLayout(texImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels, layerCount);

		// if mip maps are generated then the final image will already be READ_ONLY_OPTIMAL
		//generateMipmaps(); // shift this to the device class asap, you have the code base in this file
		texImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Uncomment if doing manual memory management instead of using PrxBuffer
		/*vkDestroyBuffer(prxDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(prxDevice.device(), stagingBufferMemory, nullptr);*/
	}

	void PrxTexture::createTextureImageView(VkImageViewType viewType) {
		// image views provide metadata for an image, as Vulkan does not access images direction
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = viewType;
		imageViewCreateInfo.format = texFormat;
		imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
		imageViewCreateInfo.image = texImage;

		if (vkCreateImageView(prxDevice.device(), &imageViewCreateInfo, nullptr, &texImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image view");
		}
	}

	void PrxTexture::createTextureSampler() {
		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST; // use the nearer pixels in the sampler so image is not blurred
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST; // experiment with this and mag filter and VK_FILTER_LINEAR
		
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		
		// anistropy settings
		samplerCreateInfo.anisotropyEnable = VK_TRUE; // enable anistropic filtering
		samplerCreateInfo.maxAnisotropy = 4.0;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		// used for percentage close filtering for shadow maps
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;

		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = static_cast<float>(mipLevels);
		
		if (vkCreateSampler(prxDevice.device(), &samplerCreateInfo, nullptr, &texSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create sampler!");
		}

	}

	void PrxTexture::assignID() {
		id = next_id;
		next_id++;
	}

	PrxTexture::~PrxTexture() {
		vkDestroyImage(prxDevice.device(), texImage, nullptr);
		vkFreeMemory(prxDevice.device(), texImageMemory, nullptr);
		vkDestroyImageView(prxDevice.device(), texImageView, nullptr);
		vkDestroySampler(prxDevice.device(), texSampler, nullptr);
	}

	// Generate Mipmaps using blitting
	void PrxTexture::generateMipmaps() {
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(prxDevice.getPhysicalDevice(), texFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("Texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = prxDevice.beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = texImage;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		int32_t mipWidth = texExtent.width;
		int32_t mipHeight = texExtent.height;

		// set each mip level
		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1,
								   mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer, texImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texImage,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);


			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;

		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		prxDevice.endSingleTimeCommands(commandBuffer);

	}

	void PrxTexture::transitionLayout(
		VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = texImage;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (texFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || texFormat == VK_FORMAT_D24_UNORM_S8_UINT) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (
			oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (
			oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (
			oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
			newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (
			oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
			newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			// This says that any cmd that acts in color output or after (dstStage)
			// that needs read or write access to a resource
			// must wait until all previous read accesses in fragment shader
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			barrier.dstAccessMask =
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}
		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage,
			destinationStage,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);
	}
}