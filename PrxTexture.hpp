#pragma once
#include "PrxDevice.hpp"
#include <string>
#include <memory>

#define TEXTURE_ARRAY_SIZE 8

namespace prx {

	class PrxTexture
	{
	public:

		// next available texture id
		// note: this ID system is incredibly rudamentary, and will likely be replaced
		//	another potential way is to hash the id, likely based on the
		//	texture's filepath and a few other variables
		//	Do that later, though.
		static unsigned int next_id;

		PrxTexture(PrxDevice& device, const std::string& filepath);
		PrxTexture(PrxDevice& device, VkFormat format, VkExtent3D extent,
			VkImageUsageFlags usage, VkSampleCountFlagBits);
		~PrxTexture();

		// delete the copy constructors
		PrxTexture(const PrxTexture&) = delete;
		PrxTexture& operator=(const PrxTexture&) = delete;
		PrxTexture(PrxTexture&&) = delete;
		PrxTexture& operator=(PrxTexture&&) = delete;

		VkSampler getSampler() const { return texSampler; };
		VkImage getImage() const { return texImage; };
		VkImageView getImageView() const { return texImageView; };
		VkImageLayout getImageLayout() const { return texImageLayout; };
		VkDescriptorImageInfo getImageInfo() const { return texImageDescriptor; };
		VkFormat getFormat() const{ return texFormat; };
		VkExtent3D getExtent() const { return texExtent; };

		unsigned int getTextureID() { return id; };

		void updateDescriptor();
		void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

		static std::unique_ptr<PrxTexture> makeTextureFromFile(
			PrxDevice& device, const std::string& filepath);

	private:

		void createTextureImage(const std::string& filepath);
		void createTextureImageView(VkImageViewType viewType);
		void createTextureSampler();
		void assignID();
		void generateMipmaps();

		unsigned int id;

		uint32_t mipLevels{ 1 }, layerCount{ 1 };

		PrxDevice& prxDevice;

		VkSampler texSampler = nullptr;
		VkImage texImage = nullptr;
		VkDeviceMemory texImageMemory = nullptr;
		VkImageView texImageView = nullptr; // image views provide metadata for an image, as Vulkan does not access images direction
		VkDescriptorImageInfo texImageDescriptor;
		VkImageLayout texImageLayout;
		VkFormat texFormat;
		VkExtent3D texExtent;
	};
}