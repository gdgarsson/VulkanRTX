#pragma once

// libs
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp";

namespace prx {

	class PrxCamera
	{
	public:
		void setOrthographicProjection(
			float left, float right, float top, float bottom, float near, float far);

		void setPerspectiveProjection(float fovy, float aspect, float near, float far);

		// Note: position is also known as "eye", direction is also known as "at"
		//	Up is initialized to the -y axis by default
		void setViewDirection(glm::vec3 position,
			glm::vec3 direction,
			glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});

		// Note: position is also known as "eye"
		//	This is akin to a "lookAt" function
		void setViewTarget(glm::vec3 position,
			glm::vec3 target,
			glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });

		// Note: position is also known as "eye"
		// Uses Euler angles to specify the orientation of the camera
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& getProjection() const { return projectionMatrix; };
		const glm::mat4& getView() const { return viewMatrix; };
		const glm::mat4& getInverseView() const { return inverseViewMatrix; };
		
		// Note: in the future, try doing an order-independent rendering method
		const glm::vec3 getPosition() const { return glm::vec3(inverseViewMatrix[3]); };


	private:
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		glm::mat4 inverseViewMatrix{1.f};
	};

}

