#pragma once

// prx
#include "PrxGameObject.hpp"
#include "PrxWindow.hpp"
#include "PrxDevice.hpp"
#include "PrxRenderer.hpp"
#include "PrxDescriptors.hpp"

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

		// Any descriptors that should be shared by multiple systems can use this pool
		// Notes: Order of Declaration matters
		//		  If a given system's lifespan is temporary and using this pool, be sure to 
		//			free the respective descriptors in the given system's destructor.
		std::unique_ptr<PrxDescriptorPool> globalPool{}; 

		PrxGameObject::Map gameObjects; // note; this is likely to change into an overall "models" or "objects" pool


	};
}


