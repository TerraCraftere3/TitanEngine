#include "ScriptGlue.h"
#include "ScriptEngine.h"
#include "Titan/Core/Input.h"
#include "Titan/Core/KeyCodes.h"
#include "Titan/Core/UUID.h"
#include "Titan/PCH.h"
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

        TI_ADD_INTERNAL_CALL(Entity_HasComponent);
        TI_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
        TI_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

        TI_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
        TI_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);

        TI_ADD_INTERNAL_CALL(Input_IsKeyDown);
    }

} // namespace Titan