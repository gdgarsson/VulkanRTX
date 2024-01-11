#pragma once
#include "PrxDevice.hpp"
#include <string>


namespace prx {

	class PrxTexture
	{
	public:
		PrxTexture(PrxDevice& device, const std::string& filepath);
		~PrxTexture();

		// delete the copy constructors
		PrxTexture(const PrxTexture&) = delete;
		PrxTexture& operator=(const PrxTexture&) = delete;
		PrxTexture(PrxTexture&&) = delete;
		PrxTexture& operator=(PrxTexture&&) = delete;


		VkSampler getSampler(){ return sampler; };
		VkImageView getImageView(){ return imageView; };
		VkImageLayout getImageLayout(){ return imageLayout; };

		void bindTexture();

	private:
		void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
		void generateMipmaps();

		int width, height, mipLevels;

		PrxDevice& prxDevice;
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView; // image views provide metadata for an image, as Vulkan does not access images direction
		VkSampler sampler;

		VkFormat imageFormat;
		VkImageLayout imageLayout;
	};
}

