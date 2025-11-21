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

    public class BoxCollider2DComponent : Component
    {
        public Vector2 Offset
        {
            get { InternalCalls.BoxCollider2DComponent_GetOffset(Entity.ID, out Vector2 offset); return offset; }
            set { InternalCalls.BoxCollider2DComponent_SetOffset(Entity.ID, ref value); }
        }

        public Vector2 Size
        {
            get { InternalCalls.BoxCollider2DComponent_GetSize(Entity.ID, out Vector2 size); return size; }
            set { InternalCalls.BoxCollider2DComponent_SetSize(Entity.ID, ref value); }
        }

        public string MaterialPath
        {
            get { return InternalCalls.BoxCollider2DComponent_GetMaterialPath(Entity.ID); }
            set { InternalCalls.BoxCollider2DComponent_SetMaterialPath(Entity.ID, value); }
        }
    }

    public class CircleCollider2DComponent : Component
    {
        public Vector2 Offset
        {
            get { InternalCalls.CircleCollider2DComponent_GetOffset(Entity.ID, out Vector2 offset); return offset; }
            set { InternalCalls.CircleCollider2DComponent_SetOffset(Entity.ID, ref value); }
        }

        public float Radius
        {
            get { return InternalCalls.CircleCollider2DComponent_GetRadius(Entity.ID); }
            set { InternalCalls.CircleCollider2DComponent_SetRadius(Entity.ID, value); }
        }

        public string MaterialPath
        {
            get { return InternalCalls.CircleCollider2DComponent_GetMaterialPath(Entity.ID); }
            set { InternalCalls.CircleCollider2DComponent_SetMaterialPath(Entity.ID, value); }
        }
    }

    public class SpriteRendererComponent : Component
    {
        public Vector4 Color { get; set; } = new Vector4(1f, 1f, 1f, 1f);

        public void SetTexture(string path)
        {
            InternalCalls.SpriteRendererComponent_SetTexture(Entity.ID, path);
        }

        public string GetTexture()
        {
            return InternalCalls.SpriteRendererComponent_GetTexture(Entity.ID);
        }
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
        // Path to mesh asset on disk (string until asset interfaces exist)
        public void SetMesh(string path)
        {
            InternalCalls.MeshRendererComponent_SetMesh(Entity.ID, path);
        }

        public string GetMesh()
        {
            return InternalCalls.MeshRendererComponent_GetMesh(Entity.ID);
        }
        
    }
    
    // Note: the managed SpriteRendererComponent is implemented above
    
    public class CircleRendererComponent : Component{
        public Vector4 Color
        {
            get { InternalCalls.CircleRendererComponent_GetColor(Entity.ID, out Vector4 c); return c; }
            set { InternalCalls.CircleRendererComponent_SetColor(Entity.ID, ref value); }
        }

        public float Thickness
        {
            get { return InternalCalls.CircleRendererComponent_GetThickness(Entity.ID); }
            set { InternalCalls.CircleRendererComponent_SetThickness(Entity.ID, value); }
        }

        public float Fade
        {
            get { return InternalCalls.CircleRendererComponent_GetFade(Entity.ID); }
            set { InternalCalls.CircleRendererComponent_SetFade(Entity.ID, value); }
        }
    }

    // ======================================================================
    //
    //            ADDITIONAL RENDERER COMPONENTS
    //
    // ======================================================================
    public class SkyboxComponent : Component{
        public enum Mode
        {
            HDRI = 0,
            Colorramp = 1
        }

        public Mode CurrentMode
        {
            get { return (Mode)InternalCalls.SkyboxComponent_GetMode(Entity.ID); }
            set { InternalCalls.SkyboxComponent_SetMode(Entity.ID, (int)value); }
        }

        // HDRI/cubemap asset paths
        public string SkyboxPath
        {
            get { return InternalCalls.SkyboxComponent_GetSkyboxPath(Entity.ID); }
            set { InternalCalls.SkyboxComponent_SetSkyboxPaths(Entity.ID, value, IrradiancePath ?? ""); }
        }

        public string IrradiancePath
        {
            get { return InternalCalls.SkyboxComponent_GetIrradiancePath(Entity.ID); }
            set { InternalCalls.SkyboxComponent_SetSkyboxPaths(Entity.ID, SkyboxPath ?? "", value); }
        }

        // Colorramp settings
        public Vector3 TopColor
        {
            get { Vector3 v; InternalCalls.SkyboxComponent_GetTopColor(Entity.ID, out v); return v; }
            set {
                var top = value;
                InternalCalls.SkyboxComponent_SetTopColor(Entity.ID, top);
            }
        }

        public Vector3 BottomColor
        {
            get { Vector3 v; InternalCalls.SkyboxComponent_GetBottomColor(Entity.ID, out v); return v; }
            set {
                var bot = value;
                InternalCalls.SkyboxComponent_SetBottomColor(Entity.ID, bot);
            }
        }
    }
    

    public class CameraComponent : Component{
        public bool Primary
        {
            get { return InternalCalls.CameraComponent_GetPrimary(Entity.ID); }
            set { InternalCalls.CameraComponent_SetPrimary(Entity.ID, value); }
        }

        public bool FixedAspectRatio
        {
            get { return InternalCalls.CameraComponent_GetFixedAspectRatio(Entity.ID); }
            set { InternalCalls.CameraComponent_SetFixedAspectRatio(Entity.ID, value); }
        }

        // Additional camera controls can be added later
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