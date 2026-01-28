#pragma once

#include "Titan/Core.h"
#include "Titan/Core/LayerStack.h"
#include "Titan/Core/Window.h"
#include "Titan/Events/ApplicationEvent.h"
#include "Titan/Events/Event.h"
#include "Titan/ImGuiLayer.h"
#include "Titan/PCH.h"
#include "Titan/Utils/Measurement.h"

namespace Titan
{
    // Forward declarations
    class Layer;

    struct ApplicationCommandLineArgs
    {
        int Count = 0;
        char** Args = nullptr;

        const char* operator[](int index) const
        {
            TI_CORE_ASSERT(index < Count);
            return Args[index];
        }
    };

    struct ApplicationSpecification
    {
        std::string Name = "Titan Application";
        WindowProps WindowProperties;
        UITheme Theme = UITheme::Dark;
        bool DisableImGui = false;
        ApplicationCommandLineArgs CommandLineArgs;
    };

    /// @brief The Application that manages Render Context, Window, Scenes, etc.
    class TI_API Application
    {
    public:
        /// @brief Creates the Application (which manages window, etc)
        /// @param name The Window Title
        Application(const ApplicationSpecification& specification);
        /// @brief Destructs the Application
        virtual ~Application();

        /// @brief Closes the window on the next update
        void Close();
        /// @brief Starts the runtime loop
        void Run();
        /// @brief Triggers on a event
        /// @param event the event
        void OnEvent(Event& event);

        /// @brief Gets the CPU time measurement
        /// @return the measurement
        inline Ref<Measurement> GetCPUTimeMeasurement() { return m_CPUTimeMeasurement; }

        /// @brief Gets the GPU time measurement
        /// @return the measurement
        inline Ref<Measurement> GetGPUTimeMeasurement() { return m_GPUTimeMeasurement; }

        /// @brief Pushes a layer to the stack
        /// @param layer the layer
        void PushLayer(Layer* layer);
        /// @brief Pushes a layer to the back of the stack
        /// @param layer the layer
        void PushOverlay(Layer* layer);

        /// @brief Updates the theme of the ImGui layer
        /// @param theme the new theme
        void UpdateTheme(UITheme theme);

        /// @brief Gets the window (not native)
        /// @return the window of this instance
        inline Window& GetWindow() { return *m_Window; }

        /// @brief Submits a function that should be executed in the main thread
        /// @param function the function
        void SubmitToMainThread(const std::function<void()>& function);

        /// @brief Gets the current public instance
        /// @return the instance
        inline static Application* GetInstance() { return s_Instance; };

        /// @brief Gets the application specification
        /// @return the specification
        inline const ApplicationSpecification& GetSpecification() const { return m_Specification; }

    private:
        bool OnWindowClosed(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

        void ExecuteMainThreadQueue();

    private:
        static Application* s_Instance;
        ApplicationSpecification m_Specification;
        Scope<Window> m_Window;
        bool m_Running = true;
        bool m_Minimized = false;
        bool m_ImGuiEnabled = true;
        LayerStack m_LayerStack;
        ImGuiLayer* m_ImGuiLayer;
        float m_LastFrameTime = 0.0f;
        std::vector<std::function<void()>> m_MainThreadQueue;
        std::mutex m_MainThreadQueueMutex;
        Ref<Measurement> m_CPUTimeMeasurement;
        Ref<Measurement> m_GPUTimeMeasurement;
    };

    void TI_API DeleteApplication(Application* app);

    // Declared by Client
    Application* CreateApplication(ApplicationCommandLineArgs args);

} // namespace Titan