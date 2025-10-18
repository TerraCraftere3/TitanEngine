#pragma once

#include "Terra/PCH.h"
#include "Core.h"
#include "Events/Event.h"
#include "Window.h"

namespace Terra
{
	class TERRA_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// Declared by Client
	Application *CreateApplication();

} // namespace Terra