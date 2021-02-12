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

    std::unique_ptr<Camera> camera;

public:
    void Init();
    void Deinit();
    void Update();
};
