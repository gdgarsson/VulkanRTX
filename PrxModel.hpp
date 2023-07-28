#pragma once
#include "PrxDevice.hpp"

// libs
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp";

// std
#include <vector>

namespace prx {

	class PrxModel
	{
	public:

		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;
			

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		struct ModelData {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
		};

		PrxModel(PrxDevice& device, const PrxModel::ModelData &data);
		~PrxModel();

		// do not allow for copying
		//	Especially for Models, due to them handling device and vertex memory
		PrxModel(const PrxModel&) = delete;
		void operator=(const PrxModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffer(const std::vector<uint32_t>& indices);

		PrxDevice& prxDevice;

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;

		VkBuffer indexBuffer;
		VkDeviceMemory indexBufferMemory;
		uint32_t indexCount;

		bool hasIndexBuffer = false;

	};
}

