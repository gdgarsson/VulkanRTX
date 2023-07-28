#include "PrxWindow.hpp"

#include <stdexcept>

namespace prx {
	PrxWindow::PrxWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	PrxWindow::~PrxWindow() {
		glfwDestroyWindow(window); // Delete the window from the screen and free up memory

		glfwTerminate(); // Terminate GLFW, removing all instances of GLFW cursors/windows from memory
	}

	void PrxWindow::initWindow() {
		glfwInit(); // initialize GFLW

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Ensure GLFW doesn't create a OpenGL Instance (this program is using Vulkan)
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Enable resize capability

		window = glfwCreateWindow(width, width, windowName.c_str(), nullptr, nullptr); // Create a GLFW Window
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback); // calls frameBufferResizedCallback whenever window is resized with the window's new width/height

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void PrxWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void PrxWindow::frameBufferResizedCallback(GLFWwindow* window, int width, int height) {
		auto prxWindow = reinterpret_cast<PrxWindow*>(glfwGetWindowUserPointer(window));
		prxWindow->frameBufferResized = true;
		prxWindow->width = width;
		prxWindow->height = height;
	}
}