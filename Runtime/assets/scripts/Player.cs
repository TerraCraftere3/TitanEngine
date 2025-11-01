using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Titan;

namespace Sandbox
{
    public class Player : Entity
    {
        private TransformComponent m_Transform;
        private Rigidbody2DComponent m_Rigidbody;
        public float Speed = 0.1f;
        public float Time = 0.0f;

        void OnCreate()
        {
            m_Transform = GetComponent<TransformComponent>();
            m_Rigidbody = GetComponent<Rigidbody2DComponent>();
        }

        void OnUpdate(float ts)
        {
            Time += ts;
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.Up))
                velocity.Y = 1.0f;
            else if (Input.IsKeyDown(KeyCode.Down))
                velocity.Y = -1.0f;

            if (Input.IsKeyDown(KeyCode.Left))
                velocity.X = -1.0f;
            else if (Input.IsKeyDown(KeyCode.Right))
                velocity.X = 1.0f;

            Entity cameraEntity = FindEntityByName("Camera");
            if (cameraEntity != null)
            {
                Camera camera = cameraEntity.As<Camera>();

                if (Input.IsKeyDown(KeyCode.Q))
                    camera.DistanceFromPlayer += Speed * 10.0f * ts;
                else if (Input.IsKeyDown(KeyCode.E))
                    camera.DistanceFromPlayer -= Speed * 10.0f * ts;
            }

            velocity *= Speed;

            m_Rigidbody.ApplyLinearImpulse(velocity.XY, true);
        }
    }
}