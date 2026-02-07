#include "SandboxLayer.h"
#include <Titan/Core/Application.h>
#include <Titan/Project/Project.h>
#include <Titan/Renderer/Blit.h>
#include <Titan/Renderer/RenderCommand.h>
#include <Titan/Renderer/Systems/GeometryRenderer.h>
#include <Titan/Scene/Assets.h>
#include <Titan/Scene/SceneSerializer.h>

namespace Titan
{
    SandboxLayer::SandboxLayer() : Layer("SandboxLayer") {}

    void SandboxLayer::OnAttach()
    {
        Application::GetInstance()->GetWindow().SetVSync(true);
        m_SceneRenderer = SceneRenderer::Create();
        m_ActiveScene = CreateRef<Scene>();
        m_ActiveScene->OnViewportResize(Application::GetInstance()->GetWindow().GetWidth(),
                                        Application::GetInstance()->GetWindow().GetHeight());
        m_ActiveScene->OnRuntimeStart();

        m_PipelineState = PipelineState::Create();

        OpenScene(Project::GetAssetDirectory() / Project::GetActive()->GetConfig().StartScene);
    }

    void SandboxLayer::OnDetach()
    {
        m_ActiveScene->OnRuntimeStop();
        m_ActiveScene = nullptr;
    }

    void SandboxLayer::OnUpdate(Timestep ts)
    {
        RenderCommand::ResetStats();

        // Resize
        m_ActiveScene->OnViewportResize(Application::GetInstance()->GetWindow().GetWidth(),
                                        Application::GetInstance()->GetWindow().GetHeight());
        m_SceneRenderer->Resize(Application::GetInstance()->GetWindow().GetWidth(),
                                Application::GetInstance()->GetWindow().GetHeight());

        // Update
        m_ActiveScene->OnUpdateRuntime(ts);

        // Render
        m_SceneRenderer->RenderSceneRuntime(m_ActiveScene);
        Blit::Execute(m_SceneRenderer->GetFramebuffer(), RenderCommand::GetSwapchainTarget(), BlitMode::ColorOnly);
    }

    void SandboxLayer::OnEvent(Event& event) {}

    void SandboxLayer::OnImGuiRender(ImGuiContext* ctx) {}

    void SandboxLayer::OpenScene(const std::filesystem::path& path)
    {
        if (m_ActiveScene)
            m_ActiveScene->OnRuntimeStop();
        if (path.extension().string() != ".titan")
        {
            TI_WARN("Could not load {0} - not a scene file", path.filename().string());
            return;
        }
        Ref<Scene> newScene = Assets::Load<Scene>(path);
        newScene->OnViewportResize(Application::GetInstance()->GetWindow().GetWidth(),
                                   Application::GetInstance()->GetWindow().GetHeight());
        newScene->OnRuntimeStart();

        m_ActiveScene = newScene;

        GeometryRenderer::ClearCache();
    }
} // namespace Titan