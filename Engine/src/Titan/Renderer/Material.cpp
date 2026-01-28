#include "Material.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
#include "RHI/Texture.h"
#include "Titan/PCH.h"
#include "Titan/Project/Project.h"
#include "Titan/Scene/Assets.h"

namespace YAML
{
    template <>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template <>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
} // namespace YAML

namespace Titan
{
    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v);

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v);

    void Material3D::Save()
    {
        if (SourcePath.empty())
        {
            TI_CORE_WARN("Cannot save material without SourcePath");
            return;
        }

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Name" << YAML::Value << Name;
        out << YAML::Key << "AlbedoColor" << YAML::Value << AlbedoColor;

        if (AlbedoTexture)
            out << YAML::Key << "AlbedoTexture" << YAML::Value << AlbedoTexture->GetInternalPath();

        if (EmissionTexture)
            out << YAML::Key << "EmissionTexture" << YAML::Value << EmissionTexture->GetInternalPath();

        if (MetallicTexture)
            out << YAML::Key << "MetallicTexture" << YAML::Value << MetallicTexture->GetInternalPath();

        if (RoughnessTexture)
            out << YAML::Key << "RoughnessTexture" << YAML::Value << RoughnessTexture->GetInternalPath();

        if (NormalTexture)
            out << YAML::Key << "NormalTexture" << YAML::Value << NormalTexture->GetInternalPath();

        if (AOTexture)
            out << YAML::Key << "AOTexture" << YAML::Value << AOTexture->GetInternalPath();

        out << YAML::Key << "UVRepeat" << YAML::Value << UVRepeat;
        out << YAML::EndMap;

        std::ofstream fout(SourcePath);
        if (fout.is_open())
        {
            fout << out.c_str();
            TI_CORE_INFO("Saved material to {}", SourcePath);
        }
        else
        {
            TI_CORE_ERROR("Failed to open file for writing: {}", SourcePath);
        }
    }

    Ref<Material3D> Material3D::Create(const std::string& path)
    {
        if (!std::filesystem::exists(path))
        {
            TI_CORE_WARN("Material file not found: {}", path);
            return nullptr;
        }

        try
        {
            YAML::Node data = YAML::LoadFile(path);
            auto assetDir = Project::GetAssetDirectory();
            auto material = CreateRef<Material3D>();
            material->SourcePath = path;

            if (data["Name"])
                material->Name = data["Name"].as<std::string>();

            if (data["AlbedoColor"])
                material->AlbedoColor = data["AlbedoColor"].as<glm::vec4>();

            if (data["AlbedoTexture"])
            {
                std::string texPath = data["AlbedoTexture"].as<std::string>();
                material->AlbedoTexture = Assets::Load<Texture2D>(assetDir / texPath);
                material->AlbedoTexture->SetInternalPath(texPath);
            }

            if (data["EmissionTexture"])
            {
                std::string texPath = data["EmissionTexture"].as<std::string>();
                material->EmissionTexture = Assets::Load<Texture2D>(assetDir / texPath);
                material->EmissionTexture->SetInternalPath(texPath);
            }
            if (data["MetallicTexture"])
            {
                std::string texPath = data["MetallicTexture"].as<std::string>();
                material->MetallicTexture = Assets::Load<Texture2D>(assetDir / texPath);
                material->MetallicTexture->SetInternalPath(texPath);
            }

            if (data["RoughnessTexture"])
            {
                std::string texPath = data["RoughnessTexture"].as<std::string>();
                material->RoughnessTexture = Assets::Load<Texture2D>(assetDir / texPath);
                material->RoughnessTexture->SetInternalPath(texPath);
            }
            if (data["NormalTexture"])
            {
                std::string texPath = data["NormalTexture"].as<std::string>();
                material->NormalTexture = Assets::Load<Texture2D>(assetDir / texPath);
                material->NormalTexture->SetInternalPath(texPath);
            }

            if (data["AOTexture"])
            {
                std::string texPath = data["AOTexture"].as<std::string>();
                material->AOTexture = Assets::Load<Texture2D>(assetDir / texPath);
                material->AOTexture->SetInternalPath(texPath);
            }

            if (data["UVRepeat"])
                material->UVRepeat = data["UVRepeat"].as<glm::vec2>();

            return material;
        }
        catch (const std::exception& e)
        {
            TI_CORE_ERROR("Failed to load material: {}", e.what());
            return nullptr;
        }
    }
} // namespace Titan
