#pragma once

#include "GUI.hpp"
#include "Render.hpp"
#include "Camera.hpp"

#include "gl/OpenGL.hpp"
#include "gl/Texture2D.hpp"

class Main
{
private:
    GUI gui;
    Render render;

    float speed = 0.2f;
    float mouse_sensitivity = 75.0f;
    std::unique_ptr<Camera> camera;

    uint32_t raytracing_framebuffer_width = 1280;
    uint32_t raytracing_framebuffer_height = 720;

    const uint32_t scene_data_width = 4;
    const uint32_t scene_data_height = 1024;

    std::unique_ptr<GL::Texture2D<TexDataFloatRGBA>> environment;
    std::unique_ptr<GL::Texture2D<TexDataFloatRGBA>> scene;
    std::unique_ptr<GL::Texture2D<TexDataByteRGBA>> noise_0;
    std::unique_ptr<GL::Texture2D<TexDataByteRGBA>> noise_1;

public:
    void Init();
    void Deinit();
    void Update();
};
