#pragma once


#include "Timing.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"

#include "properties/Property.hpp"
#include "interfaces/IApplication.hpp"

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
    int selected_resolution = 3;
    int selected_samples = 2;
    int selected_bounces = 1;

    float move_speed = 0.1f;
    float forward_speed = 0.0f;
    float strafe_speed = 0.0f;
    float mouse_speed = 75.0f;

    uint16_t raytracing_samples = 4;
    uint16_t raytracing_bounces = 4;
    uint32_t raytracing_framebuffer_width = 0;
    uint32_t raytracing_framebuffer_height = 0;

    GUI gui;
    GL::Pipeline pipeline;

    Properties::Property<float> prop;

    std::vector<Sphere> geometry;
    std::unique_ptr<Camera> camera;

    void SetupScene();
    bool GuiUpdate();

public:
    void Init();
    void Deinit();
    void Update();
};
