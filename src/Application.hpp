#pragma once

#include "Timing.hpp"

#include "properties/Property.hpp"
#include "interfaces/IApplication.hpp"
#include "pipelines/Cascades.hpp"

#include "gl/ImGui.hpp"
#include "gl/OpenGL.hpp"
#include "gl/Pipeline.hpp"

class Application : public IApplication
{
private:
    const float fps_alpha = 0.9f;
    hrc::time_point fps_time;
    float fps_time_avg = 60;

    bool upscale = true;

    // Painting controls
    float emitter_color[3] = { 1.0f, 0.67f, 0.17f };
    float emitter_intensity = 1.5f;
    float brush_radius_emitter = 4.0f;
    float brush_radius_occluder = 8.0f;
    glm::vec2 previous_paint_uv = glm::vec2(0.0f);
    int paint_button = -1;

    uint32_t raytracing_framebuffer_width = 512;
    uint32_t raytracing_framebuffer_height = 512;

    glm::mat4 projection;
    glm::mat4 view;

    GUI gui;
    Pipelines::Cascades pipeline;

    Properties::Property<float> prop;

    void ViewScale();
    bool GuiUpdate();
    void LoadExampleScene();

public:
    void Init();
    void Deinit();
    void Update();
};
