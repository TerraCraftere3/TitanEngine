using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Titan
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    public class TransformComponent : Component
    {
        public Vector3 Translation
        {
            get {
                InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
                return translation;
            }
            set {
                InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get {
                InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 rotation);
                return rotation;
            }
            set {
                InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
            }
        }

        public Vector3 Scale
        {
            get {
                InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
                return scale;
            }
            set {
                InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
            }
        }
    }
    
    // ======================================================================
    //
    //            COLLIDER COMPONENTS
    //
    // ======================================================================

    public class BoxCollider2DComponent : Component{

    }
    
    public class CircleCollider2DComponent : Component{
        
    }

    public class Rigidbody2DComponent : Component
    {
        public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition, wake);
        }

        public void ApplyLinearImpulse(Vector2 impulse, bool wake)
        {
            InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
        }
    }
    
    // ======================================================================
    //
    //            RENDERER COMPONENTS
    //
    // ======================================================================
    public class MeshRendererComponent : Component{
        
    }
    
    public class SpriteRendererComponent : Component{
        
    }
    
    public class CircleRendererComponent : Component{
        
    }

    // ======================================================================
    //
    //            ADDITIONAL RENDERER COMPONENTS
    //
    // ======================================================================
    public class SkyboxComponent : Component{
        
    }

    public class CameraComponent : Component{
        
    }
    
    public class FXAASettings
    {
        private Entity m_Entity;

        internal FXAASettings(Entity entity)
        {
            m_Entity = entity;
        }

        public bool Enabled
        {
            get => InternalCalls.PostFXComponent_GetFXAAEnabled(m_Entity.ID);
            set => InternalCalls.PostFXComponent_SetFXAAEnabled(m_Entity.ID, value);
        }
    }

    public class PostFXComponent : Component
    {
        public FXAASettings FXAA { get; }

        public PostFXComponent()
        {
            FXAA = new FXAASettings(Entity);
        }
    }

    
    // ======================================================================
    //
    //            LIGHT COMPONENTS
    //
    // ======================================================================
    public class DirectionalLightComponent : Component{
        public Vector3 Direction
        {
            get {
                InternalCalls.DirectionalLightComponent_GetDirection(Entity.ID, out Vector3 direction);
                return direction;
            }
            set {
                InternalCalls.DirectionalLightComponent_SetDirection(Entity.ID, ref value);
            }
        }

    }

    // ======================================================================
    //
    //            SCRIPT COMPONENTS
    //
    // ======================================================================

    public class ScriptComponent : Component{
        
    }
    
    public class NativeScriptComponent : Component{
        
    }

    // ======================================================================
    //
    //            CONSTRAINT COMPONENTS
    //
    // ======================================================================
    public class LookAtComponent : Component{
        
    }
}