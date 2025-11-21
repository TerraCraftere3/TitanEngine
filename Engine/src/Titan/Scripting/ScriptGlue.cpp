#include "ScriptGlue.h"
#include "ScriptEngine.h"
#include "Titan/Core/Input.h"
#include "Titan/Core/KeyCodes.h"
#include "Titan/Core/UUID.h"
#include "Titan/PCH.h"
#include "Titan/Scene/Assets.h"
#include "Titan/Scene/Components.h"
#include "Titan/Scene/Entity.h"
#include "Titan/Scene/Scene.h"
#include "box2d/b2_body.h"
#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

namespace Titan
{

    static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

#define TI_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Titan.InternalCalls::" #Name, Name)

    static void InternalLogTrace(MonoString* string)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        TI_CORE_TRACE(str);
    }

    static void InternalLogInfo(MonoString* string)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        TI_CORE_INFO(str);
    }

    static void InternalLogWarn(MonoString* string)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        TI_CORE_WARN(str);
    }

    static void InternalLogError(MonoString* string)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        TI_CORE_ERROR(str);
    }

    static void InternalLogCritical(MonoString* string)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        TI_CORE_CRITICAL(str);
    }

    static void InternalClientLogTrace(MonoString* string)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        TI_TRACE(str);
    }

    static void InternalClientLogInfo(MonoString* string)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        TI_INFO(str);
    }

    static void InternalClientLogWarn(MonoString* string)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        TI_WARN(str);
    }

    static void InternalClientLogError(MonoString* string)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        TI_ERROR(str);
    }

    static void InternalClientLogCritical(MonoString* string)
    {
        char* cStr = mono_string_to_utf8(string);
        std::string str(cStr);
        mono_free(cStr);
        TI_CRITICAL(str);
    }

    static MonoObject* GetScriptInstance(UUID entityID)
    {
        return ScriptEngine::GetManagedInstance(entityID);
    }

    static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        MonoType* managedType = mono_reflection_type_get_type(componentType);
        TI_CORE_ASSERT(s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end());
        return s_EntityHasComponentFuncs.at(managedType)(entity);
    }

    static uint64_t Entity_FindEntityByName(MonoString* name)
    {
        char* nameCStr = mono_string_to_utf8(name);

        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->FindEntityByName(nameCStr);
        mono_free(nameCStr);

        if (!entity)
            return 0;

        return entity.GetUUID();
    }

    static void DirectionalLightComponent_GetDirection(UUID entityID, glm::vec3* outDirection)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        *outDirection = entity.GetComponent<DirectionalLightComponent>().Direction;
    }

    static void DirectionalLightComponent_SetDirection(UUID entityID, glm::vec3* direction)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<DirectionalLightComponent>().Direction = *direction;
    }

    // ----------------------- SpriteRenderer -----------------------
    static void SpriteRendererComponent_SetTexture(UUID entityID, MonoString* path)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        char* cStr = mono_string_to_utf8(path);
        std::string p(cStr);
        mono_free(cStr);

        auto& comp = entity.GetComponent<SpriteRendererComponent>();
        comp.Tex = Titan::Assets::Load<Texture2D>(p);
    }

    static MonoString* SpriteRendererComponent_GetTexture(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        auto& comp = entity.GetComponent<SpriteRendererComponent>();
        std::string p = "[internal]";
        if (comp.Tex)
            p = comp.Tex->GetPath();
        return mono_string_new(ScriptEngine::GetMonoDomain(), p.c_str());
    }

    // ----------------------- MeshRenderer -----------------------
    static void MeshRendererComponent_SetMesh(UUID entityID, MonoString* path)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        char* cStr = mono_string_to_utf8(path);
        std::string p(cStr);
        mono_free(cStr);

        auto& comp = entity.GetComponent<MeshRendererComponent>();
        comp.MeshRef = Titan::Assets::Load<Mesh>(p);
    }

    static MonoString* MeshRendererComponent_GetMesh(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        auto& comp = entity.GetComponent<MeshRendererComponent>();
        std::string p = "[internal]";
        if (comp.MeshRef)
            p = comp.MeshRef->GetFilePath();
        return mono_string_new(ScriptEngine::GetMonoDomain(), p.c_str());
    }

    // ----------------------- CircleRenderer -----------------------
    static void CircleRendererComponent_GetColor(UUID entityID, glm::vec4* outColor)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        *outColor = entity.GetComponent<CircleRendererComponent>().Color;
    }

    static void CircleRendererComponent_SetColor(UUID entityID, glm::vec4* color)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<CircleRendererComponent>().Color = *color;
    }

    static float CircleRendererComponent_GetThickness(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        return entity.GetComponent<CircleRendererComponent>().Thickness;
    }

    static void CircleRendererComponent_SetThickness(UUID entityID, float thickness)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<CircleRendererComponent>().Thickness = thickness;
    }

    static float CircleRendererComponent_GetFade(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        return entity.GetComponent<CircleRendererComponent>().Fade;
    }

    static void CircleRendererComponent_SetFade(UUID entityID, float fade)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<CircleRendererComponent>().Fade = fade;
    }

    // ----------------------- Skybox -----------------------
    static void SkyboxComponent_SetSkyboxPaths(UUID entityID, MonoString* skyboxPath, MonoString* irradiancePath)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        char* a = mono_string_to_utf8(skyboxPath);
        std::string sky(a);
        mono_free(a);

        char* b = mono_string_to_utf8(irradiancePath);
        std::string irr(b);
        mono_free(b);

        auto& comp = entity.GetComponent<SkyboxComponent>();
        comp.hdriSettings.Skybox = Titan::Assets::Load<Cubemap>(sky);
        comp.hdriSettings.Irradiance = Titan::Assets::Load<Cubemap>(irr);
    }

    static MonoString* SkyboxComponent_GetSkyboxPath(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        auto& comp = entity.GetComponent<SkyboxComponent>();
        std::string p = "[internal]";
        if (comp.hdriSettings.Skybox)
            p = comp.hdriSettings.Skybox->GetPath();
        return mono_string_new(ScriptEngine::GetMonoDomain(), p.c_str());
    }

    static MonoString* SkyboxComponent_GetIrradiancePath(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        auto& comp = entity.GetComponent<SkyboxComponent>();
        std::string p = "[internal]";
        if (comp.hdriSettings.Irradiance)
            p = comp.hdriSettings.Irradiance->GetPath();
        return mono_string_new(ScriptEngine::GetMonoDomain(), p.c_str());
    }

    static void SkyboxComponent_SetMode(UUID entityID, int mode)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<SkyboxComponent>().mode = (SkyboxComponent::Mode)mode;
    }

    static int SkyboxComponent_GetMode(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        return (int)entity.GetComponent<SkyboxComponent>().mode;
    }

    static void SkyboxComponent_SetTopBottomColor(UUID entityID, glm::vec3 top, glm::vec3 bottom)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        auto& comp = entity.GetComponent<SkyboxComponent>();
        comp.colorrampSettings.TopColor = top;
        comp.colorrampSettings.BottomColor = bottom;
    }

    static void SkyboxComponent_GetTopColor(UUID entityID, glm::vec3* outTop)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        *outTop = entity.GetComponent<SkyboxComponent>().colorrampSettings.TopColor;
    }

    static void SkyboxComponent_GetBottomColor(UUID entityID, glm::vec3* outBottom)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        *outBottom = entity.GetComponent<SkyboxComponent>().colorrampSettings.BottomColor;
    }

    static void SkyboxComponent_SetTopColor(UUID entityID, glm::vec3 top)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<SkyboxComponent>().colorrampSettings.TopColor = top;
    }

    static void SkyboxComponent_SetBottomColor(UUID entityID, glm::vec3 bottom)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<SkyboxComponent>().colorrampSettings.BottomColor = bottom;
    }

    // ----------------------- Camera -----------------------
    static bool CameraComponent_GetPrimary(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        return entity.GetComponent<CameraComponent>().Primary;
    }

    static void CameraComponent_SetPrimary(UUID entityID, bool primary)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<CameraComponent>().Primary = primary;
    }

    static bool CameraComponent_GetFixedAspectRatio(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        return entity.GetComponent<CameraComponent>().FixedAspectRatio;
    }

    static void CameraComponent_SetFixedAspectRatio(UUID entityID, bool value)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<CameraComponent>().FixedAspectRatio = value;
    }

    // ----------------------- BoxCollider -----------------------
    static void BoxCollider2DComponent_GetOffset(UUID entityID, glm::vec2* outOffset)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        *outOffset = entity.GetComponent<BoxCollider2DComponent>().Offset;
    }

    static void BoxCollider2DComponent_SetOffset(UUID entityID, glm::vec2* offset)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<BoxCollider2DComponent>().Offset = *offset;
    }

    static void BoxCollider2DComponent_GetSize(UUID entityID, glm::vec2* outSize)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        *outSize = entity.GetComponent<BoxCollider2DComponent>().Size;
    }

    static void BoxCollider2DComponent_SetSize(UUID entityID, glm::vec2* size)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<BoxCollider2DComponent>().Size = *size;
    }

    static void BoxCollider2DComponent_SetMaterialPath(UUID entityID, MonoString* path)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        char* s = mono_string_to_utf8(path);
        std::string p(s);
        mono_free(s);

        auto& comp = entity.GetComponent<BoxCollider2DComponent>();
        comp.Material = Titan::Assets::Load<Physics2DMaterial>(p);
    }

    static MonoString* BoxCollider2DComponent_GetMaterialPath(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        auto& comp = entity.GetComponent<BoxCollider2DComponent>();
        std::string p = "[internal]";
        if (comp.Material)
            p = comp.Material->SourcePath;
        return mono_string_new(ScriptEngine::GetMonoDomain(), p.c_str());
    }

    // ----------------------- CircleCollider -----------------------
    static void CircleCollider2DComponent_GetOffset(UUID entityID, glm::vec2* outOffset)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        *outOffset = entity.GetComponent<CircleCollider2DComponent>().Offset;
    }

    static void CircleCollider2DComponent_SetOffset(UUID entityID, glm::vec2* offset)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<CircleCollider2DComponent>().Offset = *offset;
    }

    static float CircleCollider2DComponent_GetRadius(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        return entity.GetComponent<CircleCollider2DComponent>().Radius;
    }

    static void CircleCollider2DComponent_SetRadius(UUID entityID, float radius)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<CircleCollider2DComponent>().Radius = radius;
    }

    static void CircleCollider2DComponent_SetMaterialPath(UUID entityID, MonoString* path)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        char* s = mono_string_to_utf8(path);
        std::string p(s);
        mono_free(s);

        auto& comp = entity.GetComponent<CircleCollider2DComponent>();
        comp.Material = Titan::Assets::Load<Physics2DMaterial>(p);
    }

    static MonoString* CircleCollider2DComponent_GetMaterialPath(UUID entityID)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        auto& comp = entity.GetComponent<CircleCollider2DComponent>();
        std::string p = "[internal]";
        if (comp.Material)
            p = comp.Material->SourcePath;
        return mono_string_new(ScriptEngine::GetMonoDomain(), p.c_str());
    }

    static void TransformComponent_GetTranslation(UUID entityID, glm::vec3* outTranslation)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        *outTranslation = entity.GetComponent<TransformComponent>().Translation;
    }

    static void TransformComponent_SetTranslation(UUID entityID, glm::vec3* translation)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<TransformComponent>().Translation = *translation;
    }

    static void TransformComponent_GetRotation(UUID entityID, glm::vec3* outRotation)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        *outRotation = entity.GetComponent<TransformComponent>().Rotation;
    }

    static void TransformComponent_SetRotation(UUID entityID, glm::vec3* rotation)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<TransformComponent>().Rotation = *rotation;
    }

    static void TransformComponent_GetScale(UUID entityID, glm::vec3* outScale)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        *outScale = entity.GetComponent<TransformComponent>().Scale;
    }

    static void TransformComponent_SetScale(UUID entityID, glm::vec3* scale)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        entity.GetComponent<TransformComponent>().Scale = *scale;
    }

    static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, glm::vec2* impulse, glm::vec2* point, bool wake)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
    }

    static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, glm::vec2* impulse, bool wake)
    {
        Scene* scene = ScriptEngine::GetSceneContext();
        TI_CORE_ASSERT(scene);
        Entity entity = scene->GetEntityByUUID(entityID);
        TI_CORE_ASSERT(entity);

        auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
        b2Body* body = (b2Body*)rb2d.RuntimeBody;
        body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
    }

    static bool Input_IsKeyDown(KeyCode keycode)
    {
        return Input::IsKeyPressed(keycode);
    }

    template <typename... Component>
    static void RegisterComponent()
    {
        (
            []()
            {
                std::string_view typeName = typeid(Component).name();
                size_t pos = typeName.find_last_of(':');
                std::string_view structName = typeName.substr(pos + 1);
                std::string managedTypename = fmt::format("Titan.{}", structName);

                MonoType* managedType =
                    mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
                if (!managedType)
                {
                    TI_CORE_ERROR("Could not find component type {}", managedTypename);
                    return;
                }
                s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
            }(),
            ...);
    }

    template <typename... Component>
    static void RegisterComponent(ComponentGroup<Component...>)
    {
        RegisterComponent<Component...>();
    }

    void ScriptGlue::RegisterComponents()
    {
        s_EntityHasComponentFuncs.clear();
        RegisterComponent(AllComponents{});
    }

    void ScriptGlue::RegisterFunctions()
    {
        TI_ADD_INTERNAL_CALL(InternalLogTrace);
        TI_ADD_INTERNAL_CALL(InternalLogInfo);
        TI_ADD_INTERNAL_CALL(InternalLogWarn);
        TI_ADD_INTERNAL_CALL(InternalLogError);
        TI_ADD_INTERNAL_CALL(InternalLogCritical);

        TI_ADD_INTERNAL_CALL(InternalClientLogTrace);
        TI_ADD_INTERNAL_CALL(InternalClientLogInfo);
        TI_ADD_INTERNAL_CALL(InternalClientLogWarn);
        TI_ADD_INTERNAL_CALL(InternalClientLogError);
        TI_ADD_INTERNAL_CALL(InternalClientLogCritical);

        TI_ADD_INTERNAL_CALL(GetScriptInstance);

        TI_ADD_INTERNAL_CALL(Entity_HasComponent);
        TI_ADD_INTERNAL_CALL(Entity_FindEntityByName);

        TI_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
        TI_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
        TI_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
        TI_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
        TI_ADD_INTERNAL_CALL(TransformComponent_GetScale);
        TI_ADD_INTERNAL_CALL(TransformComponent_SetScale);

        TI_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
        TI_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);

        TI_ADD_INTERNAL_CALL(DirectionalLightComponent_GetDirection);
        TI_ADD_INTERNAL_CALL(DirectionalLightComponent_SetDirection);

        TI_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTexture);
        TI_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTexture);

        TI_ADD_INTERNAL_CALL(MeshRendererComponent_SetMesh);
        TI_ADD_INTERNAL_CALL(MeshRendererComponent_GetMesh);

        TI_ADD_INTERNAL_CALL(CircleRendererComponent_GetColor);
        TI_ADD_INTERNAL_CALL(CircleRendererComponent_SetColor);
        TI_ADD_INTERNAL_CALL(CircleRendererComponent_GetThickness);
        TI_ADD_INTERNAL_CALL(CircleRendererComponent_SetThickness);
        TI_ADD_INTERNAL_CALL(CircleRendererComponent_GetFade);
        TI_ADD_INTERNAL_CALL(CircleRendererComponent_SetFade);

        TI_ADD_INTERNAL_CALL(SkyboxComponent_SetSkyboxPaths);
        TI_ADD_INTERNAL_CALL(SkyboxComponent_GetSkyboxPath);
        TI_ADD_INTERNAL_CALL(SkyboxComponent_GetIrradiancePath);
        TI_ADD_INTERNAL_CALL(SkyboxComponent_SetMode);
        TI_ADD_INTERNAL_CALL(SkyboxComponent_GetMode);
        TI_ADD_INTERNAL_CALL(SkyboxComponent_SetTopBottomColor);
        TI_ADD_INTERNAL_CALL(SkyboxComponent_SetTopColor);
        TI_ADD_INTERNAL_CALL(SkyboxComponent_SetBottomColor);
        TI_ADD_INTERNAL_CALL(SkyboxComponent_GetTopColor);
        TI_ADD_INTERNAL_CALL(SkyboxComponent_GetBottomColor);

        TI_ADD_INTERNAL_CALL(CameraComponent_GetPrimary);
        TI_ADD_INTERNAL_CALL(CameraComponent_SetPrimary);
        TI_ADD_INTERNAL_CALL(CameraComponent_GetFixedAspectRatio);
        TI_ADD_INTERNAL_CALL(CameraComponent_SetFixedAspectRatio);

        TI_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetOffset);
        TI_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetOffset);
        TI_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetSize);
        TI_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetSize);
        TI_ADD_INTERNAL_CALL(BoxCollider2DComponent_SetMaterialPath);
        TI_ADD_INTERNAL_CALL(BoxCollider2DComponent_GetMaterialPath);

        TI_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetOffset);
        TI_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetOffset);
        TI_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetRadius);
        TI_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetRadius);
        TI_ADD_INTERNAL_CALL(CircleCollider2DComponent_SetMaterialPath);
        TI_ADD_INTERNAL_CALL(CircleCollider2DComponent_GetMaterialPath);

        TI_ADD_INTERNAL_CALL(Input_IsKeyDown);
    }

} // namespace Titan