#include "SceneHierarchyPanel.h"
#include <vector>
#include "../Components.h"
#include "ImReflect.hpp"
#include "Titan/Project/Project.h"
#include "Titan/Renderer/Systems/GeometryRenderer.h"
#include "Titan/Renderer/Systems/Renderer2D.h"
#include "Titan/Scene/Assets.h"
#include "Titan/Scene/Components.h"
#include "Titan/Scripting/ScriptEngine.h"
#include "Titan/Utils/PlatformUtils.h"
#include "Titan/Vendor/FontAwesome7.h"
#include "Titan/Vendor/ImReflect_glm.hpp"

namespace Titan
{
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

    void SceneHierarchyPanel::OnImGuiRender(bool* openHierarchy, bool* openProperties)
    {
        TI_PROFILE_FUNCTION();
        if (openHierarchy && !*openHierarchy)
            return;

        ImGui::Begin(ICON_FA_SITEMAP " Scene Hierarchy", openHierarchy);

        // Build list of root entities (entities without a parent)
        std::vector<Entity> roots;
        auto view = m_Context->m_Registry.view<entt::entity>();
        for (auto entity : view)
        {
            Entity e{entity, m_Context.get()};
            if (!e)
                continue;
            Entity parent = e.GetParent();
            if (!parent)
                roots.push_back(e);
        }

        // Draw hierarchical tree
        for (auto& root : roots)
        {
            DrawEntityNode(root);
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

            if (ImGui::MenuItem("Create Empty Entity"))
                m_Context->CreateEntity("Empty Entity");

            if (ImGui::MenuItem("Create Camera Entity"))
            {
                Entity cameraEntity = m_Context->CreateEntity("Camera");
                cameraEntity.AddComponent<CameraComponent>();
            }
            if (ImGui::MenuItem("Create PostFX Entity"))
            {
                Entity postFXEntity = m_Context->CreateEntity("Post Processing");
                postFXEntity.AddComponent<PostFXComponent>();
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
            ImGui::SeparatorText("Terrain");
            if (ImGui::MenuItem("Create Terrain"))
            {
                Entity terrainEntity = m_Context->CreateEntity("Terrain");
                auto& trc = terrainEntity.AddComponent<TerrainRendererComponent>();
                trc.texture = Renderer2D::GetWhiteTexture();
            }
            ImGui::SeparatorText("Lights");
            if (ImGui::MenuItem("Create Directional Light"))
            {
                Entity quadEntity = m_Context->CreateEntity("Directional Light");
                auto& dlc = quadEntity.AddComponent<DirectionalLightComponent>(glm::vec3(1.0f, 1.0f, 1.0f));
            }
            if (ImGui::MenuItem("Create Skybox"))
            {
                Entity quadEntity = m_Context->CreateEntity("Skybox");
                auto& dlc = quadEntity.AddComponent<SkyboxComponent>();
            }
            ImGui::SeparatorText("Audio");
            if (ImGui::MenuItem("Create Audio Source"))
            {
                Entity audioEntity = m_Context->CreateEntity("Audio Source");
                auto& component = audioEntity.AddComponent<AudioSourceComponent>();
            }

            ImGui::EndPopup();
        }

        ImGui::End();

        if (!(openProperties && !*openProperties))
            ImGui::Begin(ICON_FA_SLIDERS " Properties", openProperties);
        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);

            if (ImGui::Button("Add Component"))
                ImGui::OpenPopup("AddComponent");

            if (ImGui::BeginPopup("AddComponent"))
            {
                DrawAddComponent<CameraComponent>(m_SelectionContext, "Camera");

                ImGui::SeparatorText("Rendering");
                DrawAddComponent<PostFXComponent>(m_SelectionContext, "Post Effects");
                DrawAddComponent<MeshRendererComponent>(m_SelectionContext, "Mesh Renderer");
                DrawAddComponent<TerrainRendererComponent>(m_SelectionContext, "Terrain Renderer");
                DrawAddComponent<SpriteRendererComponent>(m_SelectionContext, "Sprite Renderer");
                DrawAddComponent<CircleRendererComponent>(m_SelectionContext, "Circle Renderer");

                ImGui::SeparatorText("Lights");
                DrawAddComponent<SkyboxComponent>(m_SelectionContext, "Skybox");
                DrawAddComponent<DirectionalLightComponent>(m_SelectionContext, "Directional Light");

                ImGui::SeparatorText("Physics");
                DrawAddComponent<RigidbodyComponent>(m_SelectionContext, "Rigidbody");
                DrawAddComponent<CubeColliderComponent>(m_SelectionContext, "Cube Collider");
                DrawAddComponent<SphereColliderComponent>(m_SelectionContext, "Sphere Collider");

                ImGui::SeparatorText("2D Physics");
                DrawAddComponent<Rigidbody2DComponent>(m_SelectionContext, "Rigidbody 2D");
                DrawAddComponent<BoxCollider2DComponent>(m_SelectionContext, "Box Collider 2D");
                DrawAddComponent<CircleCollider2DComponent>(m_SelectionContext, "Circle Collider 2D");

                ImGui::SeparatorText("Audio");
                DrawAddComponent<AudioSourceComponent>(m_SelectionContext, "Audio Source");
                DrawAddComponent<AudioListenerComponent>(m_SelectionContext, "Audio Listener");

                ImGui::SeparatorText("Constraints");
                DrawAddComponent<LookAtComponent>(m_SelectionContext, "Look At");

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

        if (!(openProperties && !*openProperties))
            ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        if (!entity)
            return;

        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags =
            ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;

        // If entity has children, show as folder, else leaf
        bool hasChildren = entity.GetChildren().size() > 0;
        if (!hasChildren)
            flags |= ImGuiTreeNodeFlags_Leaf;

        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());

        // Selection
        if (ImGui::IsItemClicked())
            m_SelectionContext = entity;

        // Drag source
        if (ImGui::BeginDragDropSource())
        {
            uint32_t payload = (uint32_t)entity;
            ImGui::SetDragDropPayload("SCENE_ENTITY", &payload, sizeof(uint32_t));
            ImGui::TextUnformatted(tag.c_str());
            ImGui::EndDragDropSource();
        }

        // Accept drops (parenting)
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_ENTITY"))
            {
                uint32_t src = *(const uint32_t*)payload->Data;
                Entity srcEntity((entt::entity)src, m_Context.get());
                if (srcEntity && srcEntity != entity)
                {
                    srcEntity.SetParent(entity);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (opened)
        {
            // Draw children recursively
            auto children = entity.GetChildren();
            for (auto& child : children)
                DrawEntityNode(child);

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
                std::filesystem::path fullPath = Project::GetAssetDirectory() / path;
                texture = Assets::Load<Texture2D>(fullPath.string());
                texture->SetInternalPath(std::filesystem::path(path).string());
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
            ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT " Transform", entity,
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
            ICON_FA_CAMERA " Camera", entity,
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

        DrawComponent<PostFXComponent>(
            ICON_FA_SLIDERS " Post Processing", entity,
            [](auto& component)
            {
                // FXAA Section
                if (ImGui::TreeNodeEx("FXAA"))
                {
                    ImGui::Checkbox("Enabled", &component.FXAASettings.isEnabled);
                    ImGui::TreePop();
                }

                // Tonemapping Section
                if (ImGui::TreeNodeEx("Tonemapping"))
                {
                    ImGui::Checkbox("Enabled", &component.TonemappingSettings.isEnabled);
                    {
                        auto config = ImSettings();
                        config.push<TonemappingOperator>().as_dropdown().pop();
                        ImReflect::Input("Mode", component.TonemappingSettings.Operator, config);
                    }

                    ImGui::DragFloat("Exposure", &component.TonemappingSettings.Exposure, 0.01f, 0.0f, 10.0f);
                    ImGui::DragFloat("Gamma", &component.TonemappingSettings.Gamma, 0.01f, 0.1f, 5.0f);
                    ImGui::DragFloat("White Point", &component.TonemappingSettings.WhitePoint, 0.1f, 0.0f, 50.0f);
                    ImGui::TreePop();
                }
            });

        DrawComponent<AudioSourceComponent>(
            ICON_FA_VOLUME_HIGH " Audio Source", entity,
            [](auto& component)
            {
                float buttonWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Button(std::format("Source: {}",
                                          component.Sound ? component.Sound->GetBuffer()->GetInternalPath() : "None")
                                  .c_str(),
                              ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        std::filesystem::path soundPath = Project::GetAssetDirectory() / path;
                        component.Sound = Assets::Load<AudioSource>(soundPath.string());
                        component.Sound->GetBuffer()->SetInternalPath(std::filesystem::path(path).string());
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::DragFloat("Volume", &component.Volume, 0.01f, 0.0f, 2.0f);
                ImGui::DragFloat("Pitch", &component.Pitch, 0.01f, 0.1f, 5.0f);
                ImGui::Checkbox("Looping", &component.Looping);
            });

        DrawComponent<AudioListenerComponent>(ICON_FA_HEADPHONES " Audio Listener", entity, [](auto& component) {});

        DrawComponent<SpriteRendererComponent>(ICON_FA_BRUSH " Sprite Renderer", entity,
                                               [](auto& component)
                                               {
                                                   auto config = ImSettings();
                                                   config.push<glm::vec4>().as_color().pop();
                                                   ImReflect::Input("Color", component.Color, config);
                                                   DrawTextureSlot("Texture", component.Tex);
                                               });
        DrawComponent<CircleRendererComponent>(ICON_FA_CIRCLE " Circle Renderer", entity,
                                               [](auto& component)
                                               {
                                                   auto config = ImSettings();
                                                   config.push<glm::vec4>().as_color().pop();
                                                   ImReflect::Input("Color", component.Color, config);
                                                   ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f,
                                                                    1.0f);
                                                   ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f, 1.0f);
                                               });
        DrawComponent<MeshRendererComponent>(
            ICON_FA_CUBE " Mesh Renderer", entity,
            [](auto& component)
            {
                bool changed = false;

                float buttonWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Button(
                    std::format("Mesh: {}", component.MeshRef ? component.MeshRef->GetInternalPath() : "None").c_str(),
                    ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        std::filesystem::path meshPath = Project::GetAssetDirectory() / path;
                        component.MeshRef = Assets::Load<Mesh>(meshPath.string());
                        component.MeshRef->SetInternalPath(std::filesystem::path(path).string());
                        changed = true;
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
                        // Name input
                        char nameBuffer[256];
                        strcpy_s(nameBuffer, sizeof(nameBuffer), mat->Name.c_str());
                        if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
                        {
                            mat->Name = nameBuffer;
                            changed = true;
                        }

                        auto config = ImSettings();
                        config.push<glm::vec4>().as_color().pop();
                        auto response = ImResponse();
                        ImReflect::Input("Diffuse", mat->AlbedoColor, config, response);
                        changed |= response.get<glm::vec4>().is_changed();
                        changed |= DrawTextureSlot("Diffuse Texture", mat->AlbedoTexture);
                        changed |= DrawTextureSlot("Emission Texture", mat->EmissionTexture);
                        changed |= DrawTextureSlot("Metallic Texture", mat->MetallicTexture);
                        changed |= DrawTextureSlot("Roughness Texture", mat->RoughnessTexture);
                        changed |= DrawTextureSlot("Normal Texture", mat->NormalTexture);
                        changed |= DrawTextureSlot("Ambient Occlusion Texture", mat->AOTexture);
                        auto config2 = ImSettings();
                        config2.push<glm::vec2>().as_position().pop();
                        auto response2 = ImResponse();
                        ImReflect::Input("UV Repeat", mat->UVRepeat, config2, response2);
                        changed |= response2.get<glm::vec2>().is_changed();

                        // Save material button
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();

                        if (ImGui::Button("Save Material"))
                        {
                            if (mat->InternalPath.empty())
                            {
                                // Generate default path if not set
                                auto meshPath = std::filesystem::path(component.MeshRef->GetInternalPath());
                                auto materialDir = meshPath.parent_path() / "Materials";
                                auto materialFileName =
                                    meshPath.stem().string() + "_Mat" + std::to_string(index) + ".mat";
                                mat->InternalPath = (materialDir / materialFileName).string();
                                std::filesystem::create_directories(materialDir);
                            }
                            mat->Save();
                        }

                        ImGui::SameLine();

                        if (ImGui::Button("Save As..."))
                        {
                            std::string filepath = FileDialogs::SaveFile("Material (*.mat)\0*.mat\0");
                            if (!filepath.empty())
                            {
                                // Ensure .mat extension
                                if (filepath.find(".mat") == std::string::npos)
                                {
                                    filepath += ".mat";
                                }
                                mat->InternalPath = filepath;
                                mat->Save();
                            }
                        }

                        ImGui::SameLine();

                        if (ImGui::Button("Load..."))
                        {
                            std::string filepath = FileDialogs::OpenFile("Material (*.mat)\0*.mat\0");
                            if (!filepath.empty())
                            {
                                auto loadedMat = Material3D::Create(filepath);
                                if (loadedMat)
                                {
                                    *mat = *loadedMat;
                                    mat->SetInternalPath(filepath);
                                    changed = true;
                                }
                            }
                        }

                        if (!mat->InternalPath.empty())
                        {
                            ImGui::TextDisabled("Path: %s", mat->InternalPath.c_str());
                        }

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                    index++;
                }

                if (changed)
                    GeometryRenderer::ClearCache();
            });

        DrawComponent<TerrainRendererComponent>(ICON_FA_MOUNTAIN " Terrain Renderer", entity, [](auto& component) {
                DrawTextureSlot("Texture", component.texture);
        });

        DrawComponent<DirectionalLightComponent>(ICON_FA_SUN " Directional Light", entity, [](auto& component)
                                                 { Component::DirectionControl("Direction", component.Direction); });
        DrawComponent<SkyboxComponent>(
            ICON_FA_CLOUD " Skybox", entity,
            [](auto& component)
            {
                {
                    auto config = ImSettings();
                    config.push<SkyboxComponent::Mode>().as_dropdown().pop();
                    ImReflect::Input("Mode", component.mode, config);
                }
                if (component.mode == SkyboxComponent::Mode::HDRI)
                {
                    ImGui::TextUnformatted("Cubemap");
                    if (component.hdriSettings.Skybox)
                    {
                        ImGui::Button(component.hdriSettings.Skybox->GetInternalPath().c_str());
                    }
                    else
                    {
                        ImGui::Button("Empty");
                    }
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                        {
                            const wchar_t* path = (const wchar_t*)payload->Data;
                            std::filesystem::path cubemapPath = Project::GetAssetDirectory() / path;
                            component.hdriSettings.Skybox = Assets::Load<Cubemap>(cubemapPath.string());
                            component.hdriSettings.Irradiance = component.hdriSettings.Skybox->CreateIrradianceMap();
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
                else if (component.mode == SkyboxComponent::Mode::Colorramp)
                {
                    auto config = ImSettings();
                    config.push<glm::vec3>().as_color().pop();
                    ImReflect::Input("Top Color", component.colorrampSettings.TopColor, config);
                    ImReflect::Input("Bottom Color", component.colorrampSettings.BottomColor, config);
                }
                else if (component.mode == SkyboxComponent::Mode::Normal)
                {
                    ImGui::DragFloat("Time", &component.normalSettings.Time, 0.05f);
                }
            });

        DrawComponent<RigidbodyComponent>(ICON_FA_GEAR " Rigidbody", entity,
                                          [](auto& component)
                                          {
                                              const char* bodyTypeStrings[] = {"Static", "Dynamic", "Kinematic"};
                                              const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
                                              if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
                                              {
                                                  for (int i = 0; i < 3; i++)
                                                  {
                                                      bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                                                      if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                                                      {
                                                          currentBodyTypeString = bodyTypeStrings[i];
                                                          component.Type = (RigidbodyComponent::BodyType)i;
                                                      }

                                                      if (isSelected)
                                                          ImGui::SetItemDefaultFocus();
                                                  }

                                                  ImGui::EndCombo();
                                              }

                                              ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
                                          });

        DrawComponent<CubeColliderComponent>(
            ICON_FA_CUBE " Cube Collider", entity,
            [](auto& component)
            {
                auto config = ImSettings();
                config.push<glm::vec3>().as_position().pop();
                ImReflect::Input("Offset", component.Offset, config);
                ImReflect::Input("Size", component.Size, config);
                float buttonWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Button(std::format("Material: {}", component.Material->InternalPath).c_str(),
                              ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        std::filesystem::path materialPath = Project::GetAssetDirectory() / path;
                        component.Material = Assets::Load<PhysicsMaterial>(materialPath.string());
                    }
                    ImGui::EndDragDropTarget();
                }
            });

        DrawComponent<SphereColliderComponent>(
            ICON_FA_CIRCLE " Sphere Collider", entity,
            [](auto& component)
            {
                auto config = ImSettings();
                config.push<glm::vec3>().as_position().pop();
                ImReflect::Input("Offset", component.Offset, config);
                ImGui::DragFloat("Radius", &component.Radius);
                float buttonWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Button(std::format("Material: {}", component.Material->InternalPath).c_str(),
                              ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        std::filesystem::path materialPath = Project::GetAssetDirectory() / path;
                        component.Material = Assets::Load<PhysicsMaterial>(materialPath.string());
                    }
                    ImGui::EndDragDropTarget();
                }
            });

        DrawComponent<Rigidbody2DComponent>(ICON_FA_GEARS " Rigidbody 2D", entity,
                                            [](auto& component)
                                            {
                                                const char* bodyTypeStrings[] = {"Static", "Dynamic", "Kinematic"};
                                                const char* currentBodyTypeString =
                                                    bodyTypeStrings[(int)component.Type];
                                                if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
                                                {
                                                    for (int i = 0; i < 3; i++)
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
            ICON_FA_BOX " Box Collider 2D", entity,
            [](auto& component)
            {
                auto config = ImSettings();
                config.push<glm::vec2>().as_position().pop();
                ImReflect::Input("Offset", component.Offset, config);
                ImReflect::Input("Size", component.Size, config);
                float buttonWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Button(std::format("Material: {}", component.Material->InternalPath).c_str(),
                              ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        std::filesystem::path materialPath = Project::GetAssetDirectory() / path;
                        component.Material = Assets::Load<Physics2DMaterial>(materialPath.string());
                    }
                    ImGui::EndDragDropTarget();
                }
            });

        DrawComponent<CircleCollider2DComponent>(
            ICON_FA_CIRCLE " Circle Collider 2D", entity,
            [](auto& component)
            {
                auto config = ImSettings();
                config.push<glm::vec2>().as_position().pop();
                ImReflect::Input("Offset", component.Offset, config);
                ImGui::DragFloat("Radius", &component.Radius);
                float buttonWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Button(std::format("Material: {}", component.Material->InternalPath).c_str(),
                              ImVec2(buttonWidth, 0.0f));
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        std::filesystem::path materialPath = Project::GetAssetDirectory() / path;
                        component.Material = Assets::Load<Physics2DMaterial>(materialPath.string());
                    }
                    ImGui::EndDragDropTarget();
                }
            });

        DrawComponent<LookAtComponent>(ICON_FA_EYE " Constraint: Look At", entity,
                                       [](auto& component)
                                       {
                                           auto config = ImSettings();
                                           config.push<glm::vec3>().as_position().pop();
                                           ImReflect::Input("Position", component.Position, config);
                                       });

        DrawComponent<ScriptComponent>(
            ICON_FA_CODE " Script", entity,
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
                                auto config = ImSettings();
                                config.push<glm::vec2>().as_position().pop();
                                auto response = ImResponse();
                                ImReflect::Input(name.c_str(), data, config, response);
                                if (response.get<glm::vec2>().is_changed())
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::Vector3)
                            {
                                glm::vec3 data = scriptInstance->GetFieldValue<glm::vec3>(name);
                                auto config = ImSettings();
                                config.push<glm::vec3>().as_position().pop();
                                auto response = ImResponse();
                                ImReflect::Input(name.c_str(), data, config, response);
                                if (response.get<glm::vec3>().is_changed())
                                    scriptInstance->SetFieldValue(name, data);
                            }
                            else if (field.Type == ScriptFieldType::Vector4)
                            {
                                glm::vec4 data = scriptInstance->GetFieldValue<glm::vec4>(name);
                                auto config = ImSettings();
                                config.push<glm::vec4>().as_position().pop();
                                auto response = ImResponse();
                                ImReflect::Input(name.c_str(), data, config, response);
                                if (response.get<glm::vec4>().is_changed())
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
                                    auto config = ImSettings();
                                    config.push<glm::vec2>().as_position().pop();
                                    auto response = ImResponse();
                                    ImReflect::Input(name.c_str(), data, config, response);
                                    if (response.get<glm::vec2>().is_changed())
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::Vector3:
                                {
                                    glm::vec3 data =
                                        fieldAlreadyExists ? scriptField->GetValue<glm::vec3>() : glm::vec3(0.0f);
                                    auto config = ImSettings();
                                    config.push<glm::vec3>().as_position().pop();
                                    auto response = ImResponse();
                                    ImReflect::Input(name.c_str(), data, config, response);
                                    if (response.get<glm::vec3>().is_changed())
                                        scriptField->SetValue(data);
                                    break;
                                }
                                case ScriptFieldType::Vector4:
                                {
                                    glm::vec4 data =
                                        fieldAlreadyExists ? scriptField->GetValue<glm::vec4>() : glm::vec4(0.0f);
                                    auto config = ImSettings();
                                    config.push<glm::vec4>().as_position().pop();
                                    auto response = ImResponse();
                                    ImReflect::Input(name.c_str(), data, config, response);
                                    if (response.get<glm::vec4>().is_changed())
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