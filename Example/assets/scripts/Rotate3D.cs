using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Titan;

namespace Sandbox
{
    public class Rotation3D : Entity
    {
        public Vector3 Speed;

        void OnCreate() {}

        void OnUpdate(float ts) { Rotation += Speed * ts; }
    }
}