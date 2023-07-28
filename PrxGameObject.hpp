#pragma once

#include "PrxModel.hpp"

//lib
#include <glm/gtc/matrix_transform.hpp>

//std
#include <memory>

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
		glm::mat4 mat4() {
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
				{translation.x, translation.y, translation.z, 1.0f} };
		}
	};

	class PrxGameObject
	{
	public:
		using id_t = unsigned int;

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

