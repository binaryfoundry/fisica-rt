#pragma once

#include "GUI.hpp"
#include "Camera.hpp"
#include "Geometry.hpp"

#include "gl/OpenGL.hpp"
#include "gl/Pipeline.hpp"

class Main
{
private:
    GUI gui;
    GL::Pipeline pipeline;

    bool upscale = true;
    int selected_resolution = 3;
    int selected_samples = 2;
    int selected_bounces = 2;

    float move_speed = 0.1f;

    float forward_speed = 0.0f;
    float strafe_speed = 0.0f;
    float mouse_speed = 75.0f;
    std::unique_ptr<Camera> camera;

    uint16_t raytracing_samples = 4;
    uint16_t raytracing_bounces = 4;
    uint32_t raytracing_framebuffer_width = 0;
    uint32_t raytracing_framebuffer_height = 0;

    std::vector<Sphere> geometry;

    void SetupScene();

public:
    void Init();
    void Deinit();
    void Update();
};
