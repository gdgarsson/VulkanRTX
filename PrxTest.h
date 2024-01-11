#pragma once
#include <memory>
#include "PrxDescriptors.hpp"

namespace prx {
	class PrxTest
	{
	public:
		static void bindTexture();

		static std::unique_ptr<PrxDescriptorPool> globalPool;

	};

}

