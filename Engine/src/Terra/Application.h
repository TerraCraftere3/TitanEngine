#pragma once

#include "Core.h"

namespace Terra
{
	class TERRA_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// Declared by Client
	Application *CreateApplication();

} // namespace Terra