#pragma once

#include "Titan/Core.h"
#include "Titan/Core/Log.h"
#include "Titan/Scene/Assets.h"

namespace Titan
{

    class Thumbnails
    {
    public:
        static void Init();
        static void Shutdown();
        static Ref<Texture2D> GetThumbnailForFile(const std::filesystem::path& filePath);
    };

} // namespace Titan