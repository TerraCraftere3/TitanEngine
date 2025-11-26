#pragma once

#include <entt/entt.hpp>
#include <vector>
#include "PhysicsMaterial.h"
#include "SceneCamera.h"
#include "Titan/Core/UUID.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/Cubemap.h"
#include "Titan/Renderer/Material.h"
#include "Titan/Renderer/Mesh.h"
#include "Titan/Renderer/PostProcessing/TonemappingTypes.h"
#include "Titan/Renderer/Texture.h"

namespace Titan
{
    struct IDComponent
    {
        UUID ID;

        IDComponent() = default;
        IDComponent(UUID uuid) : ID(uuid) {};
        IDComponent(const IDComponent&) = default;
    };

    struct TagComponent
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag) : Tag(tag) {}
    };

    struct TransformComponent
    {
        glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
        glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

        // Cached world transform (computed by Scene when a parent relation exists)
        glm::mat4 WorldTransform = glm::mat4(1.0f);
        // When true, GetTransform() will return WorldTransform instead of local transform
        bool UseWorldTransform = false;

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation) : Translation(translation) {}

        glm::mat4 GetLocalTransform() const
        {
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

            return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
        }

        glm::mat4 GetTransform() const { return UseWorldTransform ? WorldTransform : GetLocalTransform(); }
    };

    // Relationship component to store parent/children hierarchy (entt-style)
    struct RelationshipComponent
    {
        entt::entity Parent = entt::null;
        std::vector<entt::entity> Children;

        RelationshipComponent() = default;
        RelationshipComponent(const RelationshipComponent&) = default;
    };

    struct SpriteRendererComponent
    {
        Ref<Texture2D> Tex;
        glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const Ref<Texture2D> texture, const glm::vec4& color) : Tex(texture), Color(color) {}
    };

    struct CircleRendererComponent
    {
        glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};
        float Thickness = 1.0f;
        float Fade = 0.005f;

        CircleRendererComponent() = default;
        CircleRendererComponent(const CircleRendererComponent&) = default;
    };

    struct MeshRendererComponent
    {
        Ref<Mesh> MeshRef;

        MeshRendererComponent() = default;
        MeshRendererComponent(const MeshRendererComponent&) = default;
        MeshRendererComponent(const Ref<Mesh>& mesh) : MeshRef(mesh) {}
    };

    struct DirectionalLightComponent
    {
        glm::vec3 Direction;

        DirectionalLightComponent() = default;
        DirectionalLightComponent(const DirectionalLightComponent&) = default;
        DirectionalLightComponent(glm::vec3 dir) : Direction(dir) {}
    };

    struct SkyboxComponent
    {
        struct HDRISettings
        {
            Ref<Cubemap> Skybox;
            Ref<Cubemap> Irradiance;
        } hdriSettings;
        struct ColorrampSettings
        {
            glm::vec3 TopColor = glm::vec3(0.1f, 0.3f, 0.7f);
            glm::vec3 BottomColor = glm::vec3(0.9f, 0.9f, 1.0f);
        } colorrampSettings;
        enum Mode
        {
            HDRI = 0,
            Colorramp = 1,
            _COUNT
        } mode = HDRI;

        SkyboxComponent() = default;
        SkyboxComponent(const SkyboxComponent&) = default;
    };

    struct CameraComponent
    {
        SceneCamera Camera;
        bool Primary = true;
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    struct PostFXComponent
    {
        struct FXAA
        {
            bool isEnabled = false;
        };

        struct Tonemapping
        {
            bool isEnabled = true;
            TonemappingOperator Operator = TonemappingOperator::Filmic;
            float Exposure = 1.0f;
            float Gamma = 2.2f;
            float WhitePoint = 11.2f; // Used by Uncharted2
        };

        FXAA FXAASettings;
        Tonemapping TonemappingSettings;

        PostFXComponent() = default;
        PostFXComponent(const PostFXComponent&) = default;
    };

    struct RigidbodyComponent
    {
        enum class BodyType
        {
            Static = 0,
            Dynamic,
            Kinematic
        };
        BodyType Type = BodyType::Static;
        bool FixedRotation = false;
        void* RuntimeBody = nullptr;

        RigidbodyComponent() = default;
        RigidbodyComponent(const RigidbodyComponent&) = default;
    };

    struct CubeColliderComponent
    {
        glm::vec3 Offset = {0.0f, 0.0f, 0.0f};
        glm::vec3 Size = {0.5f, 0.5f, 0.5f};

        Ref<PhysicsMaterial> Material;

        CubeColliderComponent() { Material = CreateRef<PhysicsMaterial>(); };
        CubeColliderComponent(const CubeColliderComponent&) = default;
    };

    struct SphereColliderComponent
    {
        glm::vec3 Offset = {0.0f, 0.0f, 0.0f};
        float Radius = 0.5f;

        Ref<PhysicsMaterial> Material;

        SphereColliderComponent() { Material = CreateRef<PhysicsMaterial>(); };
        SphereColliderComponent(const SphereColliderComponent&) = default;
    };

    struct Rigidbody2DComponent
    {
        enum class BodyType
        {
            Static = 0,
            Dynamic,
            Kinematic
        };
        BodyType Type = BodyType::Static;
        bool FixedRotation = false;

        void* RuntimeBody = nullptr;

        Rigidbody2DComponent() = default;
        Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
    };

    struct BoxCollider2DComponent
    {
        glm::vec2 Offset = {0.0f, 0.0f};
        glm::vec2 Size = {0.5f, 0.5f};

        Ref<Physics2DMaterial> Material;

        void* RuntimeFixture = nullptr;

        BoxCollider2DComponent() { Material = CreateRef<Physics2DMaterial>(); };
        BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
    };

    struct CircleCollider2DComponent
    {
        glm::vec2 Offset = {0.0f, 0.0f};
        float Radius = 0.5f;

        Ref<Physics2DMaterial> Material;

        void* RuntimeFixture = nullptr;

        CircleCollider2DComponent() { Material = CreateRef<Physics2DMaterial>(); };
        CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
    };

    struct ScriptComponent
    {
        std::string ClassName;

        ScriptComponent() = default;
        ScriptComponent(const ScriptComponent&) = default;
    };

    struct LookAtComponent
    {
        glm::vec3 Position;

        LookAtComponent() = default;
        LookAtComponent(const LookAtComponent&) = default;
    };

    template <typename... Component>
    struct ComponentGroup
    {
    };

    using AllComponents =
        ComponentGroup<TransformComponent, SpriteRendererComponent, CircleRendererComponent, MeshRendererComponent,
                       DirectionalLightComponent, SkyboxComponent, CameraComponent, PostFXComponent, ScriptComponent,
                       RigidbodyComponent, CubeColliderComponent, SphereColliderComponent, Rigidbody2DComponent,
                       BoxCollider2DComponent, CircleCollider2DComponent, LookAtComponent, RelationshipComponent>;
    namespace Utils
    {
        inline const char* SkyboxModeToString(SkyboxComponent::Mode mode)
        {
            switch (mode)
            {
                case SkyboxComponent::Mode::HDRI:
                    return "HDRI";
                case SkyboxComponent::Mode::Colorramp:
                    return "Colorramp";
            }
            return "Colorramp";
        }

        inline SkyboxComponent::Mode StringToSkyboxMode(const std::string& str)
        {
            std::string s = str;
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });

            if (s == "hdri")
                return SkyboxComponent::Mode::HDRI;

            if (s == "colorramp")
                return SkyboxComponent::Mode::Colorramp;

            return SkyboxComponent::Mode::Colorramp;
        }

    } // namespace Utils
} // namespace Titan