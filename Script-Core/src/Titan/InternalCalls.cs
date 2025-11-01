using System;
using System.Runtime.CompilerServices;

namespace Titan
{
    public static class InternalCalls
    {
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

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static ulong Entity_FindEntityByName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

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