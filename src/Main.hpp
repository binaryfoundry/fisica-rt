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

    float move_speed = 0.1f;

    float forward_speed = 0.0f;
    float strafe_speed = 0.0f;
    float mouse_speed = 75.0f;
    std::unique_ptr<Camera> camera;

    uint32_t raytracing_framebuffer_width = 1280;
    uint32_t raytracing_framebuffer_height = 720;

    std::vector<Sphere> geometry;

    void SetupScene();

public:
    void Init();
    void Deinit();
    void Update();
};
