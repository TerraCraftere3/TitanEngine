#include "SceneHierarchyPanel.h"
#include "../Components.h"
#include "Titan/Renderer/Renderer2D.h"
#include "Titan/Scene/Components.h"
#include "Titan/Scripting/ScriptEngine.h"

#include "Titan/Scene/Assets.h"

namespace Titan
{
    extern const std::filesystem::path g_AssetPath;

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
    {
        m_Context = context;
        m_SelectionContext = {};
    }

    template <typename T>
    void DrawAddComponent(Entity& entity, const char* name)
    {
        if (!entity.HasComponent<T>())
        {
            if (ImGui::MenuItem(name))
            {
                entity.AddComponent<T>();
                ImGui::CloseCurrentPopup();
            }
        }
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

        auto view = m_Context->m_Registry.view<entt::entity>();
        for (auto entity : view)
        {
            Entity e{entity, m_Context.get()};
            DrawEntityNode(e);
        }

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            m_SelectionContext = {};

        if (ImGui::BeginPopupContextWindow(0, 1))
        {
            if (m_SelectionContext)
            {
                if (ImGui::MenuItem("Delete"))
                {
                    m_Context->DestroyEntity(m_SelectionContext);
                    m_SelectionContext = {};
                }
                if (ImGui::MenuItem("Duplicate"))
                {
                    m_Context->DuplicateEntity(m_SelectionContext);
                }
            }
            else
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                    m_Context->CreateEntity("Empty Entity");

                if (ImGui::MenuItem("Create Camera Entity"))
                {
                    Entity cameraEntity = m_Context->CreateEntity("Camera");
                    cameraEntity.AddComponent<CameraComponent>();
                }
                ImGui::SeparatorText("2D");
                if (ImGui::MenuItem("Create Sprite"))
                {
                    Entity spriteEntity = m_Context->CreateEntity("Sprite");
                    spriteEntity.AddComponent<SpriteRendererComponent>();
                }
                if (ImGui::MenuItem("Create Circle"))
                {
                    Entity circleEntity = m_Context->CreateEntity("Circle");
                    circleEntity.AddComponent<CircleRendererComponent>();
                }
                ImGui::SeparatorText("3D Primitives");
                if (ImGui::MenuItem("Create Cube"))
                {
                    Entity cubeEntity = m_Context->CreateEntity("Cube");
                    auto& mrc = cubeEntity.AddComponent<MeshRendererComponent>();
                    mrc.MeshRef = Mesh::CreateCube();
                }
                if (ImGui::MenuItem("Create Quad"))
                {
                    Entity quadEntity = m_Context->CreateEntity("Quad");
                    auto& mrc = quadEntity.AddComponent<MeshRendererComponent>();
                    mrc.MeshRef = Mesh::CreateQuad();
                }
            }

            ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::Begin("Properties");
        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);

            if (ImGui::Button("Add Component"))
                ImGui::OpenPopup("AddComponent");

            if (ImGui::BeginPopup("AddComponent"))
            {
                DrawAddComponent<CameraComponent>(m_SelectionContext, "Camera");

                ImGui::SeparatorText("Rendering");
                DrawAddComponent<MeshRendererComponent>(m_SelectionContext, "Mesh Renderer");
                DrawAddComponent<SpriteRendererComponent>(m_SelectionContext, "Sprite Renderer");
                DrawAddComponent<CircleRendererComponent>(m_SelectionContext, "Circle Renderer");

                ImGui::SeparatorText("Physics");
                DrawAddComponent<Rigidbody2DComponent>(m_SelectionContext, "Rigidbody 2D");
                DrawAddComponent<BoxCollider2DComponent>(m_SelectionContext, "Box Collider 2D");

                DrawAddComponent<CircleCollider2DComponent>(m_SelectionContext, "Circle Collider 2D");

                ImGui::SeparatorText("Scripts");
                DrawAddComponent<ScriptComponent>(m_SelectionContext, "Empty Script");
                for (const auto& [name, scriptClass] : ScriptEngine::GetEntityClasses())
                {
                    if (!m_SelectionContext.HasComponent<ScriptComponent>())
                    {
                        if (ImGui::MenuItem(("\"" + name + "\" Script").c_str()))
                        {
                            auto& sc = m_SelectionContext.AddComponent<ScriptComponent>();
                            sc.ClassName = name;
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }

                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags =
            ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
        if (ImGui::IsItemClicked())
        {
            m_SelectionContext = entity;
        }

        if (opened)
        {
            ImGui::TreePop();
        }
    }

    template <typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, bool canDelete = true)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                 ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding |
                                                 ImGuiTreeNodeFlags_AllowOverlap;
        if (entity.HasComponent<T>())
        {
            auto& component = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
            float lineHeight = ImGui::GetFont()->LegacySize + ImGui::GetStyle().FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
            ImGui::PopStyleVar();
            if (canDelete)
            {
                ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
                if (ImGui::Button("+", ImVec2{lineHeight, lineHeight}))
                {
                    ImGui::OpenPopup("ComponentSettings");
                }
            }

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSettings"))
            {
                if (ImGui::MenuItem("Remove component"))
                    removeComponent = true;

                ImGui::EndPopup();
            }

            if (open)
            {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent)
                entity.RemoveComponent<T>();
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), tag.c_str());
            if (ImGui::InputText("Name", buffer, sizeof(buffer)))
            {
                tag = std::string(buffer);
            }
        }

        DrawComponent<TransformComponent>(
            "Transform", entity,
            [](auto& component)
            {
                Component::Vec3Control("Translation", component.Translation);
                glm::vec3 rotation = glm::degrees(component.Rotation);
                Component::Vec3Control("Rotation", rotation);
                component.Rotation = glm::radians(rotation);
                Component::Vec3Control("Scale", component.Scale, 1.0f);
            },
            false);

        DrawComponent<CameraComponent>(
            "Camera", entity,
            [](auto& component)
            {
                auto& camera = component.Camera;

                ImGui::Checkbox("Primary", &component.Primary);

                const char* projectionTypeStrings[] = {"Perspective", "Orthographic"};
                const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
                if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
                {
                    for (int i = 0; i < 2; i++)
                    {
                        bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
                        if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
                        {
                            currentProjectionTypeString = projectionTypeStrings[i];
                            camera.SetProjectionType((SceneCamera::ProjectionType)i);
                        }

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }

                    ImGui::EndCombo();
                }

                if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
                {
                    float perspectiveVerticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
                    if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
                        camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveVerticalFov));

                    float perspectiveNear = camera.GetPerspectiveNearClip();
                    if (ImGui::DragFloat("Near", &perspectiveNear))
                        camera.SetPerspectiveNearClip(perspectiveNear);

                    float perspectiveFar = camera.GetPerspectiveFarClip();
                    if (ImGui::DragFloat("Far", &perspectiveFar))
                        camera.SetPerspectiveFarClip(perspectiveFar);
                }

                if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
                {
                    float orthoSize = camera.GetOrthographicSize();
                    if (ImGui::DragFloat("Size", &orthoSize))
                        camera.SetOrthographicSize(orthoSize);

                    float orthoNear = camera.GetOrthographicNearClip();
                    if (ImGui::DragFloat("Near", &orthoNear))
                        camera.SetOrthographicNearClip(orthoNear);

                    float orthoFar = camera.GetOrthographicFarClip();
                    if (ImGui::DragFloat("Far", &orthoFar))
                        camera.SetOrthographicFarClip(orthoFar);

                    ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);
                }
            });
        DrawComponent<SpriteRendererComponent>(
            "Sprite Renderer", entity,
            [](auto& component)
            {
                ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
                if (component.Tex)
                    ImGui::ImageButton("Texture", component.Tex->GetNativeTexture(), {64, 64}, ImVec2(0, 1),
                                       ImVec2(1, 0));
                else
                    ImGui::Button("Texture", ImVec2(100.0f, 0.0f));

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
                        component.Tex = Assets::Load<Texture2D>(texturePath.string());
                    }
                    ImGui::EndDragDropTarget();
                }
            });
        DrawComponent<CircleRendererComponent>("Circle Renderer", entity,
                                               [](auto& component)
                                               {
                                                   ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
                                                   ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f,
                                                                    1.0f);
                                                   ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
                                               });
        DrawComponent<MeshRendererComponent>(
            "Mesh Renderer", entity,
            [](auto& component)
            {
                float buttonWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Button(
                    std::format("Mesh: {}", component.MeshRef ? component.MeshRef->GetFilePath() : "None").c_str(),
                    ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        std::filesystem::path meshPath = std::filesystem::path(g_AssetPath) / path;
                        component.MeshRef = Assets::Load<Mesh>(meshPath.string());
                    }
                    ImGui::EndDragDropTarget();
                }
            });
        DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity,
                                            [](auto& component)
                                            {
                                                const char* bodyTypeStrings[] = {"Static", "Dynamic", "Kinematic"};
                                                const char* currentBodyTypeString =
                                                    bodyTypeStrings[(int)component.Type];
                                                if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
                                                {
                                                    for (int i = 0; i < 2; i++)
                                                    {
                                                        bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                                                        if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                                                        {
                                                            currentBodyTypeString = bodyTypeStrings[i];
                                                            component.Type = (Rigidbody2DComponent::BodyType)i;
                                                        }

                                                        if (isSelected)
                                                            ImGui::SetItemDefaultFocus();
                                                    }

                                                    ImGui::EndCombo();
                                                }

                                                ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
                                            });

        DrawComponent<BoxCollider2DComponent>(
            "Box Collider 2D", entity,
            [](auto& component)
            {
                ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
                ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
                float buttonWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Button(std::format("Material: {}", component.Material->SourcePath).c_str(),
                              ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        std::filesystem::path materialPath = std::filesystem::path(g_AssetPath) / path;
                        component.Material = Assets::Load<Physics2DMaterial>(materialPath.string());
                    }
                    ImGui::EndDragDropTarget();
                }
            });

        DrawComponent<CircleCollider2DComponent>(
            "Circle Collider 2D", entity,
            [](auto& component)
            {
                ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
                ImGui::DragFloat("Radius", &component.Radius);
                float buttonWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Button(std::format("Material: {}", component.Material->SourcePath).c_str(),
                              ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        std::filesystem::path materialPath = std::filesystem::path(g_AssetPath) / path;
                        component.Material = Assets::Load<Physics2DMaterial>(materialPath.string());
                    }
                    ImGui::EndDragDropTarget();
                }
            });

        DrawComponent<ScriptComponent>(
            "Script", entity,
            [entity, scene = m_Context](auto& component) mutable
            {
                bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

                static char buffer[64];
                strcpy_s(buffer, sizeof(buffer), component.ClassName.c_str());

                if (!scriptClassExists)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));

                if (ImGui::InputText("Class", buffer, sizeof(buffer)))
                    component.ClassName = buffer;

                // Fields
                bool sceneRunning = scene->IsRunning();
                if (sceneRunning)
                {
                    Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
                    if (scriptInstance)
                    {
                        const auto& fields = scriptInstance->GetScriptClass()->GetFields();
                        for (const auto& [name, field] : fields)
                        {
                            if (field.Type == ScriptFieldType::Float)
                            {
                                float data = scriptInstance->GetFieldValue<float>(name);
                                if (ImGui::DragFloat(name.c_str(), &data))
                                {
                                    scriptInstance->SetFieldValue(name, data);
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (scriptClassExists)
                    {
                        Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName);
                        const auto& fields = entityClass->GetFields();

                        auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
                        for (const auto& [name, field] : fields)
                        {
                            // Field has been set in editor
                            if (entityFields.find(name) != entityFields.end())
                            {
                                ScriptFieldInstance& scriptField = entityFields.at(name);

                                // Display control to set it maybe
                                if (field.Type == ScriptFieldType::Float)
                                {
                                    float data = scriptField.GetValue<float>();
                                    if (ImGui::DragFloat(name.c_str(), &data))
                                        scriptField.SetValue(data);
                                }
                            }
                            else
                            {
                                // Display control to set it maybe
                                if (field.Type == ScriptFieldType::Float)
                                {
                                    float data = 0.0f;
                                    if (ImGui::DragFloat(name.c_str(), &data))
                                    {
                                        ScriptFieldInstance& fieldInstance = entityFields[name];
                                        fieldInstance.Field = field;
                                        fieldInstance.SetValue(data);
                                    }
                                }
                            }
                        }
                    }
                }

                if (!scriptClassExists)
                    ImGui::PopStyleColor();
            });
    }

} // namespace Titan