#pragma once

#ifdef TERRA_PLATFORM_WINDOWS
#ifdef TERRA_BUILD_DLL
#define TERRA_API _declspec(dllexport)
#else
#define TERRA_API _declspec(dllimport)
#endif
#else
#error Terra Engine only supports Windows for now!
#endif

#define BIT(x) (1 << x)