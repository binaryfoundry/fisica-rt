#pragma once

#include <vector>

#include "GUI.hpp"

class Main
{
private:
    GUI gui;

public:
    void Init();
    void Deinit();
    void Update();
};
