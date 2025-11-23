#pragma once

#include "PxPhysicsAPI.h"
#include "Titan/Core/Timestep.h"
#include "Titan/PCH.h"
#include "Titan/Scene/Components.h"

namespace Titan::Physics3D
{
    void Init();
    void Shutdown();

    physx::PxFoundation* GetFoundation();
    physx::PxPhysics* GetPhysics();
    physx::PxPvd* GetPvd();
    physx::PxDefaultCpuDispatcher* GetDispatcher();

} // namespace Titan::Physics3D
