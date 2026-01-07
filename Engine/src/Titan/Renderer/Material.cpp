#include "Material.h"
#include "Texture.h"
#include "Titan/Scene/Assets.h"
#include "Titan/PCH.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

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
}

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
            out << YAML::Key << "AlbedoTexture" << YAML::Value << AlbedoTexture->GetPath();

        if (EmissionTexture)
            out << YAML::Key << "EmissionTexture" << YAML::Value << EmissionTexture->GetPath();

        if (MetallicTexture)
            out << YAML::Key << "MetallicTexture" << YAML::Value << MetallicTexture->GetPath();

        if (RoughnessTexture)
            out << YAML::Key << "RoughnessTexture" << YAML::Value << RoughnessTexture->GetPath();

        if (NormalTexture)
            out << YAML::Key << "NormalTexture" << YAML::Value << NormalTexture->GetPath();

        if (AOTexture)
            out << YAML::Key << "AOTexture" << YAML::Value << AOTexture->GetPath();

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
            auto material = CreateRef<Material3D>();
            material->SourcePath = path;

            if (data["Name"])
                material->Name = data["Name"].as<std::string>();

            if (data["AlbedoColor"])
                material->AlbedoColor = data["AlbedoColor"].as<glm::vec4>();

            if (data["AlbedoTexture"])
                material->AlbedoTexture = Assets::Load<Texture2D>(data["AlbedoTexture"].as<std::string>());

            if (data["EmissionTexture"])
                material->EmissionTexture = Assets::Load<Texture2D>(data["EmissionTexture"].as<std::string>());

            if (data["MetallicTexture"])
                material->MetallicTexture = Assets::Load<Texture2D>(data["MetallicTexture"].as<std::string>());

            if (data["RoughnessTexture"])
                material->RoughnessTexture = Assets::Load<Texture2D>(data["RoughnessTexture"].as<std::string>());

            if (data["NormalTexture"])
                material->NormalTexture = Assets::Load<Texture2D>(data["NormalTexture"].as<std::string>());

            if (data["AOTexture"])
                material->AOTexture = Assets::Load<Texture2D>(data["AOTexture"].as<std::string>());

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
