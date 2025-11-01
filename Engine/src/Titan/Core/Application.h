#pragma once

#include "Titan/Core.h"
#include "Titan/Core/LayerStack.h"
#include "Titan/Core/Timestep.h"
#include "Titan/Core/Window.h"
#include "Titan/Events/Event.h"
#include "Titan/ImGuiLayer.h"
#include "Titan/PCH.h"

namespace Titan
{
    /// @brief The Application that manages Render Context, Window, Scenes, etc.
    class TI_API Application
    {
    public:
        /// @brief Creates the Application (which manages window, etc)
        /// @param name The Window Title
        Application(const std::string& name = "Titan App");
        /// @brief Destructs the Application
        virtual ~Application();

        /// @brief Closes the window on the next update
        void Close();
        /// @brief Starts the runtime loop
        void Run();
        /// @brief Triggers on a event
        /// @param event the event
        void OnEvent(Event& event);

        /// @brief Pushes a layer to the stack
        /// @param layer the layer
        void PushLayer(Layer* layer);
        /// @brief Pushes a layer to the back of the stack
        /// @param layer the layer
        void PushOverlay(Layer* layer);

        /// @brief Gets the window (not native)
        /// @return the window of this instance
        inline Window& GetWindow() { return *m_Window; }

        /// @brief Submits a function that should be executed in the main thread
        /// @param function the function
        void SubmitToMainThread(const std::function<void()>& function);

        /// @brief Gets the current public instance
        /// @return the instance
        inline static Application* GetInstance() { return s_Instance; };

    private:
        bool OnWindowClosed(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

        void ExecuteMainThreadQueue();

    private:
        static Application* s_Instance;
        Scope<Window> m_Window;
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        ImGuiLayer* m_ImGuiLayer;
        float m_LastFrameTime = 0.0f;
        std::vector<std::function<void()>> m_MainThreadQueue;
        std::mutex m_MainThreadQueueMutex;
    };

    void TI_API DeleteApplication(Application* app);

    // Declared by Client
    Application* CreateApplication();

} // namespace Titan