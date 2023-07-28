#include "PrxApp.hpp"

#include "KeyboardMovementController.hpp"
#include "SimpleRenderSystem.hpp"
#include "PrxCamera.hpp"

// libs
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp";
#include "glm/gtc/constants.hpp"

// std
#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>

namespace prx {


	PrxApp::PrxApp() {
		loadGameObjects();
	}

	PrxApp::~PrxApp() {
		
	}

	// This is the game loop function
	void PrxApp::run() {


		SimpleRenderSystem simpleRenderSystem{ prxDevice, prxRenderer.getSwapChainRenderPass()};
        PrxCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = PrxGameObject::createGameObject();
        KeyboardMovementController cameraController{};
        glfwGetCursorPos(prxWindow.getGLFWwindow(), &cameraController.mouse.xPos, &cameraController.mouse.yPos);

        auto currentTime = std::chrono::high_resolution_clock::now();


		while (!prxWindow.shouldClose()) {
			glfwPollEvents(); // Note: while resizing the window, this does not draw on Windows or Linux likely due to blocking on glfwPollEvents()
							  //	Come up with a solution to draw while resizing

            // handle timing
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            // handle camera movement
            //  Note: arrow keys currently allow for rotation!
            cameraController.moveInPlaneXZ(prxWindow.getGLFWwindow(), frameTime, viewerObject);
            cameraController.toggleMouseCursor(prxWindow.getGLFWwindow());
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            // handle camera rotation
            auto newMouseX = 0.;
            auto newMouseY = 0.;
            KeyboardMovementController::MouseInfo newMouseInfo{};
            glfwGetCursorPos(prxWindow.getGLFWwindow(), &newMouseX, &newMouseY);
            
            cameraController.mouse.setMouseDelta(newMouseX, newMouseY);
            cameraController.mouse.xPos = newMouseX;
            cameraController.mouse.yPos = newMouseY;
            cameraController.handleMouseLook(prxWindow.getGLFWwindow(), frameTime, viewerObject);

            float aspect = prxRenderer.getAspectRatio();

            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			if (auto commandBuffer = prxRenderer.beginFrame()) {
				prxRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				prxRenderer.endSwapChainRenderPass(commandBuffer);
				prxRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(prxDevice.device());
	}

    // temporary helper function, creates a 1x1x1 cube centered at offset with an index buffer
    std::unique_ptr<PrxModel> createCubeModel(PrxDevice& device, glm::vec3 offset) {
        PrxModel::ModelData modelBuilder{};
        modelBuilder.vertices = {
            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        };
        for (auto& v : modelBuilder.vertices) {
            v.position += offset;
        }

        modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                                12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

        return std::make_unique<PrxModel>(device, modelBuilder);
    }
	// load models used in the program
	void PrxApp::loadGameObjects() {
        std::shared_ptr<PrxModel> prxModel = createCubeModel(prxDevice, { 0.f, 0.f, 0.f });

        auto cube = PrxGameObject::createGameObject();
        cube.model = prxModel;
        cube.transform.translation = { .0f, .0f, 2.5f }; // move the cube back a bit
        cube.transform.scale = { .5f, .5f, .5f }; // scale by .5x, .5y, .5z

		gameObjects.push_back(std::move(cube));
	}


}