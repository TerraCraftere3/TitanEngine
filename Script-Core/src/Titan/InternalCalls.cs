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
        internal extern static void InternalLogTrace(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InternalLogInfo(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InternalLogWarn(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InternalLogError(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InternalLogCritical(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InternalClientLogTrace(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InternalClientLogInfo(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InternalClientLogWarn(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InternalClientLogError(string message);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void InternalClientLogCritical(string message);

        // ======================================================================
        //            ENTITY CALLS
        // ======================================================================
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_FindEntityByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong entityID);

        // ======================================================================
        //            COMPONENT CALLS
        // ======================================================================
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetRotation(ulong entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetScale(ulong entityID, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DirectionalLightComponent_GetDirection(ulong entityID, out Vector3 direction);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DirectionalLightComponent_SetDirection(ulong entityID, ref Vector3 direction);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool PostFXComponent_GetFXAAEnabled(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void PostFXComponent_SetFXAAEnabled(ulong entityID, bool enabled);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse,
                                                                            ref Vector2 point, bool wake);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse,
                                                                                    bool wake);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(KeyCode keycode);
    }
}