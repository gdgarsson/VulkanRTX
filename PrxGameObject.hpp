#pragma once

#include "PrxModel.hpp"
#include "PrxTexture.hpp"

//lib
#include <glm/gtc/matrix_transform.hpp>

//std
#include <memory>
#include <unordered_map>

namespace prx {

	struct TransformComponent {
		glm::vec3 translation{}; // position offset
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f }; // size scaling
		glm::vec3 rotation{}; // rotation (in radians)
		

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		//	Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		//	https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		//	
		//	Returns the object's mat4 in the shared world space
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	struct PointLightComponent {
		float lightIntensity = 1.f;
	};

	struct GameObjectBufferData {
		glm::mat4 modelMatrix{1.f};
		glm::mat4 normalMatrix{1.f};
	};

	struct RenderSettings {
		bool generatesShadows = true;
	};

	class PrxGameObjectManager; // forward declaration for proper compatibility with PrxGameObject

	class PrxGameObject
	{
	public:
		// Note: this method of entity component systems is inefficient.
		//	Look at https://austinmorlan.com/posts/entity_component_system/ for a better system
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, PrxGameObject>;

		// No copying - avoid dupicate game objects
		PrxGameObject(const PrxGameObject&) = delete;
		PrxGameObject& operator=(const PrxGameObject&) = delete;
		PrxGameObject& operator=(PrxGameObject&&) = delete;

		// make move function and assign operator to use the default
		PrxGameObject(PrxGameObject&&) = default;

		id_t getId() { return id; }

		VkDescriptorBufferInfo getBufferInfo(int frameIndex);

		glm::vec3 color{};
		TransformComponent transform{};

		// Optional pointer components
		std::shared_ptr<PrxModel> model{};
		std::shared_ptr<PrxTexture> diffuseMap = nullptr;
		std::unique_ptr<PointLightComponent> pointLight = nullptr;

	private:
		PrxGameObject(id_t objId, const PrxGameObjectManager& manager) : id{ objId }, gameObjectManager{ manager } {}

		id_t id;
		const PrxGameObjectManager& gameObjectManager;

		friend class PrxGameObjectManager;
	};

	class PrxGameObjectManager {
	public:
		static constexpr int MAX_GAME_OBJECTS = 1000;

		PrxGameObjectManager(PrxDevice& device);
		
		// ensure there is only 1 game object manager!
		PrxGameObjectManager(const PrxGameObjectManager&) = delete;
		PrxGameObjectManager &operator=(const PrxGameObjectManager&) = delete;
		PrxGameObjectManager(PrxGameObjectManager&&) = delete;
		PrxGameObjectManager &operator=(PrxGameObjectManager&&) = delete;

		PrxGameObject &createGameObject() {
			assert(currentID < MAX_GAME_OBJECTS && "Max game object count exceeded");
			auto gameObj = PrxGameObject{ currentID++, *this };
			auto gameObjId = gameObj.getId();
			gameObj.diffuseMap = textureDefault;
			gameObjects.emplace(gameObjId, std::move(gameObj));
			return gameObjects.at(gameObjId);
		}
		
		PrxGameObject& makePointLight(
			float intensity = 10.f, float radius = 1.0f, glm::vec3 color = glm::vec3{ 1.f });

		VkDescriptorBufferInfo getBufferInfoForGameObject(int frameIndex, PrxGameObject::id_t gameObjId) const {
			return uboBuffers[frameIndex]->descriptorInfoForIndex(gameObjId);
		};

		void updateBuffer(int frameIndex);

		
		PrxGameObject::Map gameObjects{};
		std::vector<std::unique_ptr<PrxBuffer>> uboBuffers{PrxSwapChain::MAX_FRAMES_IN_FLIGHT};

	private:
		PrxGameObject::id_t currentID = 0;
		std::shared_ptr<PrxTexture> textureDefault;

	};
}

