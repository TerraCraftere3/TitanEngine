#include "Assets.h"

namespace Titan
{

    void Assets::Init() {}

    void Assets::Shutdown() {}

    AssetType Assets::GetTypeForFile(const std::filesystem::path& filePath)
    {
        if (std::filesystem::is_directory(filePath))
        {
            return AssetType::None;
        }

        std::string ext = filePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
            return AssetType::Texture2D;

        if (ext == ".vert" || ext == ".frag" || ext == ".vs" || ext == ".fs" || ext == ".shader" || ext == ".glsl" ||
            ext == ".hlsl")
            return AssetType::Shader;

        if (ext == ".titan")
            return AssetType::Scene;

        return AssetType::None;
    }

    std::string Assets::AssetTypeToString(AssetType type)
    {
        switch (type)
        {
            case AssetType::Texture2D:
                return "Texture";
            case AssetType::Cubemap:
                return "Cubemap";
            case AssetType::Scene:
                return "Scene";
            case AssetType::Shader:
                return "Shader";
            case AssetType::Physics2DMaterial:
                return "Physics 2D Material";
            case AssetType::PhysicsMaterial:
                return "Physics Material";
            case AssetType::Mesh:
                return "Mesh";
            case AssetType::None:
            default:
                return "Unknown";
        }
    }
} // namespace Titan