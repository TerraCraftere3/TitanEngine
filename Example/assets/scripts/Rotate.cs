using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Titan;

namespace Sandbox
{
    public class Rotation : Entity
    {
        public float Speed = 0.1f;

        void OnCreate() {}

        void OnUpdate(float ts) { Rotation += new Vector3(0.0f, Speed * ts, 0.0f); }
    }
}