#include "SceneSerializer.h"
#include <yaml-cpp/yaml.h>
#include "Assets.h"
#include "Components.h"
#include "Entity.h"
#include "Titan/Core/UUID.h"
#include "Titan/PCH.h"
#include "Titan/Project/Project.h"
#include "Titan/Renderer/Systems/Renderer2D.h"
#include "Titan/Scripting/ScriptEngine.h"

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
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
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

    template <>
    struct convert<Titan::UUID>
    {
        static Node encode(const Titan::UUID& uuid)
        {
            Node node;
            node.push_back((uint64_t)uuid);
            return node;
        }

        static bool decode(const Node& node, Titan::UUID& uuid)
        {
            uuid = node.as<uint64_t>();
            return true;
        }
    };

} // namespace YAML
namespace Titan
{
#define WRITE_SCRIPT_FIELD(FieldType, Type)  \
    case ScriptFieldType::FieldType:         \
        out << scriptField.GetValue<Type>(); \
        break

#define READ_SCRIPT_FIELD(FieldType, Type)          \
    case ScriptFieldType::FieldType:                \
    {                                               \
        Type data = scriptField["Data"].as<Type>(); \
        fieldInstance.SetValue(data);               \
        break;                                      \
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    SceneSerializer::SceneSerializer(const Ref<Scene>& scene) : m_Scene(scene) {}

    static std::string RigidBody2DBodyTypeToString(Rigidbody2DComponent::BodyType bodyType)
    {
        switch (bodyType)
        {
            case Rigidbody2DComponent::BodyType::Static:
                return "Static";
            case Rigidbody2DComponent::BodyType::Dynamic:
                return "Dynamic";
            case Rigidbody2DComponent::BodyType::Kinematic:
                return "Kinematic";
        }

        TI_CORE_ASSERT(false, "Unknown body type");
        return {};
    }

    static Rigidbody2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
    {
        if (bodyTypeString == "Static")
            return Rigidbody2DComponent::BodyType::Static;
        if (bodyTypeString == "Dynamic")
            return Rigidbody2DComponent::BodyType::Dynamic;
        if (bodyTypeString == "Kinematic")
            return Rigidbody2DComponent::BodyType::Kinematic;

        TI_CORE_ASSERT(false, "Unknown body type");
        return Rigidbody2DComponent::BodyType::Static;
    }

    static std::string RigidBodyBodyTypeToString(RigidbodyComponent::BodyType bodyType)
    {
        switch (bodyType)
        {
            case RigidbodyComponent::BodyType::Static:
                return "Static";
            case RigidbodyComponent::BodyType::Dynamic:
                return "Dynamic";
            case RigidbodyComponent::BodyType::Kinematic:
                return "Kinematic";
        }

        TI_CORE_ASSERT(false, "Unknown body type");
        return {};
    }

    static RigidbodyComponent::BodyType RigidBodyBodyTypeFromString(const std::string& bodyTypeString)
    {
        if (bodyTypeString == "Static")
            return RigidbodyComponent::BodyType::Static;
        if (bodyTypeString == "Dynamic")
            return RigidbodyComponent::BodyType::Dynamic;
        if (bodyTypeString == "Kinematic")
            return RigidbodyComponent::BodyType::Kinematic;

        TI_CORE_ASSERT(false, "Unknown body type");
        return RigidbodyComponent::BodyType::Static;
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        TI_CORE_ASSERT(entity.HasComponent<IDComponent>());

        out << YAML::BeginMap; // Entity
        out << YAML::Key << "Entity" << YAML::Value << (size_t)entity.GetUUID();

        if (entity.HasComponent<TagComponent>())
        {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap; // TagComponent

            auto& tag = entity.GetComponent<TagComponent>().Tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;

            out << YAML::EndMap; // TagComponent
        }

        if (entity.HasComponent<TransformComponent>())
        {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap; // TransformComponent

            auto& tc = entity.GetComponent<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
            out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

            out << YAML::EndMap; // TransformComponent
        }

        if (entity.HasComponent<CameraComponent>())
        {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap; // CameraComponent

            auto& cameraComponent = entity.GetComponent<CameraComponent>();
            auto& camera = cameraComponent.Camera;

            out << YAML::Key << "Camera" << YAML::Value;
            out << YAML::BeginMap; // Camera
            out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
            out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
            out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
            out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
            out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
            out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
            out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
            out << YAML::EndMap; // Camera

            out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
            out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

            out << YAML::EndMap; // CameraComponent
        }

        if (entity.HasComponent<PostFXComponent>())
        {
            out << YAML::Key << "PostFXComponent";
            out << YAML::BeginMap; // PostFXComponent

            auto& fxc = entity.GetComponent<PostFXComponent>();
            {
                out << YAML::Key << "FXAA";
                out << YAML::BeginMap; // FXAA
                out << YAML::Key << "Enabled" << YAML::Value << fxc.FXAASettings.isEnabled;
                out << YAML::EndMap; // FXAA
            }

            out << YAML::EndMap; // PostFXComponent
        }

        if (entity.HasComponent<SpriteRendererComponent>())
        {
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap; // SpriteRendererComponent

            auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
            if (spriteRendererComponent.Tex)
                out << YAML::Key << "Texture" << YAML::Value << spriteRendererComponent.Tex->GetInternalPath();

            out << YAML::EndMap; // SpriteRendererComponent
        }

        if (entity.HasComponent<CircleRendererComponent>())
        {
            out << YAML::Key << "CircleRendererComponent";
            out << YAML::BeginMap; // CircleRendererComponent

            auto& circleRendererComponent = entity.GetComponent<CircleRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.Color;
            out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.Thickness;
            out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.Fade;

            out << YAML::EndMap; // CircleRendererComponent
        }

        if (entity.HasComponent<MeshRendererComponent>())
        {
            out << YAML::Key << "MeshRendererComponent";
            out << YAML::BeginMap; // MeshRendererComponent

            auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();
            if (meshRendererComponent.MeshRef)
                out << YAML::Key << "Mesh" << YAML::Value << meshRendererComponent.MeshRef->GetInternalPath();

            out << YAML::Key << "Materials";
            out << YAML::BeginSeq; // Materials
            int matIndex = 0;
            for (auto mat : meshRendererComponent.MeshRef->GetMaterials())
            {
                // Generate a material file path based on mesh path and material index
                auto meshPath = std::filesystem::path(meshRendererComponent.MeshRef->GetInternalPath());
                auto materialDir = meshPath.parent_path() / "Materials";
                auto materialFileName = meshPath.stem().string() + "_Mat" + std::to_string(matIndex) + ".mat";
                auto materialFilePath = materialDir / materialFileName;

                // Save the material to disk
                std::filesystem::create_directories(materialDir);
                mat->Save();

                // Save just the path in the scene file
                out << mat->GetInternalPath();
                matIndex++;
            }
            out << YAML::EndSeq; // Materials
            out << YAML::EndMap; // MeshRendererComponent
        }

        if (entity.HasComponent<DirectionalLightComponent>())
        {
            out << YAML::Key << "DirectionalLightComponent";
            out << YAML::BeginMap; // DirectionalLightComponent

            auto& dlc = entity.GetComponent<DirectionalLightComponent>();
            out << YAML::Key << "Direction" << YAML::Value << dlc.Direction;

            out << YAML::EndMap; // DirectionalLightComponent
        }

        if (entity.HasComponent<SkyboxComponent>())
        {
            out << YAML::Key << "SkyboxComponent";
            out << YAML::BeginMap; // SkyboxComponent

            auto& sc = entity.GetComponent<SkyboxComponent>();
            out << YAML::Key << "HDRI" << YAML::BeginMap;
            {
                if (sc.hdriSettings.Skybox)
                    out << YAML::Key << "Texture" << YAML::Value << sc.hdriSettings.Skybox->GetInternalPath();
            }
            out << YAML::EndMap;
            out << YAML::Key << "Colorramp" << YAML::BeginMap;
            {
                out << YAML::Key << "TopColor" << YAML::Value << sc.colorrampSettings.TopColor;
                out << YAML::Key << "BottomColor" << YAML::Value << sc.colorrampSettings.BottomColor;
            }
            out << YAML::EndMap;
            out << YAML::Key << "Mode" << YAML::Value << Utils::SkyboxModeToString(sc.mode);

            out << YAML::EndMap; // SkyboxComponent
        }

        if (entity.HasComponent<RigidbodyComponent>())
        {
            out << YAML::Key << "RigidbodyComponent";
            out << YAML::BeginMap; // RigidbodyComponent

            auto& rbComponent = entity.GetComponent<RigidbodyComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << RigidBodyBodyTypeToString(rbComponent.Type);
            out << YAML::Key << "FixedRotation" << YAML::Value << rbComponent.FixedRotation;

            out << YAML::EndMap; // RigidbodyComponent
        }

        if (entity.HasComponent<CubeColliderComponent>())
        {
            out << YAML::Key << "CubeColliderComponent";
            out << YAML::BeginMap; // CubeColliderComponent

            auto& ccComponent = entity.GetComponent<CubeColliderComponent>();
            out << YAML::Key << "Offset" << YAML::Value << ccComponent.Offset;
            out << YAML::Key << "Size" << YAML::Value << ccComponent.Size;
            if (ccComponent.Material)
                out << YAML::Key << "Material" << YAML::Value << ccComponent.Material->GetInternalPath();

            out << YAML::EndMap; // CubeColliderComponent
        }

        if (entity.HasComponent<SphereColliderComponent>())
        {
            out << YAML::Key << "SphereColliderComponent";
            out << YAML::BeginMap; // SphereColliderComponent

            auto& scComponent = entity.GetComponent<SphereColliderComponent>();
            out << YAML::Key << "Offset" << YAML::Value << scComponent.Offset;
            out << YAML::Key << "Radius" << YAML::Value << scComponent.Radius;
            if (scComponent.Material)
                out << YAML::Key << "Material" << YAML::Value << scComponent.Material->GetInternalPath();

            out << YAML::EndMap; // SphereColliderComponent
        }

        if (entity.HasComponent<Rigidbody2DComponent>())
        {
            out << YAML::Key << "Rigidbody2DComponent";
            out << YAML::BeginMap; // Rigidbody2DComponent

            auto& rb2dComponent = entity.GetComponent<Rigidbody2DComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
            out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

            out << YAML::EndMap; // Rigidbody2DComponent
        }

        if (entity.HasComponent<BoxCollider2DComponent>())
        {
            out << YAML::Key << "BoxCollider2DComponent";
            out << YAML::BeginMap; // BoxCollider2DComponent

            auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
            out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
            out << YAML::Key << "Material" << YAML::Value << bc2dComponent.Material->GetInternalPath();

            out << YAML::EndMap; // BoxCollider2DComponent
        }

        if (entity.HasComponent<CircleCollider2DComponent>())
        {
            out << YAML::Key << "CircleCollider2DComponent";
            out << YAML::BeginMap; // CircleCollider2DComponent

            auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
            out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
            out << YAML::Key << "Material" << YAML::Value << cc2dComponent.Material->GetInternalPath();

            out << YAML::EndMap; // CircleCollider2DComponent
        }

        if (entity.HasComponent<AudioSourceComponent>())
        {
            out << YAML::Key << "AudioSourceComponent";
            out << YAML::BeginMap; // AudioSourceComponent

            auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();
            if (audioSourceComponent.Sound)
                out << YAML::Key << "Sound" << YAML::Value
                    << audioSourceComponent.Sound->GetBuffer()->GetInternalPath();

            out << YAML::Key << "Volume" << YAML::Value << audioSourceComponent.Volume;
            out << YAML::Key << "Pitch" << YAML::Value << audioSourceComponent.Pitch;
            out << YAML::Key << "Looping" << YAML::Value << audioSourceComponent.Looping;

            out << YAML::EndMap; // AudioSourceComponent
        }

        if (entity.HasComponent<AudioListenerComponent>())
        {
            out << YAML::Key << "AudioListenerComponent";
            out << YAML::BeginMap; // AudioListenerComponent

            out << YAML::EndMap; // AudioListenerComponent
        }

        if (entity.HasComponent<ScriptComponent>())
        {
            auto& scriptComponent = entity.GetComponent<ScriptComponent>();

            out << YAML::Key << "ScriptComponent";
            out << YAML::BeginMap; // ScriptComponent
            out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;
            Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
            const auto& fields = entityClass->GetFields();
            if (fields.size() > 0)
            {
                out << YAML::Key << "ScriptFields" << YAML::Value;
                auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
                out << YAML::BeginSeq;
                for (const auto& [name, field] : fields)
                {
                    if (entityFields.find(name) == entityFields.end())
                        continue;

                    out << YAML::BeginMap; // ScriptField
                    out << YAML::Key << "Name" << YAML::Value << name;
                    out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);

                    out << YAML::Key << "Data" << YAML::Value;
                    ScriptFieldInstance& scriptField = entityFields.at(name);

                    switch (field.Type)
                    {
                        WRITE_SCRIPT_FIELD(Float, float);
                        WRITE_SCRIPT_FIELD(Double, double);
                        WRITE_SCRIPT_FIELD(Bool, bool);
                        WRITE_SCRIPT_FIELD(Char, char);
                        WRITE_SCRIPT_FIELD(Byte, int8_t);
                        WRITE_SCRIPT_FIELD(Short, int16_t);
                        WRITE_SCRIPT_FIELD(Int, int32_t);
                        WRITE_SCRIPT_FIELD(Long, int64_t);
                        WRITE_SCRIPT_FIELD(UByte, uint8_t);
                        WRITE_SCRIPT_FIELD(UShort, uint16_t);
                        WRITE_SCRIPT_FIELD(UInt, uint32_t);
                        WRITE_SCRIPT_FIELD(ULong, uint64_t);
                        WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
                        WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
                        WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
                        WRITE_SCRIPT_FIELD(Entity, UUID);
                    }
                    out << YAML::EndMap; // ScriptFields
                }
                out << YAML::EndSeq;
            }
            out << YAML::EndMap; // ScriptComponent
        }

        if (entity.HasComponent<RelationshipComponent>())
        {
            auto parent = entity.GetParent();
            if (parent)
            {
                out << YAML::Key << "RelationshipComponent";
                out << YAML::BeginMap; // RelationshipComponent
                out << YAML::Key << "Parent" << YAML::Value << (size_t)parent.GetUUID();
                out << YAML::EndMap; // RelationshipComponent
            }
        }

        out << YAML::EndMap; // Entity
    }

    void SceneSerializer::Serialize(const std::string& filepath)
    {
        TI_CORE_ASSERT(!filepath.empty(), "SceneSerializer::Serialize - Filepath is empty!");
        TI_CORE_ASSERT(m_Scene, "SceneSerializer::Serialize - Scene is null!");
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        auto view = m_Scene->m_Registry.view<entt::entity>();
        for (auto it = view.rbegin(); it != view.rend(); ++it)
        {
            entt::entity entityID = *it;
            Entity entity = {entityID, m_Scene.get()};
            if (!entity)
                continue;

            SerializeEntity(out, entity);
        };
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        TI_CORE_ASSERT(fout.is_open(), "SceneSerializer::Serialize - Failed to open file: {}", filepath);
        fout << out.c_str();
        TI_CORE_INFO("Saved scene to {}", filepath);
    }

    void SceneSerializer::SerializeRuntime(const std::string& filepath)
    {
        // Not implemented
        TI_CORE_ASSERT(false);
    }

    bool SceneSerializer::Deserialize(const std::string& filepath)
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filepath);
        }
        catch (YAML::ParserException e)
        {
            return false;
        }

        std::string sceneName = data["Scene"].as<std::string>();

        auto entities = data["Entities"];
        std::vector<std::pair<UUID, UUID>> pendingParentLinks;
        if (entities)
        {
            for (auto entity : entities)
            {
                uint64_t uuid = entity["Entity"].as<uint64_t>();

                std::string name;
                auto tagComponent = entity["TagComponent"];
                if (tagComponent)
                    name = tagComponent["Tag"].as<std::string>();

                Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

                auto transformComponent = entity["TransformComponent"];
                if (transformComponent)
                {
                    // Entities always have transforms
                    auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                    tc.Translation = transformComponent["Translation"].as<glm::vec3>();
                    tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    tc.Scale = transformComponent["Scale"].as<glm::vec3>();
                }

                auto cameraComponent = entity["CameraComponent"];
                if (cameraComponent)
                {
                    auto& cc = deserializedEntity.AddComponent<CameraComponent>();

                    auto cameraProps = cameraComponent["Camera"];
                    cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

                    cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
                    cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
                    cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

                    cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
                    cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
                    cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

                    cc.Primary = cameraComponent["Primary"].as<bool>();
                    cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
                }

                auto postFXComponent = entity["PostFXComponent"];
                if (postFXComponent)
                {
                    auto& fxc = deserializedEntity.AddComponent<PostFXComponent>();

                    auto fxaaNode = postFXComponent["FXAA"];
                    if (fxaaNode)
                    {
                        fxc.FXAASettings.isEnabled = fxaaNode["Enabled"].as<bool>();
                    }
                }

                auto spriteRendererComponent = entity["SpriteRendererComponent"];
                if (spriteRendererComponent)
                {
                    auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
                    src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
                    if (spriteRendererComponent["Texture"])
                    {
                        src.Tex = Assets::Load<Texture2D>(Project::GetAssetDirectory() /
                                                          spriteRendererComponent["Texture"].as<std::string>());
                        src.Tex->SetInternalPath(spriteRendererComponent["Texture"].as<std::string>());
                    }
                }

                auto meshRendererComponent = entity["MeshRendererComponent"];
                if (meshRendererComponent)
                {
                    auto& mrc = deserializedEntity.AddComponent<MeshRendererComponent>();
                    if (meshRendererComponent["Mesh"])
                    {
                        std::string path = meshRendererComponent["Mesh"].as<std::string>();
                        if (path == "quad")
                            mrc.MeshRef = Mesh::CreateQuad();
                        else if (path == "cube")
                            mrc.MeshRef = Mesh::CreateCube();
                        else
                            mrc.MeshRef = Assets::Load<Mesh>(Project::GetAssetDirectory() / path);
                        mrc.MeshRef->SetInternalPath(path);
                    }
                    auto materials = meshRendererComponent["Materials"];
                    int matIndex = 0;
                    for (auto materialPath : materials)
                    {
                        auto mat = mrc.MeshRef->GetMaterial(matIndex);
                        if (mat)
                        {
                            std::string matPathStr =
                                (Project::GetAssetDirectory() / materialPath.as<std::string>()).string();
                            if (!matPathStr.empty() && std::filesystem::exists(matPathStr))
                            {
                                // Load material from .mat file
                                auto loadedMat = Material3D::Create(matPathStr);
                                if (loadedMat)
                                {
                                    mat->Name = loadedMat->Name;
                                    mat->AlbedoColor = loadedMat->AlbedoColor;
                                    mat->AlbedoTexture = loadedMat->AlbedoTexture;
                                    mat->EmissionTexture = loadedMat->EmissionTexture;
                                    mat->MetallicTexture = loadedMat->MetallicTexture;
                                    mat->RoughnessTexture = loadedMat->RoughnessTexture;
                                    mat->NormalTexture = loadedMat->NormalTexture;
                                    mat->AOTexture = loadedMat->AOTexture;
                                    mat->UVRepeat = loadedMat->UVRepeat;
                                    mat->SourcePath = matPathStr; // Set the source path for saving
                                    mat->SetInternalPath(materialPath.as<std::string>());
                                }
                            }
                        }
                        matIndex++;
                    }
                }

                auto directionalLightComponent = entity["DirectionalLightComponent"];
                if (directionalLightComponent)
                {
                    auto& dlc = deserializedEntity.AddComponent<DirectionalLightComponent>();
                    dlc.Direction = directionalLightComponent["Direction"].as<glm::vec3>();
                }

                auto skyboxComponent = entity["SkyboxComponent"];
                if (skyboxComponent)
                {
                    auto& sc = deserializedEntity.AddComponent<SkyboxComponent>();

                    auto hdriNode = skyboxComponent["HDRI"];
                    if (hdriNode)
                    {
                        if (hdriNode["Texture"])
                        {
                            std::string path =
                                (Project::GetAssetDirectory() / hdriNode["Texture"].as<std::string>()).string();
                            sc.hdriSettings.Skybox = Assets::Load<Cubemap>(path);
                            sc.hdriSettings.Skybox->SetInternalPath(hdriNode["Texture"].as<std::string>());

                            if (sc.hdriSettings.Skybox)
                                sc.hdriSettings.Irradiance = sc.hdriSettings.Skybox->CreateIrradianceMap();
                        }
                    }
                    auto crNode = skyboxComponent["Colorramp"];
                    if (crNode)
                    {
                        if (crNode["TopColor"])
                            sc.colorrampSettings.TopColor = crNode["TopColor"].as<glm::vec3>();

                        if (crNode["BottomColor"])
                            sc.colorrampSettings.BottomColor = crNode["BottomColor"].as<glm::vec3>();
                    }

                    if (skyboxComponent["Mode"])
                    {
                        std::string modeStr = skyboxComponent["Mode"].as<std::string>();
                        sc.mode = Utils::StringToSkyboxMode(modeStr);
                    }
                }

                auto rigidbodyComponent = entity["RigidbodyComponent"];
                if (rigidbodyComponent)
                {
                    auto& rb = deserializedEntity.AddComponent<RigidbodyComponent>();
                    rb.Type = RigidBodyBodyTypeFromString(rigidbodyComponent["BodyType"].as<std::string>());
                    rb.FixedRotation = rigidbodyComponent["FixedRotation"].as<bool>();
                }

                auto cubeColliderComponent = entity["CubeColliderComponent"];
                if (cubeColliderComponent)
                {
                    auto& cube = deserializedEntity.AddComponent<CubeColliderComponent>();
                    cube.Offset = cubeColliderComponent["Offset"].as<glm::vec3>();
                    cube.Size = cubeColliderComponent["Size"].as<glm::vec3>();
                    if (cubeColliderComponent["Material"])
                        cube.Material = Assets::Load<PhysicsMaterial>(
                            Project::GetAssetDirectory() / cubeColliderComponent["Material"].as<std::string>());
                    cube.Material->SetInternalPath(cubeColliderComponent["Material"].as<std::string>());
                }

                auto sphereColliderComponent = entity["SphereColliderComponent"];
                if (sphereColliderComponent)
                {
                    auto& sphere = deserializedEntity.AddComponent<SphereColliderComponent>();
                    sphere.Offset = sphereColliderComponent["Offset"].as<glm::vec3>();
                    sphere.Radius = sphereColliderComponent["Radius"].as<float>();
                    if (sphereColliderComponent["Material"])
                        sphere.Material = Assets::Load<PhysicsMaterial>(
                            Project::GetAssetDirectory() / sphereColliderComponent["Material"].as<std::string>());
                    sphere.Material->SetInternalPath(sphereColliderComponent["Material"].as<std::string>());
                }

                auto circleRendererComponent = entity["CircleRendererComponent"];
                if (circleRendererComponent)
                {
                    auto& crc = deserializedEntity.AddComponent<CircleRendererComponent>();
                    crc.Color = circleRendererComponent["Color"].as<glm::vec4>();
                    crc.Thickness = circleRendererComponent["Thickness"].as<float>();
                    crc.Fade = circleRendererComponent["Fade"].as<float>();
                }

                auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
                if (rigidbody2DComponent)
                {
                    auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
                    rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
                    rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
                }

                auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
                if (boxCollider2DComponent)
                {
                    auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
                    bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
                    bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
                    bc2d.Material = Assets::Load<Physics2DMaterial>(
                        Project::GetAssetDirectory() / boxCollider2DComponent["Material"].as<std::string>());
                    bc2d.Material->SetInternalPath(boxCollider2DComponent["Material"].as<std::string>());
                }

                auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
                if (circleCollider2DComponent)
                {
                    auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
                    cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
                    cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
                    cc2d.Material = Assets::Load<Physics2DMaterial>(
                        Project::GetAssetDirectory() / circleCollider2DComponent["Material"].as<std::string>());
                    cc2d.Material->SetInternalPath(circleCollider2DComponent["Material"].as<std::string>());
                }

                auto audioSourceComponent = entity["AudioSourceComponent"];
                if (audioSourceComponent)
                {
                    auto& asc = deserializedEntity.AddComponent<AudioSourceComponent>();
                    if (audioSourceComponent["Sound"])
                    {
                        asc.Sound = Assets::Load<AudioSource>(Project::GetAssetDirectory() /
                                                              audioSourceComponent["Sound"].as<std::string>());
                        asc.Sound->GetBuffer()->SetInternalPath(audioSourceComponent["Sound"].as<std::string>());
                    }
                    if (audioSourceComponent["Volume"])
                        asc.Volume = audioSourceComponent["Volume"].as<float>();

                    if (audioSourceComponent["Pitch"])
                        asc.Pitch = audioSourceComponent["Pitch"].as<float>();

                    if (audioSourceComponent["Looping"])
                        asc.Looping = audioSourceComponent["Looping"].as<bool>();
                }

                auto audioListenerComponent = entity["AudioListenerComponent"];
                if (audioListenerComponent)
                {
                    deserializedEntity.AddComponent<AudioListenerComponent>();
                }

                auto scriptComponent = entity["ScriptComponent"];
                if (scriptComponent)
                {
                    auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
                    sc.ClassName = scriptComponent["ClassName"].as<std::string>();

                    auto scriptFields = scriptComponent["ScriptFields"];
                    if (scriptFields)
                    {
                        Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
                        TI_CORE_ASSERT(entityClass);
                        const auto& fields = entityClass->GetFields();
                        auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

                        for (auto scriptField : scriptFields)
                        {
                            std::string name = scriptField["Name"].as<std::string>();
                            std::string typeString = scriptField["Type"].as<std::string>();
                            ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

                            ScriptFieldInstance& fieldInstance = entityFields[name];

                            TI_CORE_ASSERT(fields.find(name) != fields.end());

                            if (fields.find(name) == fields.end())
                                continue;

                            fieldInstance.Field = fields.at(name);

                            switch (type)
                            {
                                READ_SCRIPT_FIELD(Float, float);
                                READ_SCRIPT_FIELD(Double, double);
                                READ_SCRIPT_FIELD(Bool, bool);
                                READ_SCRIPT_FIELD(Char, char);
                                READ_SCRIPT_FIELD(Byte, int8_t);
                                READ_SCRIPT_FIELD(Short, int16_t);
                                READ_SCRIPT_FIELD(Int, int32_t);
                                READ_SCRIPT_FIELD(Long, int64_t);
                                READ_SCRIPT_FIELD(UByte, uint8_t);
                                READ_SCRIPT_FIELD(UShort, uint16_t);
                                READ_SCRIPT_FIELD(UInt, uint32_t);
                                READ_SCRIPT_FIELD(ULong, uint64_t);
                                READ_SCRIPT_FIELD(Vector2, glm::vec2);
                                READ_SCRIPT_FIELD(Vector3, glm::vec3);
                                READ_SCRIPT_FIELD(Vector4, glm::vec4);
                                READ_SCRIPT_FIELD(Entity, UUID);
                            }
                        }
                    }
                }

                // Record relationship parent if present; resolution happens after all entities are created
                auto relationshipComponent = entity["RelationshipComponent"];
                if (relationshipComponent)
                {
                    uint64_t parentUUID = relationshipComponent["Parent"].as<uint64_t>();
                    if (parentUUID != 0)
                        pendingParentLinks.emplace_back(UUID(uuid), UUID(parentUUID));
                }
            }
        }

        // Resolve parent links
        for (auto& pr : pendingParentLinks)
        {
            Entity child = m_Scene->GetEntityByUUID(pr.first);
            Entity parent = m_Scene->GetEntityByUUID(pr.second);
            if (child && parent)
                m_Scene->SetParent(child, parent);
        }

        return true;
    }

    bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
    {
        // Not implemented
        TI_CORE_ASSERT(false);
        return false;
    }

} // namespace Titan