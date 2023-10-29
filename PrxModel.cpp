#include "PrxModel.hpp"

#include "PrxUtils.hpp"

// lib
#define TINYOBJLOADER_IMPLEMENTATION
#define TINY
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <cstring>
#include <iostream>

namespace std {
	template<>
	struct hash<prx::PrxModel::Vertex> {
		size_t operator()(prx::PrxModel::Vertex const& vertex) const {
			size_t seed = 0;
			prx::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace prx {

	PrxModel::PrxModel(PrxDevice& device, const PrxModel::ModelData& data) : prxDevice{ device } {
		createVertexBuffers(data.vertices);
		createIndexBuffer(data.indices);
		createMaterials(data.mats);
	}

	PrxModel::~PrxModel() {

	}

	std::unique_ptr<PrxModel> PrxModel::createModelFromFile(PrxDevice& device, const std::string& filepath) {
		ModelData builder{};
		builder.loadModel(filepath);
		return std::make_unique<PrxModel>(device, builder);
	}

	void PrxModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());

		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		PrxBuffer stagingBuffer{
			prxDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data()); // MUST cast to a void*

		vertexBuffer = std::make_unique<PrxBuffer>(
			prxDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		// copy the staging buffer to the vertex buffer
		prxDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);

	}

	void PrxModel::createIndexBuffer(const std::vector<uint32_t>& indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) return;

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);

		PrxBuffer stagingBuffer{
			prxDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());

		indexBuffer = std::make_unique<PrxBuffer>(
			prxDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		// copy the staging buffer to the vertex buffer
		prxDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);

	}

	void PrxModel::createMaterials(const std::vector<MtlData>& mats)
	{
		for (auto& m : mats) {
			std::unique_ptr<PrxMaterial> mat = std::make_unique<PrxMaterial>(prxDevice);

			mat.get()->name = m.name;

			mat.get()->diffuse = m.diffuse;
			mat.get()->specular = m.specular;
			mat.get()->ambient = m.ambient;
			mat.get()->emission = m.emission;
			mat.get()->transmittance = m.transmittance;
			mat.get()->opacity = m.opacity;
			mat.get()->shininess = m.shininess;
			mat.get()->ior = m.ior;

			if (!m.diffuseTexFilePath.empty()) {
				mat.get()->makeDiffuseTexture(m.diffuseTexFilePath);
			}

			materials.push_back(mat);
		}
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
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
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
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({ 0,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
		attributeDescriptions.push_back({ 1,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
		attributeDescriptions.push_back({ 2,0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
		attributeDescriptions.push_back({ 3,0,VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

		return attributeDescriptions;
	};

	void PrxModel::ModelData::loadModel(const std::string& filepath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& material : materials) {
			MtlData m;
			m.name = material.name;
			m.diffuse = glm::vec3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);
			m.specular = glm::vec3(material.specular[0], material.specular[1], material.specular[2]);
			m.ambient = glm::vec3(material.ambient[0], material.ambient[1], material.ambient[2]);
			m.emission = glm::vec3(material.emission[0], material.emission[1], material.emission[2]);
			m.transmittance = glm::vec3(material.transmittance[0], material.transmittance[1], material.transmittance[2]);
			m.opacity = material.dissolve;
			m.shininess = material.shininess;
			m.ior = material.ior;

			if (!material.diffuse_texname.empty()) {
				m.diffuseTexFilePath = filepath + material.diffuse_texname;
			}

			mats.push_back(m);
		}

		for (const auto& shape : shapes) {
			Vertex vertex{};
			unsigned int material_id, last_mat_id = 0;
			if (shape.mesh.material_ids.size() > 0) {
				material_id = last_mat_id = shape.mesh.material_ids[0];
			}

			int i = 0;
			for (const auto& index : shape.mesh.indices) {

				if (i % 3 == 0) {
					last_mat_id = material_id;
					material_id = shape.mesh.material_ids[i / 3];
				}


				if (index.vertex_index >= 0) {
					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.color = {
						attrib.colors[3 * index.vertex_index + 0],
						attrib.colors[3 * index.vertex_index + 1],
						attrib.colors[3 * index.vertex_index + 2]
					};

				}
				
				if (index.normal_index >= 0) {
					vertex.normal = {
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					};
				}

				if (index.texcoord_index >= 0) {
					vertex.uv = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};

					vertex.mat_id = last_mat_id;
				}

				// if the vertex is new, add to unique vertex map
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);

				i++;
			}
		}

	}


}