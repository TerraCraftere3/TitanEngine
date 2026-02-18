using System;
using System.Runtime.CompilerServices;

namespace Titan
{
    public static class InternalCalls
    {
        // ======================================================================
        //            LOGGING CALLS
        // ======================================================================
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void InternalLogTrace(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void InternalLogInfo(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void InternalLogWarn(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void InternalLogError(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void InternalLogCritical(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void InternalClientLogTrace(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void InternalClientLogInfo(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void InternalClientLogWarn(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void InternalClientLogError(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void InternalClientLogCritical(string message);

        // ======================================================================
        //            ENTITY CALLS
        // ======================================================================
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern bool Entity_HasComponent(ulong entityID, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern ulong Entity_FindEntityByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern object GetScriptInstance(ulong entityID);

        // ======================================================================
        //            COMPONENT CALLS
        // ======================================================================
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetRotation(ulong entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetScale(ulong entityID, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

        // Relationship component
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern ulong RelationshipComponent_GetParent(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern uint RelationshipComponent_GetChildrenCount(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern ulong RelationshipComponent_GetChildByIndex(ulong entityID, uint index);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void RelationshipComponent_SetParent(ulong entityID, ulong parentID);

        // Rigidbody2D component
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern int Rigidbody2DComponent_GetType(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void Rigidbody2DComponent_SetType(ulong entityID, int type);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern bool Rigidbody2DComponent_GetFixedRotation(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void Rigidbody2DComponent_SetFixedRotation(ulong entityID, bool value);

        // Rigidbody component (3D)
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern int RigidbodyComponent_GetType(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetType(ulong entityID, int type);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern bool RigidbodyComponent_GetFixedRotation(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void RigidbodyComponent_SetFixedRotation(ulong entityID, bool value);

        // Cube collider component (3D)
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CubeColliderComponent_GetOffset(ulong entityID, out Vector3 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CubeColliderComponent_SetOffset(ulong entityID, ref Vector3 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CubeColliderComponent_GetSize(ulong entityID, out Vector3 size);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CubeColliderComponent_SetSize(ulong entityID, ref Vector3 size);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CubeColliderComponent_SetMaterialPath(ulong entityID, string path);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern string CubeColliderComponent_GetMaterialPath(ulong entityID);

        // Sphere collider component (3D)
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SphereColliderComponent_GetOffset(ulong entityID, out Vector3 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SphereColliderComponent_SetOffset(ulong entityID, ref Vector3 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern float SphereColliderComponent_GetRadius(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SphereColliderComponent_SetRadius(ulong entityID, float radius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SphereColliderComponent_SetMaterialPath(ulong entityID, string path);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern string SphereColliderComponent_GetMaterialPath(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void DirectionalLightComponent_GetDirection(ulong entityID, out Vector3 direction);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void DirectionalLightComponent_SetDirection(ulong entityID, ref Vector3 direction);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern bool PostFXComponent_GetFXAAEnabled(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void PostFXComponent_SetFXAAEnabled(ulong entityID, bool enabled);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse,
                                                                            ref Vector2 point, bool wake);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse,
                                                                                    bool wake);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyDown(KeyCode keycode);

        // Sprite/Mesh renderer
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SpriteRendererComponent_SetTexture(ulong entityID, string path);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern string SpriteRendererComponent_GetTexture(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void MeshRendererComponent_SetMesh(ulong entityID, string path);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern string MeshRendererComponent_GetMesh(ulong entityID);

        // Circle renderer
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CircleRendererComponent_GetColor(ulong entityID, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CircleRendererComponent_SetColor(ulong entityID, ref Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern float CircleRendererComponent_GetThickness(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CircleRendererComponent_SetThickness(ulong entityID, float thickness);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern float CircleRendererComponent_GetFade(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CircleRendererComponent_SetFade(ulong entityID, float fade);

        // Skybox
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SkyboxComponent_SetSkyboxPaths(ulong entityID, string skyboxPath,
                                                                   string irradiancePath);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern string SkyboxComponent_GetSkyboxPath(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern string SkyboxComponent_GetIrradiancePath(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SkyboxComponent_SetMode(ulong entityID, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern int SkyboxComponent_GetMode(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SkyboxComponent_SetTopBottomColor(ulong entityID, Vector3 top, Vector3 bottom);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SkyboxComponent_GetTopColor(ulong entityID, out Vector3 top);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SkyboxComponent_GetBottomColor(ulong entityID, out Vector3 bottom);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SkyboxComponent_SetTopColor(ulong entityID, Vector3 top);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SkyboxComponent_SetBottomColor(ulong entityID, Vector3 bottom);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void SkyboxComponent_SetTimeOfDay(ulong entityID, float timeOfDay);

        // Camera
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern bool CameraComponent_GetPrimary(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CameraComponent_SetPrimary(ulong entityID, bool primary);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern bool CameraComponent_GetFixedAspectRatio(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CameraComponent_SetFixedAspectRatio(ulong entityID, bool value);

        // Box collider
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void BoxCollider2DComponent_GetOffset(ulong entityID, out Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void BoxCollider2DComponent_SetOffset(ulong entityID, ref Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void BoxCollider2DComponent_GetSize(ulong entityID, out Vector2 size);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void BoxCollider2DComponent_SetSize(ulong entityID, ref Vector2 size);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void BoxCollider2DComponent_SetMaterialPath(ulong entityID, string path);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern string BoxCollider2DComponent_GetMaterialPath(ulong entityID);

        // Circle collider
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CircleCollider2DComponent_GetOffset(ulong entityID, out Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CircleCollider2DComponent_SetOffset(ulong entityID, ref Vector2 offset);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern float CircleCollider2DComponent_GetRadius(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CircleCollider2DComponent_SetRadius(ulong entityID, float radius);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern void CircleCollider2DComponent_SetMaterialPath(ulong entityID, string path);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal static extern string CircleCollider2DComponent_GetMaterialPath(ulong entityID);
    }
}