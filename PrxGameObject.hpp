#pragma once

#include "PrxModel.hpp"

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

	class PrxGameObject
	{
	public:
		// Note: this method of entity component systems is inefficient.
		//	Look at https://austinmorlan.com/posts/entity_component_system/ for a better system
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, PrxGameObject>;

		static PrxGameObject createGameObject() {
			static id_t currentId = 0;
			return PrxGameObject{ currentId++ };
		}

		// No copying - avoid dupicate game objects
		PrxGameObject(const PrxGameObject&) = delete;
		PrxGameObject &operator=(const PrxGameObject&) = delete;
		
		// make move function and assign operator to use the default
		PrxGameObject(PrxGameObject&&) = default;
		PrxGameObject& operator=(PrxGameObject&&) = default;

		id_t getId() { return id; }

		std::shared_ptr<PrxModel> model{};
		glm::vec3 color{};
		TransformComponent transform{};

	private:
		PrxGameObject(id_t objId) : id{ objId } {}

		id_t id;
	};
}

