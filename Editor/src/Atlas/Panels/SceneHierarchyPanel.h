#pragma once

#include "Titan/Core.h"
#include "Titan/Core/Log.h"
#include "Titan/Scene/Scene.h"
#include "Titan/Scene/Entity.h"

namespace Titan {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(const Ref<Scene>& scene);

		void OnImGuiRender();
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_Context;
		Entity m_SelectionContext;
	};

}