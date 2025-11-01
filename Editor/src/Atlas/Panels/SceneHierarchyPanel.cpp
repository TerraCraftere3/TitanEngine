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
            if (ImGui::MenuItem("Create Empty Entity"))
                m_Context->CreateEntity("Empty Entity");

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
                if (!m_SelectionContext.HasComponent<CameraComponent>())
                {
                    if (ImGui::MenuItem("Camera"))
                    {
                        m_SelectionContext.AddComponent<CameraComponent>();
                        ImGui::CloseCurrentPopup();
                    }
                }

                ImGui::SeparatorText("Rendering");

                if (!m_SelectionContext.HasComponent<SpriteRendererComponent>())
                {
                    if (ImGui::MenuItem("Sprite Renderer"))
                    {
                        m_SelectionContext.AddComponent<SpriteRendererComponent>();
                        ImGui::CloseCurrentPopup();
                    }
                }

                if (!m_SelectionContext.HasComponent<CircleRendererComponent>())
                {
                    if (ImGui::MenuItem("Circle Renderer"))
                    {
                        m_SelectionContext.AddComponent<CircleRendererComponent>();
                        ImGui::CloseCurrentPopup();
                    }
                }

                ImGui::SeparatorText("Physics");

                if (!m_SelectionContext.HasComponent<Rigidbody2DComponent>())
                {
                    if (ImGui::MenuItem("Rigidbody 2D"))
                    {
                        m_SelectionContext.AddComponent<Rigidbody2DComponent>();
                        ImGui::CloseCurrentPopup();
                    }
                }

                if (!m_SelectionContext.HasComponent<BoxCollider2DComponent>())
                {
                    if (ImGui::MenuItem("Box Collider 2D"))
                    {
                        m_SelectionContext.AddComponent<BoxCollider2DComponent>();
                        ImGui::CloseCurrentPopup();
                    }
                }

                if (!m_SelectionContext.HasComponent<CircleCollider2DComponent>())
                {
                    if (ImGui::MenuItem("Circle Collider 2D"))
                    {
                        m_SelectionContext.AddComponent<CircleCollider2DComponent>();
                        ImGui::CloseCurrentPopup();
                    }
                }

                ImGui::SeparatorText("Scripts");
                if (!m_SelectionContext.HasComponent<ScriptComponent>())
                {
                    if (ImGui::MenuItem("Script Component"))
                    {
                        m_SelectionContext.AddComponent<ScriptComponent>();
                        ImGui::CloseCurrentPopup();
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
            [entity](auto& component) mutable
            {
                bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

                static char buffer[64];
                strcpy(buffer, component.ClassName.c_str());

                if (!scriptClassExists)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));

                if (ImGui::InputText("Class", buffer, sizeof(buffer)))
                    component.ClassName = buffer;

                Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
                if (scriptInstance)
                {
                    const auto& fields = scriptInstance->GetScriptClass()->GetFields();

                    for (const auto& [name, field] : fields)
                    {
                        switch (field.Type)
                        {
                            case ScriptFieldType::Float:
                            {
                                float data = scriptInstance->GetFieldValue<float>(name);
                                if (ImGui::DragFloat(name.c_str(), &data))
                                    scriptInstance->SetFieldValue(name, data);
                                break;
                            }
                            case ScriptFieldType::Double:
                            {
                                double data = scriptInstance->GetFieldValue<double>(name);
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, &data, 0.1f))
                                    scriptInstance->SetFieldValue(name, data);
                                break;
                            }
                            case ScriptFieldType::Bool:
                            {
                                bool data = scriptInstance->GetFieldValue<bool>(name);
                                if (ImGui::Checkbox(name.c_str(), &data))
                                    scriptInstance->SetFieldValue(name, data);
                                break;
                            }
                            case ScriptFieldType::Char:
                            {
                                char data = scriptInstance->GetFieldValue<char>(name);
                                char buf[2] = {data, '\0'};
                                if (ImGui::InputText(name.c_str(), buf, sizeof(buf)))
                                    scriptInstance->SetFieldValue(name, buf[0]);
                                break;
                            }
                            case ScriptFieldType::Byte:
                            {
                                int8_t data = scriptInstance->GetFieldValue<int8_t>(name);
                                int tmp = data;
                                if (ImGui::DragInt(name.c_str(), &tmp, 1, -128, 127))
                                    scriptInstance->SetFieldValue(name, static_cast<int8_t>(tmp));
                                break;
                            }
                            case ScriptFieldType::Short:
                            {
                                int16_t data = scriptInstance->GetFieldValue<int16_t>(name);
                                int tmp = data;
                                if (ImGui::DragInt(name.c_str(), &tmp, 1, INT16_MIN, INT16_MAX))
                                    scriptInstance->SetFieldValue(name, static_cast<int16_t>(tmp));
                                break;
                            }
                            case ScriptFieldType::Int:
                            {
                                int data = scriptInstance->GetFieldValue<int>(name);
                                if (ImGui::DragInt(name.c_str(), &data))
                                    scriptInstance->SetFieldValue(name, data);
                                break;
                            }
                            case ScriptFieldType::Long:
                            {
                                int64_t data = scriptInstance->GetFieldValue<int64_t>(name);
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &data))
                                    scriptInstance->SetFieldValue(name, data);
                                break;
                            }
                            case ScriptFieldType::UByte:
                            {
                                uint8_t data = scriptInstance->GetFieldValue<uint8_t>(name);
                                int tmp = data;
                                if (ImGui::DragInt(name.c_str(), &tmp, 1, 0, 255))
                                    scriptInstance->SetFieldValue(name, static_cast<uint8_t>(tmp));
                                break;
                            }
                            case ScriptFieldType::UShort:
                            {
                                uint16_t data = scriptInstance->GetFieldValue<uint16_t>(name);
                                int tmp = data;
                                if (ImGui::DragInt(name.c_str(), &tmp, 1, 0, UINT16_MAX))
                                    scriptInstance->SetFieldValue(name, static_cast<uint16_t>(tmp));
                                break;
                            }
                            case ScriptFieldType::UInt:
                            {
                                uint32_t data = scriptInstance->GetFieldValue<uint32_t>(name);
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U32, &data))
                                    scriptInstance->SetFieldValue(name, data);
                                break;
                            }
                            case ScriptFieldType::ULong:
                            {
                                uint64_t data = scriptInstance->GetFieldValue<uint64_t>(name);
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
                                    scriptInstance->SetFieldValue(name, data);
                                break;
                            }
                            case ScriptFieldType::Vector2:
                            {
                                glm::vec2 data = scriptInstance->GetFieldValue<glm::vec2>(name);
                                if (ImGui::DragFloat2(name.c_str(), glm::value_ptr(data)))
                                    scriptInstance->SetFieldValue(name, data);
                                break;
                            }
                            case ScriptFieldType::Vector3:
                            {
                                glm::vec3 data = scriptInstance->GetFieldValue<glm::vec3>(name);
                                if (ImGui::DragFloat3(name.c_str(), glm::value_ptr(data)))
                                    scriptInstance->SetFieldValue(name, data);
                                break;
                            }
                            case ScriptFieldType::Vector4:
                            {
                                glm::vec4 data = scriptInstance->GetFieldValue<glm::vec4>(name);
                                if (ImGui::DragFloat4(name.c_str(), glm::value_ptr(data)))
                                    scriptInstance->SetFieldValue(name, data);
                                break;
                            }
                            case ScriptFieldType::Entity:
                            {
                                uint64_t entityID = scriptInstance->GetFieldValue<uint64_t>(name);
                                ImGui::Text("%s: Entity(%llu)", name.c_str(), entityID);
                                break;
                            }
                            case ScriptFieldType::None:
                            default:
                                ImGui::Text("%s: <Unsupported type>", name.c_str());
                                break;
                        }
                    }
                }

                if (!scriptClassExists)
                    ImGui::PopStyleColor();
            });
    }

} // namespace Titan