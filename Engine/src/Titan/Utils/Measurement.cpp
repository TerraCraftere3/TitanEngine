#include "Measurement.h"
#include <chrono>
#include "Titan/Platform/OpenGL/OpenGLMeasurement.h"
#include "Titan/Renderer/Renderer.h"

namespace Titan
{
    class CPUMeasurement : public Measurement
    {
    public:
        CPUMeasurement() = default;
        virtual void Start() override { startTime = std::chrono::high_resolution_clock::now(); }

        virtual void Stop() override { endTime = std::chrono::high_resolution_clock::now(); }

        virtual double GetElapsedMilliseconds() override
        {
            return std::chrono::duration<double, std::milli>(endTime - startTime).count();
        }

        virtual double GetElapsedSeconds() override
        {
            return std::chrono::duration<double>(endTime - startTime).count();
        }

    private:
        std::chrono::high_resolution_clock::time_point startTime;
        std::chrono::high_resolution_clock::time_point endTime;
    };

    Ref<Measurement> Measurement::CreateCPU()
    {
        return CreateRef<CPUMeasurement>();
    }

    Ref<Measurement> Measurement::CreateGPU()
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                TI_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLMeasurement>();
        }
        TI_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
} // namespace Titan