#pragma once

#include "PhysicsMaterial.h"
#include "SceneCamera.h"
#include "ScriptableEntity.h"
#include "Titan/Core/UUID.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/Mesh.h"
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

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation) : Translation(translation) {}

        glm::mat4 GetTransform() const
        {
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

            return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
        }
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

    struct CameraComponent
    {
        SceneCamera Camera;
        bool Primary = true;
        bool FixedAspectRatio = false;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;
    };

    struct NativeScriptComponent
    {
        ScriptableEntity* Instance = nullptr;

        ScriptableEntity* (*InstantiateScript)();
        void (*DestroyScript)(NativeScriptComponent*);

        template <typename T>
        void Bind()
        {
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc)
            {
                delete nsc->Instance;
                nsc->Instance = nullptr;
            };
        }
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

    template <typename... Component>
    struct ComponentGroup
    {
    };

    using AllComponents = ComponentGroup<TransformComponent, SpriteRendererComponent, CircleRendererComponent,
                                         MeshRendererComponent, CameraComponent, ScriptComponent, NativeScriptComponent,
                                         Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent>;
} // namespace Titan