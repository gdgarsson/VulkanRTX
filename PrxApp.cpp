#include "PrxApp.hpp"
#include "PrxGlobalVars.hpp"
#include "KeyboardMovementController.hpp"
#include "PrxCamera.hpp"
#include "PrxBuffer.hpp"

#include "systems/SimpleRenderSystem.hpp"
#include "systems/PointLightSystem.hpp"

#include "PrxTexture.hpp"

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
        appGlobalPool = PrxDescriptorPool::Builder(prxDevice)
            .setMaxSets(PrxSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, PrxSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, PrxSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
		loadGameObjects();
	}

	PrxApp::~PrxApp() {
		
	}

	// This is the game loop function
	void PrxApp::run() {

        uboBuffers = std::vector<std::unique_ptr<PrxBuffer>>(PrxSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<PrxBuffer>(
                prxDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                prxDevice.properties.limits.minUniformBufferOffsetAlignment);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = PrxDescriptorSetLayout::Builder(prxDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // textures should only go to the fragment shader
            .build();

        PrxTexture texture = PrxTexture(prxDevice, "textures/texture.jpg");

        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = texture.getSampler();
        imageInfo.imageView = texture.getImageView();
        imageInfo.imageLayout = texture.getImageLayout();

        std::vector<VkDescriptorSet> globalDescriptorSets(PrxSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            PrxDescriptorWriter(*globalSetLayout, *appGlobalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        }

		SimpleRenderSystem simpleRenderSystem{ prxDevice, 
            prxRenderer.getSwapChainRenderPass(), 
            globalSetLayout->getDescriptorSetLayout()};
        
        PointLightSystem pointLightSystem{ prxDevice,
            prxRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()};

        PrxCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = PrxGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
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

            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);

			if (auto commandBuffer = prxRenderer.beginFrame()) {
                int frameIndex = prxRenderer.getFrameIndex();
                FrameInfo frameInfo{ frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects};
                
                // update objects
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
				prxRenderer.beginSwapChainRenderPass(commandBuffer);

                // order matters! Render solids first, then semi-transparents
				simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);
				
                prxRenderer.endSwapChainRenderPass(commandBuffer);
				prxRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(prxDevice.device());
        
        for (int i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i].reset();
        }
	}

	// load models used in the program
	void PrxApp::loadGameObjects() {
        std::shared_ptr<PrxModel> carModel = PrxModel::createModelFromFile(prxDevice, "models/Cadillac/Cadillac_CT4_V_2022.obj");
        auto car = PrxGameObject::createGameObject();
        car.model = carModel;
        car.transform.scale = { 0.5f, 0.5f, 0.5f };
        gameObjects.emplace(car.getId(), std::move(car));

        std::shared_ptr<PrxModel> flatVaseModel = PrxModel::createModelFromFile(prxDevice, "models/flat_vase.obj");

        auto flatVase = PrxGameObject::createGameObject();
        flatVase.model = flatVaseModel;
        flatVase.transform.translation = { .5f, .5f, 0.f };
        flatVase.transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        std::shared_ptr<PrxModel> smoothVaseModel = PrxModel::createModelFromFile(prxDevice, "models/smooth_vase.obj");

        auto smoothVase = PrxGameObject::createGameObject();
        smoothVase.model = smoothVaseModel;
        smoothVase.transform.translation = { -.5f, .5f, 0.f };
        smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        std::shared_ptr<PrxModel> quad = PrxModel::createModelFromFile(prxDevice, "models/quad.obj");

        auto floor = PrxGameObject::createGameObject();
        floor.model = quad;
        floor.transform.translation = { 0.f, .5f, 0.f }; // move the floor down a tad
        floor.transform.scale = { 3.f, 1.f, 3.f }; // scale by 3x, 1y, 3z
        gameObjects.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            { .1f, .1f, 1.f },
            { .1f, 1.f, .1f },
            { 1.f, 1.f, .1f },
            { .1f, 1.f, 1.f },
            { 1.f, 1.f, 1.f }  //
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = PrxGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];

            // divide circle into equal size slices, then rotate each point light around the circumference
            //  Effectively makes a "ring" of lights
            auto rotateLight = glm::rotate(glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
	}

    // don't use this yet, its untested and not done
    void PrxApp::unloadGameObjects() {
        std::unordered_map<unsigned int, PrxGameObject>::iterator it;

        for (auto const& obj : gameObjects) {
            obj.second.~PrxGameObject();
        }
    }
}