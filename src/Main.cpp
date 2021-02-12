#include "Main.hpp"

#include "sdl/SDL.hpp"
#include "sdl/SDLFile.hpp"

#include "imgui/imgui.h"

//if (backend->IsScanCodeDown(Scancode::SCANCODE_W))
//{
//    position += camera->direction * speed;
//}
//if (backend->IsScanCodeDown(Scancode::SCANCODE_S))
//{
//    position += camera->direction * -speed;
//}
//if (backend->IsScanCodeDown(Scancode::SCANCODE_A))
//{
//    strafe(speed);
//}
//if (backend->IsScanCodeDown(Scancode::SCANCODE_D))
//{
//    strafe(-speed);
//}

void Main::Init()
{
    camera = std::make_unique<Camera>(
        1.0f,
        75.0f,
        glm::vec3(0, 0, 0),
        Angles());

    render.Init(
        1280,
        720);

    gui.Init();

    sdl_key_up_callback = [=](uint16_t key)
    {
    };

    sdl_key_down_callback = [=](uint16_t key)
    {
    };
}

void Main::Deinit()
{
    render.Deinit();
    gui.Deinit();
}

void Main::Update()
{
    ImGui::NewFrame();

    ImGui::Begin(
        "Menu",
        NULL,
        ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::End();

    const float window_aspect_ratio =
        static_cast<float>(sdl_window_width) /
        static_cast<float>(sdl_window_height);

    orientation.yaw +=
        static_cast<float>(sdl_captured_mouse_delta_x) /
        (150.0f * window_aspect_ratio);

    orientation.pitch +=
        static_cast<float>(sdl_captured_mouse_delta_y) / 150.0f;

    camera->SetPosition(position);
    camera->SetAngles(orientation);

    render.Draw(
        sdl_window_width,
        sdl_window_height);

    gui.Draw(
        sdl_window_width,
        sdl_window_height);
}
