#pragma once
#include "Titan/Core/UUID.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/Shader.h"
#include "Titan/Renderer/Texture.h"
#include "Titan/Scene/PhysicsMaterial.h"
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
        Shader,
        Physics2DMaterial
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

        static const AssetMeta* GetMeta(const std::filesystem::path& path)
        {
            auto absPath = std::filesystem::absolute(path);
            auto it = s_AssetMeta.find(absPath.string());
            if (it != s_AssetMeta.end())
                return &it->second;
            return nullptr;
        }

        template <typename T>
        static void Add(const std::filesystem::path& path, const Ref<T>& asset, const AssetMeta& meta)
        {
            auto absPath = std::filesystem::absolute(path).string();
            s_Assets[absPath] = asset;
            s_AssetMeta[absPath] = meta;
        }

        static void AddMeta(const std::filesystem::path& path, const AssetMeta& meta)
        {
            auto absPath = std::filesystem::absolute(path).string();
            s_AssetMeta[absPath] = meta;
        }

        static void Remove(const std::filesystem::path& path)
        {
            auto absPath = std::filesystem::absolute(path).string();
            s_Assets.erase(absPath);
            s_AssetMeta.erase(absPath);
        }

        static bool Exists(const std::filesystem::path& path)
        {
            return s_Assets.contains(std::filesystem::absolute(path).string());
        }

        static bool ExistsMeta(const std::filesystem::path& path)
        {
            return s_AssetMeta.contains(std::filesystem::absolute(path).string());
        }

        static void Clear()
        {
            s_Assets.clear();
            s_AssetMeta.clear();
        }

        static void UpdateMeta(const std::filesystem::path& path, const AssetMeta& meta)
        {
            s_AssetMeta[std::filesystem::absolute(path).string()] = meta;
        }

    private:
        inline static std::unordered_map<std::string, Ref<void>> s_Assets;
        inline static std::unordered_map<std::string, AssetMeta> s_AssetMeta;
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

        static void UpdateMeta(const std::filesystem::path& path, const AssetMeta& meta)
        {
            AssetLibrary::UpdateMeta(path, meta);
        }

        static void SaveMetaToDisk(const std::filesystem::path& path)
        {
            auto absPath = std::filesystem::absolute(path).string();
            if (AssetLibrary::GetMeta(path) != nullptr)
                SaveMeta(*AssetLibrary::GetMeta(path));
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
            else if constexpr (std::is_same_v<T, Shader>)
            {
                meta.Type = AssetType::Shader;
            }
            else if constexpr (std::is_same_v<T, Scene>)
            {
                meta.Type = AssetType::Scene;
            }
            else if constexpr (std::is_same_v<T, Physics2DMaterial>)
            {
                meta.Type = AssetType::Physics2DMaterial;
            }
            else
                static_assert(always_false<T>::value, "Unsupported asset type in Assets::GenerateDefaultMeta");
            return meta;
        }

        template <typename T>
        AssetMeta LoadMetaFromDisk(const std::filesystem::path& assetPath)
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
        AssetMeta LoadMeta(const std::filesystem::path& assetPath)
        {
            if (AssetLibrary::ExistsMeta(assetPath))
                return *AssetLibrary::GetMeta(assetPath);

            auto meta = LoadMetaFromDisk<T>(assetPath);
            AssetLibrary::AddMeta(assetPath, meta);
            return meta;
        }

        template <typename T>
        Ref<T> Load(const std::filesystem::path& path)
        {
            if (AssetLibrary::Exists(path))
            {
                TI_CORE_TRACE("Loading existing Asset {}", path.generic_string());
                return AssetLibrary::Get<T>(path);
            }

            TI_CORE_TRACE("Loading new Asset {}", path.generic_string());

            AssetMeta meta = LoadMeta<T>(path);
            Ref<T> asset = nullptr;

            if constexpr (std::is_same_v<T, Texture2D>)
            {
                asset = Texture2D::Create(std::filesystem::relative(path).string());
            }
            else if constexpr (std::is_same_v<T, Shader>)
            {
                asset = Shader::Create(std::filesystem::relative(path).string());
            }
            else if constexpr (std::is_same_v<T, Scene>)
            {
                asset = CreateRef<Scene>();
                SceneSerializer serializer(asset);
                serializer.Deserialize(std::filesystem::relative(path).string());
            }
            else if constexpr (std::is_same_v<T, Physics2DMaterial>)
            {
                asset = Physics2DMaterial::Create(std::filesystem::relative(path).string());
            }
            else
            {
                static_assert(always_false<T>::value, "Unsupported asset type in Assets::Load<T>");
            }

            if (asset)
                AssetLibrary::Add(path, asset, meta);

            return asset;
        }

        inline void Unload(const std::filesystem::path& path)
        {
            if (AssetLibrary::Exists(path))
                AssetLibrary::Remove(path);
        }

        template <typename T>
        void Reload(const std::filesystem::path& path)
        {
            if (!AssetLibrary::Exists(path))
            {
                Load<T>(path);
                return;
            }

            Ref<T> existing = AssetLibrary::Get<T>(path);
            AssetLibrary::Remove(path);
            Ref<T> reloaded = Load<T>(path);

            if (reloaded)
                *existing = *reloaded;

            AssetLibrary::Add(path, existing);
        }

    } // namespace Assets

} // namespace Titan