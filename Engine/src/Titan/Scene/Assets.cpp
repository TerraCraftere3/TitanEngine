#include "Assets.h"

namespace Titan
{
    struct AssetData
    {
        Ref<Texture2D> DirectoryIcon;
        Ref<Texture2D> DirectoryOpenIcon;
        Ref<Texture2D> FileTextIcon;
        Ref<Texture2D> FileCodeIcon;
        Ref<Texture2D> FileImageIcon;
        Ref<Texture2D> FileMaterialIcon;
    };

    AssetData* g_AssetData = nullptr;

    void Assets::Init()
    {
        g_AssetData = new AssetData();

        g_AssetData->DirectoryIcon = Load<Texture2D>("resources/icons/folder.svg");
        g_AssetData->DirectoryOpenIcon = Load<Texture2D>("resources/icons/folder-opened.svg");
        g_AssetData->FileTextIcon = Load<Texture2D>("resources/icons/file.svg");
        g_AssetData->FileCodeIcon = Load<Texture2D>("resources/icons/file-code.svg");
        g_AssetData->FileImageIcon = Load<Texture2D>("resources/icons/file-media.svg");
        g_AssetData->FileMaterialIcon = Load<Texture2D>("resources/icons/file-material.svg");
    }

    void Assets::Shutdown()
    {
        delete g_AssetData;
        g_AssetData = nullptr;
    }

    Ref<Texture2D> Assets::GetThumbnailForFile(const std::filesystem::path& filePath)
    {
        if (std::filesystem::is_directory(filePath))
        {
            if (!std::filesystem::is_empty(filePath))
                return g_AssetData->DirectoryOpenIcon;
            else
                return g_AssetData->DirectoryIcon;
        }

        std::string ext = filePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
            return Load<Texture2D>(std::filesystem::relative(filePath).string());

        if (ext == ".vert" || ext == ".frag" || ext == ".vs" || ext == ".fs" || ext == ".shader" || ext == ".glsl" ||
            ext == ".hlsl" || ext == ".slang")
            return g_AssetData->FileCodeIcon;
        if (ext == ".cs")
            return g_AssetData->FileCodeIcon;

        if (ext == ".phys2d" || ext == ".phys")
            return g_AssetData->FileMaterialIcon;

        return g_AssetData->FileTextIcon;
    }

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