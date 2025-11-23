#include "Physics3D.h"
#include "PxPhysicsAPI.h"

namespace Titan::Physics3D
{

    using namespace physx;

    namespace
    {
        class PhysXErrorCallback : public PxErrorCallback
        {
        public:
            void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override
            {
                TI_CORE_ERROR("PhysX Error: {} ({}:{}) code={}", message ? message : "(null)",
                              file ? file : "(unknown)", line, static_cast<int>(code));
            }
        };

        PxDefaultAllocator* g_Allocator = nullptr;
        PhysXErrorCallback* g_ErrorCallback = nullptr;
        PxFoundation* g_Foundation = nullptr;
        PxPhysics* g_Physics = nullptr;
        PxPvd* g_Pvd = nullptr;
        PxPvdTransport* g_PvdTransport = nullptr;
        PxDefaultCpuDispatcher* g_Dispatcher = nullptr;
    } // namespace

    void Init()
    {
        // Allocator and error callback (custom error callback used here)
        g_Allocator = new PxDefaultAllocator();
        g_ErrorCallback = new PhysXErrorCallback();

        // Foundation
        g_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *g_Allocator, *g_ErrorCallback);
        if (!g_Foundation)
        {
            TI_CORE_ERROR("Failed to create PhysX Foundation");
            return;
        }

        // PVD (PhysX Visual Debugger) - optional but useful for debugging
        g_Pvd = PxCreatePvd(*g_Foundation);
        if (g_Pvd)
        {
            g_PvdTransport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 1000);
            if (g_PvdTransport)
                g_Pvd->connect(*g_PvdTransport, PxPvdInstrumentationFlag::eALL);
        }

        // Create physics instance
        PxTolerancesScale scale; // default scale is fine for most projects
        g_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *g_Foundation, scale, true, g_Pvd);
        if (!g_Physics)
        {
            TI_CORE_ERROR("Failed to create PhysX Physics instance");
            // Clean up what we created so far
            if (g_Pvd && g_PvdTransport)
            {
                g_Pvd->disconnect();
                g_PvdTransport->release();
                g_Pvd->release();
                g_Pvd = nullptr;
                g_PvdTransport = nullptr;
            }
            g_Foundation->release();
            g_Foundation = nullptr;
            delete g_ErrorCallback;
            g_ErrorCallback = nullptr;
            delete g_Allocator;
            g_Allocator = nullptr;
            return;
        }

        // Initialize extensions (registers cooking, etc.)
        PxInitExtensions(*g_Physics, g_Pvd);

        // Create a default CPU dispatcher (thread pool for simulation)
        g_Dispatcher = PxDefaultCpuDispatcherCreate(2);

        TI_CORE_INFO("PhysX initialized successfully");
    }

    PxFoundation* GetFoundation()
    {
        return g_Foundation;
    }

    PxPhysics* GetPhysics()
    {
        return g_Physics;
    }

    PxPvd* GetPvd()
    {
        return g_Pvd;
    }

    PxDefaultCpuDispatcher* GetDispatcher()
    {
        return g_Dispatcher;
    }

    void Shutdown()
    {
        // Ensure proper shutdown order
        if (g_Physics)
        {
            PxCloseExtensions();
        }

        if (g_Dispatcher)
        {
            g_Dispatcher->release();
            g_Dispatcher = nullptr;
        }

        if (g_Physics)
        {
            g_Physics->release();
            g_Physics = nullptr;
        }

        if (g_Pvd)
        {
            // disconnect then release transport and PVD
            g_Pvd->disconnect();
            if (g_PvdTransport)
            {
                g_PvdTransport->release();
                g_PvdTransport = nullptr;
            }
            g_Pvd->release();
            g_Pvd = nullptr;
        }

        if (g_Foundation)
        {
            g_Foundation->release();
            g_Foundation = nullptr;
        }

        delete g_ErrorCallback;
        g_ErrorCallback = nullptr;

        delete g_Allocator;
        g_Allocator = nullptr;

        TI_CORE_INFO("PhysX shutdown complete");
    }

} // namespace Titan::Physics3D
