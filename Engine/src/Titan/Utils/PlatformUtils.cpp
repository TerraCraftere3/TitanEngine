#include "PlatformUtils.h"

std::string Titan::Platform::GetPlatform()
{
    return std::string(TI_PLATFORM_STRING);
}

std::string Titan::Platform::GetArchitecture()
{
    return std::string(TI_ARCH_STRING);
}

std::string Titan::Platform::GetVariant()
{
    return std::string(TI_VARIANT_STRING);
}