#include "Thumbnails.h"

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

    AssetData* g_ThumbnailData = nullptr;

    void Thumbnails::Init()
    {
        g_ThumbnailData = new AssetData();

        g_ThumbnailData->DirectoryIcon = Assets::Load<Texture2D>("resources/icons/folder.svg");
        g_ThumbnailData->DirectoryOpenIcon = Assets::Load<Texture2D>("resources/icons/folder-opened.svg");
        g_ThumbnailData->FileTextIcon = Assets::Load<Texture2D>("resources/icons/file.svg");
        g_ThumbnailData->FileCodeIcon = Assets::Load<Texture2D>("resources/icons/file-code.svg");
        g_ThumbnailData->FileImageIcon = Assets::Load<Texture2D>("resources/icons/file-media.svg");
        g_ThumbnailData->FileMaterialIcon = Assets::Load<Texture2D>("resources/icons/file-material.svg");
    }

    void Thumbnails::Shutdown()
    {
        delete g_ThumbnailData;
        g_ThumbnailData = nullptr;
    }

    Ref<Texture2D> Thumbnails::GetThumbnailForFile(const std::filesystem::path& filePath)
    {
        if (std::filesystem::is_directory(filePath))
        {
            if (!std::filesystem::is_empty(filePath))
                return g_ThumbnailData->DirectoryOpenIcon;
            else
                return g_ThumbnailData->DirectoryIcon;
        }

        std::string ext = filePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
            return Assets::LoadAsync<Texture2D>(std::filesystem::relative(filePath));

        if (ext == ".vert" || ext == ".frag" || ext == ".vs" || ext == ".fs" || ext == ".shader" || ext == ".glsl" ||
            ext == ".hlsl" || ext == ".slang")
            return g_ThumbnailData->FileCodeIcon;
        if (ext == ".cs")
            return g_ThumbnailData->FileCodeIcon;

        if (ext == ".phys2d" || ext == ".phys")
            return g_ThumbnailData->FileMaterialIcon;

        return g_ThumbnailData->FileTextIcon;
    }
} // namespace Titan