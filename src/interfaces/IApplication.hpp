#pragma once

#include <functional>

#include "../Context.hpp"
#include "../Input.hpp"

class IApplication
{
public:
    IApplication()
    {
        context = std::make_unique<Context>();

        key_up_callback = [](Scancode) {};
        key_down_callback = [](Scancode) {};

        controller_button_up_callback = [](uint16_t) {};
        controller_button_down_callback = [](uint16_t) {};

        controller_dpad_up_callback = [](DPadDirection) {};
        controller_dpad_down_callback = [](DPadDirection) {};
    }

    std::unique_ptr<Context> context;

    virtual void Init() = 0;
    virtual void Deinit() = 0;
    virtual void Update() = 0;

    virtual ~IApplication() {};

    int window_width;
    int window_height;

    bool mouse_captured;
    int16_t mouse_x;
    int16_t mouse_y;
    int16_t mouse_delta_x;
    int16_t mouse_delta_y;
    int16_t captured_mouse_x;
    int16_t captured_mouse_y;
    float captured_mouse_delta_x;
    float captured_mouse_delta_y;

    std::function<void(Scancode key)> key_up_callback;
    std::function<void(Scancode key)> key_down_callback;

    std::function<void(uint16_t id)> controller_button_up_callback;
    std::function<void(uint16_t id)> controller_button_down_callback;

    std::function<void(DPadDirection dir)> controller_dpad_up_callback;
    std::function<void(DPadDirection dir)> controller_dpad_down_callback;
};
