#pragma once
#include "PrxDevice.hpp"
#include "PrxBuffer.hpp"
#include "PrxTexture.hpp"
#include "PrxMaterial.hpp"

// libs
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

// std
#include <memory>
#include <vector>

namespace prx {

	class PrxModel
	{
	public:

		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{};
			glm::vec3 normal{};
			glm::vec2 uv{}; // texture coordinates

			// this is inefficient - better way is to store per-face, but its what we're rolling with now
			int mat_id = 0;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position
					&& color == other.color
					&& normal == other.normal
					&& uv == other.uv;
			}
		};

		struct MtlData {
			std::string name;

			glm::vec3 diffuse;
			glm::vec3 specular;
			glm::vec3 ambient;
			glm::vec3 emission;
			glm::vec3 transmittance;

			float opacity;
			float shininess;
			float ior;

			std::string diffuseTexFilePath; // if empty, has no texture filepath

		};

		struct ModelData {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
			std::vector<MtlData> mats{};

			void loadModel(const std::string& filepath);
		};

		PrxModel(PrxDevice& device, const PrxModel::ModelData &data);
		~PrxModel();

		// do not allow for copying
		//	Especially for Models, due to them handling device and vertex memory
		PrxModel(const PrxModel&) = delete;
		void operator=(const PrxModel&) = delete;

		static std::unique_ptr<PrxModel> createModelFromFile(PrxDevice& device, const std::string& filepath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffer(const std::vector<uint32_t>& indices);
		void createMaterials(const std::vector<MtlData>& mats);

		PrxDevice& prxDevice;

		std::unique_ptr<PrxBuffer> vertexBuffer;
		uint32_t vertexCount;
		
		std::unique_ptr<PrxBuffer> indexBuffer;
		uint32_t indexCount;

		std::vector<std::unique_ptr<PrxMaterial>> materials;
		
		bool hasIndexBuffer = false;

	};
}

