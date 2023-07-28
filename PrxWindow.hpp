#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace prx {
	class PrxWindow
	{
	private:
		void initWindow();

		int width;
		int height;
		bool frameBufferResized = false;

		std::string windowName;
		GLFWwindow* window;

		static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);

	public:
		PrxWindow(int w, int h, std::string name);
		~PrxWindow();

		PrxWindow(const PrxWindow&) = delete;
		PrxWindow& operator=(const PrxWindow&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

		bool wasWindowResized() { return frameBufferResized; }
		void resetWindowResizedFlag() { frameBufferResized = false; }

		GLFWwindow* getGLFWwindow() const { return window; }
	};
}



