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

// macros
//	Set assimp to split the polygons into triangles, generate smooth normals for lighting,
//		flip the UVs along the y-axis, and join identical vertices (aka dupe-handling)
#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)

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

	PrxModel::PrxModel(PrxDevice& device, const PrxModel::OldModelData& data) : prxDevice{ device } {
		/*texDescriptorPool = PrxDescriptorPool::Builder(prxDevice)
			.setMaxSets(1) // increase this to 2 or more when making room for materials in the future
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) 
			.build();
		texMatsDescriptorSets = std::vector<VkDescriptorSet>(1);*/
		
		createVertexBuffers(data.vertices);
		createIndexBuffer(data.indices);
	}

	/*PrxModel::PrxModel(PrxDevice& device, PrxModel::MeshEntryData& data) : prxDevice{device}, modelData{data} {
		createVertexBuffers(data.vertices);
		createIndexBuffer(data.indices);
	}*/

	PrxModel::PrxModel(PrxDevice& device, const PrxModel::ModelData& data) : prxDevice{device} {
		createVertexBuffers(data.vertices);
		createIndexBuffer(data.indices);
		// move the mesh data and texture data with std::move

	}

	PrxModel::~PrxModel() {
		freeBuffers();
	}

	std::unique_ptr<PrxModel> PrxModel::createModelFromFile(PrxDevice& device, const std::string& filepath) {
		OldModelData builder{};
		builder.loadModel(filepath);
		return std::make_unique<PrxModel>(device, builder);
	}

	std::unique_ptr<PrxModel> PrxModel::createModelFromFileAssimp(PrxDevice& device, const std::string& filepath) {
		ModelData builder{ device };
		builder.loadModel(filepath);
		return std::make_unique<PrxModel>(device, builder);
	}

	void PrxModel::createTexture(const std::string& texFP) {
		tex = std::make_unique<PrxTexture>(prxDevice, texFP);
		setupModelTexture();
	}

	void PrxModel::setupModelTexture() {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = tex.get()->getSampler();
		imageInfo.imageView = tex.get()->getImageView();
		imageInfo.imageLayout = tex.get()->getImageLayout();

		auto imageSetLayout = PrxDescriptorSetLayout::Builder(prxDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // textures should only go to the fragment shader
			.build();

		PrxDescriptorWriter(*imageSetLayout, *texDescriptorPool)
			.writeImage(0, &imageInfo)
			.build(texMatsDescriptorSets[0]);
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

	void PrxModel::freeBuffers() {
		indexBuffer.reset();
		vertexBuffer.reset();
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

	/*void PrxModel::bind(VkCommandBuffer commandBuffer, int baseVertex) {
		VkBuffer buffers[] = { vertexBuffer->getBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, baseVertex, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}
	
	void PrxModel::drawAssimp(VkCommandBuffer commandBuffer) {
		
		for (int i = 0; i < meshes.size(); i++) {
			int materialIndex = meshes[i].matIntex;

			assert(materialIndex < textures.size());

			if (textures[materialIndex]) {
				textures[materialIndex]->bindTexture();
			}

			// there has to be a way to draw multiple meshes (one at a time) with only 1
			//	vertex and index buffer...
			bind(commandBuffer, meshes[i].baseVertex);
			
			if (hasIndexBuffer) {
				vkCmdDrawIndexed(commandBuffer, meshes[i].numIndices, 1, meshes[i].baseIndex, 0, 0);
			}
			else { // failsafe: draw the whole dang thing lol
				vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
			}
		}
	}*/


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

	bool PrxModel::OldModelData::loadModel(const std::string& filepath) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		// clear previous vertices and indices arrays
		vertices.clear();
		indices.clear();

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
			throw std::runtime_error(warn + err);
		}

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
			/*unsigned int material_id, last_mat_id = 0;
			if (shape.mesh.material_ids.size() > 0) {
				material_id = last_mat_id = shape.mesh.material_ids[0];
			}*/

			int i = 0;
			for (const auto& index : shape.mesh.indices) {

				/*if (i % 3 == 0) {
					last_mat_id = material_id;
					material_id = shape.mesh.material_ids[i / 3];
				}*/


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

					//vertex.mat_id = last_mat_id;
				}

				// if the vertex is new, add to unique vertex map
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);

				//i++;
			}
		}

		// if here, load successful
		return true;

	}

	bool PrxModel::ModelData::loadModel(const std::string& filepath) {
		Assimp::Importer importer;
		bool ret;

		const aiScene* pScene = importer.ReadFile(filepath, ASSIMP_LOAD_FLAGS);

		if (pScene) {
			ret = initFromScene(pScene, filepath);
		}
		else {
			throw std::runtime_error("failed to parse file: " + filepath + ". " + importer.GetErrorString());
		}

		return ret;

	}

	bool PrxModel::ModelData::initFromScene(const aiScene* pScene, const std::string& filepath) {
		meshes.resize(pScene->mNumMeshes);
		textures.resize(pScene->mNumTextures);
		
		int numVerts = 0;
		int numIndices = 0;
		bool ret;

		countVerticesAndIndices(pScene, numVerts, numIndices);

		reserveSpace(numVerts, numIndices);

		for (int i = 0; i < meshes.size(); i++) {
			const aiMesh* paiMesh = pScene->mMeshes[i];
			initSingleMesh(paiMesh);
		}

		if (!initMaterials(pScene, filepath)) {
			return false;
		}
		
		return true;

	}

	void PrxModel::ModelData::countVerticesAndIndices(const aiScene* pScene, int& numVertices, int& numIndices) {
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i].matIntex = pScene->mMeshes[i]->mMaterialIndex;
			if (pScene->mMeshes[i]->mPrimitiveTypes == aiPrimitiveType_TRIANGLE) {
				meshes[i].numIndices = pScene->mMeshes[i]->mNumFaces * 3; // multiplied by 3 because triangle has 3 vertices
			}
			else continue; // do not load this mesh if doesn't have triangles; not currently supporting
			meshes[i].baseVertex = numVertices;
			meshes[i].baseIndex = numIndices;

			numVertices += pScene->mMeshes[i]->mNumVertices;
			numIndices += meshes[i].numIndices;
		}
	}

	void PrxModel::ModelData::reserveSpace(int numVertices, int numIndices) {
		vertices.clear();
		indices.clear();

		vertices.resize(numVertices);
		indices.resize(numIndices);
	}

	void PrxModel::ModelData::initSingleMesh(const aiMesh* paiMesh) {
		const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (int i = 0; i < paiMesh->mNumVertices; i++) {
			const aiVector3D& pPos = paiMesh->mVertices[i];
			const aiVector3D& pNormal = paiMesh->mNormals[i];
			const aiVector3D& pUV = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : zero3D;

			Vertex v{ glm::vec3(pPos.x, pPos.y, pPos.z),
				glm::vec3(zero3D.x, zero3D.y, zero3D.z),
				glm::vec3(pNormal.x, pNormal.y, pNormal.z),
				glm::vec2(pUV.x, pUV.y) };

			vertices.push_back(v);
		}

		for (int i = 0; i < paiMesh->mNumFaces; i++) {
			const aiFace& face = paiMesh->mFaces[i];
			assert(face.mNumIndices == 3 && "face does not contain exactly 3 indices");
			indices.push_back(static_cast<uint32_t>(face.mIndices[0]));
			indices.push_back(static_cast<uint32_t>(face.mIndices[1]));
			indices.push_back(static_cast<uint32_t>(face.mIndices[2]));
		}
	}

	bool PrxModel::ModelData::initMaterials(const aiScene* pScene, const std::string& filepath) {
		std::string::size_type slash = filepath.find_last_of("/");
		std::string dir;

		if (slash == std::string::npos) {
			dir = ".";
		}
		else if (slash == 0) {
			dir = "/";
		}
		else {
			dir = filepath.substr(0, slash);
		}

		bool ret = true;

		for (int i = 0; i < pScene->mNumMaterials; i++) {
			const aiMaterial* pMaterial = pScene->mMaterials[i];

			textures[i] = nullptr;
			
			if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString path;

				if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path,
					NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					std::string p(path.data);

					if (p.substr(0, 2) == ".\\") {
						p = p.substr(2, p.size() - 2);
					}

					std::string fullpath = dir + "/" + p;

					// TODO: in the future, have a way to handle failure of loading textures
					//	without throwing a runtime error
					textures[i] = new PrxTexture(prxDevice, fullpath);
				}
			}
		}

		return true;

	}
}