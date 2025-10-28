#pragma once
#include "Titan/Core/UUID.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/Texture.h"
#include "Titan/Scene/Scene.h"

#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <unordered_map>

namespace Titan
{

    enum class AssetType
    {
        None = 0,
        Texture2D,
        Scene,
    };

    struct AssetMeta
    {
        UUID ID;
        AssetType Type = AssetType::None;
        std::filesystem::path SourcePath;
        std::unordered_map<std::string, std::string> Properties;
    };

    class AssetLibrary
    {
    public:
        template <typename T>
        static Ref<T> Get(const std::filesystem::path& path)
        {
            auto absPath = std::filesystem::absolute(path);
            auto it = s_Assets.find(absPath.string());
            if (it != s_Assets.end())
                return std::static_pointer_cast<T>(it->second);
            return nullptr;
        }

        template <typename T>
        static void Add(const std::filesystem::path& path, const Ref<T>& asset)
        {
            s_Assets[std::filesystem::absolute(path).string()] = asset;
        }

        static void Remove(const std::filesystem::path& path)
        {
            s_Assets.erase(std::filesystem::absolute(path).string());
        }

        static bool Exists(const std::filesystem::path& path)
        {
            return s_Assets.contains(std::filesystem::absolute(path).string());
        }

        static void Clear() { s_Assets.clear(); }

    private:
        inline static std::unordered_map<std::string, Ref<void>> s_Assets;
    };

    namespace Assets
    {
        template <class T>
        struct always_false : std::false_type
        {
        };

        inline void SaveMeta(const AssetMeta& meta)
        {
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "ID" << YAML::Value << meta.ID;
            out << YAML::Key << "Type" << YAML::Value << (int)meta.Type;
            out << YAML::Key << "Source" << YAML::Value << meta.SourcePath.string();

            if (!meta.Properties.empty())
            {
                out << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
                for (auto& [k, v] : meta.Properties)
                    out << YAML::Key << k << YAML::Value << v;
                out << YAML::EndMap;
            }

            out << YAML::EndMap;

            std::ofstream fout(meta.SourcePath.string() + ".meta");
            fout << out.c_str();
        }

        template <typename T>
        AssetMeta GenerateDefaultMeta(const std::filesystem::path& assetPath)
        {
            AssetMeta meta;
            meta.ID = UUID();
            meta.SourcePath = assetPath;
            if constexpr (std::is_same_v<T, Texture2D>)
            {
                meta.Type = AssetType::Texture2D;
                meta.Properties["WrapS"] = "Repeat";
                meta.Properties["WrapT"] = "Repeat";
                meta.Properties["MinFilter"] = "Linear";
                meta.Properties["MagFilter"] = "Linear";
            }
            else if constexpr (std::is_same_v<T, Scene>)
            {
                meta.Type = AssetType::Scene;
            }
            else
                static_assert(always_false<T>::value, "Unsupported asset type in Assets::GenerateDefaultMeta");
            return meta;
        }

        template <typename T>
        AssetMeta LoadMeta(const std::filesystem::path& assetPath)
        {
            auto metaPath = assetPath;
            metaPath += ".meta";

            AssetMeta meta;
            if (!std::filesystem::exists(metaPath))
            {
                meta = GenerateDefaultMeta<T>(assetPath);
                SaveMeta(meta);
                return meta;
            }

            YAML::Node data = YAML::LoadFile(metaPath.string());
            meta.ID = data["ID"].as<uint64_t>();
            meta.Type = (AssetType)data["Type"].as<int>();
            meta.SourcePath = data["Source"].as<std::string>();

            if (auto props = data["Properties"])
            {
                for (auto it : props)
                    meta.Properties[it.first.as<std::string>()] = it.second.as<std::string>();
            }

            return meta;
        }

        template <typename T>
        Ref<T> Load(const std::filesystem::path& path)
        {
            if (AssetLibrary::Exists(path))
            {
                TI_CORE_TRACE("Loading existing Asset {}", path.string());
                return AssetLibrary::Get<T>(path);
            }

            TI_CORE_TRACE("Loading new Asset {}", path.string());

            AssetMeta meta = LoadMeta<T>(path);
            Ref<T> asset = nullptr;

            if constexpr (std::is_same_v<T, Texture2D>)
            {
                asset = Texture2D::Create(path.string());
            }
            else if constexpr (std::is_same_v<T, Scene>)
            {
                asset = CreateRef<Scene>();
                SceneSerializer serializer(asset);
                serializer.Deserialize(path.string());
            }
            else
            {
                static_assert(always_false<T>::value, "Unsupported asset type in Assets::Load<T>");
            }

            if (asset)
                AssetLibrary::Add(path, asset);

            return asset;
        }

        inline void Unload(const std::filesystem::path& path)
        {
            if (AssetLibrary::Exists(path))
                AssetLibrary::Remove(path);
        }
    } // namespace Assets

} // namespace Titan