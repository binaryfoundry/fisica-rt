#pragma once

#include <vector>

#include "GUI.hpp"
#include "Render.hpp"

class Main
{
private:
    GUI gui;
    Render render;

public:
    void Init();
    void Deinit();
    void Update();
};
