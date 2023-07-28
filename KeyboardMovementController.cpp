#include "KeyboardMovementController.hpp"

#include <limits>

namespace prx {

	void KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, PrxGameObject& gameObject) {
		glm::vec3 rotate{ 0 };

		if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
		if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;

		if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
		if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;



		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			// update independent of frame rate (via dt)
			//	normalize rotate so the game object doesn't rotate faster diagonally
			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}

		// Not necessary, but for now, limit game objects from being able to go upside down
		//	Pitch limited to roughly +/- 85 degrees off x axis
		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>()); // prevents repeat spinning causing an overflow

		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		glm::vec3 moveDir{ 0.f };

		if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
		if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;

		if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
		if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;

		if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
		if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;


		if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
			// update independent of frame rate (via dt)
			//	normalize moveDir so the game object doesn't translate faster diagonally
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		}
	}


	void KeyboardMovementController::handleMouseLook(GLFWwindow* window, float dt, PrxGameObject& gameObject) {
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) return; // do not do anything if cursor mode is normal

		glm::vec3 rotate{ 0 };

		rotate.y += mouse.mouseDX;
		rotate.x -= mouse.mouseDY; // subtract because vulkan flips things

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			// update independent of frame rate (via dt)
			//	normalize rotate so the game object doesn't rotate faster diagonally
			gameObject.transform.rotation += mouseLookSpeed * dt * glm::normalize(rotate);
		}

		// Not necessary, but for now, limit game objects from being able to go upside down
		//	Pitch limited to roughly +/- 85 degrees off x axis
		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>()); // prevents repeat spinning causing an overflow
	}

	void KeyboardMovementController::toggleMouseCursor(GLFWwindow* window) {
		
		if (glfwGetKey(window, keys.pause) == GLFW_PRESS
			&& glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			return; // return so that the rest of the code doesn't run
		}

		// Under current implementation, this ONLY happens if the mouse is OVER the window.
		//	No need to worry if you want to escape out and click on something else
		if (glfwGetMouseButton(window, keys.click) == GLFW_PRESS
			&& glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			return;
		}
	}
}