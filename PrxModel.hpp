#pragma once
#include "PrxDevice.hpp"
#include "PrxBuffer.hpp"
#include "PrxDescriptors.hpp"
#include "PrxTexture.hpp"
#include "PrxMaterial.hpp"

// libs
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

// assimp
#include <assimp/Importer.hpp>// C++ importer interface
#include <assimp/scene.h>      // Output data structure
#include <assimp/postprocess.h> // Post processing flags

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

		struct OldModelData {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};
			std::vector<MtlData> mats{};

			bool loadModel(const std::string& filepath);
		};

		struct MeshEntryData {
			MeshEntryData() {
				numIndices = 0;
				baseVertex = 0;
				baseIndex = 0;
				matIntex = 0;
			}

			unsigned int numIndices;
			unsigned int baseVertex; // index of the first vertex of the current mesh
			unsigned int baseIndex;
			unsigned int matIntex;
		};

		struct ModelData {
			ModelData(PrxDevice& device) : prxDevice{ device } {

			}

			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			std::vector<MeshEntryData> meshes;
			std::vector<PrxTexture*> textures;

			PrxDevice& prxDevice;

			bool loadModel(const std::string& filepath);
			bool initFromScene(const aiScene* pScene, const std::string& filepath);
			void initSingleMesh(const aiMesh* paiMesh);
			bool initMaterials(const aiScene* pScene, const std::string& filepath);

			// helpers
			void countVerticesAndIndices(const aiScene* pScene, int& numVertices, int& numIndices);
			void reserveSpace(int numVertices, int numIndices);

		};

		PrxModel(PrxDevice& device, const PrxModel::OldModelData &data);
		PrxModel(PrxDevice& device, const PrxModel::ModelData& data);
		~PrxModel();

		// do not allow for copying
		//	Especially for Models, due to them handling device and vertex memory
		PrxModel(const PrxModel&) = delete;
		void operator=(const PrxModel&) = delete;

		static std::unique_ptr<PrxModel> createModelFromFile(PrxDevice& device, const std::string& filepath);
		static std::unique_ptr<PrxModel> createModelFromFileAssimp(PrxDevice& device, const std::string& filepath);
		
		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
		
		void bind(VkCommandBuffer commandBuffer, int baseVertex);
		void drawAssimp(VkCommandBuffer commandBuffer);

		std::unique_ptr<PrxDescriptorPool> texDescriptorPool;
		std::vector<VkDescriptorSet> texMatsDescriptorSets;

	private:

		PrxModel(PrxDevice& device);

		//MeshEntryData& modelData;

		void createVertexBuffers(const std::vector<Vertex>& vertices);
		void createIndexBuffer(const std::vector<uint32_t>& indices);
		
		void createTexture(const std::string& texFP);
		void setupModelTexture();

		void freeBuffers();

		PrxDevice& prxDevice;

		std::unique_ptr<PrxBuffer> vertexBuffer;
		uint32_t vertexCount;

		std::unique_ptr<PrxBuffer> indexBuffer;
		uint32_t indexCount;

		std::vector<MeshEntryData> meshes;
		std::vector<PrxTexture*> textures;

		std::vector<std::unique_ptr<PrxMaterial>> materials;
		std::string texFilePath;
		std::unique_ptr<PrxTexture> tex;

		bool hasIndexBuffer = false;

	};
}

