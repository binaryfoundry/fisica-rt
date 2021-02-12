#pragma once

#include <vector>

#include "GUI.hpp"
#include "Render.hpp"
#include "Camera.hpp"

class Main
{
private:
    GUI gui;
    Render render;

    float speed = 0.2f;
    glm::vec3 position;
    Angles orientation;
    std::unique_ptr<Camera> camera;

public:
    void Init();
    void Deinit();
    void Update();
};
