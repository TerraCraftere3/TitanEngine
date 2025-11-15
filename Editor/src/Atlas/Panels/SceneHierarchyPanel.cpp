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
                ImGui::SeparatorText("Lights");
                if (ImGui::MenuItem("Create Directional Light"))
                {
                    Entity quadEntity = m_Context->CreateEntity("Directional Light");
                    auto& dlc = quadEntity.AddComponent<DirectionalLightComponent>(glm::vec3(1.0f, 1.0f, 1.0f));
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

                ImGui::SeparatorText("Lights");
                DrawAddComponent<DirectionalLightComponent>(m_SelectionContext, "Directional Light");

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

    bool DrawTextureSlot(const char* label, Ref<Texture2D>& texture, const ImVec2& previewSize = {64, 64})
    {
        ImGui::TextUnformatted(label);

        // --- Texture preview / button ---
        if (texture)
        {
            ImGui::ImageButton(label, texture->GetNativeTexture(), previewSize, ImVec2(0, 1), ImVec2(1, 0));
        }
        else
        {
            ImGui::Button((std::string("Empty##") + label).c_str(), previewSize);
        }

        bool changed = false;

        // --- Drag & Drop ---
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                std::filesystem::path fullPath = std::filesystem::path(g_AssetPath) / path;
                texture = Assets::Load<Texture2D>(fullPath.string());
                changed = true;
            }
            ImGui::EndDragDropTarget();
        }

        // --- Right-click context menu ---
        if (ImGui::BeginPopupContextItem(label))
        {
            if (ImGui::MenuItem("Remove"))
            {
                texture = nullptr;
                changed = true;
            }
            ImGui::EndPopup();
        }

        return changed;
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
        DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity,
                                               [](auto& component)
                                               {
                                                   ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));
                                                   DrawTextureSlot("Texture", component.Tex);
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
                int index = 0;
                for (auto mat : component.MeshRef->GetMaterials())
                {
                    ImGui::PushID(index);
                    std::string label = mat->Name;

                    if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_Framed))
                    {
                        ImGui::ColorEdit4("Diffuse", glm::value_ptr(mat->AlbedoColor));
                        DrawTextureSlot("Diffuse Texture", mat->AlbedoTexture);
                        DrawTextureSlot("Metallic Texture", mat->MetallicTexture);
                        DrawTextureSlot("Roughness Texture", mat->RoughnessTexture);
                        DrawTextureSlot("Normal Texture", mat->NormalTexture);
                        DrawTextureSlot("Ambient Occlusion Texture", mat->AOTexture);
                        ImGui::DragFloat2("UV Repeat", glm::value_ptr(mat->UVRepeat), 0.1f, 0.01f, 100.0f);

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                    index++;
                }
            });
        DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto& component)
                                                 { Component::DirectionControl("Direction", component.Direction); });
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
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::Double)
                            {
                                double data = scriptInstance->GetFieldValue<double>(name);
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, &data))
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::Bool)
                            {
                                bool data = scriptInstance->GetFieldValue<bool>(name);
                                if (ImGui::Checkbox(name.c_str(), &data))
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::Char)
                            {
                                char data = scriptInstance->GetFieldValue<char>(name);
                                int temp = static_cast<int>(data);
                                if (ImGui::InputInt(name.c_str(), &temp))
                                    scriptInstance->SetFieldValue(name, static_cast<char>(temp));
                            }
                            else if (field.Type == ScriptFieldType::Byte)
                            {
                                int8_t data = scriptInstance->GetFieldValue<int8_t>(name);
                                int temp = static_cast<int>(data);
                                if (ImGui::InputInt(name.c_str(), &temp))
                                    scriptInstance->SetFieldValue(name, static_cast<int8_t>(temp));
                            }
                            else if (field.Type == ScriptFieldType::Short)
                            {
                                int16_t data = scriptInstance->GetFieldValue<int16_t>(name);
                                int temp = static_cast<int>(data);
                                if (ImGui::InputInt(name.c_str(), &temp))
                                    scriptInstance->SetFieldValue(name, static_cast<int16_t>(temp));
                            }
                            else if (field.Type == ScriptFieldType::Int)
                            {
                                int data = scriptInstance->GetFieldValue<int>(name);
                                if (ImGui::DragInt(name.c_str(), &data))
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::Long)
                            {
                                int64_t data = scriptInstance->GetFieldValue<int64_t>(name);
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &data))
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::UByte)
                            {
                                uint8_t data = scriptInstance->GetFieldValue<uint8_t>(name);
                                int temp = static_cast<int>(data);
                                if (ImGui::InputInt(name.c_str(), &temp))
                                    scriptInstance->SetFieldValue(name, static_cast<uint8_t>(temp));
                            }
                            else if (field.Type == ScriptFieldType::UShort)
                            {
                                uint16_t data = scriptInstance->GetFieldValue<uint16_t>(name);
                                int temp = static_cast<int>(data);
                                if (ImGui::InputInt(name.c_str(), &temp))
                                    scriptInstance->SetFieldValue(name, static_cast<uint16_t>(temp));
                            }
                            else if (field.Type == ScriptFieldType::UInt)
                            {
                                uint32_t data = scriptInstance->GetFieldValue<uint32_t>(name);
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U32, &data))
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::ULong)
                            {
                                uint64_t data = scriptInstance->GetFieldValue<uint64_t>(name);
                                if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::Vector2)
                            {
                                glm::vec2 data = scriptInstance->GetFieldValue<glm::vec2>(name);
                                if (ImGui::DragFloat2(name.c_str(), &data.x))
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::Vector3)
                            {
                                glm::vec3 data = scriptInstance->GetFieldValue<glm::vec3>(name);
                                if (ImGui::DragFloat3(name.c_str(), &data.x))
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::Vector4)
                            {
                                glm::vec4 data = scriptInstance->GetFieldValue<glm::vec4>(name);
                                if (ImGui::DragFloat4(name.c_str(), &data.x))
                                    scriptInstance->SetFieldValue(name, data);
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
                            bool fieldAlreadyExists = entityFields.find(name) != entityFields.end();
                            ScriptFieldInstance* scriptField = nullptr;

                            if (fieldAlreadyExists)
                            {
                                scriptField = &entityFields.at(name);
                            }
                            else
                            {
                                // Create field if it doesn't exist
                                ScriptFieldInstance& newField = entityFields[name];
                                newField.Field = field;
                                scriptField = &newField;
                            }

                            switch (field.Type)
                            {
                                case ScriptFieldType::Float:
                                {
                                    float data = fieldAlreadyExists ? scriptField->GetValue<float>() : 0.0f;
                                    if (ImGui::DragFloat(name.c_str(), &data))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::Double:
                                {
                                    double data = fieldAlreadyExists ? scriptField->GetValue<double>() : 0.0;
                                    if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, &data))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::Bool:
                                {
                                    bool data = fieldAlreadyExists ? scriptField->GetValue<bool>() : false;
                                    if (ImGui::Checkbox(name.c_str(), &data))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::Char:
                                {
                                    char data = fieldAlreadyExists ? scriptField->GetValue<char>() : 0;
                                    int temp = static_cast<int>(data);
                                    if (ImGui::InputInt(name.c_str(), &temp))
                                        scriptField->SetValue(static_cast<char>(temp));
                                    break;
                                }
                                case ScriptFieldType::Byte:
                                {
                                    int8_t data = fieldAlreadyExists ? scriptField->GetValue<int8_t>() : 0;
                                    int temp = static_cast<int>(data);
                                    if (ImGui::InputInt(name.c_str(), &temp))
                                        scriptField->SetValue(static_cast<int8_t>(temp));
                                    break;
                                }
                                case ScriptFieldType::Short:
                                {
                                    int16_t data = fieldAlreadyExists ? scriptField->GetValue<int16_t>() : 0;
                                    int temp = static_cast<int>(data);
                                    if (ImGui::InputInt(name.c_str(), &temp))
                                        scriptField->SetValue(static_cast<int16_t>(temp));
                                    break;
                                }
                                case ScriptFieldType::Int:
                                {
                                    int data = fieldAlreadyExists ? scriptField->GetValue<int>() : 0;
                                    if (ImGui::DragInt(name.c_str(), &data))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::Long:
                                {
                                    int64_t data = fieldAlreadyExists ? scriptField->GetValue<int64_t>() : 0;
                                    if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &data))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::UByte:
                                {
                                    uint8_t data = fieldAlreadyExists ? scriptField->GetValue<uint8_t>() : 0;
                                    int temp = static_cast<int>(data);
                                    if (ImGui::InputInt(name.c_str(), &temp))
                                        scriptField->SetValue(static_cast<uint8_t>(temp));
                                    break;
                                }
                                case ScriptFieldType::UShort:
                                {
                                    uint16_t data = fieldAlreadyExists ? scriptField->GetValue<uint16_t>() : 0;
                                    int temp = static_cast<int>(data);
                                    if (ImGui::InputInt(name.c_str(), &temp))
                                        scriptField->SetValue(static_cast<uint16_t>(temp));
                                    break;
                                }
                                case ScriptFieldType::UInt:
                                {
                                    uint32_t data = fieldAlreadyExists ? scriptField->GetValue<uint32_t>() : 0;
                                    if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U32, &data))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::ULong:
                                {
                                    uint64_t data = fieldAlreadyExists ? scriptField->GetValue<uint64_t>() : 0;
                                    if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::Vector2:
                                {
                                    glm::vec2 data =
                                        fieldAlreadyExists ? scriptField->GetValue<glm::vec2>() : glm::vec2(0.0f);
                                    if (ImGui::DragFloat2(name.c_str(), &data.x))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::Vector3:
                                {
                                    glm::vec3 data =
                                        fieldAlreadyExists ? scriptField->GetValue<glm::vec3>() : glm::vec3(0.0f);
                                    if (ImGui::DragFloat3(name.c_str(), &data.x))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::Vector4:
                                {
                                    glm::vec4 data =
                                        fieldAlreadyExists ? scriptField->GetValue<glm::vec4>() : glm::vec4(0.0f);
                                    if (ImGui::DragFloat4(name.c_str(), &data.x))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::Entity:
                                {
                                    uint64_t data = fieldAlreadyExists ? scriptField->GetValue<uint64_t>() : 0;
                                    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_U64, &data))
                                        scriptField->SetValue(data);
                                    break;
                                }
                                default:
                                    break;
                            }
                        }
                    }
                }

                if (!scriptClassExists)
                    ImGui::PopStyleColor();
            });
    }

} // namespace Titan