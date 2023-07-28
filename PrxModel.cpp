#include "PrxModel.hpp"

// lib
#define TINYOBJECTLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

// std
#include <cassert>
#include <cstring>

namespace prx {

	PrxModel::PrxModel(PrxDevice& device, const PrxModel::ModelData& data) : prxDevice{ device } {
		createVertexBuffers(data.vertices);
		createIndexBuffer(data.indices);
	}
	
	
	PrxModel::~PrxModel() {
		vkDestroyBuffer(prxDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(prxDevice.device(), vertexBufferMemory, nullptr);

		if (hasIndexBuffer) {
			vkDestroyBuffer(prxDevice.device(), indexBuffer, nullptr);
			vkFreeMemory(prxDevice.device(), indexBufferMemory, nullptr);
		}
	}
	
	void PrxModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());

		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		prxDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		vkMapMemory(prxDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(prxDevice.device(), stagingBufferMemory);

		// using local data on the GPU allows for faster performance
		prxDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferMemory);

		// copy the staging buffer to the vertex buffer
		prxDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		// free the staging buffer from memory
		vkDestroyBuffer(prxDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(prxDevice.device(), stagingBufferMemory, nullptr);

	}

	void PrxModel::createIndexBuffer(const std::vector<uint32_t>& indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;
		
		if (!hasIndexBuffer) return;

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		prxDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		vkMapMemory(prxDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(prxDevice.device(), stagingBufferMemory);

		// using local data on the GPU allows for faster performance
		prxDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferMemory);

		// copy the staging buffer to the vertex buffer
		prxDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		// free the staging buffer from memory
		vkDestroyBuffer(prxDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(prxDevice.device(), stagingBufferMemory, nullptr);

	}

	void PrxModel::draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
		
	}

	void PrxModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}
	}
	
	// Note: you could just return a vector containing a single struct with the 3
	//	fields below as its data, but for readability purposes, that's not done here.
	std::vector<VkVertexInputBindingDescription> PrxModel::Vertex::getBindingDescriptions() {

		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescriptions;
	};


	std::vector<VkVertexInputAttributeDescription> PrxModel::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0; // location 0
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1; // location 1
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
		attributeDescriptions[1].offset = offsetof(Vertex, color); // offsetof calculates the byte offset of the given member in the struct

		return attributeDescriptions;
	};

}