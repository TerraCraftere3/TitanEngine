#pragma once

#include <string>

#include "Titan/Core.h"

namespace Titan
{

    class TI_API Cubemap
    {
    public:
        virtual ~Cubemap() = default;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual std::string GetPath() const = 0;

        virtual void SetData(void* data, uint32_t size) = 0;
        virtual void SetFaceData(uint32_t face, void* data, uint32_t size) = 0;

        virtual void* GetNativeTexture() const = 0;

        virtual Ref<Cubemap> CreateIrradianceMap(uint32_t resolution = 32) = 0;

        static Ref<Cubemap> Create(const std::string& path);
    };

} // namespace Titan