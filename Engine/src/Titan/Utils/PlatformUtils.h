#pragma once

#include "Titan/PCH.h"

namespace Titan {

	class TI_API FileDialogs
	{
	public:
		// These return empty strings if cancelled
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};

}