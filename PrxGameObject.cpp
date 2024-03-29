#include "PrxGameObject.hpp"

// std
#include <numeric>

namespace prx {
	glm::mat4 TransformComponent::mat4() {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{translation.x, translation.y, translation.z, 1.0f} 
		};
	}

	// Returns the normal matrix
	// Note: consider looking at 
	glm::mat3 TransformComponent::normalMatrix() {
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 invScale = 1.f / scale;
		return glm::mat3{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1)
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3)
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2)
			}
		};

	}


	PrxGameObject& PrxGameObjectManager::makePointLight(float intensity,
		float radius, glm::vec3 color) {
		auto& gameObj = createGameObject();
		gameObj.color = color;
		gameObj.transform.scale.x = radius;
		gameObj.pointLight = std::make_unique<PointLightComponent>();
		gameObj.pointLight->lightIntensity = intensity;

		return gameObj;
	}

	PrxGameObjectManager::PrxGameObjectManager(PrxDevice& device) {
		
		// setup memory alignment
		// including nonCoherentAtomSize enables flushing a specific index at once
		int alignment = std::lcm(
			device.properties.limits.nonCoherentAtomSize,
			device.properties.limits.minUniformBufferOffsetAlignment
		);

		// setup UBOs
		// Note: find where the hell you put the UBOs and change it to here
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<PrxBuffer>(
				device,
				sizeof(GameObjectBufferData),
				PrxGameObjectManager::MAX_GAME_OBJECTS,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, // setup for usage as a uniform buffer
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 
				alignment);
			uboBuffers[i]->map(); // map the UBO to memory
		}
	}

	void PrxGameObjectManager::updateBuffer(int frameIndex) {
		// copy model and normal matrices for each game object into the buffer for this frame
		for (auto& kv : gameObjects) {
			auto& obj = kv.second;
			GameObjectBufferData data{};
			data.modelMatrix = obj.transform.mat4();
			data.normalMatrix = obj.transform.normalMatrix();
			uboBuffers[frameIndex]->writeToIndex(&data, kv.first);
		}
		uboBuffers[frameIndex]->flush();
	}

	VkDescriptorBufferInfo PrxGameObject::getBufferInfo(int frameIndex) {
		return gameObjectManager.getBufferInfoForGameObject(frameIndex, id);
	}

}
