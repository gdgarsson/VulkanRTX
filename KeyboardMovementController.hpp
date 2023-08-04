#pragma once

#include "PrxGameObject.hpp"
#include "PrxWindow.hpp"

namespace prx {

	class KeyboardMovementController
	{
	public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_SPACE;
            int moveDown = GLFW_KEY_LEFT_CONTROL;

            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;

            int pause = GLFW_KEY_ESCAPE;
            int click = GLFW_MOUSE_BUTTON_1;
        };

        struct MouseInfo {
            // stored as doubles to be compatible with GLFW
            double xPos;
            double yPos;

            float mouseDX;
            float mouseDY;

            void setMouseDelta(double newXPos, double newYPos) {
                mouseDX = static_cast<float>(newXPos - xPos);
                mouseDY = static_cast<float>(newYPos - yPos);
            }
        };

        void moveInPlaneXZ(GLFWwindow* window, float dt, PrxGameObject& gameObject);
        void handleMouseLook(GLFWwindow* window, float dt, PrxGameObject& gameObject);
        void toggleMouseCursor(GLFWwindow* window);

        KeyMappings keys{};
        MouseInfo mouse{};

        float moveSpeed{3.f};
        float lookSpeed{ 1.5f };
        float mouseLookSpeed{ 10.f };

	};
}


