#pragma once

// prx
#include "PrxGameObject.hpp"
#include "PrxWindow.hpp"
#include "PrxDevice.hpp"
#include "PrxRenderer.hpp"

// std
#include <memory>
#include <vector>

namespace prx {

	class PrxApp
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		PrxApp();
		~PrxApp();

		// do not allow for copying
		PrxApp(const PrxApp&) = delete;
		void operator=(const PrxApp&) = delete;

		void run();
	private:
		void loadGameObjects();

		PrxWindow prxWindow{ WIDTH, HEIGHT, "Hello, Vulkan!" };
		PrxDevice prxDevice{ prxWindow };
		PrxRenderer prxRenderer{ prxWindow, prxDevice };

		std::vector<PrxGameObject> gameObjects; // note; this is likely to change into an overall "models" or "objects" pool


	};
}


