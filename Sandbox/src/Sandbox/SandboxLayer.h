#pragma once
#include <Titan/Core/Layer.h>
#include <Titan/Renderer/PipelineState.h>
#include <Titan/Renderer/SceneRenderer.h>

namespace Titan
{
    class SandboxLayer : public Layer
    {
    public:
        SandboxLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate(Timestep ts) override;
        virtual void OnEvent(Event& event) override;
        virtual void OnImGuiRender(ImGuiContext* ctx) override;

    private:
        void OpenScene(const std::filesystem::path& path);

    private:
        Ref<PipelineState> m_PipelineState;
        Ref<SceneRenderer> m_SceneRenderer;
        Ref<Scene> m_ActiveScene;
    };
} // namespace Titan