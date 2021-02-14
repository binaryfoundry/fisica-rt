#pragma once

#include <vector>

#include "GUI.hpp"
#include "Render.hpp"
#include "Camera.hpp"

#include "gl/GL.hpp"
#include "gl/Texture2D.hpp"

class Main
{
private:
    GUI gui;
    Render render;

    float speed = 0.2f;
    glm::vec3 position = glm::vec3(0, 5, 35);
    Angles orientation;
    std::unique_ptr<Camera> camera;

    std::unique_ptr<GL::Texture2D<GL::TexDataFloatRGBA>> environment;
    std::unique_ptr<GL::Texture2D<GL::TexDataByteRGBA>> noise;

public:
    void Init();
    void Deinit();
    void Update();
};
